#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// =============================================================================
// 1. WiFi & Network Configuration
// =============================================================================
struct WiFiCredential {
  const char* ssid;
  const char* password;
};

// Priority list for Station connection:
//   Priority #1: Primary Subnet (192.168.0.x)
//   Priority #2: Secondary Subnet (192.168.1.x)
const WiFiCredential KNOWN_NETWORKS[] = {
  {"MATHA ELECTRONICS_ Dwan", "qwerty09"},
  {"MATHA BSNL FTTH",          "qwerty09"}
};
const size_t NUM_NETWORKS = sizeof(KNOWN_NETWORKS) / sizeof(KNOWN_NETWORKS[0]);

// Autonomous SoftAP (Always active, static IP: 192.168.4.1)
const char* const AP_SSID = "ESP32-Biped-Robot";
const char* const AP_PASS = "12345678";

// mDNS URL: http://biped.local
const char* const MDNS_HOSTNAME = "biped";

// =============================================================================
// 2. Hardware Pinout & I2C Bus Configuration
// =============================================================================
#define PIN_I2C_SDA          21
#define PIN_I2C_SCL          22
#define I2C_FREQUENCY_HZ     400000 // 400kHz Fast I2C Mode for Zero-Lag PCA9685
#define PCA9685_I2C_ADDR     0x40
#define PCA9685_PWM_FREQ_HZ  50

#define SERVOMIN_US          544
#define SERVOMAX_US          2400

// =============================================================================
// 3. 6-DOF Joint Enumeration, Channel Map & Calibrated Angles
// =============================================================================
enum JointID {
  JOINT_R_HIP  = 0,
  JOINT_R_KNEE = 1,
  JOINT_R_FOOT = 2,
  JOINT_L_HIP  = 3,
  JOINT_L_KNEE = 4,
  JOINT_L_FOOT = 5,
  NUM_JOINTS   = 6
};

#define NUM_SERVOS NUM_JOINTS // Backward compatibility alias

// PCA9685 Physical Channel Mappings
#define CH_R_HIP   0
#define CH_R_KNEE  1
#define CH_R_FOOT  12
#define CH_L_HIP   4
#define CH_L_KNEE  8
#define CH_L_FOOT  15

// Right Leg Neutral Baselines & Safety Bracket Limits
#define R_HIP_HOME         94.0f
#define R_HIP_MIN          85.0f
#define R_HIP_MAX          103.0f

#define R_KNEE_HOME        8.0f
#define R_KNEE_MIN         8.0f
#define R_KNEE_MAX         16.5f

#define R_FOOT_HOME        94.0f
#define R_FOOT_MIN         94.0f
#define R_FOOT_MAX         94.0f

// Left Leg Neutral Baselines & Safety Bracket Limits
#define L_HIP_HOME         94.0f
#define L_HIP_MIN          85.0f
#define L_HIP_MAX          103.0f

#define L_KNEE_HOME        94.0f
#define L_KNEE_MIN         85.5f
#define L_KNEE_MAX         94.0f

#define L_FOOT_HOME        94.0f
#define L_FOOT_MIN         94.0f
#define L_FOOT_MAX         94.0f

// Maximum knee lift angle change (matches hip stride delta)
#define MAX_KNEE_LIFT_DEG  8.5f

struct JointConfig {
  int channel;
  const char* name;
  float homeAngle;
  float minLimit;
  float maxLimit;
};

const JointConfig DEFAULT_JOINTS[NUM_JOINTS] = {
  {CH_R_HIP,  "Right Hip",  R_HIP_HOME,  R_HIP_MIN,  R_HIP_MAX},  // Index 0
  {CH_R_KNEE, "Right Knee", R_KNEE_HOME, R_KNEE_MIN, R_KNEE_MAX}, // Index 1
  {CH_R_FOOT, "Right Foot", R_FOOT_HOME, R_FOOT_MIN, R_FOOT_MAX}, // Index 2
  {CH_L_HIP,  "Left Hip",   L_HIP_HOME,  L_HIP_MIN,  L_HIP_MAX},  // Index 3
  {CH_L_KNEE, "Left Knee",  L_KNEE_HOME, L_KNEE_MIN, L_KNEE_MAX}, // Index 4
  {CH_L_FOOT, "Left Foot",  L_FOOT_HOME, L_FOOT_MIN, L_FOOT_MAX}  // Index 5
};

// =============================================================================
// 4. Kinematics & Locomotion Tuning Defaults
// =============================================================================
// Robot Leg Link Dimensions (cm)
#define LINK_THIGH_L1_CM           5.00f  // Thigh link length
#define LINK_SHIN_L2_CM            5.70f  // Shin link length
#define MAX_LEG_EXTENSION_CM       (LINK_THIGH_L1_CM + LINK_SHIN_L2_CM) // 10.70cm

// Default Walking Parameters
#define DEFAULT_WALK_FREQ_HZ       0.60f  // Stride frequency (cadence)
#define DEFAULT_STEP_LENGTH_CM     0.85f  // Stable forward stride length
#define DEFAULT_STEP_CLEARANCE_CM  0.40f  // Ground clearance lift during swing
#define DEFAULT_STEP_HEIGHT_CM     10.00f // Standing stance height (Z nominal)

#define ROBOT_TOTAL_WEIGHT_GRAMS   500.0f // Nominal robot mass for force estimation

// FreeRTOS Dedicated Locomotion Task Timing
#define LOCOMOTION_TASK_FREQ_HZ    50     // 50Hz update rate
#define LOCOMOTION_PERIOD_MS       (1000 / LOCOMOTION_TASK_FREQ_HZ) // 20ms

#endif // CONFIG_H
