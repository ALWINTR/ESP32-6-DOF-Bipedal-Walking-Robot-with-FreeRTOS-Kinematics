# ESP32 6-DOF Bipedal Robot Kinematics & Concurrency Rules

This rule defines the core real-time concurrency, kinematic constraints, trajectory generation, and hardware safety limits for the ESP32 6-DOF Bipedal Walking Robot. All agents and modifications to this codebase MUST strictly adhere to these invariants.

---

## 1. FreeRTOS Dual-Core Concurrency & Thread Isolation
- **Core 1 Isolation**: The locomotion task (`locomotionTask`) must remain pinned to Core 1 running at a fixed 50Hz cycle (`pdMS_TO_TICKS(20)`) using `vTaskDelayUntil()`. It is the SOLE task permitted to drive the PCA9685 PWM servos during active locomotion.
- **Core 0 Web Task**: All WiFi, DNS, HTTP routing, and JSON API handling must remain pinned to Core 0. Never invoke kinematic calculations or blocking delays inside Core 0 request handlers.
- **Mutex Discipline**:
  - `i2cMutex`: Must guard all `Wire` / PCA9685 bus transactions. Bus scans must acquire and release per address to avoid starving Core 1.
  - `locomotionMutex`: Guards gait mode switches, tuning parameters, and manual overrides.
- **Non-Blocking Telemetry Snapshots**: Telemetry readouts (e.g. `/api/status`, `/get_angles`) must obtain joint angles via an atomic snapshot function (`getAllJointSnapshots()`, $\le 20\,\mu\text{s}$) rather than locking the I2C bus or recalculating angles.

---

## 2. Closed-Form 2D Inverse Kinematics & 6-Joint Dynamic Drive
- **Simultaneous 6-Joint Motion**: All 6 leg joints (Right: Hip Ch 0, Knee Ch 1, Foot Ch 12; Left: Hip Ch 4, Knee Ch 8, Foot Ch 15) must move dynamically and simultaneously according to closed-form 2D Inverse Kinematics (`solveIK(x, z)`).
- **Ground-Parallel Sole Alignment**: Ankle/foot pitch must dynamically counter-rotate against hip and knee angles:
  $$\theta_{\text{ankle}} = \theta_{\text{hip}} - \theta_{\text{knee}}$$
  This maintains the sole parallel to the walking plane ($|\text{error}| < 10^{-4}\,\text{rad}$) across both stance and swing phases.
- **Mirroring Symmetry**:
  - Right Leg: $\text{Hip} = \text{HOME} + \Delta\theta_{\text{hip}}$, $\text{Knee} = \text{HOME} + \Delta\theta_{\text{knee}}$, $\text{Foot} = \text{HOME} + \Delta\theta_{\text{ankle}}$
  - Left Leg: $\text{Hip} = \text{HOME} - \Delta\theta_{\text{hip}}$, $\text{Knee} = \text{HOME} - \Delta\theta_{\text{knee}}$, $\text{Foot} = \text{HOME} - \Delta\theta_{\text{ankle}}$

---

## 3. $C^2$-Continuous Zero-Jerk Trajectory Profiles
- **Swing Foot Clearance**: Vertical clearance $z(t)$ during the swing phase must use a $C^2$-continuous quintic trajectory:
  $$f(u) = 64 u^3 (1-u)^3 \quad (u \in [0, 1])$$
  First and second derivatives must strictly equal zero at lift-off ($u=0$) and touch-down ($u=1$) to prevent floor scraping and ground impact shocks. Never use discontinuous step functions or deadband buffers.
- **Stance Foot Monotonicity**: Horizontal ground progression $x(t)$ during stance must follow a monotonic two-phase quintic smoothstep:
  $$s(t) = 6t^5 - 15t^4 + 10t^3$$
  Never use sinusoidal profiles that introduce negative velocity spikes or backwards foot drag.
- **Dynamic Forward Lean**: Forward walking must incorporate a Center-of-Mass forward lean bias ($x_{\text{bias}} \approx -0.30\,\text{cm}$) relative to foot position to harness gravity (inverted pendulum model) for positive forward propulsion without slip.

---

## 4. Hardware Safety Brackets & Parameter Clamping
- Clamp user-adjustable and gait parameters within hardware safety bounds:
  - Step Length $s_{\text{length}} \in [0.40, 1.20]\,\text{cm}$ (default: $0.90\,\text{cm}$)
  - Step Clearance $s_{\text{clear}} \in [0.15, 0.65]\,\text{cm}$ (default: $0.60\,\text{cm}$)
  - Stance Height $s_{\text{height}} \in [9.40, 10.10]\,\text{cm}$ (default: $10.00\,\text{cm}$)
  - Crouch Squat $\Delta h \le 0.65\,\text{cm}$
- Ensure the right knee target angle strictly respects the bracket physical limit range ($5^\circ$ to $28^\circ$, home $8^\circ$) and the left knee respects ($74^\circ$ to $97^\circ$, home $94^\circ$).

---

## 5. Deterministic State Transitions
- On stance-to-locomotion mode switches (`walk`, `turn_left`, `turn_right`, `backward`), always reset `walkTime = 0.0f` and `currentGaitScale = 0.0f` to ramp up motion smoothly without angular step impulses.
