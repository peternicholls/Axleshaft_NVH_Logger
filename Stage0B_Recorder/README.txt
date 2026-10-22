PHASE 0B — RECORDING CONTROLS AND CONTINUOUS SD LOGGING — 0.2.1

Copy this entire folder. Open Stage0B_Recorder.ino in Arduino IDE.
Board: ESP32 Dev Module (conventional ESP32-WROOM). Verified compilation with
Espressif esp32 package 3.3.12 and Arduino CLI 1.5.1; actual board unconfirmed.
No extra libraries. Serial Monitor: 115200 baud.
CLI: arduino-cli compile --fqbn esp32:esp32:esp32 Stage0B_Recorder
Upload through IDE after selecting the actual USB port, or:
arduino-cli upload --fqbn esp32:esp32:esp32 --port YOUR_PORT Stage0B_Recorder
Do not use Bluetooth/debug-console ports as a substitute for the ESP32 port.

WIRING (power off before changes)
SD: GND->GND, verified 3V3->3V3, 5V disconnected; CS13 SCK18 MISO19 MOSI23.
SPI remains 1 MHz. Check cradle jumper and actual module voltage behaviour.
Button: GPIO32 to normally-open contact, other contact GND. Internal pull-up
is enabled; proposed external 4.7k pull-up to 3V3 for the remote cable.
Inspect the three-pin module first: do not assume S/+/- internal wiring.
Recording indication uses the built-in programmable LED. No external LED or
GPIO25 wiring is required. Uses LED_BUILTIN when defined, otherwise GPIO2
for the generic ESP32 Dev Module target. Set RECORDING_LED_PIN and
RECORDING_LED_ACTIVE_LOW in the sketch if your board differs. A fixed power
LED cannot be controlled; the actual board LED mapping is not yet verified.
GPIO33 reserved, held LOW; no optical sensor or IMU needed.

OPERATION
Release button for at least 30 ms after boot. First debounced press starts;
release and press again to stop. Holding cannot repeat. Presses during saving
are ignored; release again before the next run. LED solid recording, slow
flashing starting/saving, off after successful close/read-back, rapid on error.
Errors latch until reset. There is no automatic duration/size stop.

Each run is highest existing RUN number plus one, e.g. RUN0001.BIN then
RUN0002.BIN, including across resets. Gaps are not reused. RUN9999 exhausts
names. Existing paths are checked before opening; never overwritten.

Samples: 1000 scheduled slots/sec, 6 signed 16-bit little-endian channels,
12 bytes/sample (12,000 bytes/sec). Channel c at slot n is
((n*(2*c+1)+997*c) mod 65536)-32768. No header/footer/padding is written.
A dedicated timer produces into a static 12 KiB queue (1024 records).
Storage uses 504-byte batches (42 whole samples), with a short final batch.
Task stack is 6144 bytes plus OS/SD overhead, separate from the queue budget.
Late slots are counted, never invented. Queue overflow stops acquisition;
any loss makes the final result an error. Button/LED polling runs independently
of SD writes. Timing under real SD load must still be measured.

On stop, producer cutoff freezes, queue drains, file flushes/closes, every byte
is read back against the sequential pattern. Serial prints duration_us,
generated/persisted samples, bytes, missed/dropped slots, max_write_us,
peak_records/peak_bytes and flush_close_us, followed by PASS or ERROR.
Flush/close return no success status; read-back checks size and contents.
Errors/short writes retain the partial file for diagnosis.

VALIDATE (Python 3, standard library only)
After PASS, preserve the Serial summary. Power down before removing the card.
Copy recording to the computer and use the exact PASS count:
python3 validate_recording.py RUN0001.BIN --expected-samples 60000
Use the actual count, not always 60000. A 60-second button press is approximate;
compare count to duration_us / 1000 (one boundary-slot tolerance), and expect
about 720,000 bytes. Zero missed/dropped counts are required.
No embedded completion marker exists: file alone cannot prove a clean stop.
The pattern repeats after 65536 samples; whole-period losses cannot be detected
from bytes alone. Serial count, duration, loss counters and PASS are essential.

BENCH ACCEPTANCE — NOT YET RUN
1. Check built-in LED operation and button wiring; hold at boot and confirm no recording starts.
2. Start, release, record ~60 s, stop. Keep Serial summary; require zero losses,
   count/duration agreement, full firmware read-back and host validator PASS.
3. Repeat several short recordings; hold button 2 s, verify one event per press.
   Reboot and confirm numbering continues and previous file hashes unchanged.
4. Preserve wanted files, then use disposable card contents to leave insufficient
   space for a run. Require short-write ERROR, acquisition stopped, partial file
   retained, reset required. Revalidate earlier recordings.
5. Observe saving LED, max write latency, queue peak and stop responsiveness.
The small SDSC should fit a 720 KB test if enough free space remains. Do not
format or delete wanted files just to test. No hardware acceptance is claimed
by a successful compile or host test. Stage 1/2 remain gated until this passes.
