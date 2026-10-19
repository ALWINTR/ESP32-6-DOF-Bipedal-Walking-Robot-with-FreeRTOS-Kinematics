#ifndef PCA_DRIVER_H
#define PCA_DRIVER_H

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>
#include "config.h"

// Real-time joint state tracking
struct JointState {
  int channel;
  const char* name;
  float currentAngle;
  float targetAngle;
  float homeAngle;
  float minLimit;
  float maxLimit;
  int lastPulseLen;
};

// =============================================================================
// PCA9685 Driver Management & Status
// =============================================================================
bool initPCA();
bool isPCAFound();
bool checkPCAConnection();

// =============================================================================
// Servo Actuation & State Accessors
// =============================================================================
// Set angle by PCA9685 channel with safety limits and PWM pulse caching
void setServoAngle(int channel, float angle);

// Set angle by JointID enum
void setJointAngle(JointID id, float angle);

// Access joint states
JointState* getJoint(int index);
JointState* getJointByID(JointID id);
int getJointIndex(int channel);

// Park all 6 servos strictly in calibrated neutral standing posture
void parkNeutralStand();

// =============================================================================
// NVS Flash Calibration Offsets
// =============================================================================
void loadNVSJointOffsets();
void saveNVSJointOffsets();
void resetNVSJointOffsets();

#endif // PCA_DRIVER_H
