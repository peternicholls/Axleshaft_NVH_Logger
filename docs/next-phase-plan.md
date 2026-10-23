# Next phase: GY-521 setup, then a marked-shaft synchronisation rig

Plan version 0.4.6, 2026-10-23. Supersedes the 0.4.5 analysis-first sequence at the user's direction. Planning only; no new firmware, uploads or physical qualification in this deliverable.

## Goal and sequence override

Set up the GY-521 first. Then use a small motor, rotating shaft with one white mark, IR detector and a repeatable mechanical click/tap once per revolution to compare optical events with the IMU response. The user explicitly advances standalone Stage 2 preparation/development despite outstanding Stage 0B checks. Keep those earlier checks open; the rig is a bench synchronisation experiment, not vehicle or release acceptance.

The analysis bench remains planned, initially providing only the decoding/plots needed for this experiment. Full UI, comparisons and wireless control are deferred. [Existing optical storage evidence](evidence/2026-10-22-stage1-sd-button-report.md) proves preservation of accepted events, not rotation accuracy or IMU timing.

## Work packages

| Step | Deliverable | Exit evidence |
|---|---|---|
| G1: GY-521 bring-up | Standalone `Stage2_IMU_Test/` with matching sketch, README.txt and all local helpers | Compiles here; separate workflow confirms actual module identity, register read-back, stationary/orientation response and tap response |
| G2: FIFO acquisition and storage | Raw six-axis sampling targeting 1000 Hz, data-ready timing anchors, sequence/counters, independent storage task and host validator in that folder | Separate 60-second acquisition has no unexplained missing samples, FIFO overflow, queue drops or resets; file integrity verified on host |
| G3: rig setup | Document shaft/mark, sensor gap, pot setting, tap arrangement, IMU orientation and mark-to-tap geometry | Independently counted revolutions produce one accepted optical event and one useful impulse per revolution; no unexplained false pulses/clipping |
| G4: combined timing test | New standalone `Stage3_Sync_Bench/` after G1–G3 checks, retaining independently usable earlier stages | Raw samples, optical timestamps and timing anchors can be mapped into one monotonic domain; no FIFO-read-time substitution |
| G5: analysis and repeatability | Small reproducible Python decoding/overlay/phase report, then expand the planned analysis bench | Optical intervals, acceleration response, phase, delay variation and drift can be inspected and independently reproduced |

Implement each bounded slice with local tests/build and updated VERSION/progress/changelog/development log. Uploads, wiring and captures remain in the separate hardware workflow. Combined testing starts only after standalone IMU acquisition is demonstrated and this rig's optical detection is checked. This does not declare all older Stage 0B gates passed.

## GY-521 wiring and initial setup

| GY-521 / peripheral | ESP32 |
|---|---|
| VCC | Verified 3V3, subject to actual module verification |
| GND | Common GND |
| SDA | GPIO21 |
| SCL | GPIO22 |
| INT / data ready | GPIO27 |
| AD0 | Define/document actual level and resulting I2C address; do not leave address ambiguous |
| Optical digital OUT | GPIO26 (only used for combined rig test) |
| Start/stop button | GPIO32 to GND, input pull-up |
| Indicator | Built-in programmable LED; GPIO2 fallback remains board-specific |
| SD | CS13, SCK18, MISO19, MOSI23, 1 MHz; verified 3V3/common ground, 5V disconnected |

Keep I2C short, confirm actual labels/pull-ups and voltage behaviour, and mount the IMU rigidly on the stationary structure receiving the tap. Do not place the ESP32 or IMU electronics on the rotating shaft. No assumptions about photo-identified module schematics.

Initial proposed ranges remain ±8 g / ±500 degrees/s, with raw signed 16-bit values and explicit scale metadata. Read identity/configuration back and record all register choices. Verify FIFO channel order, complete frames and overflow/reset handling against the [official MPU-6000/6050 register map](https://invensense.tdk.com/wp-content/uploads/2015/02/MPU-6000-Register-Map.pdf). Hardware output rate depends on sample divider and filter setting; acquisition code must prove its actual cadence rather than use delay(1).

Separate baseline gravity/orientation/noise checks from later calibration claims. A modest manual tap must appear on the expected acceleration axes without saturating; it need not produce strong gyro readings.

## Motor/shaft/mark/click arrangement

Use an available small motor or equivalent slow rotating setup first; no specific purchase is required by this plan. Secure the shaft and sensing bracket, keep cables clear, and begin with slow manual rotation before motor operation.

- Put one white reference mark on a stable dark shaft/disc surface. Fix the IR sensor position and pot setting so mark and background are distinguishable. Record active polarity, gap, mark width and speed-dependent dwell times.
- Use a small cam/tab or equivalent once-per-turn feature to actuate a light repeatable tap against the stationary IMU bracket. The audible click is useful feedback; the test signal is the mechanical acceleration impulse, since the GY-521 is not a microphone.
- Arrange the tap at the mark passage initially, and allow its angle to be changed deliberately for a later phase check. Document real alignment rather than assuming perfectly simultaneous events.
- Avoid uncontrolled multiple contacts/rebounds and clipping. Keep motor background vibration identifiable by recording motor-on/no-tap and tap-enabled baselines. Use the same IMU mount between runs.

First confirm 20 known complete revolutions manually, starting/ending clear. Require 20 accepted optical events and 20 distinguishable tap cycles in the complete capture. A tap can ring across several samples; do not count each ringing peak as another revolution. Physical contact repeatability must be inspected before interpreting sub-millisecond offsets.

## Timing model and what the rig can prove

Use ESP32 monotonic microseconds for optical candidate-edge times and data-ready anchors. Preserve the optical filter settings/qualification separately from the candidate timestamp. Preserve raw IMU samples, sample index, FIFO/reset epochs and actual timing anchors; FIFO read completion is not acquisition time. Estimate sample-clock period/drift and anchor residuals, with any ambiguous or lost segments explicitly marked.

The optical event is a phase reference, not an external clock locking the MPU-6050. At nominal 1000 Hz, samples are about 1 ms apart. Digital filtering adds response delay: the register-map table lists about 3 ms accelerometer delay for its 94 Hz bandwidth setting. Do not shift samples until a plot looks aligned or claim the tap peak should occur at zero delay. Save filter/range metadata and distinguish fixed delay, sample timing uncertainty, mechanical phase offset and contact jitter.

A repeatable impulse near each index lets us inspect phase consistency and drift. It cannot by itself separate every source of delay. To claim digital timing accuracy independently, corroborate data-ready/optical anchor timing with a suitable measured reference if available; otherwise state the combined measurement uncertainty.

## Combined record and analysis requirements

Define a new explicit versioned combined format before G4; do not mix the existing headerless optical and synthetic sample formats. Persist firmware/configuration, raw six-axis samples with indices, optical event sequence/timestamps, data-ready anchors, start/stop boundaries, FIFO resets/overflows, queue/storage errors and completion/checksum evidence. Use dated filenames with explicit UNSET fallback and retain host validation/provenance.

Initial analysis outputs:

1. Raw acceleration/gyro traces with optical-event lines on the recovered time axis.
2. Optical period/RPM and event-to-impulse lag per revolution, using a documented onset/peak rule; show ambiguous/missing impulses rather than forcing a match.
3. Per-revolution acceleration overlays aligned to the optical event, and mean waveform with variation.
4. Lag/jitter/drift summary, raw versus expected mark-to-tap phase and baseline/no-tap comparison.
5. Source hashes, configuration, timing model/fit, exclusions and software version, so results can be repeated.

Moving the tap angle by known Δtheta gives expected lag change Δtheta/360 times the revolution period under approximately steady rotation. This controlled change is more informative than assuming the initial impact is exactly simultaneous with the optical edge. Keep physical angle uncertainty and speed variation in that comparison.

## Acceptance plan and remaining uncertainty

- G1: identity and configuration read-back match; stationary orientation and manual tap data are plausible, with axes/ranges documented.
- G2: independently verify a 60-second raw acquisition and count/timing anchors, no unexplained losses, FIFO resets, overflows or clipping. Record actual rate and drift; set a numerical rate tolerance against the chosen reference before collection.
- G3: known-count manual 20-revolution test passes, then test three documented motor speeds against an independent revolution/RPM reference. Establish dwell time margin for the current 1 ms optical stable-level filter; never silently shorten it to achieve a result.
- G4: host validates complete combined records and recoverable timing across SD writes and stop. Test optical event count, IMU continuity and loss flags; stress tests must expose failures rather than conceal them.
- G5: acquire at least 100 complete revolutions at each of three steady speed settings, plus no-tap baseline; report event-to-impulse distributions and start-to-end drift. Repeat a known tap-angle change and compare measured phase change with geometry/reference uncertainty.
- Before the first motor dataset, record a numerical lag-repeatability target based on measured contact repeatability and sampling/filter limits. If no independent timing/angle reference exists, report observed repeatability only; do not label it absolute synchronisation accuracy.

Retain the older recorder 60-second, storage-full, stall, LED and reboot/name-preservation checks as open until executed and evidenced; [progress](progress.md) remains the status authority. No calibrated rotation, absolute clock synchronisation, or final NVH readiness is claimed by this plan.

## Experiment protocol — 0.4.7

Use [NVH-TIME-01](experiments/timing-protocol.md), with its [illustrated printable PDF](../output/pdf/NVH-TIME-01-experiment-protocol.pdf), as the detailed procedure for G1–G5 timing work. It defines component-delay references, Level A/B capabilities, electrical/optical/IMU/phase/SD tests, data requirements, uncertainty and worksheets. Implement the listed instrumentation before trying to execute unsupported measurements. Current optical logs cannot isolate all delays.

## Coding breakdown — 0.4.8

[Stage 2 coding outline](stage2-coding-outline.md) defines folder/file responsibilities, interfaces, hardware/ISR/FIFO/SD ownership, lifecycle, format decisions and five implementation slices. Begin with a working I2C identity/configuration/raw-response bring-up sketch (I1), then FIFO, anchors and storage; no empty final-logger skeletons or premature combined claims.
