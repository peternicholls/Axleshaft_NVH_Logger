Polestar 2 NVH Logger — Stage 1: Optical sensor test — 0.3.0
=========================================================
Standalone bench sketch. No SD, GY-521, button or external LED required.
Use the narrow three-pin reflective module first. Module identity/pin order
is not confirmed: follow its printed VCC/GND/OUT labels, NOT physical order.

WIRING — switch power off before changing wires
Optical module VCC -> ESP32 verified 3V3
Optical module GND -> ESP32 GND
Optical module OUT (or digital DO) -> ESP32 GPIO26
Built-in LED -> no wire needed (LED_BUILTIN or GPIO2 fallback, active high)
GPIO33 -> reserved for a future external rotation LED; no connection required.
GPIO25 -> unused. SD/IMU/button pins are unused by this sketch.

Check the expansion cradle voltage-selection jumper. Confirm the rail really
is 3.3 V. Verify the module supports 3.3 V and its OUT cannot exceed 3.3 V before
connecting OUT to GPIO26. Never feed a 5 V output into the ESP32. The input
pull-up is to ESP32 logic voltage; it does not make a 5 V output safe.
If the module has AO and DO, use DO. If its output/power requirements are
unknown, verify the module first; do not guess from wire colour or pin order.

BUILD / UPLOAD — same approach as Stage0A_SD_Test and Stage0B_Recorder
1. Copy this whole folder and open Stage1_Optical_Test.ino in Arduino IDE.
2. Select ESP32 Dev Module for the conventional ESP32-WROOM board.
   Compile checked with Arduino-ESP32 3.3.12, Arduino CLI 1.5.1.
3. Select the actual ESP32 USB serial port and upload.
4. Open Serial Monitor at 115200 baud. Press RESET to restart counters.
CLI, from the parent folder:
arduino-cli compile --fqbn esp32:esp32:esp32 --warnings all Stage1_Optical_Test
arduino-cli upload --fqbn esp32:esp32:esp32 --port YOUR_PORT Stage1_Optical_Test

BENCH PROCEDURE
1. Start stationary. Observe level=0/1 with dark background and a white mark.
   Adjust sensitivity and distance if the module has an adjustment. The correct
   distance must be measured experimentally for the actual sensor.
2. Default selected edge is FALLING (high to low). Check that one mark passage
   produces one accepted count. Change OPTICAL_EDGE to RISING if appropriate.
   Do not use CHANGE: it normally counts entry and exit as two events.
3. Pass the mark manually 20 times; accepted count should increase by exactly
   20. Hold it stationary and confirm no extra counts. Repeat at several gaps,
   angles and lighting levels. Built-in LED lights briefly for accepted edges.
4. Progress to controlled rotation with ONE mark per revolution. Compare counts
   to independently known revolutions/RPM; record expected and observed counts,
   min/max intervals, rejected counts, sensing gap, surface and illumination.
5. Test the intended speed range and ensure no missing or additional pulses.
   No vehicle trial or final mounting acceptance is implied by this test.

DIAGNOSTICS / SETTINGS
Serial prints once per second: instantaneous output level, accepted/rejected
edges, latest accepted timestamp (microseconds since boot), last/min/max
accepted interval, instantaneous RPM, run-wide average RPM and stale flag.
RPM assumes one accepted edge per revolution. First edge has no interval.
After 2 s without an edge rpm shows zero/stale=1; extremely slow rotation may
therefore appear stale between pulses. Average RPM is across accepted event
intervals over this boot, excludes the initial wait, and retains pauses.
No per-event log is persisted; Serial snapshots do not capture every event.

MIN_PULSE_INTERVAL_US defaults to 1000 us: provisional noise rejection, allowing
up to 60000 accepted edges/minute mathematically, NOT a verified speed rating.
Rejected edges do not move the last accepted timestamp. Choose a threshold
shorter than 60000000 / highest intended RPM, with margin for genuine variation.
Filtering cannot guarantee one pulse per revolution; prove it on the bench.
Interrupt captures time and updates counters; LED/Serial run separately.
At high speed LED flashes can merge. Configure INDICATOR_PIN and
INDICATOR_ACTIVE_LOW if your board's programmable LED differs. A fixed power
LED cannot be controlled.

ACCEPTANCE / LIMITS
Require one accepted index per actual revolution across the documented range,
with no unexplained missing/extra pulses. Save the Serial log and test setup.
Firmware compilation and host filter tests are not physical optical acceptance.
Stage 0B's remaining acceptance checks are still open. This stage proceeds
independently by user instruction; it does not integrate with recording yet.
