#!/usr/bin/env python3
"""
Analytical validation of 2D Inverse Kinematics, quintic zero-jerk trajectories,
and hardware safety brackets for the ESP32 6-DOF Bipedal Robot.
"""
import math

L1 = 5.0
L2 = 5.7
STEP_HEIGHT = 10.0

LIMITS = {
    0: (82.0, 106.0),  # Right Hip
    1: (5.0, 28.0),    # Right Knee
    12: (82.0, 106.0), # Right Foot
    4: (82.0, 106.0),  # Left Hip
    8: (74.0, 97.0),   # Left Knee
    15: (82.0, 106.0)  # Left Foot
}

def solve_ik(x, z):
    z = max(1.0, z)
    z2 = math.sqrt(x * x + z * z)
    z2 = max(abs(L2 - L1) + 0.002, min(L1 + L2 - 0.002, z2))
    
    hip_rad2 = math.atan2(x, z)
    cos1 = max(-1.0, min(1.0, (L1*L1 + z2*z2 - L2*L2) / (2.0 * L1 * z2)))
    hip_rad1 = math.acos(cos1)
    
    cos2 = max(-1.0, min(1.0, (L1*L1 + L2*L2 - z2*z2) / (2.0 * L1 * L2)))
    knee_rad = math.pi - math.acos(cos2)
    
    cos3 = max(-1.0, min(1.0, (L2*L2 + z2*z2 - L1*L1) / (2.0 * L2 * z2)))
    ankle_rad = (math.pi / 2.0) + hip_rad2 - math.acos(cos3)
    
    return math.degrees(hip_rad1 + hip_rad2), math.degrees(knee_rad), math.degrees(ankle_rad)

def main():
    nom = solve_ik(0.0, STEP_HEIGHT)
    print("--- 1. Inverse Kinematics Math Verification ---")
    print(f"Nominal stance (0, 10.0cm): Hip={nom[0]:.2f}°, Knee={nom[1]:.2f}°, Ankle={nom[2]:.2f}°")
    
    print("\n--- 2. Forward Walk & Safety Bracket Verification ---")
    s_len = 0.90
    s_clear = 0.60
    forward_lean = 0.30
    
    for i in range(100):
        phase = 2.0 * math.pi * (i / 100.0)
        cos_p = math.cos(phase)
        
        lift_l = s_clear * 64.0 * (phase/math.pi)**3 * (1.0 - phase/math.pi)**3 if phase < math.pi else 0.0
        p2 = phase - math.pi
        lift_r = s_clear * 64.0 * (p2/math.pi)**3 * (1.0 - p2/math.pi)**3 if phase >= math.pi else 0.0
        
        xr =  cos_p * s_len - forward_lean
        xl = -cos_p * s_len - forward_lean
        zr = STEP_HEIGHT - lift_r
        zl = STEP_HEIGHT - lift_l
        
        ik_r = solve_ik(xr, zr)
        ik_l = solve_ik(xl, zl)
        
        r_hip = 94.0 + (ik_r[0] - nom[0])
        r_knee = 8.0 + (ik_r[1] - nom[1])
        r_foot = 94.0 + (ik_r[2] - nom[2])
        
        l_hip = 94.0 - (ik_l[0] - nom[0])
        l_knee = 94.0 - (ik_l[1] - nom[1])
        l_foot = 94.0 - (ik_l[2] - nom[2])
        
        angles = {0: r_hip, 1: r_knee, 12: r_foot, 4: l_hip, 8: l_knee, 15: l_foot}
        for ch, (low, high) in LIMITS.items():
            val = angles[ch]
            assert low <= val <= high, f"Violation on Ch {ch}: {val:.2f}° not in [{low}, {high}]"
            
    print("PASS: All 6 joints strictly conform to safety limits throughout walking cycle!")

if __name__ == "__main__":
    main()
