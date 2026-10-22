#pragma once
#include <stdint.h>
struct PulseCounter {
  uint64_t accepted = 0, rejected = 0, firstUs = 0, lastUs = 0;
  uint64_t intervalUs = 0, minUs = 0, maxUs = 0;
  // Exactly-at-limit pulses are valid. Rejections do not move the reference.
  bool edge(uint64_t now, uint32_t minimumUs) {
    if (accepted && now - lastUs < minimumUs) { ++rejected; return false; }
    if (accepted) {
      intervalUs = now - lastUs;
      if (!minUs || intervalUs < minUs) minUs = intervalUs;
      if (intervalUs > maxUs) maxUs = intervalUs;
    } else firstUs = now;
    lastUs = now; ++accepted; return true;
  }
};

// Qualify both levels: one event on stable detection, rearm only on stable clear.
struct DetectionCycle {
  bool active = false, armed = false, qualified = false;
  uint64_t changedUs = 0;
  void change(bool detected, uint64_t now) {
    active = detected; changedUs = now; qualified = false;
  }
  bool poll(uint64_t now, uint32_t stableUs) {
    if (qualified || now - changedUs < stableUs) return false;
    qualified = true;
    if (!active) { armed = true; return false; }
    if (!armed) return false;
    armed = false; return true;
  }
};
