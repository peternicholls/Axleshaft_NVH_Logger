Polestar 2 NVH Logger — Stage 1 optical rotation test — 0.3.8
==========================================================
Standalone sketch for a rotating axle/substitute with ONE white reference mark.
No SD recording or IMU integration. Code preparation/build only in this chat;
uploading and physical capture are handled separately.

WIRING — disconnect power before changing wires
Optical module VCC -> verified ESP32 3V3
Optical module GND -> ESP32 GND
Optical module digital OUT / DO -> GPIO26
Button normally-open contact -> GPIO32; other contact -> GND
Built-in LED -> no wire (LED_BUILTIN, or GPIO2 fallback, active high)
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
First debounced press: START, reset counts/statistics for a new test, LED ON.
Release then press again: STOP, freeze counts, print summary, LED OFF.
Holding the button does not repeat. Next start resets counters again: preserve
Serial output before starting a new run. No files are written by this sketch.
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
START resets a session. STOP prints duration_us, accepted, rejected and
raw_transitions. Periodic snapshots show running, level, accepted/rejected,
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
