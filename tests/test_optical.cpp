#include "../Stage1_Optical_Test/PulseCounter.h"
#include <cassert>
int main() {
  DetectionCycle gate;
  gate.change(true, 0); // boot covered must not count
  assert(!gate.poll(1000,1000));
  gate.change(false,2000); assert(!gate.poll(3000,1000));
  gate.change(true,4000); assert(!gate.poll(4999,1000));
  assert(gate.poll(5000,1000)); assert(!gate.poll(6000,1000));
  // Trailing-edge bounce must neither rearm nor flash.
  gate.change(false,7000); assert(!gate.poll(7500,1000));
  gate.change(true,7600); assert(!gate.poll(8600,1000));
  gate.change(false,9000); assert(!gate.poll(10000,1000));
  gate.change(true,11000); assert(gate.poll(12000,1000));
  PulseCounter p;
  assert(p.edge(0,1000));
  assert(!p.edge(999,1000));
  assert(p.lastUs==0 && p.rejected==1);
  assert(p.edge(1000,1000));
  assert(!p.edge(1500,1000));
  assert(p.edge(3000,1000));
  assert(p.accepted==3 && p.rejected==2 && p.intervalUs==2000);
  assert(p.minUs==1000 && p.maxUs==2000 && p.firstUs==0);
  assert(p.edge(100000000000ULL,1000));
  assert(p.lastUs==100000000000ULL);
}
