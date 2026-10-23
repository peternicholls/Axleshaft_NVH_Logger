#pragma once
#include <Arduino.h>
#include <Wire.h>
#include "Imu6050.h"
// Adapter also accepts a fake Wire port for host regression tests.
template<class Port> class WireBus {
 public:
  explicit WireBus(Port &port) : port_(port) {}
  imu6050::Status write(uint8_t address, uint8_t reg, uint8_t value) {
    port_.beginTransmission(address);
    if (port_.write(reg) != 1 || port_.write(value) != 1) {
      port_.endTransmission(true); return imu6050::Status::TransmitFailed;
    }
    return transmitStatus(port_.endTransmission(true));
  }
  imu6050::Status read(uint8_t address, uint8_t reg, uint8_t *out, size_t count) {
    if (!out || count == 0 || count > 14) return imu6050::Status::InvalidArgument;
    port_.beginTransmission(address);
    if (port_.write(reg) != 1) {
      port_.endTransmission(true); return imu6050::Status::TransmitFailed;
    }
    auto status = transmitStatus(port_.endTransmission(false));
    if (status != imu6050::Status::Ok) return status;
    size_t received = port_.requestFrom(address,count,true);
    // Short read may include timeout: Wire exposes no separate receive error here.
    if (received != count) {
      while (port_.available()) port_.read();
      return imu6050::Status::ShortRead;
    }
    uint8_t scratch[14];
    for (size_t i=0; i<count; ++i) {
      int value = port_.read();
      if (value < 0) return imu6050::Status::ShortRead;
      scratch[i] = uint8_t(value);
    }
    for (size_t i=0; i<count; ++i) out[i] = scratch[i];
    return imu6050::Status::Ok;
  }
  void waitMs(uint32_t ms) { delay(ms); }
 private:
  Port &port_;
  static imu6050::Status transmitStatus(uint8_t result) {
    if (result == 0) return imu6050::Status::Ok;
    if (result == 5) return imu6050::Status::Timeout;
    return imu6050::Status::TransmitFailed;
  }
};
