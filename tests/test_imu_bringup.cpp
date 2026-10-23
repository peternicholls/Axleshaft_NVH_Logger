#include "../Stage2_IMU_Test/Imu6050.h"
#include "../Stage2_IMU_Test/WireBus.h"
#include <array>
#include <vector>
#include <cassert>
#include <iostream>
using imu6050::Status;
struct FakeBus {
  std::array<uint8_t,256> regs{};
  Status failure=Status::Ok;
  int failReg=-1, mismatchReg=-1;
  bool resetStuck=false;
  unsigned writes=0, waited=0;
  uint8_t address=0;
  FakeBus() { regs[0x75]=0x68; regs[0x3a]=1; }
  Status read(uint8_t addr,uint8_t reg,uint8_t *out,size_t count) {
    address=addr;
    if (reg==failReg) return failure;
    for (size_t i=0;i<count;++i) out[i]=uint8_t(reg+i)==mismatchReg ? uint8_t(regs[reg+i]^1) : regs[reg+i];
    return Status::Ok;
  }
  Status write(uint8_t addr,uint8_t reg,uint8_t value) {
    address=addr; ++writes;
    if (reg==failReg) return failure;
    regs[reg]=(reg==0x6b && value==0x80 && !resetStuck) ? 0x40 : value;
    return Status::Ok;
  }
  void waitMs(uint32_t ms) { waited+=ms; }
};
struct FakePort {
  int transmit=0, shortRequest=-1, shortByte=-1;
  bool writeFails=false, readFails=false, stopped=true;
  unsigned byteIndex=0;
  std::vector<uint8_t> data;
  void beginTransmission(uint8_t) {}
  size_t write(uint8_t) { return writeFails ? 0 : 1; }
  uint8_t endTransmission(bool stop) { stopped=stop; return transmit; }
  size_t requestFrom(uint8_t,size_t size,bool stop) {
    assert(!stopped); // register read must use repeated START
    stopped=stop; byteIndex=0;
    data.assign(size,0x5a);
    return shortRequest<0 ? size : size_t(shortRequest);
  }
  int available() { return int(data.size()-byteIndex); }
  int read() {
    if (byteIndex>=data.size()) return -1;
    if (readFails && int(byteIndex)==shortByte) { ++byteIndex; return -1; }
    return data[byteIndex++];
  }
};
int main() {
  const uint8_t snapshotBytes[]={0x80,0,0x7f,0xff,0xff,0xff,0x12,0x34,0,0,0,1,0xab,0xcd};
  imu6050::Snapshot snapshot{};
  assert(imu6050::decodeSnapshot(snapshotBytes,14,snapshot));
  assert(snapshot.axes.ax==-32768 && snapshot.axes.ay==32767 && snapshot.axes.az==-1);
  assert(snapshot.temperature==0x1234 && snapshot.axes.gx==0 && snapshot.axes.gy==1 && snapshot.axes.gz==-21555);
  auto original=snapshot;
  for (size_t bad : {0,12,13,15}) {
    assert(!imu6050::decodeSnapshot(snapshotBytes,bad,snapshot));
    assert(snapshot.axes.gz==original.axes.gz);
  }
  assert(!imu6050::decodeSnapshot(nullptr,14,snapshot));
  const uint8_t fifoBytes[]={0x80,0,0x7f,0xff,0xff,0xff,0,0,0,1,0xab,0xcd};
  imu6050::RawFrame frame{};
  assert(imu6050::decodeFifoFrame(fifoBytes,12,frame));
  assert(frame.gx==0 && frame.gy==1 && frame.gz==-21555);
  assert(!imu6050::decodeFifoFrame(snapshotBytes,14,frame));

  for (uint8_t address : {0x68,0x69}) {
    FakeBus bus; imu6050::Imu6050<FakeBus> imu(bus,address);
    assert(imu.begin()==Status::Ok && bus.address==address && imu.identity()==0x68);
    assert(bus.waited==200 && bus.regs[0x23]==0 && bus.regs[0x38]==0);
    assert(bus.regs[0x1a]==2 && bus.regs[0x1b]==8 && bus.regs[0x1c]==16);
    for (unsigned i=0;i<14;++i) bus.regs[0x3b+i]=snapshotBytes[i];
    assert(imu.snapshot(snapshot)==Status::Ok && snapshot.temperature==0x1234);
    bus.regs[0x3a]=0; snapshot.axes.ax=123;
    assert(imu.snapshot(snapshot)==Status::NoFreshData && snapshot.axes.ax==123);
    bus.regs[0x3a]=1; bus.failReg=0x3b; bus.failure=Status::ShortRead;
    assert(imu.snapshot(snapshot)==Status::ShortRead && snapshot.axes.ax==123);
    bus.failReg=-1; bus.regs[0x1c]=0;
    assert(imu.verifyConfiguration()==Status::ConfigMismatch);
    assert(imu.snapshot(snapshot)==Status::ConfigMismatch);
  }
  {
    FakeBus bus; imu6050::Imu6050<FakeBus> imu(bus,0x70);
    assert(imu.begin()==Status::InvalidArgument && bus.writes==0);
  }
  for (uint8_t who : {0,0x69,0x70,0xff}) {
    FakeBus bus; bus.regs[0x75]=who; imu6050::Imu6050<FakeBus> imu(bus,0x68);
    assert(imu.begin()==Status::IdentityMismatch && bus.writes==0);
  }
  {
    FakeBus bus; bus.resetStuck=true; imu6050::Imu6050<FakeBus> imu(bus,0x68);
    assert(imu.begin()==Status::ResetNotComplete);
  }
  for (Status failure : {Status::TransmitFailed,Status::Timeout,Status::ShortRead}) {
    FakeBus bus; bus.failReg=0x75; bus.failure=failure; imu6050::Imu6050<FakeBus> imu(bus,0x68);
    assert(imu.begin()==failure && bus.writes==0);
  }
  for (const auto &setting : imu6050::BRINGUP_SETTINGS) {
    FakeBus bus; bus.mismatchReg=setting.reg; imu6050::Imu6050<FakeBus> imu(bus,0x68);
    assert(imu.begin()==Status::ConfigMismatch);
    assert(imu.lastRegister()==setting.reg);
  }
  {
    FakeBus bus; bus.failReg=0x1a; bus.failure=Status::TransmitFailed;
    imu6050::Imu6050<FakeBus> imu(bus,0x68); assert(imu.begin()==Status::TransmitFailed);
  }
  FakePort port; WireBus<FakePort> adapter(port); uint8_t out[14];
  out[0]=9;
  assert(adapter.read(0x68,0x3b,out,14)==Status::Ok && out[0]==0x5a);
  assert(adapter.read(0x68,0x3b,out,15)==Status::InvalidArgument);
  assert(adapter.read(0x68,0x3b,nullptr,14)==Status::InvalidArgument);
  for (int code : {2,3,4,5}) {
    port.transmit=code; out[0]=9;
    assert(adapter.read(0x68,0x3b,out,14)==(code==5?Status::Timeout:Status::TransmitFailed));
    assert(out[0]==9);
  }
  port.transmit=0;
  for (int count : {0,5,13}) {
    port.shortRequest=count; out[0]=9;
    assert(adapter.read(0x68,0x3b,out,14)==Status::ShortRead && out[0]==9);
  }
  port.shortRequest=-1; port.readFails=true; port.shortByte=5; out[0]=9;
  assert(adapter.read(0x68,0x3b,out,14)==Status::ShortRead && out[0]==9);
  port.readFails=false; port.writeFails=true;
  assert(adapter.write(0x68,0x1a,2)==Status::TransmitFailed);
  std::cout << "PASS: register/FIFO layout, address/identity, configuration, fresh snapshots and bounded bus failure paths\n";
}
