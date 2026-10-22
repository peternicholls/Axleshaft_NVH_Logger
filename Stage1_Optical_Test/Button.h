#pragma once
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

namespace button_test {
constexpr uint32_t DEBOUNCE_MS = 30;

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

} // namespace button_test
