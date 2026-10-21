---
name: biped-control
description: "Workflows and procedures for building, flashing, calibrating, and controlling the ESP32 6-DOF Bipedal Walking Robot."
---

# ESP32 6-DOF Bipedal Robot Control Skill

Use this skill when developing, testing, flashing, calibrating, or diagnosing the ESP32 6-DOF Bipedal Walking Robot.

---

## 1. Hardware Overview & Pinout

- **MCU**: ESP32 DevKit (Dual-core Xtensa @ 240MHz)
- **Servo Driver**: Adafruit PCA9685 16-channel 12-bit PWM at I2C address `0x40` (400kHz Fast I2C)
  - `SDA` $\to$ `GPIO 21`
  - `SCL` $\to$ `GPIO 22`
  - `VCC` $\to$ `3.3V` (Logic)
  - `V+`  $\to$ External 5V-6V UBEC / Battery (Actuator Power)
- **Joint Channel Map**:
  - **Right Leg**: Hip `CH 0` (Home $94^\circ$), Knee `CH 1` (Home $8^\circ$), Foot `CH 12` (Home $94^\circ$)
  - **Left Leg**: Hip `CH 4` (Home $94^\circ$), Knee `CH 8` (Home $94^\circ$), Foot `CH 15` (Home $94^\circ$)
- **Leg Geometry**: Thigh $l_1 = 5.0\,\text{cm}$, Shin $l_2 = 5.7\,\text{cm}$, Nominal Stance $Z = 10.0\,\text{cm}$

---

## 2. Standard Workflows

### A. Build Firmware
Compile the project using PlatformIO:
```powershell
cd "C:\Users\matha\.gemini\antigravity\scratch\esp32-bipedal-robot"
& "C:\Program Files\Python310\python.exe" -m platformio run
```

### B. Flash Firmware to Hardware
Upload to the ESP32 on `COM3`:
```powershell
cd "C:\Users\matha\.gemini\antigravity\scratch\esp32-bipedal-robot"
& "C:\Program Files\Python310\python.exe" -m platformio run -t upload
```

### C. Check Live Telemetry & Health
Query joint positions, contact state, and PCA9685 connection:
```powershell
# Via HTTP:
curl.exe -s http://192.168.1.24/get_angles

# Via Serial Monitor:
& "C:\Program Files\Python310\python.exe" -c "
import serial, time
s = serial.Serial('COM3', 115200, timeout=1, dsrdtr=False, rtscts=False)
s.dtr = False; s.rts = False
time.sleep(0.5)
s.write(b'status\n')
time.sleep(0.5)
while s.in_waiting: print(s.readline().decode('utf-8', errors='ignore').strip())
s.close()
"
```

### D. Trigger Locomotion Gaits
Send motion commands over HTTP or Serial:
- Forward Walk: `http://192.168.1.24/run_gait?gait=walk` (or serial `walk`)
- Single Stride: `http://192.168.1.24/run_gait?gait=step` (or serial `step`)
- Stop / Neutral: `http://192.168.1.24/run_gait?gait=stand` (or serial `stop`)
- In-Place March: `http://192.168.1.24/run_gait?gait=march`
- Diagnostics Sweep: `http://192.168.1.24/run_gait?gait=diag` (or serial `diag`)

---

## 3. Calibration Runbook

1. Open Mission Control Studio at `http://192.168.1.24` (or `http://192.168.4.1` on SoftAP).
2. Adjust individual servo sliders until the robot stands upright with both feet flat on the ground.
3. Click **"Save to Flash"** (calls `/save_offsets`), which commits the calibrated offsets to ESP32 NVS non-volatile flash memory.
4. Subsequent reboots automatically load your custom offsets from NVS.

---

## 4. Troubleshooting Guide

| Issue | Root Cause | Solution |
| :--- | :--- | :--- |
| **Servos don't move at all** | No power to PCA9685 `V+` screw terminals | Turn on external battery switch; verify 5V-6V on `V+` / `GND`. |
| **`pca_found: false`** | I2C wiring disconnected | Check `GPIO 21 (SDA)` and `GPIO 22 (SCL)` jumper wires. |
| **Robot shuffles/slips in place** | Low friction on smooth surface or low clearance | Add rubber/foam tape to foot soles; ensure clearance $\ge 0.60\,\text{cm}$. |
| **Robot tips to side during walk** | Center-of-mass balance | Slightly increase step frequency ($0.55 \to 0.70\,\text{Hz}$) or shorten stride. |
