Polestar NVH Logger — Stage 2 I1: GY-521 bring-up — 0.5.0
========================================================
Purpose: verify address/identity/configuration and observe raw acceleration and
angular velocity. This is the FIRST slice only: no FIFO logging, continuous
1 kHz capture, SD recording, optical integration or timing synchronisation yet.
Code/build preparation in this chat; uploads/hardware tests run separately.

WIRING — disconnect power first; verify module labels and supply compatibility
GY-521 VCC -> verified ESP32 3V3 (subject to module verification)
GY-521 GND -> ESP32 GND
GY-521 SDA -> GPIO21
GY-521 SCL -> GPIO22
GY-521 AD0 -> GND for default address 0x68
GY-521 INT -> GPIO27 reserved for later data-ready capture (unused in I1)
GY-521 XDA/XCL -> not connected
Built-in programmable LED -> no external wire; LED_BUILTIN or GPIO2 fallback.
GPIO33 reserved; no external recording LED. Button32 is unused in I1.

AD0 high selects bus address 0x69: change IMU_ADDRESS in the sketch accordingly.
WHO_AM_I should still read 0x68 at either address. Other returned identities
are rejected, not silently treated as MPU-6050. Identity is a compatibility
check, not a guarantee the module is genuine. Confirm address/pull-ups if NACK.
Check cradle jumper and actual 3.3 V rail. ESP32 inputs must not receive 5 V.
Keep I2C short; confirm SDA/SCL pull-ups go to 3V3 on your actual breakout.
The SD may remain wired as before but is not used: CS13 SCK18 MISO19 MOSI23,
3V3/GND, SD 5V disconnected. No SPI/SD calls occur in this sketch.
Mount sensor rigidly for observations. Do not mount electronics on the shaft.

BUILD — same individual-folder convention as Stage 0A / 0B / 1
Copy this whole folder; open Stage2_IMU_Test.ino in Arduino IDE.
Use ESP32 Dev Module for the conventional ESP32-WROOM board.
Compile checked against Arduino-ESP32 3.3.12, Arduino CLI 1.5.1.
No extra sensor libraries or dependencies required.
From the repository root:
arduino-cli compile --fqbn esp32:esp32:esp32 --warnings all Stage2_IMU_Test
Use the separate upload workflow when ready. Serial Monitor: 115200 baud.
No upload or hardware operation is performed in this programming chat.

IMPLEMENTED CONFIGURATION (all stable registers read back before success)
PWR_MGMT_1 0x6B=0x01: awake, X gyro PLL reference
PWR_MGMT_2 0x6C=0x00: all measurement axes enabled
USER_CTRL  0x6A=0x00: FIFO, DMP and auxiliary I2C master disabled
FIFO_EN    0x23=0x00: no FIFO channels
INT_ENABLE 0x38=0x00: no generated data-ready interrupt
INT_PIN_CFG 0x37=0x00: active-high/push-pull pulse, status-read clear policy
SMPLRT_DIV 0x19=0x00, CONFIG 0x1A=0x02: nominal 1000 Hz hardware updates,
  filter setting 2 (acceleration ~94 Hz / gyro ~98 Hz); actual rate unmeasured
GYRO_CONFIG 0x1B=0x08: +/-500 degrees/s; 65.5 raw counts/(degree/s)
ACCEL_CONFIG 0x1C=0x10: +/-8 g; 4096 raw counts/g
Reset and setup settling each wait 100 ms; not a measured readiness guarantee.
I2C runs at 100 kHz with explicit 10 ms Wire timeout for bring-up only.
Future 12 KB/s FIFO acquisition will require faster verified I2C and budgeting.

OUTPUT AND STATES
Rapid LED flash = latched error; correct wiring/configuration then reset.
Steady LED on = configured diagnostic readout, NOT recording to SD.
Set LED_ACTIVE_LOW/pin if your actual built-in LED differs. A power LED is fixed.
After identity/configuration read-back PASS, CSV-like rows print ~10 times/s:
snapshot,read_start_us,read_end_us,ax,ay,az,gx,gy,gz,ax_g,ay_g,az_g,gx_dps,gy_dps,gz_dps
Timestamps bound the register READ, not sensor acquisition time. Rows are latest
coherent register snapshots and intentionally skip most hardware updates.
Snapshot index is not a hardware sample sequence. No sample-loss/rate claim.
Temperature is correctly skipped between acceleration and gyro fields; it is
not accidentally interpreted as gyro X. Raw values are signed 16-bit.
Scaled columns use configured full-scale conversion only; no bias/calibration,
gravity removal or filter-delay correction. HEALTH every 5 s rechecks config.
DATA_RDY status is checked before each burst; no-fresh-data is not printed as a
new reading. One second without any successful fresh snapshot latches error.
NACK, timeout, short read or configuration drift also latch error. Receive
failure may be reported as short read (Wire API does not distinguish all cases).

SEPARATE HARDWARE PROCEDURE / REQUIRED EVIDENCE
1. With motor off, power and capture full boot/configuration output. Require
   expected address/identity and actual read-back PASS, without errors.
2. Leave stationary for 30 s. Record orientation and raw axes. With a sensor
   axis vertical, its gravity component should be approximately +/-1 g and
   swap sign when inverted; other axes depend on the actual orientation.
   Gyro should be near stationary baseline, with real bias/noise, not exact zero.
3. Slowly tilt/invert about each axis; check the expected acceleration changes.
   Gently rotate and note gyro response; it measures angular velocity, not angle.
4. Lightly tap the stationary mounting bracket and save any observed response.
   ~10 Hz snapshots CAN MISS short impacts; absence of a displayed tap does not
   fail sensor response. FIFO acquisition is needed for impulse/timing testing.
5. Capture at least 60 s of diagnostic readout and HEALTH output. This checks
   basic communication/config stability only, not continuous 1 kHz sampling.
6. Record wiring photo, module labels/AD0, supply/pull-up checks, firmware/core,
   raw capture and axes/orientation. Save through the separate evidence workflow.
Do not claim continuous acquisition, calibrated axes or synchronisation from
this test. FIFO/anchors/SD are the next increments after bring-up evidence.

The 1-second freshness watchdog and 5-second health latching are implemented
but not exercised by the driver-only host tests. Verify their physical fault
response in the separate hardware workflow; do not disconnect powered wiring
as a routine fault-injection method. Controlled simulated faults are preferable.
