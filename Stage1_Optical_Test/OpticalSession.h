#pragma once
#include "PulseCounter.h"
struct OpticalSession {
  bool running = false;
  uint64_t startUs = 0, stopUs = 0, rawTransitions = 0;
  PulseCounter pulses;
  DetectionCycle cycle;
  void start(bool detected, uint64_t now) {
    pulses = PulseCounter(); cycle = DetectionCycle();
    rawTransitions = 0; startUs = now; stopUs = 0;
    cycle.change(detected, now); running = true;
  }
  void stop(uint64_t now) { running = false; stopUs = now; }
  void edge(bool detected, uint64_t now) {
    if (!running) return;
    cycle.change(detected, now); ++rawTransitions;
  }
  void qualify(uint64_t now, uint32_t stableUs, uint32_t minimumUs) {
    if (running && cycle.poll(now, stableUs)) pulses.edge(cycle.changedUs, minimumUs);
  }
  bool ledOn(uint64_t now) const {
    return running && !(pulses.accepted && now - pulses.lastUs < 30000);
  }
};
