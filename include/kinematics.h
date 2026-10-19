#ifndef KINEMATICS_H
#define KINEMATICS_H

#include <Arduino.h>
#include "config.h"
#include "pca_driver.h"

// Telemetry state for monitoring ground reaction force and contact status
struct LocomotionTelemetry {
  float weightLeftGrams;
  float weightRightGrams;
  float groundForceNewtons;
  String contactState;
  String activeGait;
  bool isRunning;
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
// High-Level Trajectory Routines & Diagnostics
// =============================================================================
void executeSingleStep();
void runJointDiagnostics();
void takeStep(float sLength, int stepVelocity);
void initialize();

#endif // KINEMATICS_H
