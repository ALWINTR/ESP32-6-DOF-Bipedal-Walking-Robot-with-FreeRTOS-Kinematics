#!/usr/bin/env python3
"""
Connects to the ESP32 Biped Robot via HTTP or Serial to poll and display live telemetry.
"""
import sys
import json
import urllib.request

DEFAULT_HOSTS = ["192.168.1.24", "192.168.4.1", "biped.local"]

def poll_http(host):
    url = f"http://{host}/get_angles"
    req = urllib.request.urlopen(url, timeout=2)
    data = json.loads(req.read().decode())
    print(f"\n--- ESP32 Biped Robot Telemetry ({host}) ---")
    print(f"Active Gait: {data.get('active_gait', 'unknown')} (Running: {data.get('gait_running', False)})")
    print(f"PCA9685 Ready: {data.get('pca_found', False)} | RSSI: {data.get('rssi', 0)} dBm | Free Heap: {data.get('heap', 0)} bytes")
    print(f"Contact State: {data.get('contact', 'unknown')} | Force: {data.get('force_n', 0)} N")
    print(f"Weight: Left {data.get('weight_l', 0)}g | Right {data.get('weight_r', 0)}g")
    print("\nLive Joint Angles:")
    angles = data.get("angles", {})
    mapping = {
        "0": "Right Hip ", "1": "Right Knee", "12": "Right Foot",
        "4": "Left Hip  ", "8": "Left Knee ", "15": "Left Foot "
    }
    for ch, name in mapping.items():
        print(f"  [CH {ch:>2}] {name}: {angles.get(ch, 'N/A')}°")

def main():
    target = sys.argv[1] if len(sys.argv) > 1 else None
    if target:
        poll_http(target)
        return

    for h in DEFAULT_HOSTS:
        try:
            poll_http(h)
            return
        except Exception:
            continue
    print("Could not reach robot via HTTP at", DEFAULT_HOSTS)

if __name__ == "__main__":
    main()
