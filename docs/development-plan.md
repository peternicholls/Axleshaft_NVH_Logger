# Development plan

Use [progress.md](progress.md) for live status. Stage gates require recorded evidence, not just implementation completion.

## 0 — Foundation (0.1.0)

Preserve original files, initialise Git, establish documentation, versioning and ongoing progress records. Complete.

## 1 — Stage 0B: controls and sustained synthetic logging

Detailed execution sequence and measurable gates: [Stage 0B plan](stage0b-plan.md).

1. Confirm toolchain and button/LED wiring. Retain the Stage 0A SPI setup.
2. Specify a versioned binary format: metadata, deterministic six-axis records, sequence numbers, timing, session completion and errors. Define scheduling tolerance and count accounting before testing.
3. Implement debounced GPIO32 press toggling and GPIO25 idle/recording/flushing/error states; reserve GPIO33.
4. Separate sample production from SD writes with bounded RAM buffering. Start investigation at 8–16 KiB; measure occupancy and stalls. Avoid sampling catch-up that conceals missed deadlines.
5. Add persistent non-overwriting RUN0001.BIN numbering, controlled stop/drain/flush/close, capacity/write-error handling and concise serial metrics.
6. Implement an independent host validator and corruption/truncation/sequence test fixtures.
7. Compile, then run short bench sessions, restart/numbering, sustained recording, induced stalls, storage-full and error tests. Record sample counts, elapsed time, maximum latency, peak occupancy and all losses.

Exit: all Stage 0B acceptance criteria in the handover and test procedures have recorded results. Host tests alone cannot satisfy this gate.

## 2 — Standalone optical sensor (Stage 1)

After Stage 0B acceptance, verify electrical behaviour, transitions and index timestamps on GPIO26. Test one pulse per revolution across a documented speed range, reject noise without rejecting valid pulses, and drive GPIO33 without blocking. Record mounting, distance, target material, pulse intervals and missed/extra events.

Exit: reliable one-index-per-revolution bench evidence over the intended range.

## 3 — Standalone IMU (Stage 2)

May run before or after Stage 1, after Stage 0B acceptance. Verify identity and register settings, FIFO frame handling, data-ready capture and actual sample frequency. Preserve raw six-axis data. Measure continuity, noise, FIFO overflow and reset accounting while storage stalls occur.

Exit: continuous target-rate acquisition with explicit accounting for every loss and overrun.

## 4 — Synchronised integration (Stage 3)

Integrate independently accepted subsystems. Map IMU sample indices to the ESP32 monotonic clock using timing anchors and drift measurements. Persist configuration, optical timestamps, errors and start/stop boundaries. Extend format and validator with explicit version changes.

Exit: recoverable common timebase and validated RPM, phase and rotational-order analysis from combined bench recordings.

## 5 — Analysis and vehicle qualification

Deliver CSV conversion, traces, RMS/peak, FFT, spectrogram, RPM, order tracking and angular averaging. Validate with deterministic reference signals. Document calibration, secured mounting, power and environmental limitations before vehicle trials. Keep diagnostic conclusions separate from measurements.

Exit: repeatable combined measurements and documented limitations; release acceptance is an explicit later decision.

## Sequence override — 2026-10-22

User explicitly requested standalone Stage 1 next while Phase 0B acceptance remains incomplete. Stage1_Optical_Test now provides that bench sketch independently; it does not qualify Phase 0B or permit integration. Physical optical acceptance still requires one reliable index per known revolution over a measured speed range.
