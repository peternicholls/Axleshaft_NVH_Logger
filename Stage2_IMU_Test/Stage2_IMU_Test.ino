/* Polestar NVH Logger 0.5.0 — Stage 2 I1: GY-521 bring-up only.
   No FIFO/SD/optical integration. Code preparation only: read README.txt. */
#include <Arduino.h>
#include <Wire.h>
#include <esp_timer.h>
#include "WireBus.h"
constexpr uint8_t IMU_ADDRESS = 0x68; // AD0 tied GND; AD0 HIGH uses 0x69.
constexpr uint32_t I2C_HZ = 100000; // Bring-up only. FIFO later requires faster bus.
constexpr uint16_t I2C_TIMEOUT_MS = 10;
#ifdef LED_BUILTIN
constexpr uint8_t LED_PIN = LED_BUILTIN;
#else
constexpr uint8_t LED_PIN = 2;
#endif
constexpr bool LED_ACTIVE_LOW = false;
WireBus<TwoWire> bus(Wire);
imu6050::Imu6050<WireBus<TwoWire>> imu(bus, IMU_ADDRESS);
bool failed = true;
uint64_t snapshotCount = 0, freshWaits = 0;
uint32_t lastFreshMs = 0;
static void led(bool on) { digitalWrite(LED_PIN,(on != LED_ACTIVE_LOW) ? HIGH : LOW); }
static void fail(imu6050::Status status) {
  failed = true;
  Serial.printf("ERROR: %s; address=0x%02x WHO_AM_I=0x%02x last_reg=0x%02x. Reset after correcting wiring/config.\n",
    imu6050::statusName(status),IMU_ADDRESS,imu.identity(),imu.lastRegister());
  if (status == imu6050::Status::ConfigMismatch)
    Serial.printf("CONFIG_MISMATCH reg=0x%02x readback=0x%02x\n",imu.lastRegister(),imu.lastActual());
}
void setup() {
  Serial.begin(115200); pinMode(LED_PIN,OUTPUT); led(false);
  delay(1600);
  Serial.println("\n=== STAGE 2 GY-521 BRING-UP 0.5.0 ===");
  Serial.println("SDA21 SCL22; verified 3V3/GND; AD0=GND for 0x68. INT27 reserved, not sampled yet.");
  Serial.println("10 Hz diagnostic snapshots; NOT continuous 1 kHz acquisition. No SD files or optical capture.");
  if (!Wire.begin(21,22,I2C_HZ)) {
    Serial.println("ERROR: Wire.begin failed; reset to retry."); return;
  }
  Wire.setTimeOut(I2C_TIMEOUT_MS);
  auto result = imu.begin();
  if (result != imu6050::Status::Ok) { fail(result); return; }
  Serial.printf("Identity-compatible WHO_AM_I=0x%02x at address=0x%02x; configuration read-back PASS.\n",
    imu.identity(),IMU_ADDRESS);
  for (const auto &setting : imu6050::BRINGUP_SETTINGS)
    Serial.printf("CONFIG reg=0x%02x verified=0x%02x\n",setting.reg,setting.value);
  Serial.println("Raw register snapshots include temperature internally; CSV output is six axes only.");
  Serial.println("snapshot,read_start_us,read_end_us,ax,ay,az,gx,gy,gz,ax_g,ay_g,az_g,gx_dps,gy_dps,gz_dps");
  failed = false; lastFreshMs = millis(); led(true);
}
void loop() {
  if (failed) { led((millis()/125)%2 == 0); delay(1); return; }
  static uint32_t lastPoll = 0, lastCheck = 0;
  uint32_t now = millis();
  if (uint32_t(now-lastPoll) >= 100) {
    lastPoll = now;
    imu6050::Snapshot value{};
    uint64_t before = esp_timer_get_time();
    auto status = imu.snapshot(value);
    uint64_t after = esp_timer_get_time();
    if (status == imu6050::Status::NoFreshData) {
      ++freshWaits;
      if (uint32_t(millis()-lastFreshMs) >= 1000) fail(status);
    } else if (status != imu6050::Status::Ok) fail(status);
    else {
      lastFreshMs = millis(); ++snapshotCount;
      const auto &a = value.axes;
      Serial.printf("%llu,%llu,%llu,%d,%d,%d,%d,%d,%d,%.5f,%.5f,%.5f,%.4f,%.4f,%.4f\n",
        (unsigned long long)snapshotCount,(unsigned long long)before,(unsigned long long)after,
        a.ax,a.ay,a.az,a.gx,a.gy,a.gz,
        a.ax/imu6050::ACCEL_LSB_PER_G,a.ay/imu6050::ACCEL_LSB_PER_G,a.az/imu6050::ACCEL_LSB_PER_G,
        a.gx/imu6050::GYRO_LSB_PER_DPS,a.gy/imu6050::GYRO_LSB_PER_DPS,a.gz/imu6050::GYRO_LSB_PER_DPS);
    }
  }
  if (!failed && uint32_t(now-lastCheck) >= 5000) {
    lastCheck = now;
    auto status = imu.verifyConfiguration();
    if (status != imu6050::Status::Ok) fail(status);
    else Serial.printf("HEALTH snapshots=%llu fresh_waits=%llu config=PASS (sample continuity NOT measured)\n",
      (unsigned long long)snapshotCount,(unsigned long long)freshWaits);
  }
  delay(1);
}
