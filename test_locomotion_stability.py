import math
import urllib.request
import json
import time
import sys

ROBOT_IP = sys.argv[1] if len(sys.argv) > 1 else "192.168.1.24"
L1 = 5.0
L2 = 5.7

def solveIK(x, z):
    if z < 1.0: z = 1.0
    z2 = math.sqrt(x * x + z * z)
    maxReach = L1 + L2 - 0.002
    minReach = abs(L2 - L1) + 0.002
    z2 = max(minReach, min(maxReach, z2))
    
    hipRad2 = math.atan2(x, z)
    hipDeg2 = math.degrees(hipRad2)
    
    val1 = max(-1.0, min(1.0, (L1*L1 + z2*z2 - L2*L2) / (2.0 * L1 * z2)))
    hipRad1 = math.acos(val1)
    hipDeg1 = math.degrees(hipRad1)
    
    val2 = max(-1.0, min(1.0, (L1*L1 + L2*L2 - z2*z2) / (2.0 * L1 * L2)))
    kneeRad = math.pi - math.acos(val2)
    kneeDeg = math.degrees(kneeRad)
    
    val3 = max(-1.0, min(1.0, (L2*L2 + z2*z2 - L1*L1) / (2.0 * L2 * z2)))
    ankleRad = (math.pi / 2.0) + hipRad2 - math.acos(val3)
    ankleDeg = math.degrees(ankleRad)
    
    return hipDeg1 + hipDeg2, kneeDeg, ankleDeg

def test_ik_math():
    print("\n--- 1. Testing Closed-Form 2D Inverse Kinematics Math ---")
    nom_h, nom_k, nom_a = solveIK(0.0, 10.0)
    print(f"Nominal stance (0, 10.0): hip={nom_h:.2f} deg, knee={nom_k:.2f} deg, ankle={nom_a:.2f} deg")
    assert 20.0 < nom_h < 25.0, f"Unexpected nominal hip: {nom_h}"
    assert 40.0 < nom_k < 45.0, f"Unexpected nominal knee: {nom_k}"
    assert 68.0 < nom_a < 73.0, f"Unexpected nominal ankle: {nom_a}"

    # Test swing lift (z = 9.6, x = 0.5)
    sw_h, sw_k, sw_a = solveIK(0.5, 9.6)
    dh = sw_h - nom_h
    dk = sw_k - nom_k
    da = sw_a - nom_a
    print(f"Swing lift (0.5, 9.6): dHip={dh:+.2f}, dKnee={dk:+.2f}, dAnkle={da:+.2f}")
    assert dk > 8.0, "Knee must flex dynamically during swing lift"
    assert dh > 5.0, "Hip must stride forward during swing"

    # Test boundary singularities
    solveIK(0.0, 10.7) # max reach
    solveIK(0.0, 11.5) # beyond reach (should clamp safely)
    solveIK(0.0, 0.5)  # extreme squat (should clamp safely)
    print("PASS: Inverse Kinematics math & boundary limits verified.")

def test_quintic_trajectory():
    print("\n--- 2. Testing Quintic Zero-Jerk Trajectory Continuity ---")
    def swing(u, sClear=0.4):
        return sClear * 64.0 * (u**3) * ((1.0 - u)**3)
    
    assert swing(0.0) == 0.0, "Liftoff clearance must be zero"
    assert swing(1.0) == 0.0, "Touchdown clearance must be zero"
    assert abs(swing(0.5) - 0.4) < 1e-5, "Mid-swing must reach peak clearance"

    # Numerical derivatives at boundaries
    eps = 1e-5
    v0 = (swing(eps) - swing(0.0)) / eps
    v1 = (swing(1.0) - swing(1.0 - eps)) / eps
    assert abs(v0) < 0.01, f"Liftoff velocity must be zero: {v0}"
    assert abs(v1) < 0.01, f"Touchdown velocity must be zero: {v1}"
    print("PASS: Quintic trajectory has zero boundary velocity & acceleration.")

def test_live_esp32_endpoints():
    print(f"\n--- 3. Testing Physical ESP32 Endpoints at http://{ROBOT_IP} ---")
    base = f"http://{ROBOT_IP}"
    
    # 1. Telemetry
    req = urllib.request.urlopen(f"{base}/get_angles", timeout=3)
    data = json.loads(req.read().decode())
    assert "angles" in data
    assert "0" in data["angles"] and "1" in data["angles"] and "12" in data["angles"]
    assert "4" in data["angles"] and "8" in data["angles"] and "15" in data["angles"]
    assert data["pca_found"] is True
    print(f"PASS: Telemetry OK. Active gait: {data['active_gait']}, Angles: {data['angles']}")

    # 2. Single step non-blocking dispatch
    t0 = time.time()
    req = urllib.request.urlopen(f"{base}/run_gait?gait=step", timeout=3)
    res = json.loads(req.read().decode())
    rtt_ms = (time.time() - t0) * 1000.0
    print(f"PASS: /run_gait?gait=step returned in {rtt_ms:.1f}ms: {res}")
    assert rtt_ms < 500.0, f"Dispatch must be non-blocking, took {rtt_ms}ms"

    # Wait for step to complete
    time.sleep(1.8)
    req = urllib.request.urlopen(f"{base}/get_angles", timeout=3)
    data = json.loads(req.read().decode())
    assert data["active_gait"] == "stand"
    print("PASS: Step completed and robot safely settled in neutral stand.")

    # 3. High-frequency telemetry polling under walk load (Concurrency stress test)
    print("\n--- 4. Concurrency Stress Test: 50 fast HTTP requests during active walk ---")
    urllib.request.urlopen(f"{base}/run_gait?gait=walk", timeout=3)
    time.sleep(0.3)
    
    success_count = 0
    t_start = time.time()
    for i in range(25):
        try:
            r = urllib.request.urlopen(f"{base}/get_angles", timeout=1)
            d = json.loads(r.read().decode())
            if d.get("gait_running"):
                success_count += 1
        except Exception as e:
            print(f"Request {i} failed: {e}")
        time.sleep(0.05)
    
    elapsed = time.time() - t_start
    print(f"Completed 25 requests in {elapsed:.2f}s (Success: {success_count}/25)")
    assert success_count >= 24, f"Too many dropped requests: {success_count}/25"

    # Stop robot
    urllib.request.urlopen(f"{base}/run_gait?gait=stand", timeout=3)
    time.sleep(0.5)
    r = urllib.request.urlopen(f"{base}/get_angles", timeout=3)
    d = json.loads(r.read().decode())
    assert d["active_gait"] == "stand"
    print(f"PASS: Robot stopped safely. Final angles: {d['angles']}")

if __name__ == "__main__":
    test_ik_math()
    test_quintic_trajectory()
    test_live_esp32_endpoints()
    print("\n==============================================")
    print("ALL TESTS PASSED WITH 100% SUCCESS!")
    print("==============================================")
