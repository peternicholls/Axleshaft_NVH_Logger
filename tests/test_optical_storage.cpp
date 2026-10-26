#include "../Stage1_Optical_Test/OpticalStorage.h"
#include <cassert>
#include <string>
#include <vector>
int main() {
  using namespace optical_storage;
  char name[96];
  assert(eventFilename("2026-10-22_14-30-00",1,name,sizeof(name)));
  assert(std::string(name)=="/OPT_2026-10-22_14-30-00_0001.BIN");
  assert(eventFilename("2026-10-22_14-30-00",2,name,sizeof(name)));
  assert(std::string(name)=="/OPT_2026-10-22_14-30-00_0002.BIN");
  assert(eventFilename(nullptr,3,name,sizeof(name)));
  assert(std::string(name)=="/OPT_0003.BIN");
  assert(eventFilename("",10000,name,sizeof(name)));
  assert(std::string(name)=="/OPT_10000.BIN");
  assert(eventFilename(nullptr,UINT32_MAX,name,sizeof(name)));
  assert(std::string(name)=="/OPT_4294967295.BIN");
  assert(!eventFilename("test",0,name,sizeof(name)));

  assert(!eventFilename("test",1,name,5));
  // Timed and untimed starts share one durable, monotonic device count.
  uint32_t last = 0, reserved = 99, persisted = 0;
  std::vector<std::string> names;
  for (unsigned run=1; run<=7; ++run) {
    const char *label = run == 1 || run == 4 || run == 5 ? "2026-10-26_23-00-00" : nullptr;
    assert(reserveRunIndex(last,reserved,[&](uint32_t next) { persisted=next; return true; }));
    assert(last==run && reserved==run && persisted==run);
    assert(eventFilename(label,reserved,name,sizeof(name)));
    names.emplace_back(name);
    last=persisted; // Simulated restart restores the durable reservation.
  }
  assert(names[0]=="/OPT_2026-10-26_23-00-00_0001.BIN");
  assert(names[1]=="/OPT_0002.BIN" && names[2]=="/OPT_0003.BIN");
  assert(names[3]=="/OPT_2026-10-26_23-00-00_0004.BIN");
  assert(names[4]=="/OPT_2026-10-26_23-00-00_0005.BIN");
  assert(names[5]=="/OPT_0006.BIN" && names[6]=="/OPT_0007.BIN");
  uint32_t before=last, previousReserved=reserved;
  assert(!reserveRunIndex(last,reserved,[](uint32_t) { return false; }));
  assert(last==before && reserved==previousReserved);
  // An existing filename consumes its reserved index before retrying, as does
  // a power cut/file-open failure after a successful NVS reservation.
  assert(reserveRunIndex(last,reserved,[&](uint32_t next) { persisted=next; return true; }));
  assert(reserved==8);
  last=persisted;
  assert(reserveRunIndex(last,reserved,[&](uint32_t next) { persisted=next; return true; }));
  assert(reserved==9);
  last=UINT32_MAX-1;
  assert(reserveRunIndex(last,reserved,[](uint32_t) { return true; }));
  assert(reserved==UINT32_MAX);
  bool called=false;
  assert(!reserveRunIndex(last,reserved,[&](uint32_t) { called=true; return true; }));
  assert(!called && last==UINT32_MAX && reserved==UINT32_MAX);
  ExistingRuns scan;
  assert(scan.observe("/OPT0007.BIN"));
  assert(scan.observe("OPT_2026-10-26_23-00-00_0012.BIN"));
  assert(scan.observe("OPT_UNSET_cd77abcc_UP19_0001.BIN"));
  assert(scan.observe("OPT_0003.bin"));
  assert(scan.observe("OTHER_9999.BIN") && scan.observe("OPT_9999.TXT"));
  assert(scan.observe("opt_0013.bin"));
  assert(scan.count==5 && scan.highestIndex==13 && scan.seed()==13);
  ExistingRuns legacy;
  for (unsigned i=0; i<10; ++i) assert(legacy.observe("OPT_UNSET_other_boot_0001.BIN"));
  assert(legacy.seed()==10); // Old per-session suffix 0001 must not reset count.
  assert(legacy.observe("OPT_legacy_name.BIN"));
  assert(legacy.seed()==11);
  assert(!legacy.observe("OPT_4294967296.BIN"));
  ExistingRuns bounds;
  assert(bounds.observe("OPT_4294967295.BIN"));
  assert(bounds.seed()==UINT32_MAX);
  bounds.count=UINT32_MAX;
  assert(bounds.observe("unrelated.bin"));
  assert(!bounds.observe("OPT0001.BIN"));
  uint8_t record[16]; encodeEvent(record,1,0x0102030405060708ULL);
  assert(record[0]==1 && record[1]==0 && record[8]==8 && record[15]==1);
  const uint8_t golden[]={'1','2','3','4','5','6','7','8','9'};
  assert((crcUpdate(0xffffffffU,golden,9)^0xffffffffU)==0xcbf43926U);
  assert(crcUpdate(crcUpdate(0xffffffffU,golden,3),golden+3,6)==crcUpdate(0xffffffffU,golden,9));
}
