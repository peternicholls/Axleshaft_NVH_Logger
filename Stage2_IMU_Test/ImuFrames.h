#pragma once
#include <stddef.h>
#include <stdint.h>
namespace imu6050 {
struct RawFrame { int16_t ax, ay, az, gx, gy, gz; };
struct Snapshot { RawFrame axes; int16_t temperature; };
inline int16_t signedBE(const uint8_t *bytes) {
  uint16_t raw = (uint16_t(bytes[0]) << 8) | bytes[1];
  return int16_t(raw < 32768 ? int32_t(raw) : int32_t(raw)-65536);
}
// Register burst includes temperature between acceleration and gyro.
inline bool decodeSnapshot(const uint8_t *bytes, size_t count, Snapshot &out) {
  if (!bytes || count != 14) return false;
  Snapshot value{{signedBE(bytes),signedBE(bytes+2),signedBE(bytes+4),
                  signedBE(bytes+8),signedBE(bytes+10),signedBE(bytes+12)}, signedBE(bytes+6)};
  out = value; return true;
}
// Future six-axis-only FIFO must use this decoder, not the register decoder.
inline bool decodeFifoFrame(const uint8_t *bytes, size_t count, RawFrame &out) {
  if (!bytes || count != 12) return false;
  RawFrame value{signedBE(bytes),signedBE(bytes+2),signedBE(bytes+4),
                 signedBE(bytes+6),signedBE(bytes+8),signedBE(bytes+10)};
  out = value; return true;
}
} // namespace imu6050
