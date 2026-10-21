# ESP32 6-DOF Bipedal Robot Controller Plugin

This plugin packages the complete developer toolchain, kinematics rules, and operational runbooks for the **ESP32 6-DOF Bipedal Walking Robot**.

## Features

- **FreeRTOS Dual-Core Architecture**: Strict Core 1 isolation for deterministic 50Hz locomotion timing and Core 0 web serving.
- **Closed-Form 2D Inverse Kinematics**: Coordinated 6-joint actuation with dynamic ankle counter-rotation keeping soles strictly parallel to the ground.
- **$C^2$ Quintic Zero-Jerk Trajectories**: Smooth swing clearance profiles preventing floor dragging and impact shocks.
- **Dynamic Forward Lean**: Inverted-pendulum Center-of-Mass forward bias converting stepping motions into decisive forward ground displacement.
- **Hardware Bracket Limits**: Safety clamping protecting servo horns and mechanical stops.
- **Full Operational Runbook**: PlatformIO builds, serial flashing, calibration, and live HTTP/Serial telemetry streaming.

## Components Included

- **Rule**: `rules/AGENTS.md` (Active constraints on FreeRTOS tasks, IK formulas, and joint bounds)
- **Skill**: `skills/biped-control/SKILL.md` (Procedural commands for build, upload, calibration, and debugging)
- **Scripts**:
  - `skills/biped-control/scripts/test_stability.py`
  - `skills/biped-control/scripts/read_telemetry.py`
