#include "kinematics.h"
#include <math.h>

// =============================================================================
// Static Locomotion & Gait State
// =============================================================================
static float walkFreq         = DEFAULT_WALK_FREQ_HZ;
static float stepLength       = DEFAULT_STEP_LENGTH_CM;
static float stepClearance    = DEFAULT_STEP_CLEARANCE_CM;
static float stepHeight       = DEFAULT_STEP_HEIGHT_CM;

static float currentGaitScale = 0.0f;
static float walkTime         = 0.0f;
static bool  gaitActive       = false;
static String activeGaitName  = "stand";

// Real-Time Ground Reaction Force & Contact State
static float weightLeft       = 250.0f;
static float weightRight      = 250.0f;
static float groundForceN     = 4.905f;
static String contactState    = "DUAL STANCE";

// =============================================================================
// 1. Analytical 2D Inverse Kinematics Engine
// Links: Thigh l1 = 5.0cm, Shin l2 = 5.7cm, Max Extended Height = 10.7cm
// =============================================================================
IKResult solveIK(float x, float z) {
  IKResult res;
  float hipRad2 = atan2(x, z);
  float hipDeg2 = hipRad2 * (180.0f / (float)M_PI);

  float z2 = sqrt(x * x + z * z);

  // Law of Cosines for Hip angle
  float val1 = constrain((sq(LINK_THIGH_L1_CM) + sq(z2) - sq(LINK_SHIN_L2_CM)) /
                         (2.0f * LINK_THIGH_L1_CM * z2), -1.0f, 1.0f);
  float hipRad1 = acos(val1);
  float hipDeg1 = hipRad1 * (180.0f / (float)M_PI);

  // Law of Cosines for Knee angle
  float val2 = constrain((sq(LINK_THIGH_L1_CM) + sq(LINK_SHIN_L2_CM) - sq(z2)) /
                         (2.0f * LINK_THIGH_L1_CM * LINK_SHIN_L2_CM), -1.0f, 1.0f);
  float kneeRad = (float)M_PI - acos(val2);
  float kneeDeg = kneeRad * (180.0f / (float)M_PI);

  // Law of Cosines for Ankle angle
  float val3 = constrain((sq(LINK_SHIN_L2_CM) + sq(z2) - sq(LINK_THIGH_L1_CM)) /
                         (2.0f * LINK_SHIN_L2_CM * z2), -1.0f, 1.0f);
  float ankleRad = ((float)M_PI / 2.0f) + hipRad2 - acos(val3);
  float ankleDeg = ankleRad * (180.0f / (float)M_PI);

  res.hipDeg   = hipDeg1 + hipDeg2;
  res.kneeDeg  = kneeDeg;
  res.ankleDeg = ankleDeg;
  return res;
}

// Zero-Impact Double-Support Swing Lift (Soft parabolic bell curve)
static inline float calculateSwingClearance(float stepPhase, float sClear) {
  const float ds = 0.15f * (float)M_PI; // 15% double support buffer on each transition (30% total)
  if (stepPhase < ds || stepPhase > ((float)M_PI - ds)) {
    return 0.0f; // Firm double-support ground contact
  }
  float u = (stepPhase - ds) / ((float)M_PI - 2.0f * ds);
  float s = sin(u * (float)M_PI);
  return sClear * (s * s);
}

// =============================================================================
// =============================================================================
// 2. Coordinated Joint Actuation Layer (True 6-Joint Dynamic Inverse Kinematics)
// Drives ALL 6 joints (Hips, Knees, Feet) simultaneously and harmoniously:
//   - Right Leg: Hip = R_HIP_HOME + dHip,  Knee = R_KNEE_HOME + dKnee,  Foot = R_FOOT_HOME + dAnkle
//   - Left Leg:  Hip = L_HIP_HOME - dHip,  Knee = L_KNEE_HOME - dKnee,  Foot = L_FOOT_HOME - dAnkle
//   - In Swing: Hip swings forward, Knee bends dynamically to lift foot, Ankle articulates for ground clearance
//   - In Stance: Hip drives backward, Knee & Ankle maintain constant height and keep sole flat on ground
// =============================================================================
void updateServoPos(float dHip, float dKnee, float dAnkle, char leg, bool isSwing) {
  if (leg == 'l') {
    // Left Leg: Inverted symmetry across all 3 joints
    setServoAngle(CH_L_HIP,  constrain(L_HIP_HOME  - dHip,   L_HIP_MIN,  L_HIP_MAX));
    setServoAngle(CH_L_KNEE, constrain(L_KNEE_HOME - dKnee,  L_KNEE_MIN, L_KNEE_MAX));
    setServoAngle(CH_L_FOOT, constrain(L_FOOT_HOME - dAnkle, L_FOOT_MIN, L_FOOT_MAX));
  } else if (leg == 'r') {
    // Right Leg: Direct symmetry across all 3 joints
    setServoAngle(CH_R_HIP,  constrain(R_HIP_HOME  + dHip,   R_HIP_MIN,  R_HIP_MAX));
    setServoAngle(CH_R_KNEE, constrain(R_KNEE_HOME + dKnee,  R_KNEE_MIN, R_KNEE_MAX));
    setServoAngle(CH_R_FOOT, constrain(R_FOOT_HOME + dAnkle, R_FOOT_MIN, R_FOOT_MAX));
  }
}

void updateAllJoints(float xr, float zr, float xl, float zl) {
  // Solve analytical 2D Inverse Kinematics for both legs simultaneously
  IKResult ikR = solveIK(xr, zr);
  IKResult ikL = solveIK(xl, zl);
  IKResult nom = solveIK(0.0f, stepHeight);

  // Joint deltas relative to neutral standing height
  float dHipR   = ikR.hipDeg - nom.hipDeg;
  float dKneeR  = ikR.kneeDeg - nom.kneeDeg;
  float dAnkleR = ikR.ankleDeg - nom.ankleDeg;

  float dHipL   = ikL.hipDeg - nom.hipDeg;
  float dKneeL  = ikL.kneeDeg - nom.kneeDeg;
  float dAnkleL = ikL.ankleDeg - nom.ankleDeg;

  bool isSwingR = (zr < stepHeight - 0.03f);
  bool isSwingL = (zl < stepHeight - 0.03f);

  // Actuate all joints simultaneously according to exact Inverse Kinematics
  updateServoPos(dHipR, dKneeR, dAnkleR, 'r', isSwingR);
  updateServoPos(dHipL, dKneeL, dAnkleL, 'l', isSwingL);
}

void pos(float x, float z, char leg) {
  IKResult ik  = solveIK(x, z);
  IKResult nom = solveIK(0.0f, stepHeight);

  float dHip   = ik.hipDeg - nom.hipDeg;
  float dKnee  = ik.kneeDeg - nom.kneeDeg;
  float dAnkle = ik.ankleDeg - nom.ankleDeg;
  bool isSwing = (z < stepHeight - 0.03f);

  updateServoPos(dHip, dKnee, dAnkle, leg, isSwing);
}

// =============================================================================
// 3. Real-Time Locomotion Generator (Called at 50Hz by FreeRTOS Task)
// =============================================================================
void updateLocomotion(float dt) {
  // Smooth Anti-Jerk Envelope
  if (gaitActive) {
    if (currentGaitScale < 1.0f) {
      currentGaitScale = min(1.0f, currentGaitScale + 0.025f);
    }
    walkTime += dt;
  } else {
    if (currentGaitScale > 0.0f) {
      currentGaitScale = max(0.0f, currentGaitScale - 0.04f);
      walkTime += dt;
    } else {
      return; // Fully settled at neutral baseline
    }
  }

  // Continuous Harmonic Phase (0 to 2*PI)
  float phase = fmod(2.0f * (float)M_PI * walkFreq * walkTime, 2.0f * (float)M_PI);
  if (phase < 0.0f) phase += 2.0f * (float)M_PI;

  float cosP = cos(phase);

  float sLength = stepLength * currentGaitScale;
  float sClear  = min(0.35f, stepClearance) * currentGaitScale;
  float sHeight = stepHeight;

  float xr = 0.0f;
  float xl = 0.0f;
  float zr = sHeight;
  float zl = sHeight;

  // Zero-impact swing clearance profiles with double-support weight transfer
  float liftL = (phase < (float)M_PI) ? calculateSwingClearance(phase, sClear) : 0.0f;
  float liftR = (phase >= (float)M_PI) ? calculateSwingClearance(phase - (float)M_PI, sClear) : 0.0f;

  if (activeGaitName == "backward" || activeGaitName == "back") {
    xr = -cosP * sLength;
    xl =  cosP * sLength;
    zr = sHeight - liftR;
    zl = sHeight - liftL;
  } else if (activeGaitName == "march") {
    xr = 0.0f;
    xl = 0.0f;
    zr = sHeight - liftR * 1.2f;
    zl = sHeight - liftL * 1.2f;
  } else if (activeGaitName == "turn_left" || activeGaitName == "left") {
    xr =  cosP * sLength;
    xl =  cosP * (sLength * 0.35f);
    zr = sHeight - liftR;
    zl = sHeight - liftL;
  } else if (activeGaitName == "turn_right" || activeGaitName == "right") {
    xr = -cosP * (sLength * 0.35f);
    xl = -cosP * sLength;
    zr = sHeight - liftR;
    zl = sHeight - liftL;
  } else if (activeGaitName == "crouch" || activeGaitName == "squat") {
    xr = 0.0f;
    xl = 0.0f;
    float squat = (0.5f - 0.5f * cosP) * 1.2f * currentGaitScale;
    zr = sHeight - squat;
    zl = sHeight - squat;
  } else {
    // Default Forward Dynamic Walk
    xr =  cosP * sLength;
    xl = -cosP * sLength;
    zr = sHeight - liftR;
    zl = sHeight - liftL;
  }

  // Actuate joints in strict sequence: Hips -> Knees -> Feet
  updateAllJoints(xr, zr, xl, zl);

  // Dynamic Ground Reaction Force & Support State Telemetry
  if (currentGaitScale > 0.10f) {
    if (liftL > 0.05f) {
      weightRight = min(500.0f, ROBOT_TOTAL_WEIGHT_GRAMS * (0.85f + 0.15f * (liftL / sClear)));
      weightLeft  = max(0.0f, ROBOT_TOTAL_WEIGHT_GRAMS - weightRight);
      contactState = "RIGHT SINGLE SUPPORT";
    } else if (liftR > 0.05f) {
      weightLeft  = min(500.0f, ROBOT_TOTAL_WEIGHT_GRAMS * (0.85f + 0.15f * (liftR / sClear)));
      weightRight = max(0.0f, ROBOT_TOTAL_WEIGHT_GRAMS - weightLeft);
      contactState = "LEFT SINGLE SUPPORT";
    } else {
      weightLeft   = 250.0f;
      weightRight  = 250.0f;
      contactState = "DOUBLE SUPPORT";
    }
  } else {
    weightLeft   = 250.0f;
    weightRight  = 250.0f;
    contactState = "DUAL STANCE";
  }
  groundForceN = (weightLeft + weightRight) * 0.00981f;
}

// =============================================================================
// 4. Diagnostics & High-Level Stepping Routines
// =============================================================================
void executeSingleStep() {
  parkNeutralStand();
  delay(100);
  gaitActive = true;
  activeGaitName = "step";
  float period = 1.0f / walkFreq;
  unsigned long t0 = millis();
  unsigned long durationMs = static_cast<unsigned long>(period * 1000.0f);

  while (millis() - t0 < durationMs) {
    float u = static_cast<float>(millis() - t0) / static_cast<float>(durationMs);
    currentGaitScale = sin(u * (float)M_PI); // Smooth half-sine window: 0 -> 1 -> 0
    walkTime = u * period;
    updateLocomotion(0.020f);
    delay(20);
  }
  stopLocomotion();
}

void runJointDiagnostics() {
  parkNeutralStand();
  delay(300);

  // 1. Right Hip (CH 0): 94° -> 100° -> 94° (Forward stride sweep)
  setServoAngle(CH_R_HIP, 100.0f); delay(700);
  setServoAngle(CH_R_HIP, R_HIP_HOME); delay(350);

  // 2. Right Knee (CH 1): 8° -> 16° -> 8° (Knee flexion lift sweep)
  setServoAngle(CH_R_KNEE, 16.0f); delay(700);
  setServoAngle(CH_R_KNEE, R_KNEE_HOME); delay(350);

  // 3. Right Foot (CH 12): 94° -> 99° -> 94° (Dynamic ground leveling sweep)
  setServoAngle(CH_R_FOOT, 99.0f); delay(700);
  setServoAngle(CH_R_FOOT, R_FOOT_HOME); delay(350);

  // 4. Left Hip (CH 4): 94° -> 88° -> 94° (Forward stride sweep)
  setServoAngle(CH_L_HIP, 88.0f); delay(700);
  setServoAngle(CH_L_HIP, L_HIP_HOME); delay(350);

  // 5. Left Knee (CH 8): 94° -> 86° -> 94° (Knee flexion lift sweep)
  setServoAngle(CH_L_KNEE, 86.0f); delay(700);
  setServoAngle(CH_L_KNEE, L_KNEE_HOME); delay(350);

  // 6. Left Foot (CH 15): 94° -> 89° -> 94° (Dynamic ground leveling sweep)
  setServoAngle(CH_L_FOOT, 89.0f); delay(700);
  setServoAngle(CH_L_FOOT, L_FOOT_HOME); delay(350);

  parkNeutralStand();
}

void takeStep(float sLength, int stepVelocity) {
  int delayMs = (stepVelocity > 0) ? stepVelocity : 20;
  float sClear = min(0.35f, stepClearance);

  // Phase 1: Right Leg stance, Left Leg swing
  for (float i = sLength; i >= -sLength; i -= 0.10f) {
    float u = (sLength - i) / (2.0f * sLength);
    float lift = sClear * sin(u * (float)M_PI);
    updateAllJoints(i, stepHeight, -i, stepHeight - lift);
    delay(delayMs);
  }

  // Phase 2: Right Leg swing, Left Leg stance
  for (float i = sLength; i >= -sLength; i -= 0.10f) {
    float u = (sLength - i) / (2.0f * sLength);
    float lift = sClear * sin(u * (float)M_PI);
    updateAllJoints(-i, stepHeight - lift, i, stepHeight);
    delay(delayMs);
  }

  // Settle back to neutral
  for (float i = sLength; i >= 0.0f; i -= 0.10f) {
    updateAllJoints(i, stepHeight, -i, stepHeight);
    delay(delayMs);
  }
  parkNeutralStand();
}

void initialize() {
  for (float i = MAX_LEG_EXTENSION_CM; i >= stepHeight; i -= 0.05f) {
    updateAllJoints(0.0f, i, 0.0f, i);
    delay(15);
  }
  parkNeutralStand();
}

// =============================================================================
// 5. Lifecycle, Tuning & Telemetry Interface
// =============================================================================
void initKinematics() {
  currentGaitScale = 0.0f;
  walkTime         = 0.0f;
  gaitActive       = false;
  activeGaitName   = "stand";
  weightLeft       = 250.0f;
  weightRight      = 250.0f;
  groundForceN     = 4.905f;
  contactState     = "DUAL STANCE";
}

void startLocomotion(const String& gait) {
  gaitActive = true;
  activeGaitName = gait;
  currentGaitScale = 0.0f;
  walkTime = 0.0f;
}

void stopLocomotion() {
  gaitActive = false;
  activeGaitName = "stand";
  currentGaitScale = 0.0f;
  parkNeutralStand();
  weightLeft   = 250.0f;
  weightRight  = 250.0f;
  groundForceN = 4.905f;
  contactState = "DUAL STANCE";
}

void setGaitParam(const String& param, float val) {
  if (param == "freq" || param == "speed") {
    walkFreq = constrain(val, 0.3f, 2.0f);
  } else if (param == "stride" || param == "length") {
    stepLength = constrain(val, 0.5f, 3.5f);
  } else if (param == "knee" || param == "clearance" || param == "lift") {
    stepClearance = constrain(val, 0.2f, 2.0f);
  } else if (param == "height" || param == "stance") {
    stepHeight = constrain(val, 8.5f, MAX_LEG_EXTENSION_CM);
  }
}

LocomotionTelemetry getLocomotionTelemetry() {
  LocomotionTelemetry t;
  t.weightLeftGrams    = weightLeft;
  t.weightRightGrams   = weightRight;
  t.groundForceNewtons = groundForceN;
  t.contactState       = contactState;
  t.activeGait         = activeGaitName;
  t.isRunning          = gaitActive;
  return t;
}
