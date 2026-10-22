#include "../Stage0B_Recorder/ButtonTest.h"
#include <cassert>
#include <string>
int main() {
  button_test::Button b;
  assert(!b.update(true,0)); assert(!b.update(true,2000));
  assert(!b.update(false,2001)); assert(!b.update(false,2031));
  assert(!b.update(true,2040)); assert(!b.update(false,2045));
  assert(!b.update(true,2050)); assert(!b.update(true,2079));
  assert(b.update(true,2080)); assert(!b.update(true,5000));
  b.requireRelease(); assert(!b.update(true,6000)); assert(!b.update(true,7000));
  assert(!b.update(false,7001)); assert(!b.update(false,7031));
  assert(!b.update(true,7040)); assert(b.update(true,7070));
  char name[13]; assert(button_test::nextName(7,name,13));
  assert(std::string(name)=="/RUN0008.BIN");
  assert(!button_test::nextName(9999,name,13));
  assert(button_test::runNumber("/RUN0007.BIN")==7);
  for (auto s : {"", "R", "RUN", "RUN0001.BI", "RUN0001.BINx"}) assert(!button_test::runNumber(s));
}
