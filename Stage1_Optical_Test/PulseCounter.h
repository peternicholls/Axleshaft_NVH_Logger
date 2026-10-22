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
