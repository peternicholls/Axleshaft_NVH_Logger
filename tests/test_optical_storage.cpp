#include "../Stage1_Optical_Test/OpticalStorage.h"
#include <cassert>
#include <string>
int main() {
  using namespace optical_storage;
  char name[96];
  assert(eventFilename("2026-10-22_14-30-00",1,name,sizeof(name)));
  assert(std::string(name)=="OPT_2026-10-22_14-30-00_0001.BIN");
  assert(eventFilename("2026-10-22_14-30-00",2,name,sizeof(name)));
  assert(std::string(name)=="OPT_2026-10-22_14-30-00_0002.BIN");
  assert(eventFilename("UNSET_aabbccdd_UP15",1,name,sizeof(name)));
  assert(!eventFilename("test",0,name,sizeof(name)));
  assert(!eventFilename("test",10000,name,sizeof(name)));
  assert(!eventFilename("test",1,name,5));
  uint8_t record[16]; encodeEvent(record,1,0x0102030405060708ULL);
  assert(record[0]==1 && record[1]==0 && record[8]==8 && record[15]==1);
  const uint8_t golden[]={'1','2','3','4','5','6','7','8','9'};
  assert((crcUpdate(0xffffffffU,golden,9)^0xffffffffU)==0xcbf43926U);
  assert(crcUpdate(crcUpdate(0xffffffffU,golden,3),golden+3,6)==crcUpdate(0xffffffffU,golden,9));
}
