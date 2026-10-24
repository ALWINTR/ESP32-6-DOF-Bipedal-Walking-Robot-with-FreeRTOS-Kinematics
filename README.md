# ESP32 6-DOF Bipedal Walking Robot with FreeRTOS Kinematics

<div align="center">

[![GitHub Repository](https://img.shields.io/badge/GitHub-Repository-00f0ff?style=for-the-badge&logo=github&logoColor=white)](https://github.com/ALWINTR/ESP32-6-DOF-Bipedal-Walking-Robot-with-FreeRTOS-Kinematics)
[![Developer](https://img.shields.io/badge/Developer-Alwin_T_R-0284c7?style=for-the-badge&logo=linkedin&logoColor=white)](https://www.linkedin.com/in/alwintr)
[![Portfolio](https://img.shields.io/badge/Portfolio-alwintr.github.io-00f0ff?style=for-the-badge&logo=googlechrome&logoColor=black)](https://alwintr.github.io)
[![License](https://img.shields.io/badge/License-MIT-10b981?style=for-the-badge)](LICENSE)

</div>

> Six-DOF humanoid bipedal walking robot powered by an ESP32 dual-core SoC running real-time FreeRTOS locomotion tasks, closed-form 2D Inverse Kinematics, PCA9685 16-channel PWM servo control over 400kHz I2C, and an embedded Web Control Studio.

---

## 📌 Executive Overview

Designed and validated on physical hardware, this robotics architecture implements a dynamically balanced **6-DOF bipedal walking robot** using an **ESP32 dual-core microcontroller** and an external **PCA9685 16-channel 12-bit PWM controller**. The system coordinates 6 high-torque metal-gear servos (hips, knees, and ankles) with closed-form 2D Inverse Kinematics, $C^2$-continuous zero-jerk quintic swing clearance trajectories, and monotonic stance smoothstep interpolation.

- **Primary Domain**: Autonomous Robotics & Dynamic Kinematics
- **Microcontroller Core**: ESP32 Dual-Core Xtensa LX6 (240MHz)
- **Real-Time Kernel**: FreeRTOS Dual-Core Thread Isolation (50Hz Locomotion Task on Core 1)
- **Actuation Bus**: PCA9685 12-bit PWM Driver over I2C Fast Mode (400kHz, Addr: `0x40`)
- **Actuators**: 6x TowerPro MG996R High-Torque Metal Gear Servos
- **Telemetry & Control**: Embedded Asynchronous Web Server & Web Studio on Core 0

---

## 📐 System Architecture & Signal Flow

```
+-----------------------------------------------------------------------------------+
|                           ESP32 Dual-Core SoC (240MHz)                            |
|                                                                                   |
|  [ Core 0: Web Studio & Network ]                 [ Core 1: Real-Time Locomotion ]|
|  - Asynchronous HTTP Web Server                   - Deterministic 50Hz Loop (20ms)|
|  - RESTful API & Telemetry Endpoints              - Closed-Form 2D IK (solveIK)   |
|  - SoftAP & Dual Station Wi-Fi                    - Quintic Zero-Jerk Trajectory  |
|  - Atomic Angle Snapshots (<= 20us)               - Safety Angle Clamping Engine  |
+-----------------------------------------------------------------------------------+
                                         |
                                         | I2C Fast Mode (400kHz, SDA: 21, SCL: 22)
                                         v
+-----------------------------------------------------------------------------------+
|                     PCA9685 16-Channel 12-Bit PWM Controller                      |
|                               (I2C Address: 0x40)                                 |
+-----------------------------------------------------------------------------------+
           |                                                      |
           | Right Leg Channels                                   | Left Leg Channels
           +-----------------------------+                        +-----------------------------+
           | Channel 0:  Right Hip       |                        | Channel 4:  Left Hip        |
           | Channel 1:  Right Knee      |                        | Channel 8:  Left Knee       |
           | Channel 12: Right Ankle/Foot|                        | Channel 15: Left Ankle/Foot |
           +-----------------------------+                        +-----------------------------+
                         |                                                      |
                         v                                                      v
           [ 3x MG996R Servos (Right) ]                           [ 3x MG996R Servos (Left) ]
```

---

## 📋 Hardware Bill of Materials (BOM)

| Component Name | Operating Voltage | Interface / Functionality |
| :--- | :--- | :--- |
| **ESP32-WROOM-32D** | `3.3V / 5V` | Dual Xtensa LX6 240MHz, FreeRTOS, Wi-Fi 802.11 b/g/n |
| **PCA9685 16-Channel PWM** | `3.3V Logic / 5–6V V+` | 12-bit resolution, I2C Fast Mode 400kHz, Address `0x40` |
| **6x TowerPro MG996R Servos** | `6.0V / 2.5A peak` | High-torque metal gear (13 kg-cm), 50Hz PWM control |
| **External UBEC 6V / 5A Buck** | `7.4V–12V -> 6.0V DC` | Dedicated high-current regulated power rail for servos |
| **2S LiPo Battery (7.4V)** | `7.4V Nominal (8.4V Max)`| High-discharge power source for robot chassis |
| **Logic Level Pull-Ups (I2C)** | `3.3V Bus` | 4.7kΩ pull-up resistors on SDA (GPIO 21) & SCL (GPIO 22) |

---

## 🔌 Circuit Pinout & Microcontroller Interconnect

| Subsystem / Node | ESP32 Pin / Master | PCA9685 Channel / Pin | Signal Function / Bus |
| :--- | :--- | :--- | :--- |
| **I2C Serial Data** | `GPIO 21` | `SDA` | I2C Data at 400kHz Fast Mode |
| **I2C Serial Clock** | `GPIO 22` | `SCL` | I2C Clock at 400kHz Fast Mode |
| **Right Hip Actuator** | — | `Channel 0 PWM` | Right Hip Pitch Servo Signal |
| **Right Knee Actuator** | — | `Channel 1 PWM` | Right Knee Pitch Servo Signal |
| **Right Foot/Ankle Actuator** | — | `Channel 12 PWM` | Right Ankle Ground Alignment Signal |
| **Left Hip Actuator** | — | `Channel 4 PWM` | Left Hip Pitch Servo Signal |
| **Left Knee Actuator** | — | `Channel 8 PWM` | Left Knee Pitch Servo Signal |
| **Left Foot/Ankle Actuator** | — | `Channel 15 PWM` | Left Ankle Ground Alignment Signal |
| **Common Ground** | `GND` | `GND` | Shared reference between ESP32 and 6V UBEC |

---

## ⚙️ Firmware Architecture & Locomotion Kinematics

### 1. Dual-Core FreeRTOS Thread Isolation
- **Core 1 Locomotion Task (`locomotionTask`)**: Strictly pinned to Core 1 running at a fixed 50Hz cycle (`pdMS_TO_TICKS(20)`) using `vTaskDelayUntil()`. It is the sole task permitted to drive the PCA9685 PWM servos during active locomotion.
- **Core 0 Web Studio Task**: Handles asynchronous HTTP web routing, RESTful API endpoints, Wi-Fi reconnection, and telemetry without causing jitter or blocking Core 1.
- **Thread-Safe Mutex Disciplines**:
  - `i2cMutex`: Protects all Wire / PCA9685 bus transactions.
  - `locomotionMutex`: Guards gait mode switches, tuning parameters, and manual overrides.
- **Non-Blocking Telemetry Snapshots**: Telemetry readouts (`/api/status`, `/get_angles`) obtain joint angles via an atomic snapshot routine ($\le 20\,\mu\text{s}$) rather than locking the I2C bus or re-evaluating IK.

### 2. Closed-Form 2D Inverse Kinematics (`solveIK(x, z)`)
- **Simultaneous 6-Joint Motion**: All 6 leg joints move dynamically and simultaneously according to closed-form 2D Inverse Kinematics.
- **Ground-Parallel Sole Alignment**: Ankle/foot pitch dynamically counter-rotates against hip and knee angles:
  $$\theta_{\text{ankle}} = \theta_{\text{hip}} - \theta_{\text{knee}}$$
  This maintains the sole parallel to the walking plane ($|\text{error}| < 10^{-4}\,\text{rad}$) across both stance and swing phases.
- **Mirroring Symmetry**:
  - Right Leg: $\text{Hip} = \text{HOME} + \Delta\theta_{\text{hip}}$, $\text{Knee} = \text{HOME} + \Delta\theta_{\text{knee}}$, $\text{Foot} = \text{HOME} + \Delta\theta_{\text{ankle}}$
  - Left Leg: $\text{Hip} = \text{HOME} - \Delta\theta_{\text{hip}}$, $\text{Knee} = \text{HOME} - \Delta\theta_{\text{knee}}$, $\text{Foot} = \text{HOME} - \Delta\theta_{\text{ankle}}$

### 3. $C^2$-Continuous Zero-Jerk Trajectory Profiles
- **Swing Foot Clearance**: Vertical clearance $z(t)$ during swing uses a $C^2$-continuous quintic trajectory:
  $$f(u) = 64 u^3 (1-u)^3 \quad (u \in [0, 1])$$
  First and second derivatives equal zero at lift-off ($u=0$) and touch-down ($u=1$), preventing ground impact shocks and foot scraping.
- **Stance Foot Monotonicity**: Horizontal ground progression $x(t)$ during stance follows a monotonic quintic smoothstep:
  $$s(t) = 6t^5 - 15t^4 + 10t^3$$
  Eliminates sinusoidal velocity reversals and prevents backwards foot drag.
- **Dynamic Forward Lean**: Incorporates a Center-of-Mass forward lean bias ($x_{\text{bias}} \approx -0.30\,\text{cm}$) relative to foot position to harness gravity for forward propulsion without slipping.

### 4. Hardware Safety Limits & Angle Clamping
- Step Length: $0.40\,\text{cm} \le s_{\text{length}} \le 1.20\,\text{cm}$ (default: $0.90\,\text{cm}$)
- Step Clearance: $0.15\,\text{cm} \le s_{\text{clear}} \le 0.65\,\text{cm}$ (default: $0.60\,\text{cm}$)
- Stance Height: $9.40\,\text{cm} \le s_{\text{height}} \le 10.10\,\text{cm}$ (default: $10.00\,\text{cm}$)
- Right Knee physical bracket limit: $5^\circ$ to $28^\circ$ (home: $8^\circ$).
- Left Knee physical bracket limit: $74^\circ$ to $97^\circ$ (home: $94^\circ$).

---

## 📁 Repository Structure

```
ESP32-6-DOF-Bipedal-Walking-Robot-with-FreeRTOS-Kinematics/
├── include/
│   ├── config.h                       # Pin assignments, safety brackets, joint enum & calibration
│   ├── kinematics.h                   # 2D IK solver, quintic trajectories & locomotion engine
│   ├── pca_driver.h                   # PCA9685 16-channel I2C PWM driver interface
│   └── web_server.h                   # Async web server, telemetry endpoints & HTML UI
├── src/
│   ├── kinematics.cpp                 # 50Hz FreeRTOS locomotion task & gait state machine
│   ├── main.cpp                       # Dual-core initialization, task pinning & watchdog setup
│   ├── pca_driver.cpp                 # Thread-safe I2C bus transactions & angle-to-tick mapper
│   └── web_server.cpp                 # Embedded Web Studio UI, animated rig & REST handlers
├── test_hip_knee.py                   # Python test script for joint angle validation
├── test_locomotion_stability.py       # Trajectory continuity and jerk analysis verification
├── platformio.ini                     # PlatformIO build configuration for ESP32 Dev Module
├── LICENSE                            # MIT Open Source License
└── README.md                          # Technical documentation and hardware architecture
```

---

## 🚀 Build & Deployment Instructions

### 1. Hardware Power Distribution
1. Connect external 6V 5A UBEC output to the `V+` screw terminal of the PCA9685 board.
2. Connect the UBEC Ground to both the PCA9685 `GND` and the ESP32 `GND` (common reference).
3. Connect ESP32 `GPIO 21` to PCA9685 `SDA`, and `GPIO 22` to PCA9685 `SCL`.
4. Plug servos into PCA9685:
   - Right Leg: Hip `Ch 0`, Knee `Ch 1`, Foot `Ch 12`
   - Left Leg: Hip `Ch 4`, Knee `Ch 8`, Foot `Ch 15`

### 2. Compilation with PlatformIO
```bash
# Clone the repository
git clone https://github.com/ALWINTR/ESP32-6-DOF-Bipedal-Walking-Robot-with-FreeRTOS-Kinematics.git

# Navigate into directory
cd ESP32-6-DOF-Bipedal-Walking-Robot-with-FreeRTOS-Kinematics

# Build firmware
pio run

# Flash to ESP32 over USB-UART
pio run -t upload

# Monitor serial debug output (115200 baud)
pio device monitor -b 115200
```

### 3. Compilation with Arduino IDE
1. Open Arduino IDE and install the **ESP32 Board Package** by Espressif.
2. Select Board: **ESP32 Dev Module**.
3. Install required libraries: `ESPAsyncWebServer`, `AsyncTCP`, `Wire`.
4. Open `src/main.cpp`, compile and flash to the connected ESP32 port.

### 4. Web Control Studio Access
- Connect to the robot's SoftAP: **SSID:** `ESP32-Biped-Robot`, **Password:** `12345678`
- Open browser and navigate to: `http://192.168.4.1` or `http://biped.local`
- Access real-time walking controls, forward/turn commands, live joint telemetry, and interactive oscilloscope.

---

## 👨‍💻 Author

**Alwin T R** — Robotics & Automation Engineer  
- 💼 **LinkedIn**: [linkedin.com/in/alwintr](https://www.linkedin.com/in/alwintr)  
- 🌐 **Interactive Portfolio**: [alwintr.github.io](https://alwintr.github.io)  
- 🐙 **GitHub**: [github.com/ALWINTR](https://github.com/ALWINTR)  
- ✉️ **Direct Email**: [alwintr2003@gmail.com](mailto:alwintr2003@gmail.com)

---

## 📄 License

This project is licensed under the **MIT License** — see the [LICENSE](LICENSE) file for details.
