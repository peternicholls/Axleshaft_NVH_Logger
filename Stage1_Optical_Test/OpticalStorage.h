#pragma once
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
namespace optical_storage {
// Timestamp label is prepared by the storage owner from the real UTC clock,
// or explicitly UNSET boot/uptime identity. Suffix avoids same-second collisions.
inline bool eventFilename(const char *label, unsigned suffix, char *out, size_t size) {
  if (suffix < 1 || suffix > 9999) return false;
  int count = snprintf(out, size, "/OPT_%s_%04u.BIN", label, suffix);
  return count > 0 && size_t(count) < size;
}
inline uint32_t crcUpdate(uint32_t crc, const uint8_t *bytes, size_t count) {
  for (size_t i = 0; i < count; ++i) {
    crc ^= bytes[i];
    for (unsigned bit = 0; bit < 8; ++bit)
      crc = (crc >> 1) ^ ((crc & 1) ? 0xedb88320U : 0);
  }
  return crc;
}

inline void encodeEvent(uint8_t *out, uint64_t sequence, uint64_t elapsedUs) {
  for (unsigned i=0; i<8; ++i) {
    out[i] = uint8_t(sequence >> (8*i));
    out[8+i] = uint8_t(elapsedUs >> (8*i));
  }
}
} // namespace optical_storage
