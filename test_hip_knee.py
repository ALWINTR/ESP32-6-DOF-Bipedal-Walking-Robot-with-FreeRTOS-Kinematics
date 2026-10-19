import urllib.request
import time

import sys

ROBOT_IPS = ["192.168.1.24", "192.168.1.22", "biped.local", "192.168.4.1"]
ROBOT_IP = sys.argv[1] if len(sys.argv) > 1 else None

if not ROBOT_IP:
    for ip in ROBOT_IPS:
        try:
            req = urllib.request.urlopen(f"http://{ip}/get_angles", timeout=1)
            ROBOT_IP = ip
            print(f"Connected to Robot at {ROBOT_IP}")
            break
        except Exception:
            pass

if not ROBOT_IP:
    ROBOT_IP = "192.168.1.24"

def set_angle(ch, val):
    url = f"http://{ROBOT_IP}/set_servo?ch={ch}&val={val}"
    try:
        req = urllib.request.urlopen(url, timeout=3)
        return req.read().decode()
    except Exception as e:
        print(f"Error communicating with robot: {e}")
        return None

def park():
    url = f"http://{ROBOT_IP}/run_gait?gait=stand"
    try:
        urllib.request.urlopen(url, timeout=3)
    except:
        pass

print("=== 6-DOF BIPED ROBOT: CAP-SAFE HIP FORWARD TEST ===")
print("Resetting all joints to neutral stand...")
park()
time.sleep(1.0)

# TEST 1: RIGHT HIP (CH 0) - FORWARD ONLY (94 -> 99 -> 94)
print("\n>>> TEST 1: Right Hip (CH 0) Forward Motion")
print("Moving Right Hip forward from 94° to 99° (+5°)...")
set_angle(0, 99)
time.sleep(2.0)
print("Returning Right Hip to 94°...")
set_angle(0, 94)
time.sleep(1.0)

# TEST 2: LEFT HIP (CH 4) - FORWARD ONLY (94 -> 89 -> 94)
print("\n>>> TEST 2: Left Hip (CH 4) Forward Motion")
print("Moving Left Hip forward from 94° to 89° (-5°)...")
set_angle(4, 89)
time.sleep(2.0)
print("Returning Left Hip to 94°...")
set_angle(4, 94)
time.sleep(1.0)

print("\n=== SAFE HIP TEST COMPLETE ===")
park()
