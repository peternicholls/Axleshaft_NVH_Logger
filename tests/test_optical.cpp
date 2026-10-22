#include "../Stage1_Optical_Test/PulseCounter.h"
#include <cassert>
int main() {
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
