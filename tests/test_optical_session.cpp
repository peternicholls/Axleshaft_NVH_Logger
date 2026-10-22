#include "../Stage1_Optical_Test/OpticalSession.h"
#include "../Stage1_Optical_Test/Button.h"
#include <cassert>
int main() {
  OpticalSession s;
  s.edge(true,100); s.qualify(2000,1000,1000);
  assert(!s.ledOn(2000) && s.pulses.accepted==0);
  s.start(false,3000); assert(s.ledOn(3000));
  s.qualify(4000,1000,1000); s.edge(true,5000); s.qualify(6000,1000,1000);
  assert(s.pulses.accepted==1 && !s.ledOn(6000));
  assert(s.ledOn(35000)); // off pulse ends at 30 ms from candidate timestamp
  s.edge(false,36000); s.qualify(37000,1000,1000);
  assert(s.pulses.accepted==1 && s.ledOn(37000)); // trailing edge does not flash
  s.stop(40000); s.edge(true,50000); s.qualify(51000,1000,1000);
  assert(!s.running && !s.ledOn(51000) && s.pulses.accepted==1 && s.rawTransitions==2);
  s.start(true,60000); s.qualify(61000,1000,1000);
  assert(s.pulses.accepted==0 && s.rawTransitions==0 && s.ledOn(61000));
  button_test::Button b;
  assert(!b.update(true,0)); assert(!b.update(true,1000));
  assert(!b.update(false,1001)); assert(!b.update(false,1031));
  assert(!b.update(true,1040)); assert(b.update(true,1070)); assert(!b.update(true,2000));
}
