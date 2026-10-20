#include "kinematics.h"
#include <math.h>

// =============================================================================
// Static Locomotion & Gait State
// =============================================================================
static float walkFreq         = DEFAULT_WALK_FREQ_HZ;
static float stepLength       = DEFAULT_STEP_LENGTH_CM;
static float stepClearance    = DEFAULT_STEP_CLEARANCE_CM;
static float stepHeight       = DEFAULT_STEP_HEIGHT_CM;

static LocomotionMode activeMode = GAIT_STAND;
static float currentGaitScale    = 0.0f;
static float walkTime            = 0.0f;
static float routineTime         = 0.0f;

// Real-Time Ground Reaction Force & Contact State
static float weightLeft          = 250.0f;
static float weightRight         = 250.0f;
static float groundForceN        = 4.905f;
static char  contactStateBuf[32] = "DUAL STANCE";
static char  activeGaitBuf[32]   = "stand";

// FreeRTOS Synchronization Mutex for Locomotion State
static SemaphoreHandle_t locomotionMutex = NULL;

static inline bool lockLocomotion(TickType_t timeoutMs = 25) {
  if (locomotionMutex == NULL) {
    locomotionMutex = xSemaphoreCreateMutex();
  }
  if (locomotionMutex) {
    return (xSemaphoreTake(locomotionMutex, pdMS_TO_TICKS(timeoutMs)) == pdTRUE);
  }
  return false;
}

static inline void unlockLocomotion() {
  if (locomotionMutex) {
    xSemaphoreGive(locomotionMutex);
  }
}

// =============================================================================
// 1. Analytical 2D Inverse Kinematics Engine
// Links: Thigh l1 = 5.0cm, Shin l2 = 5.7cm, Max Extended Height = 10.7cm
// =============================================================================
IKResult solveIK(float x, float z) {
  IKResult res;

  // Protect against non-positive z or extreme singular values
  if (z < 1.0f) z = 1.0f;
  float z2 = sqrtf(x * x + z * z);

  // Clamp reach to avoid domain error with acos() near physical singularity
  const float maxReach = (LINK_THIGH_L1_CM + LINK_SHIN_L2_CM) - 0.002f;
  const float minReach = fabsf(LINK_SHIN_L2_CM - LINK_THIGH_L1_CM) + 0.002f;
  z2 = constrain(z2, minReach, maxReach);

  float hipRad2 = atan2f(x, z);
  float hipDeg2 = hipRad2 * (180.0f / (float)M_PI);

  // Law of Cosines for Hip angle
  float val1 = constrain((sq(LINK_THIGH_L1_CM) + sq(z2) - sq(LINK_SHIN_L2_CM)) /
                         (2.0f * LINK_THIGH_L1_CM * z2), -1.0f, 1.0f);
  float hipRad1 = acosf(val1);
  float hipDeg1 = hipRad1 * (180.0f / (float)M_PI);

  // Law of Cosines for Knee angle
  float val2 = constrain((sq(LINK_THIGH_L1_CM) + sq(LINK_SHIN_L2_CM) - sq(z2)) /
                         (2.0f * LINK_THIGH_L1_CM * LINK_SHIN_L2_CM), -1.0f, 1.0f);
  float kneeRad = (float)M_PI - acosf(val2);
  float kneeDeg = kneeRad * (180.0f / (float)M_PI);

  // Law of Cosines for Ankle angle (maintains ground-parallel foot sole)
  float val3 = constrain((sq(LINK_SHIN_L2_CM) + sq(z2) - sq(LINK_THIGH_L1_CM)) /
                         (2.0f * LINK_SHIN_L2_CM * z2), -1.0f, 1.0f);
  float ankleRad = ((float)M_PI / 2.0f) + hipRad2 - acosf(val3);
  float ankleDeg = ankleRad * (180.0f / (float)M_PI);

  res.hipDeg   = hipDeg1 + hipDeg2;
  res.kneeDeg  = kneeDeg;
  res.ankleDeg = ankleDeg;
  return res;
}

// C2-Continuous Zero-Jerk Quintic Swing Lift
// Both velocity and acceleration are strictly zero at liftoff and touchdown.
static inline float calculateSwingClearance(float stepPhase, float sClear) {
  float u = constrain(stepPhase / (float)M_PI, 0.0f, 1.0f);
  float u1 = 1.0f - u;
  // Quintic polynomial bell curve: 64 * u^3 * (1 - u)^3 has peak = 1.0 at u = 0.5
  return sClear * (64.0f * (u * u * u) * (u1 * u1 * u1));
}

// =============================================================================
// 2. Coordinated Joint Actuation Layer (True 6-Joint Dynamic Inverse Kinematics)
// Drives ALL 6 joints (Hips, Knees, Feet) simultaneously:
//   - Right Leg: Hip = R_HIP_HOME + dHip,  Knee = R_KNEE_HOME + dKnee,  Foot = R_FOOT_HOME + dAnkle
//   - Left Leg:  Hip = L_HIP_HOME - dHip,  Knee = L_KNEE_HOME - dKnee,  Foot = L_FOOT_HOME - dAnkle
// =============================================================================
void updateServoPos(float dHip, float dKnee, float dAnkle, char leg, bool isSwing) {
  if (leg == 'l') {
    float homeHip  = getJointHomeAngle(JOINT_L_HIP);
    float homeKnee = getJointHomeAngle(JOINT_L_KNEE);
    float homeFoot = getJointHomeAngle(JOINT_L_FOOT);
    setServoAngle(CH_L_HIP,  homeHip  - dHip);
    setServoAngle(CH_L_KNEE, homeKnee - dKnee);
    setServoAngle(CH_L_FOOT, homeFoot - dAnkle);
  } else if (leg == 'r') {
    float homeHip  = getJointHomeAngle(JOINT_R_HIP);
    float homeKnee = getJointHomeAngle(JOINT_R_KNEE);
    float homeFoot = getJointHomeAngle(JOINT_R_FOOT);
    setServoAngle(CH_R_HIP,  homeHip  + dHip);
    setServoAngle(CH_R_KNEE, homeKnee + dKnee);
    setServoAngle(CH_R_FOOT, homeFoot + dAnkle);
  }
}

void updateAllJoints(float xr, float zr, float xl, float zl) {
  IKResult ikR = solveIK(xr, zr);
  IKResult ikL = solveIK(xl, zl);
  IKResult nom = solveIK(0.0f, stepHeight);

  // Closed-form joint deltas relative to calibrated neutral standing posture
  float dHipR   = ikR.hipDeg - nom.hipDeg;
  float dKneeR  = ikR.kneeDeg - nom.kneeDeg;
  float dAnkleR = ikR.ankleDeg - nom.ankleDeg;

  float dHipL   = ikL.hipDeg - nom.hipDeg;
  float dKneeL  = ikL.kneeDeg - nom.kneeDeg;
  float dAnkleL = ikL.ankleDeg - nom.ankleDeg;

  bool isSwingR = (zr < stepHeight - 0.03f);
  bool isSwingL = (zl < stepHeight - 0.03f);

  // Actuate all 6 joints simultaneously
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
// 3. Real-Time Locomotion Engine (Called at 50Hz exclusively by Core 1 Task)
// =============================================================================
void updateLocomotion(float dt) {
  if (!lockLocomotion(10)) return;

  // ---------------------------------------------------------------------------
  // A. Stand / Settling Mode
  // ---------------------------------------------------------------------------
  if (activeMode == GAIT_STAND) {
    if (currentGaitScale > 0.0f) {
      currentGaitScale = max(0.0f, currentGaitScale - 0.05f);
      walkTime += dt;
    } else {
      parkNeutralStand();
      weightLeft   = 250.0f;
      weightRight  = 250.0f;
      groundForceN = 4.905f;
      strncpy(contactStateBuf, "DUAL STANCE", sizeof(contactStateBuf) - 1);
      strncpy(activeGaitBuf, "stand", sizeof(activeGaitBuf) - 1);
      unlockLocomotion();
      return;
    }
  }

  // ---------------------------------------------------------------------------
  // B. Single Step Routine (Non-blocking, deterministic 1-stride execution)
  // ---------------------------------------------------------------------------
  // ---------------------------------------------------------------------------
  // B. Single Step Routine (Non-blocking, deterministic 1-stride execution)
  // Two monotonic phases: zero ground reversals, zero impact, zero scraping.
  // ---------------------------------------------------------------------------
  else if (activeMode == GAIT_SINGLE_STEP) {
    float period = 1.0f / walkFreq;
    routineTime += dt;
    float u = routineTime / period;

    if (u >= 1.0f) {
      // Completed full stride, return smoothly to neutral stand
      activeMode = GAIT_STAND;
      currentGaitScale = 0.0f;
      parkNeutralStand();
      weightLeft   = 250.0f;
      weightRight  = 250.0f;
      groundForceN = 4.905f;
      strncpy(contactStateBuf, "DUAL STANCE", sizeof(contactStateBuf) - 1);
      strncpy(activeGaitBuf, "stand", sizeof(activeGaitBuf) - 1);
      unlockLocomotion();
      return;
    }

    float sClear = constrain(stepClearance, 0.15f, 0.65f);
    float sLen   = constrain(stepLength, 0.40f, 1.20f);
    float xr = 0.0f, xl = 0.0f;
    float zr = stepHeight, zl = stepHeight;
    float liftL = 0.0f, liftR = 0.0f;
    const float forwardLean = 0.30f;

    if (u < 0.5f) {
      // Phase 1: Left leg swings forward (0 -> +sLen); Right leg stance pushes (0 -> -sLen)
      float s = u / 0.5f;
      float e = 6.0f * powf(s, 5) - 15.0f * powf(s, 4) + 10.0f * powf(s, 3);
      float s1 = 1.0f - s;
      liftL = sClear * (64.0f * (s * s * s) * (s1 * s1 * s1));
      xl = 0.0f + sLen * e - forwardLean;
      xr = 0.0f - sLen * e - forwardLean;
      zl = stepHeight - liftL;
      zr = stepHeight;
    } else {
      // Phase 2: Right leg swings forward (-sLen -> 0); Left leg stance pushes (+sLen -> 0)
      float s = (u - 0.5f) / 0.5f;
      float e = 6.0f * powf(s, 5) - 15.0f * powf(s, 4) + 10.0f * powf(s, 3);
      float s1 = 1.0f - s;
      liftR = sClear * (64.0f * (s * s * s) * (s1 * s1 * s1));
      xr = -sLen + sLen * e - forwardLean;
      xl =  sLen - sLen * e - forwardLean;
      zr = stepHeight - liftR;
      zl = stepHeight;
    }

    updateAllJoints(xr, zr, xl, zl);

    // Telemetry
    if (liftL > 0.03f) {
      weightRight = min(500.0f, ROBOT_TOTAL_WEIGHT_GRAMS * (0.85f + 0.15f * (liftL / sClear)));
      weightLeft  = max(0.0f, ROBOT_TOTAL_WEIGHT_GRAMS - weightRight);
      strncpy(contactStateBuf, "RIGHT SINGLE SUPPORT", sizeof(contactStateBuf) - 1);
    } else if (liftR > 0.03f) {
      weightLeft  = min(500.0f, ROBOT_TOTAL_WEIGHT_GRAMS * (0.85f + 0.15f * (liftR / sClear)));
      weightRight = max(0.0f, ROBOT_TOTAL_WEIGHT_GRAMS - weightLeft);
      strncpy(contactStateBuf, "LEFT SINGLE SUPPORT", sizeof(contactStateBuf) - 1);
    } else {
      weightLeft   = 250.0f;
      weightRight  = 250.0f;
      strncpy(contactStateBuf, "DOUBLE SUPPORT", sizeof(contactStateBuf) - 1);
    }
    groundForceN = (weightLeft + weightRight) * 0.00981f;
    strncpy(activeGaitBuf, "step", sizeof(activeGaitBuf) - 1);
    unlockLocomotion();
    return;
  }

  // ---------------------------------------------------------------------------
  // C. Joint Diagnostics Sweep (Non-blocking 50Hz safety sweep across all 6 joints)
  // ---------------------------------------------------------------------------
  else if (activeMode == GAIT_DIAGNOSTICS) {
    routineTime += dt;
    const float PHASE_DUR = 1.0f; // 1 second per joint
    int phaseIdx = static_cast<int>(routineTime / PHASE_DUR);

    if (phaseIdx >= 6) {
      activeMode = GAIT_STAND;
      parkNeutralStand();
      strncpy(activeGaitBuf, "stand", sizeof(activeGaitBuf) - 1);
      unlockLocomotion();
      return;
    }

    float u = fmodf(routineTime, PHASE_DUR) / PHASE_DUR;
    float sineSweep = sinf(u * (float)M_PI); // 0 -> 1 -> 0

    for (int i = 0; i < NUM_JOINTS; i++) {
      float target = getJointHomeAngle(static_cast<JointID>(i));
      if (i == 0 && phaseIdx == 0) target += 6.0f * sineSweep;
      else if (i == 1 && phaseIdx == 1) target += 8.0f * sineSweep;
      else if (i == 2 && phaseIdx == 2) target += 5.0f * sineSweep;
      else if (i == 3 && phaseIdx == 3) target -= 6.0f * sineSweep;
      else if (i == 4 && phaseIdx == 4) target -= 8.0f * sineSweep;
      else if (i == 5 && phaseIdx == 5) target -= 5.0f * sineSweep;
      setServoAngle(DEFAULT_JOINTS[i].channel, target);
    }

    strncpy(contactStateBuf, "DIAGNOSTIC TEST", sizeof(contactStateBuf) - 1);
    strncpy(activeGaitBuf, "diag", sizeof(activeGaitBuf) - 1);
    unlockLocomotion();
    return;
  }

  // ---------------------------------------------------------------------------
  // D. Stance Height Initialization Sweep (Non-blocking squat-to-stand)
  // ---------------------------------------------------------------------------
  else if (activeMode == GAIT_INITIALIZE) {
    routineTime += dt;
    const float initDur = 1.2f;
    float u = routineTime / initDur;

    if (u >= 1.0f) {
      activeMode = GAIT_STAND;
      parkNeutralStand();
      strncpy(activeGaitBuf, "stand", sizeof(activeGaitBuf) - 1);
      unlockLocomotion();
      return;
    }

    // Smooth quintic rise from safe squat (9.50cm) to calibrated stand (10.00cm)
    float smoothU = 6.0f * powf(u, 5) - 15.0f * powf(u, 4) + 10.0f * powf(u, 3);
    const float squatH = 9.50f;
    float curH = squatH + (stepHeight - squatH) * smoothU;
    updateAllJoints(0.0f, curH, 0.0f, curH);
    strncpy(activeGaitBuf, "initialize", sizeof(activeGaitBuf) - 1);
    unlockLocomotion();
    return;
  }

  // ---------------------------------------------------------------------------
  // E. Continuous Gaits (Walk, Backward, Turn Left/Right, March, Crouch)
  // ---------------------------------------------------------------------------
  if (currentGaitScale < 1.0f) {
    currentGaitScale = min(1.0f, currentGaitScale + 0.035f);
  }
  walkTime += dt;

  // Quintic smoothstep easing for anti-jerk acceleration
  float sScale = currentGaitScale * currentGaitScale * (3.0f - 2.0f * currentGaitScale);

  // Continuous Harmonic Phase (0 to 2*PI)
  float phase = fmodf(2.0f * (float)M_PI * walkFreq * walkTime, 2.0f * (float)M_PI);
  if (phase < 0.0f) phase += 2.0f * (float)M_PI;

  float cosP = cosf(phase);

  float sLength = constrain(stepLength, 0.40f, 1.20f) * sScale;
  float sClear  = constrain(stepClearance, 0.15f, 0.65f) * sScale;
  float sHeight = stepHeight;

  float xr = 0.0f;
  float xl = 0.0f;
  float zr = sHeight;
  float zl = sHeight;

  // Zero-impact C2 swing clearance profiles across entire swing half-cycle
  float liftL = (phase < (float)M_PI) ? calculateSwingClearance(phase, sClear) : 0.0f;
  float liftR = (phase >= (float)M_PI) ? calculateSwingClearance(phase - (float)M_PI, sClear) : 0.0f;

  if (activeMode == GAIT_BACKWARD) {
    xr = -cosP * sLength;
    xl =  cosP * sLength;
    zr = sHeight - liftR;
    zl = sHeight - liftL;
    strncpy(activeGaitBuf, "backward", sizeof(activeGaitBuf) - 1);
  } else if (activeMode == GAIT_MARCH) {
    xr = 0.0f;
    xl = 0.0f;
    zr = sHeight - liftR * 1.15f;
    zl = sHeight - liftL * 1.15f;
    strncpy(activeGaitBuf, "march", sizeof(activeGaitBuf) - 1);
  } else if (activeMode == GAIT_TURN_LEFT) {
    // Left turn: Right leg outer (full stride), Left leg inner (reduced stride, opposite phase)
    xr =  cosP * sLength;
    xl = -cosP * (sLength * 0.35f);
    zr = sHeight - liftR;
    zl = sHeight - liftL;
    strncpy(activeGaitBuf, "turn_left", sizeof(activeGaitBuf) - 1);
  } else if (activeMode == GAIT_TURN_RIGHT) {
    // Right turn: Right leg inner (reduced stride), Left leg outer (full stride, opposite phase)
    xr =  cosP * (sLength * 0.35f);
    xl = -cosP * sLength;
    zr = sHeight - liftR;
    zl = sHeight - liftL;
    strncpy(activeGaitBuf, "turn_right", sizeof(activeGaitBuf) - 1);
  } else if (activeMode == GAIT_CROUCH) {
    xr = 0.0f;
    xl = 0.0f;
    // Symmetrical squat depth: 0.65cm safe max to avoid hitting knee hardware brackets
    float squat = (0.5f - 0.5f * cosP) * 0.65f * sScale;
    zr = sHeight - squat;
    zl = sHeight - squat;
    strncpy(activeGaitBuf, "crouch", sizeof(activeGaitBuf) - 1);
  } else {
    // Default Continuous Forward Dynamic Walk
    // Dynamic forward lean (3.0mm) shifts CoM ahead of ankle axis,
    // harnessing gravity for positive forward ground propulsion without slip.
    const float forwardLean = 0.30f * sScale;
    xr =  cosP * sLength - forwardLean;
    xl = -cosP * sLength - forwardLean;
    zr = sHeight - liftR;
    zl = sHeight - liftL;
    strncpy(activeGaitBuf, "walk", sizeof(activeGaitBuf) - 1);
  }

  // Actuate all 6 joints simultaneously via closed-form Inverse Kinematics
  updateAllJoints(xr, zr, xl, zl);

  // Dynamic Ground Reaction Force & Support State Telemetry
  if (currentGaitScale > 0.10f) {
    if (liftL > 0.03f) {
      weightRight = min(500.0f, ROBOT_TOTAL_WEIGHT_GRAMS * (0.85f + 0.15f * (liftL / sClear)));
      weightLeft  = max(0.0f, ROBOT_TOTAL_WEIGHT_GRAMS - weightRight);
      strncpy(contactStateBuf, "RIGHT SINGLE SUPPORT", sizeof(contactStateBuf) - 1);
    } else if (liftR > 0.03f) {
      weightLeft  = min(500.0f, ROBOT_TOTAL_WEIGHT_GRAMS * (0.85f + 0.15f * (liftR / sClear)));
      weightRight = max(0.0f, ROBOT_TOTAL_WEIGHT_GRAMS - weightLeft);
      strncpy(contactStateBuf, "LEFT SINGLE SUPPORT", sizeof(contactStateBuf) - 1);
    } else {
      weightLeft   = 250.0f;
      weightRight  = 250.0f;
      strncpy(contactStateBuf, "DOUBLE SUPPORT", sizeof(contactStateBuf) - 1);
    }
  } else {
    weightLeft   = 250.0f;
    weightRight  = 250.0f;
    strncpy(contactStateBuf, "DUAL STANCE", sizeof(contactStateBuf) - 1);
  }
  groundForceN = (weightLeft + weightRight) * 0.00981f;

  unlockLocomotion();
}

// =============================================================================
// 4. Thread-Safe Lifecycle, Mode Selection & Telemetry Interface
// =============================================================================
void initKinematics() {
  if (locomotionMutex == NULL) {
    locomotionMutex = xSemaphoreCreateMutex();
  }
  activeMode       = GAIT_STAND;
  currentGaitScale = 0.0f;
  walkTime         = 0.0f;
  routineTime      = 0.0f;
  weightLeft       = 250.0f;
  weightRight      = 250.0f;
  groundForceN     = 4.905f;
  strncpy(contactStateBuf, "DUAL STANCE", sizeof(contactStateBuf) - 1);
  strncpy(activeGaitBuf, "stand", sizeof(activeGaitBuf) - 1);
}

void requestGaitMode(LocomotionMode mode) {
  if (lockLocomotion(20)) {
    if (activeMode == GAIT_STAND && mode != GAIT_STAND) {
      walkTime = 0.0f;
      currentGaitScale = 0.0f;
    }
    activeMode  = mode;
    routineTime = 0.0f;
    if (mode == GAIT_STAND) {
      currentGaitScale = 0.0f;
      parkNeutralStand();
      strncpy(activeGaitBuf, "stand", sizeof(activeGaitBuf) - 1);
      strncpy(contactStateBuf, "DUAL STANCE", sizeof(contactStateBuf) - 1);
    }
    unlockLocomotion();
  }
}

void startLocomotion(const String& gait) {
  String g = gait;
  g.trim();
  g.toLowerCase();

  if (g == "stand" || g == "stop" || g == "park") {
    requestGaitMode(GAIT_STAND);
  } else if (g == "step") {
    requestGaitMode(GAIT_SINGLE_STEP);
  } else if (g == "backward" || g == "back") {
    requestGaitMode(GAIT_BACKWARD);
  } else if (g == "turn_left" || g == "left") {
    requestGaitMode(GAIT_TURN_LEFT);
  } else if (g == "turn_right" || g == "right") {
    requestGaitMode(GAIT_TURN_RIGHT);
  } else if (g == "march") {
    requestGaitMode(GAIT_MARCH);
  } else if (g == "crouch" || g == "squat") {
    requestGaitMode(GAIT_CROUCH);
  } else if (g == "diag" || g == "test" || g == "test_joints") {
    requestGaitMode(GAIT_DIAGNOSTICS);
  } else if (g == "init" || g == "initialize" || g == "ref_init") {
    requestGaitMode(GAIT_INITIALIZE);
  } else {
    // Default forward walk
    requestGaitMode(GAIT_WALK);
  }
}

void stopLocomotion() {
  requestGaitMode(GAIT_STAND);
}

void setGaitParam(const String& param, float val) {
  if (lockLocomotion(20)) {
    if (param == "freq" || param == "speed") {
      walkFreq = constrain(val, 0.3f, 1.5f);
    } else if (param == "stride" || param == "length") {
      stepLength = constrain(val, 0.40f, 1.20f);
    } else if (param == "knee" || param == "clearance" || param == "lift") {
      stepClearance = constrain(val, 0.15f, 0.65f);
    } else if (param == "height" || param == "stance") {
      stepHeight = constrain(val, 9.40f, 10.10f);
    }
    unlockLocomotion();
  }
}

LocomotionTelemetry getLocomotionTelemetry() {
  LocomotionTelemetry t;
  t.weightLeftGrams    = 250.0f;
  t.weightRightGrams   = 250.0f;
  t.groundForceNewtons = 4.905f;
  strncpy(t.contactState, "DUAL STANCE", sizeof(t.contactState));
  strncpy(t.activeGait, "stand", sizeof(t.activeGait));
  t.isRunning          = false;

  if (lockLocomotion(15)) {
    t.weightLeftGrams    = weightLeft;
    t.weightRightGrams   = weightRight;
    t.groundForceNewtons = groundForceN;
    strncpy(t.contactState, contactStateBuf, sizeof(t.contactState) - 1);
    t.contactState[sizeof(t.contactState) - 1] = '\0';
    strncpy(t.activeGait, activeGaitBuf, sizeof(t.activeGait) - 1);
    t.activeGait[sizeof(t.activeGait) - 1] = '\0';
    t.isRunning          = (activeMode != GAIT_STAND);
    unlockLocomotion();
  }
  return t;
}

// =============================================================================
// 5. Asynchronous Routines Dispatched to Core 1 Task
// =============================================================================
void executeSingleStep() {
  requestGaitMode(GAIT_SINGLE_STEP);
}

void runJointDiagnostics() {
  requestGaitMode(GAIT_DIAGNOSTICS);
}

void takeStep(float sLength, int stepVelocity) {
  if (lockLocomotion(20)) {
    stepLength = constrain(sLength, 0.40f, 1.20f);
    if (stepVelocity > 0) {
      walkFreq = constrain(1000.0f / (float)(stepVelocity * 50), 0.3f, 1.5f);
    }
    unlockLocomotion();
  }
  requestGaitMode(GAIT_SINGLE_STEP);
}

void initialize() {
  requestGaitMode(GAIT_INITIALIZE);
}
