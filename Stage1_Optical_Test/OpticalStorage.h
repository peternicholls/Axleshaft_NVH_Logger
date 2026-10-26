#pragma once
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
namespace optical_storage {
// One device run index is shared by timed and untimed recordings. The storage
// owner durably reserves an index before opening any file; gaps are permitted.
inline bool eventFilename(const char *label, uint32_t index, char *out, size_t size) {
  if (!index) return false;
  int count = label && *label ?
    snprintf(out, size, "/OPT_%s_%04" PRIu32 ".BIN", label, index) :
    snprintf(out, size, "/OPT_%04" PRIu32 ".BIN", index);
  return count > 0 && size_t(count) < size;
}

inline bool nextRunIndex(uint32_t last, uint32_t &next) {
  if (last == UINT32_MAX) return false;
  next = last + 1;
  return true;
}

// Persist succeeds before either in-memory index becomes usable for file open.
// A failed reservation cannot silently advance the caller's state.
template <typename Persist>
inline bool reserveRunIndex(uint32_t &last, uint32_t &reserved, Persist persist) {
  uint32_t next;
  if (!nextRunIndex(last, next) || !persist(next)) return false;
  last = next;
  reserved = next;
  return true;
}

struct ExistingRuns {
  uint32_t count = 0;
  uint32_t highestIndex = 0;
  // Count every OPT_*.BIN (including old UNSET names), plus OPT<number>.BIN.
  // Legacy suffixes often all equal 0001, so count is also a lower-bound seed.
  bool observe(const char *path) {
    const char *name = strrchr(path, '/');
    name = name ? name + 1 : path;
    size_t length = strlen(name);
    if (length < 8 || (name[0] != 'O' && name[0] != 'o') ||
        (name[1] != 'P' && name[1] != 'p') || (name[2] != 'T' && name[2] != 't') ||
        name[length-4] != '.' || (name[length-3] != 'B' && name[length-3] != 'b') ||
        (name[length-2] != 'I' && name[length-2] != 'i') ||
        (name[length-1] != 'N' && name[length-1] != 'n')) return true;
    const char *digits = nullptr;
    if (name[3] == '_') {
      digits = strrchr(name, '_') + 1;
    } else {
      digits = name + 3;
      for (const char *p = digits; p < name + length - 4; ++p)
        if (*p < '0' || *p > '9') return true;
    }
    if (count == UINT32_MAX) return false;
    ++count;
    if (digits == name + length - 4) return true;
    for (const char *p = digits; p < name + length - 4; ++p)
      if (*p < '0' || *p > '9') return true; // Relevant legacy name without index.
    uint32_t index = 0;
    for (const char *p = digits; p < name + length - 4; ++p) {
      uint32_t digit = uint32_t(*p - '0');
      if (index > (UINT32_MAX - digit) / 10) return false;
      index = index * 10 + digit;
    }
    if (index > highestIndex) highestIndex = index;
    return true;
  }
  uint32_t seed() const { return count > highestIndex ? count : highestIndex; }
};
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
