# Stage 2 design review and resolutions

2026-10-23 | Repository 0.5.0 | Review scope: GY-521 outline and first bring-up slice

The prior outline left several implementation choices open. The decisions below close those design gaps or define a mandatory later gate; they are not physical verification results. First-slice firmware lives in Stage2_IMU_Test and does not yet implement FIFO, interrupt anchors or SD acquisition.

| Issue / severity | Resolution and verification |
|---|---|
| High: temperature can be mistaken for gyro X | Separate 14-byte register snapshot and 12-byte six-axis FIFO decoders. Golden signed-field tests prove gyro begins after temperature only in the register layout. No native struct casts. |
| High: address confused with chip identity | Default AD0 to GND/address 0x68; configured 0x69 allowed for AD0 high. Expect WHO_AM_I=0x68 at either address. Other identities fail before configuration writes; this is register compatibility, not proof of authenticity. |
| High: unspecified first configuration | Bring-up freezes wake/PLL, all axes enabled, divider 0, filter configuration 2, accel ±8 g and gyro ±500 degrees/s. FIFO/DMP/I2C master and generated interrupts disabled. Every stable configuration register is read back; mismatch halts. |
| High: successful ACK/short read yields plausible values | Explicit transmit/timeout/short-read statuses; repeated-START register burst; scratch buffer commits only on complete successful reads. Failed/stale snapshots are never printed as samples. |
| High: 100 kHz is insufficient for future 12 KB/s FIFO | 100 kHz is diagnostic bring-up only. Payload alone needs 12*9*1000=108000 I2C bit clocks/s before transaction overhead. FIFO stage targets 400 kHz after bus verification, with measured utilisation/service margins. No blind extrapolation from slow snapshots. |
| High: default bus timeout consumes FIFO budget | Bring-up explicitly sets 10 ms Wire timeout. Future acquisition must budget total transaction/service time; no unlimited retries. Receive timeouts may appear as short read in this API and must not be falsely distinguished. |
| High: Wire capacity and frame boundaries undefined | Installed 3.3.12 header defaults to a 128-byte buffer. I1 caps reads to 14. Future FIFO reads must be a whole-frame multiple within the verified driver limit, initially at most 120 bytes; do not use the I1 read limit unchanged for FIFO. |
| High: stop request mistaken for exact physical sample cutoff | Future stop records request, FIFO-writes-disabled acknowledgement and final drain times separately; preserves all complete accepted frames through the effective FIFO stop boundary. No fabricated catch-up, silent tail discard or nominal stop-time sample count. Hardware transition ambiguity is explicitly marked. |
| High: ISR ordinals treated as FIFO sample identity | Raw interrupt observations cannot prove one-to-one association alone. Future index mapping requires controlled start/reset epochs and verified count/edge behaviour; ambiguous association fails timing acceptance even if bytes validate. |
| High: continuing after fault hides lost identity | First FIFO implementation policy is stop-and-latch on bus/FIFO/ring/queue fault. No automatic in-session recovery. Lost sample count remains unknown where unmeasurable; recovery/reset creates a new session/epoch. |
| High: overflow can consume final error record space | Future logger must reserve control/error/footer capacity outside the data queue; inability to write terminal metadata leaves host status incomplete. A checksum-only PASS never implies continuity/timing PASS. |
| Medium: bring-up appears to measure every sample | Firmware clearly labels ~10 Hz snapshots. Read-start/end times are transport bounds, not acquisition timestamps. FIFO and continuous rate are untested; a brief tap may fall between snapshots. |
| Medium: config drift and sensor stops unnoticed | Verify configuration initially and every 5 s; check DATA_RDY before snapshot, latch if no successful fresh snapshot for 1 s. Report counts as snapshots only, not hardware sample counts. |
| Medium: filter correction and scale conflated | Output raw six-axis values plus range-based uncalibrated unit conversion. No bias correction, clock-fit or filter-delay subtraction in bring-up. Save raw values for later qualification. |
| Medium: folder reuse breaks copyability | Local header files use existing Arduino/Wire facilities only. Fake-bus/port tests compile the real driver/adapter; standalone copied-folder build checked. No sibling-stage runtime dependencies. |

## Later implementation gates, not unresolved permission requests

Before I2: implement 400 kHz throughput/service measurements, count/status reading, whole-frame bursts, transient incomplete-frame policy and fail-closed FIFO overflow handling. Recheck counts after draining because new frames may arrive during a read. Poll on a bounded deadline as well as interrupts so a missing notification cannot indefinitely prevent FIFO service. Set an initial 5 ms service target; measure actual worst gap and treat any exceeded target as a diagnostic, not a promise that the 1024-byte FIFO cannot overflow.

Before I3: freeze INT polarity, pulse/latch/clear semantics and acknowledge/start race tests. Define bounded anchor-ring and batch-queue capacities with separate occupancy/loss counters. Do not hold one subsystem lock while blocking in another. Partition timing epochs on resets; reject precise timing claims for unknown first-sample association. A timestamp fit without a valid sample association does not resolve this.

Before I4: freeze the combined record layout, lengths, CRC coverage, completion semantics and maximum reader lengths in docs/data-format.md. Include independently checkable counts for decoded, queued, persisted and invalid/unknown segments, plus reserved terminal records. Use explicit STARTING/RECORDING/STOPPING/VERIFYING/IDLE/ERROR states; ignore extra presses during transitions. Measure final sensor-stop/drain duration rather than inferring it from a requested stop.

## Regression proof for I1

Host tests cover signed extrema, negative channels, temperature position, wrong lengths/null input, both bus addresses, unsupported identity, reset timeout, each register mismatch, configuration drift, no-fresh-data, failed reads, bounded adapter lengths, repeated START, NACK/timeout/short-write and partial reads without output mutation. Device I2C failure tests and physical orientation/tap observations still require the separate hardware workflow.

Sources: [manufacturer register map](https://invensense.tdk.com/wp-content/uploads/2015/02/MPU-6000-Register-Map.pdf) for identity/layout/ranges/settings; installed Arduino-ESP32 3.3.12 Wire.h/Wire.cpp for buffer, API and error behaviour. Direct manufacturer PDF retrieval currently redirects/fails in the browser tool; manufacturer-hosted indexed register sections were checked for WHO_AM_I and layout. Retain a versioned manufacturer document during implementation qualification.

## Independent read-only review

A separate verifier found no blocking I1 defect and reran the host tests successfully. Corrected the README compile-command working directory. Driver tests do not execute the sketch's clock-driven one-second freshness watchdog or five-second health scheduler; those remain explicit verification gaps in the standalone instructions. Neither the host driver tests nor the build establish physical watchdog, LED or sensor behaviour.
