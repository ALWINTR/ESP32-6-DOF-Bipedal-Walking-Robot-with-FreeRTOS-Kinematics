#ifndef KINEMATICS_H
#define KINEMATICS_H

#include <Arduino.h>
#include "config.h"
#include "pca_driver.h"

// Locomotion Gait Modes
enum LocomotionMode {
  GAIT_STAND = 0,
  GAIT_WALK,
  GAIT_BACKWARD,
  GAIT_TURN_LEFT,
  GAIT_TURN_RIGHT,
  GAIT_MARCH,
  GAIT_CROUCH,
  GAIT_SINGLE_STEP,
  GAIT_DIAGNOSTICS,
  GAIT_INITIALIZE
};

// Thread-safe telemetry state for monitoring ground reaction force and contact status
struct LocomotionTelemetry {
  float weightLeftGrams;
  float weightRightGrams;
  float groundForceNewtons;
  char  contactState[32];
  char  activeGait[32];
  bool  isRunning;
};

// Analytical 2D Inverse Kinematics solution
struct IKResult {
  float hipDeg;
  float kneeDeg;
  float ankleDeg;
};

// =============================================================================
// Core Kinematics & Locomotion Lifecycle
// =============================================================================
void initKinematics();
void updateLocomotion(float dt);
void startLocomotion(const String& gait);
void requestGaitMode(LocomotionMode mode);
void stopLocomotion();
void setGaitParam(const String& param, float val);
LocomotionTelemetry getLocomotionTelemetry();

// =============================================================================
// Kinematic Solving & Coordinated Actuation
// =============================================================================
IKResult solveIK(float x, float z);
void updateAllJoints(float xr, float zr, float xl, float zl);
void pos(float x, float z, char leg);
void updateServoPos(float dHip, float dKnee, float dAnkle, char leg, bool isSwing = false);

// =============================================================================
// Asynchronous Trajectory Routines (Dispatched to Core 1 Locomotion Task)
// =============================================================================
void executeSingleStep();
void runJointDiagnostics();
void takeStep(float sLength, int stepVelocity);
void initialize();

#endif // KINEMATICS_H
