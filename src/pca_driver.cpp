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

bool lockI2C(TickType_t timeoutMs) {
  if (i2cMutex == NULL) {
    i2cMutex = xSemaphoreCreateMutex();
  }
  if (i2cMutex) {
    return (xSemaphoreTake(i2cMutex, pdMS_TO_TICKS(timeoutMs)) == pdTRUE);
  }
  return false;
}

void unlockI2C() {
  if (i2cMutex) {
    xSemaphoreGive(i2cMutex);
  }
}

bool initPCA() {
  if (i2cMutex == NULL) {
    i2cMutex = xSemaphoreCreateMutex();
  }

  if (lockI2C(50)) {
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
      unlockI2C();
      return true;
    }
    pcaConnected = false;
    Serial.printf("[PCA9685] NOT found at 0x%02X on SDA=%d, SCL=%d. Will auto-retry.\n",
                  PCA9685_I2C_ADDR, PIN_I2C_SDA, PIN_I2C_SCL);
    unlockI2C();
    return false;
  }
  return false;
}

bool isPCAFound() {
  return pcaConnected;
}

bool checkPCAConnection() {
  if (pcaConnected) {
    if (lockI2C(20)) {
      Wire.beginTransmission(PCA9685_I2C_ADDR);
      if (Wire.endTransmission() != 0) {
        pcaConnected = false;
        Serial.println("[PCA9685] I2C heartbeat failed. Flagged for reconnection.");
      }
      unlockI2C();
    }
    return pcaConnected;
  }
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

bool getJointSnapshot(int index, JointState* out) {
  if (!out || index < 0 || index >= NUM_JOINTS) return false;
  if (lockI2C(15)) {
    *out = joints[index];
    unlockI2C();
    return true;
  }
  // Fallback direct copy if mutex timed out
  *out = joints[index];
  return true;
}

bool getJointSnapshotByID(JointID id, JointState* out) {
  return getJointSnapshot(static_cast<int>(id), out);
}

bool getAllJointSnapshots(JointState out[NUM_JOINTS]) {
  if (!out) return false;
  if (lockI2C(20)) {
    memcpy(out, joints, sizeof(JointState) * NUM_JOINTS);
    unlockI2C();
    return true;
  }
  memcpy(out, joints, sizeof(JointState) * NUM_JOINTS);
  return true;
}

float getJointHomeAngle(JointID id) {
  int idx = static_cast<int>(id);
  if (idx >= 0 && idx < NUM_JOINTS) {
    return joints[idx].homeAngle;
  }
  return 94.0f;
}

float getJointCurrentAngle(JointID id) {
  int idx = static_cast<int>(id);
  if (idx >= 0 && idx < NUM_JOINTS) {
    return joints[idx].currentAngle;
  }
  return 94.0f;
}

void setServoAngle(int channel, float angle) {
  if (!pcaConnected) {
    if (!initPCA()) return;
  }

  int idx = getJointIndex(channel);
  if (idx != -1) {
    angle = constrain(angle, joints[idx].minLimit, joints[idx].maxLimit);
  } else {
    angle = constrain(angle, 0.0f, 180.0f);
  }

  // Microseconds calculation: 544us to 2400us
  float us = SERVOMIN_US + (angle / 180.0f) * (SERVOMAX_US - SERVOMIN_US);
  // 12-bit PCA9685 tick count: 20ms period @ 50Hz (4096 ticks)
  int pulselen = static_cast<int>(roundf((us * 4096.0f) / 20000.0f));

  if (lockI2C(15)) {
    if (idx != -1) {
      joints[idx].currentAngle = angle;
      joints[idx].targetAngle  = angle;
    }
    // Only perform physical I2C write if pulse width has changed
    if (idx == -1 || joints[idx].lastPulseLen != pulselen) {
      if (idx != -1) joints[idx].lastPulseLen = pulselen;
      pwm.setPWM(channel, 0, pulselen);
    }
    unlockI2C();
  }
}

void setJointAngle(JointID id, float angle) {
  int idx = static_cast<int>(id);
  if (idx >= 0 && idx < NUM_JOINTS) {
    setServoAngle(joints[idx].channel, angle);
  }
}

void parkNeutralStand() {
  for (int i = 0; i < NUM_JOINTS; i++) {
    setServoAngle(joints[i].channel, joints[i].homeAngle);
  }
}

void scanI2CDevices(String& jsonOutput) {
  jsonOutput = "{\"devices\":[";
  int count = 0;
  for (byte addr = 1; addr < 127; addr++) {
    if (lockI2C(10)) {
      Wire.beginTransmission(addr);
      if (Wire.endTransmission() == 0) {
        if (count > 0) jsonOutput += ",";
        char hexBuf[10];
        snprintf(hexBuf, sizeof(hexBuf), "\"0x%02X\"", addr);
        jsonOutput += hexBuf;
        count++;
      }
      unlockI2C();
    }
  }
  jsonOutput += "],\"pca_found\":" + String(isPCAFound() ? "true" : "false") + "}";
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
