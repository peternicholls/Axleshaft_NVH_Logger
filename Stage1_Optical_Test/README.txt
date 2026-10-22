Polestar 2 NVH Logger — Stage 1 optical rotation test — 0.4.0
==========================================================
Standalone sketch for a rotating axle/substitute with ONE white reference mark.
Optical event SD logging; no IMU integration. Code preparation/build only in this chat;
uploading and physical capture are handled separately.

WIRING — disconnect power before changing wires
Optical module VCC -> verified ESP32 3V3
Optical module GND -> ESP32 GND
Optical module digital OUT / DO -> GPIO26
Button normally-open contact -> GPIO32; other contact -> GND
Built-in LED -> no wire (LED_BUILTIN, or GPIO2 fallback, active high)
SD GND -> GND; SD 3V3 -> verified 3V3; SD 5V -> NOT CONNECTED
SD CS -> GPIO13; SCK -> GPIO18; MISO -> GPIO19; MOSI -> GPIO23
SD SPI stays at the known-working 1 MHz. Use a compatible FAT card with free
space; never format or delete wanted recordings just to run this test.
GPIO33 -> reserved, no connection. GPIO25 -> unused.

Check actual module labels, not physical pin order. Confirm 3.3 V power and
output compatibility before connecting OUT. Do not feed 5 V into GPIO26.
Check the expansion cradle jumper/rail. Use DO, not AO, if both exist.
GPIO32 uses INPUT_PULLUP. A proposed external 4.7k pull-up to 3V3 can help
with the remote cable. Inspect the button module S/+/- connections first;
only use S and GND if its internal wiring supports the simple contact circuit.
Do not assume the third terminal needs connecting.

BUILD / SEPARATE UPLOAD WORKFLOW
Copy the entire folder, open Stage1_Optical_Test.ino in Arduino IDE, select
ESP32 Dev Module for this conventional ESP32 board and the actual USB port.
Compile checked with Arduino-ESP32 3.3.12 / Arduino CLI 1.5.1. No extra libraries.
From the parent folder:
arduino-cli compile --fqbn esp32:esp32:esp32 --warnings all Stage1_Optical_Test
When ready, use your separate upload workflow. Serial Monitor: 115200 baud.
No upload or hardware acceptance is performed by this code-preparation chat.

BUTTON AND LED
Boot idle: LED off; counts frozen. Release button for at least 30 ms.
First debounced press: open a new SD file, then START with reset counts, LED ON.
Opening and saving blink slowly; wait for SD ready before starting.
Release then press again: STOP, freeze counts, drain queue, close/read back.
LED turns OFF only after successful saving. Errors flash rapidly, latch until
reset, and preserve any partial file. Do not remove the powered card.
Holding the button does not repeat. Next start resets counters again: preserve
Serial output before starting a new run. Presses during opening/saving/error
are ignored. A held press cannot restart the test when saving ends.
During a test, each accepted detection briefly switches the LED OFF (~30 ms
from candidate timestamp), then ON again. Trailing clear does not flash.
At high speed off pulses may merge; rely on counts for accuracy.
Configure INDICATOR_PIN/INDICATOR_ACTIVE_LOW if the board LED differs.
A fixed power LED cannot be controlled.

WHITE-MARK AND SENSITIVITY PROCEDURE
1. Start with a stationary axle/substitute, dark background and one distinct
   white mark. Use a fixed sensor mount and note the measured sensing gap.
2. With counting stopped, rotate slowly between mark and background. Serial
   level is still displayed while idle. Adjust the sensitivity pot in small
   increments until the output reliably distinguishes them at the chosen gap.
   For default DETECTED_LEVEL=LOW: mark should read level=0, background level=1.
   If the actual output is opposite, set DETECTED_LEVEL=HIGH before compiling.
   Note the adjustment and gap; don't tune the pot while collecting a count test.
3. Begin with the mark clear. Press to start. If started over the mark, the
   first covered state is intentionally ignored until a stable clear rearms it.
4. Manually turn exactly 20 complete revolutions, starting and ending clear.
   Press to stop; require exactly 20 accepted detections, one off pulse per mark,
   and no extra event on the mark leaving. Save the START/STOP and snapshots.
5. Repeat at several controlled speeds and compare with an independent RPM or
   revolution reference. Record speed, mark width, gap, lighting and pot setting.
   Then hold stationary with mark present and absent; check no accumulating
   counts during each observation. Re-adjust only between test runs.

FILTER / TIMING
CHANGE interrupts capture both polarities and timestamp the candidate edge.
Detection and clear each require STABLE_LEVEL_US=1000 us before qualification;
a 250 us task timer checks stability. One stable detection counts, and stable
clear must occur before another can count. Brief chatter cannot rearm it.
MIN_PULSE_INTERVAL_US=1000 is an additional provisional acceptance threshold.
Both the mark and clear dwell must exceed the stable time plus scheduling
margin. A narrow mark at speed can be missed; measure pulse widths and test
the intended range before altering the filter. No speed range is qualified yet.
Counting/qualification is independent of the button/LED/Serial polling loop.

SERIAL
START follows successful file creation and resets a session. STOP is a request;
final SUMMARY after save prints duration_us, accepted, rejected and raw counts. Periodic snapshots show state (0 starting, 1 idle, 2 running, 3 saving, 4 error),
running, level, accepted/rejected,
raw transitions, last candidate timestamp since boot, interval/min/max in us,
instantaneous RPM, run-wide average RPM and stale state.
Raw transitions include both polarities and chatter, only while running.
Rejected counts are qualified detections rejected by the minimum-interval
filter, not every raw transition suppressed by stability qualification.
RPM assumes one detection per revolution. First event has no speed estimate;
after 2 s without an event, or while stopped, instantaneous RPM is zero/stale.
Average RPM uses first-to-last accepted event spacing and includes pauses.
A full event trace is not persisted; save Serial summaries in the upload/capture
workflow and keep expected physical revolution count with the report.

ACCEPTANCE / STATUS
Previous firmware 0.3.3 passed a known 20-cycle finger test, not rotating-shaft
qualification. The new button/off-pulse behaviour requires physical retesting.
Host tests/build cannot prove optical accuracy, LED behaviour or safe mounting.
Stage 0B remaining acceptance gates are still open; no combined logger implied.

SD FILE NAMES AND CLOCK
Files use OPT_YYYY-MM-DD_HH-MM-SS_0001.BIN in UTC. The suffix increments if a
path already exists for that second. Existing files are never overwritten.
The ESP32 does not know wall-clock time after power loss. While IDLE send:
TIME <Unix-seconds>
with newline in Serial Monitor. Generate the current command on your computer:
python3 -c "import time; print('TIME', int(time.time()))"
Paste its output promptly into Serial Monitor. This is host-supplied time, not
proof of clock synchronisation. TIME is rejected while testing/saving.
Without a valid clock filenames are explicitly labelled, for example:
OPT_UNSET_aabbccdd_UP15_0001.BIN
This contains a random boot ID, uptime seconds and collision suffix. It never
pretends to contain a real date. Set time again after reset/power-up. Renaming
on the computer is safe if you retain the Serial association. Timestamped names
use FAT long filenames; older Stage 0B RUN files remain untouched.

FILE FORMAT — optical-events-v1 (firmware 0.4.0)
No header/footer/padding. One 16-byte little-endian record per accepted event:
bytes 0..7: unsigned 64-bit sequence number, starting at 1
bytes 8..15: unsigned 64-bit candidate-edge microseconds since this test START
Timestamp is the raw candidate time later qualified as stable, not SD-write
time or wall time. Timestamp differences give rotation intervals. Only stable
accepted events are written, not raw chatter, clears or rejected detections.
No event is added after stop; an unqualified short final mark is not counted.
A session with no events is a valid empty file, not proof the sensor worked.

Storage uses a fixed 256-event queue (4096 bytes), a 512-byte write batch and
6144-byte task stack plus filesystem/OS overhead. Write queued events at full
batch or ~100 ms intervals; final partial batch has no padding. SD writes never
run in ISR or qualification callback. Queue overflow stops the test and fails
accounting; short writes, open or verification failures latch error.
Serial SUMMARY prints filename, duration, accepted/persisted counts, bytes,
raw/rejected counts, queue drops/peak, maximum write time, flush-close time,
and CRC32. CHECKS distinguishes write/readback/accounting; PASS requires all.
Flush/close return no success result; reopening checks file size and CRC32
against successfully submitted bytes. This is not power-loss atomic storage.

COMPUTER VERIFICATION — Python 3 standard library only
Preserve Serial SUMMARY/PASS. After saving completes, power down and copy the
file from SD to the computer. From this folder run, substituting actual values:
python3 validate_optical.py OPT_2026-10-22_14-30-00_0001.BIN --expected-events 20 --expected-crc ACTUAL_HEX_CRC_FROM_PASS
The script verifies exact count/size, sequence 1..N, strictly increasing event
timestamps and CRC32. A file alone has no clean-stop marker, date, duration or
configuration metadata: retain the Serial capture. A byte-perfect record of
extra/missed physical marks can pass storage validation; known revolutions
are still required for optical acceptance.

SD BENCH CHECKLIST (separate upload/capture workflow)
1. Confirm all SD/sensor/button wiring and free space; upload 0.4.0 separately.
2. Set UTC time, start clear, run exactly 20 complete marked revolutions, stop.
   Require accepted=persisted=20, bytes=320, queue_drops=0, all CHECKS PASS.
3. Copy the file and run the validator using the actual CRC from Serial.
4. Repeat short runs in the same second if practical and across reset; set
   time again and confirm unique names, retained older file hashes and counts.
5. Run at the planned rotation speeds; record gap, pot setting, independent
   revolution count/RPM, pulse widths, queue peak and worst observed latency.
6. Preserve wanted data; use disposable contents to test insufficient space.
   Require ERROR, frozen acquisition, partial file retained and no false PASS.
No hardware acceptance or upload is claimed here. SD latency, error handling
and pulse accuracy must be retested together on the actual setup.
