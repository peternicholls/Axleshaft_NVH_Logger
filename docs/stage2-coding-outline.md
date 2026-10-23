# Stage 2 GY-521 coding outline

Version 0.4.8 | 2026-10-23 | Design outline, not implemented firmware

## Scope and first coding slice

Begin with the GY-521/MPU-6050 alone. First prove communication, configuration and raw acceleration/gyro response. Then add FIFO acquisition, timing anchors and SD storage. Optical integration follows separately, using [NVH-TIME-01](experiments/timing-protocol.md); do not fold everything into the first sketch.

The first implementation deliverable is a compile-checked, independently copyable `Stage2_IMU_Test/` folder with a working bring-up sketch and README.txt, not empty stubs for the final logger. Implement only files actually needed by each slice. No upload or physical operation in this chat; no new dependencies.

## Intended folder and responsibilities

```text
Stage2_IMU_Test/
  Stage2_IMU_Test.ino    setup, controls, state transitions and diagnostics
  README.txt            exact wiring, build steps, operating/test procedure
  Imu6050.h             bounded I2C access, identity/config read-back, FIFO access
  ImuFrames.h           portable raw-frame decoding and frame/accounting types
  ImuAcquisition.h      FIFO service, epochs, anchor handling and error counters
  ImuStorage.h          record encoding, queue/storage ownership and integrity
  validate_imu.py       standalone host file/sequence/configuration validation
  imu_to_csv.py         raw sample/anchor export for later timing analysis
```

Add .cpp files if implementation size warrants them; do not introduce layers merely to fill this diagram. Keep tests in repository `tests/`, using the real portable helpers and format fixtures. Reuse established button, CRC and date/UNSET naming behaviour; package required local helpers in this folder so it works when copied. Do not make the sketch depend on sibling stage folders or change earlier working stages incidentally.

## Wiring and configuration contract

GY-521: verified compatible 3V3/GND; SDA21, SCL22, INT27. Fix/document AD0 and actual address. Verify module labels, I2C pull-up voltage and short wiring. No auxiliary I2C-device connection in the first slice. Button32 to GND with input pull-up; built-in LED (GPIO2 fallback, board-dependent). SD CS13/SCK18/MISO19/MOSI23 at 1 MHz, using the existing 3V3 supply arrangement. GPIO26 is unused until the combined optical bench; GPIO33 stays reserved.

Use installed Arduino Wire/ESP32 facilities; check their actual APIs/timeouts at implementation time. No sensor library dependency is required. Every bus operation must return explicit success/short-read/timeout status. Register configuration is verified against the [official register map](https://invensense.tdk.com/wp-content/uploads/2015/02/MPU-6000-Register-Map.pdf), then read back from the real device; a successful I2C ACK is not identity/configuration proof.

Proposed initial measurement configuration: raw six-axis, target 1000 Hz, ±8 g, ±500 degrees/s, and documented filter setting. Retain the exact range/filter/divider/clock registers in diagnostics and recordings. Final register constants and supported identity policy are part of slice I1 review, not guessed from breakout-board appearance. Temperature may be read for diagnostics but is excluded from a six-axis-only FIFO frame unless explicitly added to the format.

## Data path and ownership

```text
MPU hardware sample clock -> sensor FIFO -> acquisition owner -> bounded RAM queue
          |                      |                                |
     INT on GPIO27          whole-frame reads                 storage owner
          |                      |                                |
  ISR timestamp ring ------> index/anchor evidence -----------> SD file

button + Serial commands -> recorder state machine
LED + low-rate diagnostics <- consistent state/counter snapshots
```

- ISR: capture monotonic microseconds and notification count into a bounded ring, flag ring overflow, notify acquisition; no Wire, SD, allocation or Serial output.
- Acquisition owner: sole Wire/FIFO user. Read occupancy/status, drain complete frames promptly, decode raw values and enqueue bounded batches without waiting for SD. Record FIFO partial-frame handling, overflows, errors and reset epochs explicitly.
- Storage owner: sole filesystem user. Open a unique file, write configuration/header, consume queued records, measure write/flush/close latency, then close/read back and report checks. It must not hold the acquisition lock during I/O.
- Control/state owner: accept a debounced press once; command start/stop through a handshake. No direct competing sensor/SD calls from the button loop.
- Diagnostics: bounded snapshots and modest print rate, separate acquisition errors, timing uncertainty and storage-integrity results. A combined generic ERROR must not hide which check failed.

Start with a fixed 12 KiB sample/batch queue budget, reporting actual record capacity and OS/stack/anchor-buffer overhead separately. Capacity depends on the chosen encoded record, not just its 12-byte raw payload. Keep FIFO servicing independent of storage stalls. Measure queue high-water and maximum service gaps; no latency guarantee is inferred from previous SD observations.

## Portable types and interface sketch

These are design signatures, not promises of existing callable code:

```cpp
struct RawImuFrame { int16_t ax, ay, az, gx, gy, gz; };
struct SampleIdentity { uint32_t epoch; uint64_t index; };
struct DataReadyObservation { uint64_t ordinal, espUs; };

// Each operation returns a bounded, explicit status, never a plausible fake frame.
BusStatus readRegisters(uint8_t first, uint8_t* out, size_t bytes);
BusStatus configureAndReadBack(const ImuConfig& requested, ImuConfig& actual);
DecodeStatus decodeSixAxisFrame(const uint8_t* bytes, size_t count, RawImuFrame& out);
AcquisitionStatus serviceFifo();
void captureDataReadyTimestamp();  // ISR-safe only
void requestStart();
void requestStop();
```

Decode sensor-byte order explicitly; do not cast a byte buffer to a native struct or mix sensor order with little-endian file order. Use signed boundary/golden-frame tests. Define the complete FIFO frame from the enabled channels and confirm against actual register configuration. No partial or failed read may become an apparently valid sample.

## Implementation slices and tests

| Slice | Coding deliverable | Local proof | Separate physical evidence |
|---|---|---|---|
| I1: bring-up | Initialise Wire pins/timeouts, check actual address/identity, wake/configure/read back, low-rate coherent six-axis diagnostic snapshots | ESP32 compile; fake-bus failures, short reads, register mismatch, signed raw decode tests | Identity/register dump; stationary orientation changes and light tap response |
| I2: FIFO | Hardware sampling/FIFO setup, complete-frame draining, status/reset accounting and bounded acquisition owner | Golden multi-frame decoding; partial reads/frame boundaries, FIFO overrun and reset-epoch tests | Actual cadence, FIFO service gaps and stationary/tap continuity; no SD yet |
| I3: timing | Bounded data-ready ring, observation ordinals, start/epoch association and ambiguity flags | Ring overflow, ordinal gaps, missing/duplicate anchors, reset/start/stop boundary tests | Confirm INT behaviour and sample/anchor association; independent trace if available |
| I4: recording | Button state machine, bounded batch queue and dedicated SD owner; unique dated/UNSET files, clean drain/close/read-back | Format/CRC/count tests; mock open/short-write/reopen failure, collision and queue-pressure tests; ESP32 compile | Known-duration raw files, host validator, controls/LED, stalled writes and insufficient space |
| I5: inspection | Standalone validator/CSV and minimal raw/anchor report | Empty/short/long fixtures; corruption/truncation/config/version rejection; repeatable decoded fields | Verify actual recordings and compare noise/orientation/tap behaviour |

I1 diagnostic snapshots intentionally print slowly; they do not demonstrate continuous 1 kHz acquisition. Only I2 onward may claim tested FIFO acquisition, and only when its accounting evidence exists. GY-521 setup is the next code task, not motor integration or a full analysis UI.

## Recorder lifecycle

Boot: initialise subsystems, report real configuration or latched actionable error, then idle. Start: reserve unique file and write actual metadata, establish a fresh sensor/FIFO epoch with explicit boundary/interrupt acknowledgement rules, arm acquisition, then show recording. Discarded warm-up samples must be declared; do not reuse stale FIFO data as new session data.

Stop: capture the request time, command acquisition quiescence, establish the final complete-frame boundary and record remaining sensor/queue accounting. Drain all accepted complete frames, write completion/counters/checksum, flush/close/read back, then idle. A frame straddling stop or residual FIFO bytes need a documented policy and test; no silent truncation or synthetic catch-up.

On bus/FIFO/ring/queue failure: halt or start a new explicitly invalid/uncertain epoch according to the frozen policy. Never invent the number or values of lost sensor samples. On storage failure: stop acquisition, retain partial data and Serial diagnostics, latch error. If the footer cannot be written, host validation labels the file incomplete.

## Timing and format decisions to freeze before I3/I4

The MPU and ESP32 clocks are independent. Data-ready observation time is availability evidence, not unfiltered mechanical acquisition time. A FIFO read timestamp is a transport timestamp. Establish how a known FIFO sample index is associated with data-ready observations before fitting a common timeline; ISR notification counts are not automatically sensor sample counts. Overflow, coalesced interrupts and resets can make the association ambiguous. Persist uncertainty rather than assigning unsupported precise timestamps.

Freeze a simple, new versioned binary format with explicit byte order and lengths:

- Header: magic/version, actual sensor configuration, nominal rate, firmware/build identity, session start and timing epoch.
- Typed records: raw sample batches with first index/count, data-ready observations and explicit epoch/error records. Preserve channels as signed raw integers; do not store analysis-filtered replacements.
- Completion: accepted/persisted/invalid counters, stop boundary, CRC and clean/incomplete status. Define exact byte offsets, CRC coverage, per-record bounds and truncation handling in docs/data-format.md before writing the encoder.

This intentionally adds metadata for real sensor timing; it does not change existing headerless optical files. Naming/clock behaviour should match optical recording with an IMU prefix, a checked absolute SD path, collision suffix and clearly labelled UNSET fallback. UTC filenames never substitute for monotonic timing anchors.

## Readiness and scope

I1 can be implemented now under the user's GY-521-first sequence override. Continue through locally verifiable increments while leaving physical gates explicit; do not call device response verified from a compiled sketch. Before combined mark/tap experiments, obtain standalone raw acquisition and timing evidence, plus reliable rig optical detection. Follow the [next-phase plan](next-phase-plan.md) and [illustrated timing procedure](experiments/timing-protocol.md).

No exact filter-delay correction, phase alignment or absolute timing guarantee is embedded in acquisition. The analysis bench will estimate periods and inspect taps using the saved evidence. Motor, optical, wireless and vehicle integration stay out of the initial sensor bring-up slice.
