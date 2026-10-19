#include "pca_driver.h"
#include <Preferences.h>

// PCA9685 Hardware Driver Instance
static Adafruit_PWMServoDriver pwm(PCA9685_I2C_ADDR);
static Preferences prefs;
static bool pcaConnected = false;
static JointState joints[NUM_JOINTS];
static SemaphoreHandle_t i2cMutex = NULL;

// NVS Persistent Storage Keys corresponding to JointID enum order
static const char* NVS_KEYS[NUM_JOINTS] = {
  "ch0",  // JOINT_R_HIP
  "ch1",  // JOINT_R_KNEE
  "ch12", // JOINT_R_FOOT
  "ch4",  // JOINT_L_HIP
  "ch8",  // JOINT_L_KNEE
  "ch15"  // JOINT_L_FOOT
};

bool initPCA() {
  if (i2cMutex == NULL) {
    i2cMutex = xSemaphoreCreateMutex();
  }

  if (i2cMutex && xSemaphoreTake(i2cMutex, pdMS_TO_TICKS(50)) == pdTRUE) {
    Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL);
    Wire.setClock(I2C_FREQUENCY_HZ);
    Wire.setTimeOut(50);

    Wire.beginTransmission(PCA9685_I2C_ADDR);
    if (Wire.endTransmission() == 0) {
      pwm.begin();
      pwm.setPWMFreq(PCA9685_PWM_FREQ_HZ);
      delay(20);
      pcaConnected = true;

      for (int i = 0; i < NUM_JOINTS; i++) {
        joints[i].lastPulseLen = -1;
      }
      Serial.printf("[PCA9685] Detected and initialized at 0x%02X (%dHz PWM, %dkHz I2C)\n",
                    PCA9685_I2C_ADDR, PCA9685_PWM_FREQ_HZ, I2C_FREQUENCY_HZ / 1000);
      xSemaphoreGive(i2cMutex);
      return true;
    }
    pcaConnected = false;
    Serial.printf("[PCA9685] NOT found at 0x%02X on SDA=%d, SCL=%d. Will auto-retry.\n",
                  PCA9685_I2C_ADDR, PIN_I2C_SDA, PIN_I2C_SCL);
    xSemaphoreGive(i2cMutex);
    return false;
  }
  return false;
}

bool isPCAFound() {
  return pcaConnected;
}

bool checkPCAConnection() {
  if (pcaConnected) return true;
  return initPCA();
}

int getJointIndex(int channel) {
  for (int i = 0; i < NUM_JOINTS; i++) {
    if (joints[i].channel == channel) return i;
  }
  return -1;
}

JointState* getJoint(int index) {
  if (index >= 0 && index < NUM_JOINTS) {
    return &joints[index];
  }
  return nullptr;
}

JointState* getJointByID(JointID id) {
  return getJoint(static_cast<int>(id));
}

void setServoAngle(int channel, float angle) {
  if (!pcaConnected) {
    if (!initPCA()) return;
  }

  int idx = getJointIndex(channel);
  if (idx != -1) {
    // Bracket safety clamping
    angle = constrain(angle, joints[idx].minLimit, joints[idx].maxLimit);
    joints[idx].currentAngle = angle;
  } else {
    angle = constrain(angle, 0.0f, 180.0f);
  }

  // Convert angle (0-180°) to pulse microseconds (SERVOMIN_US to SERVOMAX_US)
  float us = SERVOMIN_US + (angle / 180.0f) * (SERVOMAX_US - SERVOMIN_US);
  // Convert microseconds to 12-bit PCA9685 tick count (20ms cycle @ 50Hz)
  int pulselen = static_cast<int>(round((us * 4096.0f) / 20000.0f));

  // Pulse caching: skip redundant I2C writes if angle hasn't changed
  if (idx != -1 && joints[idx].lastPulseLen == pulselen) {
    return;
  }

  if (i2cMutex && xSemaphoreTake(i2cMutex, pdMS_TO_TICKS(15)) == pdTRUE) {
    if (idx != -1) joints[idx].lastPulseLen = pulselen;
    pwm.setPWM(channel, 0, pulselen);
    xSemaphoreGive(i2cMutex);
  }
}

void setJointAngle(JointID id, float angle) {
  if (id >= 0 && id < NUM_JOINTS) {
    setServoAngle(joints[id].channel, angle);
  }
}

void parkNeutralStand() {
  for (int i = 0; i < NUM_JOINTS; i++) {
    joints[i].targetAngle = joints[i].homeAngle;
    setServoAngle(joints[i].channel, joints[i].homeAngle);
  }
}

void loadNVSJointOffsets() {
  // 1. Initialize joint structures with compiled factory baselines
  for (int i = 0; i < NUM_JOINTS; i++) {
    joints[i].channel      = DEFAULT_JOINTS[i].channel;
    joints[i].name         = DEFAULT_JOINTS[i].name;
    joints[i].homeAngle    = DEFAULT_JOINTS[i].homeAngle;
    joints[i].currentAngle = DEFAULT_JOINTS[i].homeAngle;
    joints[i].targetAngle  = DEFAULT_JOINTS[i].homeAngle;
    joints[i].minLimit     = DEFAULT_JOINTS[i].minLimit;
    joints[i].maxLimit     = DEFAULT_JOINTS[i].maxLimit;
    joints[i].lastPulseLen = -1;
  }

  // 2. Load stored calibrations from NVS flash if present
  prefs.begin("biped_chmap", true); // read-only
  if (prefs.isKey("ch0")) {
    for (int i = 0; i < NUM_JOINTS; i++) {
      float savedVal = prefs.getFloat(NVS_KEYS[i], DEFAULT_JOINTS[i].homeAngle);
      joints[i].homeAngle    = constrain(savedVal, joints[i].minLimit, joints[i].maxLimit);
      joints[i].currentAngle = joints[i].homeAngle;
      joints[i].targetAngle  = joints[i].homeAngle;
    }
    Serial.println("[NVS] Loaded saved neutral offsets from flash.");
  } else {
    Serial.println("[NVS] Using factory calibrated neutral baselines.");
  }
  prefs.end();
}

void saveNVSJointOffsets() {
  prefs.begin("biped_chmap", false); // read-write
  for (int i = 0; i < NUM_JOINTS; i++) {
    prefs.putFloat(NVS_KEYS[i], joints[i].currentAngle);
    joints[i].homeAngle = joints[i].currentAngle;
  }
  prefs.end();
  Serial.println("[NVS] Saved current joint angles as permanent home offsets.");
}

void resetNVSJointOffsets() {
  prefs.begin("biped_chmap", false);
  prefs.clear();
  prefs.end();

  for (int i = 0; i < NUM_JOINTS; i++) {
    joints[i].homeAngle    = DEFAULT_JOINTS[i].homeAngle;
    joints[i].currentAngle = DEFAULT_JOINTS[i].homeAngle;
    joints[i].targetAngle  = DEFAULT_JOINTS[i].homeAngle;
  }
  parkNeutralStand();
  Serial.println("[NVS] Restored factory default offsets.");
}
