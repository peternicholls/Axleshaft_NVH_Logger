#pragma once
#include <stddef.h>
#include <stdint.h>
namespace recording_checks {
inline uint64_t eligible(uint64_t start, uint64_t cutoff) {
  return cutoff <= start ? 0 : (cutoff - start - 1) / 1000 + 1;
}
// A stop request can service the one current synthetic slot before quiescing.
// Earlier slots remain missed; never generate historical catch-up samples.
inline uint64_t stopSlot(uint64_t start, uint64_t cutoff) {
  return eligible(start, cutoff) - 1;  // caller requires eligible > 0
}
inline uint32_t crcUpdate(uint32_t crc, const uint8_t *bytes, size_t count) {
  for (size_t i = 0; i < count; ++i) {
    crc ^= bytes[i];
    for (unsigned bit = 0; bit < 8; ++bit)
      crc = (crc >> 1) ^ ((crc & 1) ? 0xedb88320U : 0);
  }
  return crc;
}
}
