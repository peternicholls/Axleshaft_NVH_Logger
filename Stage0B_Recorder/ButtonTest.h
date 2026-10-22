#pragma once
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

namespace button_test {
constexpr uint32_t DEBOUNCE_MS = 30;
constexpr unsigned MAX_RUN = 9999;

// A stable release is required at boot and between presses.
class Button {
 public:
  void requireRelease() { armed_ = false; initialised_ = false; }
  bool update(bool down, uint32_t now) {
    if (!initialised_) {
      initialised_ = true;
      raw_ = down;
      changed_ = now;
      return false;
    }
    if (down != raw_) {
      raw_ = down;
      changed_ = now;
    }
    if (uint32_t(now - changed_) < DEBOUNCE_MS) return false;
    if (!down) {
      armed_ = true;
      return false;
    }
    if (!armed_) return false;
    armed_ = false;
    return true;
  }
 private:
  bool initialised_ = false;
  bool raw_ = false;
  bool armed_ = false;
  uint32_t changed_ = 0;
};

// Strict 8.3 recording names only. Ignore unrelated files.
inline unsigned runNumber(const char *name) {
  if (*name == '/') ++name;
  if (name[0] != 'R' || name[1] != 'U' || name[2] != 'N') return 0;
  unsigned number = 0;
  for (unsigned i = 3; i < 7; ++i) {
    if (name[i] < '0' || name[i] > '9') return 0;
    number = number * 10 + unsigned(name[i] - '0');
  }
  if (name[7] != '.' || name[8] != 'B' || name[9] != 'I' ||
      name[10] != 'N' || name[11] != '\0') return 0;
  return number;
}

// Increment the highest existing recording; never fill an earlier gap.
// Allocate only after the caller has checked the path does not exist.
inline bool nextName(unsigned highest, char *out, size_t size) {
  if (highest >= MAX_RUN || size < 13) return false;
  snprintf(out, size, "/RUN%04u.BIN", highest + 1);
  return true;
}
}  // namespace button_test
