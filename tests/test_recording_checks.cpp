#include "../Stage0B_Recorder/RecordingChecks.h"
#include <cassert>
int main() {
  using namespace recording_checks;
  assert(eligible(1000,0)==0 && eligible(1000,1000)==0);
  assert(eligible(1000,1001)==1 && eligible(1000,2000)==1);
  assert(eligible(1000,2001)==2);
  // Reported run: pending current slot can be generated once at stop.
  uint64_t count=eligible(1000,5995734);
  assert(count==5995 && stopSlot(1000,5995734)==5994);
  uint64_t next=5994, missed=0;
  uint64_t slot=stopSlot(1000,5995734);
  missed+=slot-next; next=slot+1;
  assert(missed==0 && next==count);
  // A true earlier gap is retained, rather than backfilled.
  next=5990; missed=slot-next;
  assert(missed==4);
  const uint8_t golden[]={'1','2','3','4','5','6','7','8','9'};
  uint32_t whole=crcUpdate(0xffffffffU,golden,9);
  assert((whole ^ 0xffffffffU)==0xcbf43926U);
  assert(crcUpdate(crcUpdate(0xffffffffU,golden,4),golden+4,5)==whole);
  uint8_t corrupt[]={'1','2','3','4','5','6','7','8','0'};
  assert(crcUpdate(0xffffffffU,corrupt,9)!=whole);
}
