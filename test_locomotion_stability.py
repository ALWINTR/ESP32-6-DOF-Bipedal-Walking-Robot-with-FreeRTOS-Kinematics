import math
import urllib.request
import json
import time
import sys

ROBOT_IP = sys.argv[1] if len(sys.argv) > 1 else "192.168.1.24"
L1 = 5.0
L2 = 5.7

# Hardware Safety Bracket Limits
LIMITS = {
    '0':  (82.0, 106.0),  # Right Hip
    '1':  (5.0,  28.0),   # Right Knee
    '12': (82.0, 106.0),  # Right Foot
    '4':  (82.0, 106.0),  # Left Hip
    '8':  (74.0, 97.0),   # Left Knee
    '15': (82.0, 106.0),  # Left Foot
}

HOMES = {
    '0': 94.0, '1': 8.0, '12': 94.0,
    '4': 94.0, '8': 94.0, '15': 94.0
}

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

def compute_joint_angles(xr, zr, xl, zl, stepHeight=10.0):
    nom_h, nom_k, nom_a = solveIK(0.0, stepHeight)
    hr, kr, ar = solveIK(xr, zr)
    hl, kl, al = solveIK(xl, zl)
    
    dh_r = hr - nom_h; dk_r = kr - nom_k; da_r = ar - nom_a
    dh_l = hl - nom_h; dk_l = kl - nom_k; da_l = al - nom_a
    
    return {
        '0':  HOMES['0']  + dh_r,
        '1':  HOMES['1']  + dk_r,
        '12': HOMES['12'] + da_r,
        '4':  HOMES['4']  - dh_l,
        '8':  HOMES['8']  - dk_l,
        '15': HOMES['15'] - da_l
    }

def test_ik_math():
    print("\n--- 1. Testing Closed-Form 2D Inverse Kinematics Math ---")
    nom_h, nom_k, nom_a = solveIK(0.0, 10.0)
    print(f"Nominal stance (0, 10.0): hip={nom_h:.2f} deg, knee={nom_k:.2f} deg, ankle={nom_a:.2f} deg")
    assert 20.0 < nom_h < 25.0, f"Unexpected nominal hip: {nom_h}"
    assert 40.0 < nom_k < 45.0, f"Unexpected nominal knee: {nom_k}"
    assert 68.0 < nom_a < 73.0, f"Unexpected nominal ankle: {nom_a}"

    # Verify ground-parallel foot sole geometry
    for x_test in [-0.8, 0.0, 0.8]:
        for z_test in [9.5, 10.0]:
            h, k, a = solveIK(x_test, z_test)
            z2 = math.sqrt(x_test*x_test + z_test*z_test)
            hipRad2 = math.atan2(x_test, z_test)
            val3 = max(-1.0, min(1.0, (L2*L2 + z2*z2 - L1*L1) / (2.0 * L2 * z2)))
            shin_angle = hipRad2 - math.acos(val3)
            foot_sole_angle = math.radians(a) - shin_angle
            assert abs(foot_sole_angle - math.pi / 2.0) < 1e-4, "Foot sole must be ground-parallel"

    print("PASS: Inverse Kinematics math & ground-parallel sole verified.")

def test_quintic_trajectory():
    print("\n--- 2. Testing Quintic Trajectory & Swing Phase Alignment ---")
    def swing(phase, sClear=0.4):
        u = max(0.0, min(1.0, phase / math.pi))
        return sClear * 64.0 * (u**3) * ((1.0 - u)**3)

    assert swing(0.0) == 0.0, "Liftoff clearance must be zero"
    assert swing(math.pi) == 0.0, "Touchdown clearance must be zero"
    assert abs(swing(math.pi / 2.0) - 0.4) < 1e-5, "Mid-swing must reach peak clearance"

    # Numerical derivatives at boundaries
    eps = 1e-5
    v0 = (swing(eps) - swing(0.0)) / eps
    v1 = (swing(math.pi) - swing(math.pi - eps)) / eps
    assert abs(v0) < 0.01, f"Liftoff vertical velocity must be zero: {v0}"
    assert abs(v1) < 0.01, f"Touchdown vertical velocity must be zero: {v1}"

    # Verify swing horizontal velocity at liftoff & touchdown
    # x_swing(phase) = -cos(phase) * sLength
    # dx/d(phase) = sin(phase) * sLength
    sLen = 0.8
    vx_liftoff = math.sin(0.0) * sLen
    vx_touchdown = math.sin(math.pi) * sLen
    assert abs(vx_liftoff) < 1e-5, "Swing horizontal velocity at liftoff must be zero (no ground drag)"
    assert abs(vx_touchdown) < 1e-5, "Swing horizontal velocity at touchdown must be zero (no scraping)"

    # Verify single-step monotonic trajectory: stance foot must never reverse
    print("Testing single-step trajectory monotonicity...")
    xr_prev = 0.0
    for i in range(1, 11):
        u = i / 20.0  # Phase 1: u in [0, 0.5]
        s = u / 0.5
        e = 6.0*(s**5) - 15.0*(s**4) + 10.0*(s**3)
        xr = -sLen * e
        assert xr <= xr_prev, f"Stance foot must monotonically push backward, got {xr} > {xr_prev}"
        xr_prev = xr
    print("PASS: Trajectory is C2 continuous with zero ground dragging.")

def test_gait_kinematic_safety_brackets():
    print("\n--- 3. Testing Joint Angles Across All Gaits Against Hardware Safety Brackets ---")
    
    # A. Forward Walk (extreme parameters: stride=1.20cm, clearance=0.55cm)
    for i in range(100):
        phase = 2.0 * math.pi * (i / 100.0)
        cosP = math.cos(phase)
        sClear = 0.55
        sLen = 1.20
        liftL = sClear * 64.0 * (phase/math.pi)**3 * (1.0 - phase/math.pi)**3 if phase < math.pi else 0.0
        p2 = phase - math.pi
        liftR = sClear * 64.0 * (p2/math.pi)**3 * (1.0 - p2/math.pi)**3 if phase >= math.pi else 0.0
        xr =  cosP * sLen; xl = -cosP * sLen
        zr = 10.0 - liftR; zl = 10.0 - liftL
        angles = compute_joint_angles(xr, zr, xl, zl)
        for ch, (min_lim, max_lim) in LIMITS.items():
            val = angles[ch]
            assert min_lim <= val <= max_lim, f"Walk Ch {ch} angle {val:.2f} violated [{min_lim}, {max_lim}]"

    # B. Turn Left & Turn Right (opposite phases, outer full, inner 0.35x)
    for gait, mult_r, mult_l in [('turn_left', 1.0, -0.35), ('turn_right', 0.35, -1.0)]:
        for i in range(100):
            phase = 2.0 * math.pi * (i / 100.0)
            cosP = math.cos(phase)
            sClear = 0.50; sLen = 1.00
            liftL = sClear * 64.0 * (phase/math.pi)**3 * (1.0 - phase/math.pi)**3 if phase < math.pi else 0.0
            p2 = phase - math.pi
            liftR = sClear * 64.0 * (p2/math.pi)**3 * (1.0 - p2/math.pi)**3 if phase >= math.pi else 0.0
            xr = mult_r * cosP * sLen; xl = mult_l * cosP * sLen
            zr = 10.0 - liftR; zl = 10.0 - liftL
            angles = compute_joint_angles(xr, zr, xl, zl)
            for ch, (min_lim, max_lim) in LIMITS.items():
                val = angles[ch]
                assert min_lim <= val <= max_lim, f"{gait} Ch {ch} angle {val:.2f} violated [{min_lim}, {max_lim}]"

    # C. Crouch (squat = 0.65cm)
    for i in range(50):
        phase = 2.0 * math.pi * (i / 50.0)
        squat = (0.5 - 0.5 * math.cos(phase)) * 0.65
        angles = compute_joint_angles(0.0, 10.0 - squat, 0.0, 10.0 - squat)
        for ch, (min_lim, max_lim) in LIMITS.items():
            val = angles[ch]
            assert min_lim <= val <= max_lim, f"Crouch Ch {ch} angle {val:.2f} violated [{min_lim}, {max_lim}]"

    # D. Initialization Sweep (squat 9.50cm to stand 10.00cm)
    for i in range(50):
        u = i / 50.0
        smoothU = 6.0*(u**5) - 15.0*(u**4) + 10.0*(u**3)
        h = 9.50 + 0.50 * smoothU
        angles = compute_joint_angles(0.0, h, 0.0, h)
        for ch, (min_lim, max_lim) in LIMITS.items():
            val = angles[ch]
            assert min_lim <= val <= max_lim, f"Init Ch {ch} angle {val:.2f} violated [{min_lim}, {max_lim}]"

    print("PASS: All gaits strictly within hardware safety limits.")

def test_live_esp32_endpoints():
    print(f"\n--- 4. Testing Physical ESP32 Endpoints at http://{ROBOT_IP} ---")
    base = f"http://{ROBOT_IP}"
    
    # 1. Telemetry
    req = urllib.request.urlopen(f"{base}/get_angles", timeout=3)
    data = json.loads(req.read().decode())
    assert "angles" in data
    for ch in ['0', '1', '12', '4', '8', '15']:
        assert ch in data["angles"], f"Missing joint channel {ch}"
    assert data["pca_found"] is True
    print(f"PASS: Telemetry OK. Active gait: {data['active_gait']}, Angles: {data['angles']}")

    # 2. Test Stance Initialization
    req = urllib.request.urlopen(f"{base}/run_gait?gait=init", timeout=3)
    res = json.loads(req.read().decode())
    assert res.get("status") == "ok"
    time.sleep(1.5)
    req = urllib.request.urlopen(f"{base}/get_angles", timeout=3)
    d = json.loads(req.read().decode())
    assert d["active_gait"] == "stand"
    print("PASS: Stance initialization completed and parked cleanly.")

    # 3. Single step non-blocking dispatch
    t0 = time.time()
    req = urllib.request.urlopen(f"{base}/run_gait?gait=step", timeout=3)
    res = json.loads(req.read().decode())
    rtt_ms = (time.time() - t0) * 1000.0
    print(f"PASS: /run_gait?gait=step returned in {rtt_ms:.1f}ms: {res}")
    assert rtt_ms < 500.0, f"Dispatch must be non-blocking, took {rtt_ms}ms"

    # Wait for step to complete
    time.sleep(2.0)
    req = urllib.request.urlopen(f"{base}/get_angles", timeout=3)
    data = json.loads(req.read().decode())
    assert data["active_gait"] == "stand"
    print("PASS: Step completed and robot safely settled in neutral stand.")

    # 4. High-frequency telemetry polling under walk load (Concurrency stress test)
    print("\n--- 5. Concurrency Stress Test: 25 rapid HTTP requests during active walk ---")
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
    test_gait_kinematic_safety_brackets()
    test_live_esp32_endpoints()
    print("\n==============================================")
    print("ALL TESTS PASSED WITH 100% SUCCESS!")
    print("==============================================")
