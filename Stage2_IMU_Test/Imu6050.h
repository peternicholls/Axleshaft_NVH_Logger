#pragma once
#include <stddef.h>
#include <stdint.h>
#include "ImuFrames.h"
namespace imu6050 {
enum class Status { Ok, InvalidArgument, TransmitFailed, Timeout, ShortRead,
                    IdentityMismatch, ResetNotComplete, ConfigMismatch, NoFreshData };
inline const char *statusName(Status status) {
  switch (status) {
    case Status::Ok: return "OK";
    case Status::InvalidArgument: return "invalid argument/address";
    case Status::TransmitFailed: return "I2C transmit failed/NACK";
    case Status::Timeout: return "I2C timeout";
    case Status::ShortRead: return "I2C short read";
    case Status::IdentityMismatch: return "unsupported WHO_AM_I (expected 0x68)";
    case Status::ResetNotComplete: return "reset did not complete";
    case Status::ConfigMismatch: return "register read-back mismatch";
    case Status::NoFreshData: return "no fresh DATA_RDY snapshot";
  }
  return "unknown";
}
struct RegisterSetting { uint8_t reg, value; };
// These settings intentionally leave FIFO and interrupt generation disabled.
constexpr RegisterSetting BRINGUP_SETTINGS[] = {
  {0x6b,0x01}, // Wake, X gyro PLL clock
  {0x6c,0x00}, // All axes enabled
  {0x6a,0x00}, // No FIFO/DMP/I2C master
  {0x23,0x00}, // No FIFO channels
  {0x38,0x00}, // No generated interrupts for the snapshot-only test
  {0x37,0x00}, // Active high, push-pull, pulse; status cleared by INT_STATUS read
  {0x19,0x00}, // Divider 0 with DLPF enabled: nominal 1000 Hz hardware update
  {0x1a,0x02}, // DLPF_CFG 2: accel 94 Hz / gyro 98 Hz
  {0x1b,0x08}, // Gyro +/-500 degrees/s
  {0x1c,0x10}, // Accel +/-8 g
};
constexpr size_t SETTING_COUNT = sizeof(BRINGUP_SETTINGS)/sizeof(BRINGUP_SETTINGS[0]);
constexpr float ACCEL_LSB_PER_G = 4096.0f;
constexpr float GYRO_LSB_PER_DPS = 65.5f;

// Bus contract: bounded read/write returning Status; waitMs is setup-only.
template<class Bus> class Imu6050 {
 public:
  Imu6050(Bus &bus, uint8_t address) : bus_(bus), address_(address) {}
  Status begin() {
    ready_ = false; identity_ = 0; lastActual_ = 0; lastRegister_ = 0x75;
    if (address_ != 0x68 && address_ != 0x69) return Status::InvalidArgument;
    Status result = bus_.read(address_,0x75,&identity_,1);
    if (result != Status::Ok) return result;
    // AD0 changes bus address, not WHO_AM_I. This is compatibility, not authenticity.
    if (identity_ != 0x68) return Status::IdentityMismatch;
    lastRegister_ = 0x6b;
    result = bus_.write(address_,0x6b,0x80);
    if (result != Status::Ok) return result;
    bus_.waitMs(100);
    uint8_t power = 0;
    result = bus_.read(address_,0x6b,&power,1);
    if (result != Status::Ok) return result;
    if (power & 0x80) return Status::ResetNotComplete;
    for (const auto &setting : BRINGUP_SETTINGS) {
      lastRegister_ = setting.reg;
      result = bus_.write(address_,setting.reg,setting.value);
      if (result != Status::Ok) return result;
    }
    bus_.waitMs(100); // Consistent setup settling pause, not a calibration claim.
    result = verifyConfiguration();
    ready_ = result == Status::Ok;
    return result;
  }
  Status verifyConfiguration() {
    for (const auto &setting : BRINGUP_SETTINGS) {
      lastRegister_ = setting.reg;
      Status result = bus_.read(address_,setting.reg,&lastActual_,1);
      if (result != Status::Ok) { ready_ = false; return result; }
      if (lastActual_ != setting.value) { ready_ = false; return Status::ConfigMismatch; }
    }
    return Status::Ok;
  }
  Status snapshot(Snapshot &out) {
    if (!ready_) return Status::ConfigMismatch;
    uint8_t interruptStatus = 0;
    lastRegister_ = 0x3a;
    Status result = bus_.read(address_,0x3a,&interruptStatus,1);
    if (result != Status::Ok) return result;
    if (!(interruptStatus & 1)) return Status::NoFreshData;
    uint8_t bytes[14];
    lastRegister_ = 0x3b;
    result = bus_.read(address_,0x3b,bytes,sizeof(bytes));
    if (result != Status::Ok) return result;
    return decodeSnapshot(bytes,sizeof(bytes),out) ? Status::Ok : Status::InvalidArgument;
  }
  uint8_t identity() const { return identity_; }
  uint8_t lastRegister() const { return lastRegister_; }
  uint8_t lastActual() const { return lastActual_; }
 private:
  Bus &bus_;
  uint8_t address_, identity_ = 0, lastRegister_ = 0x75, lastActual_ = 0;
  bool ready_ = false;
};
} // namespace imu6050
