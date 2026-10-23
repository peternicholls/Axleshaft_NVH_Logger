# Changelog

Versions describe repository deliverables, not hardware qualification. The imported Stage 0A firmware has no established firmware version.

## 0.5.2 — 2026-10-23

- Add required mechanical CAD and PCB workstreams to the wheel-balancer side project, with editable sources, manufacturing outputs, shared interfaces and fabrication/physical validation gates.
- Documentation only; no CAD/EDA tool selected, design files fabricated or hardware acceptance changed. Local links, version alignment and whitespace checked.

## 0.5.1 — 2026-10-23

- Organise a separate wheel-balancer concept: no SD logging, calibrated single/two-plane sensing, pictorial correction guidance and guarded automatic positioning.
- Define independent emergency-stop/guard requirements, unresolved mechanical inputs and staged validation. Concept documentation only; no firmware or hardware acceptance change.
- Local links, version alignment and whitespace checked.

## 0.5.0 — 2026-10-23

- Review and resolve GY-521 outline gaps: register/FIFO layout, bus address versus identity, I2C throughput/read limits, fault policy, stop accounting and timing-association gates.
- Add standalone Stage2_IMU_Test I1 code and wiring/test README: identity/configuration read-back, bounded I2C and fresh slow raw/scaled six-axis snapshots. No FIFO, SD or optical integration yet.
- Add actual driver/adapter host tests; ESP32 build without warnings, copied-folder build, sanitizers, Clang static analysis and Python regressions pass. Independent review found no blocker.
- No upload/hardware test; physical sensor behaviour and sketch-level watchdog/health scheduling remain unverified.

## 0.4.8 — 2026-10-23

- Outline GY-521 coding: standalone folder/modules, interfaces, ISR/FIFO/storage ownership, start/stop/error lifecycle and five implementation slices with tests.
- Prioritise working identity/configuration/raw-response bring-up, preserving later timing/format and hardware gates. No implementation, dependencies or firmware changes.
- Documentation links, version alignment and whitespace checked.

## 0.4.7 — 2026-10-23

- Add NVH-TIME-01 illustrated timing/response experiment protocol: rig, wiring and timeline diagrams, component-delay references, five test sequences, uncertainty and run worksheet.
- Include editable Markdown, reproducible vector/PDF builder and seven-page printable PDF; link from plans, tests and project index.
- Validate PDF content and visually inspect every rendered page, plus source syntax, numerical examples, links, version alignment and whitespace. No firmware/hardware changes or experiment results.

## 0.4.6 — 2026-10-23

- Reorder next work at user direction: standalone GY-521 setup/acquisition, then motor/white-mark/mechanical-tap synchronisation bench.
- Plan common-timebase anchors, mechanically coupled impulse tests, phase/delay/drift analysis and qualified acceptance without assuming simultaneous optical/acceleration peaks.
- Preserve unpassed recorder checks; defer full analysis UI and wireless. Documentation/source review, links, version alignment, plan copy and whitespace checked; no implementation or hardware operation.

## 0.4.5 — 2026-10-23

- Sequence next work: qualification run sheet, first offline optical analysis engine/UI and separate physical evidence collection, followed by acceptance review and conditional Stage 2 entry.
- Define software and hardware acceptance tests, initial file-size scope, exact stage wiring references and remaining risks.
- Planning only; documentation links, version alignment, plan-copy consistency and whitespace checked. No implementation or hardware operation.

## 0.4.4 — 2026-10-22

- Add analysis bench software to planning: offline recording validation, optical/RPM inspection, comparison and reproducible exports; later IMU, spectral and order analysis.
- Define fixture, UI, integrity and processing acceptance gates; reuse existing validators and preserve raw sources.
- Correct stale progress version/installed-firmware header. Documentation links, version alignment and whitespace checked; no software implementation or firmware changes.

## 0.4.3 — 2026-10-22

- Independently validate the actual optical SD recording: 11 sequential events, 176 bytes, monotonic timestamps and CRC d0aa5ebe matching capture.
- Retain the small binary as deliberate evidence; add its SHA-256, decoded events and agreement with Serial timing statistics to the report.
- Close host-validation gap for this recording only; no firmware changes or expanded physical acceptance.

## 0.4.2 — 2026-10-22

- Reconcile captured optical SD statistics and user clarification that informal finger passages explain the imprecise expected physical count.
- Record that binary remains on ESP32 SD; independent host validation pending computer copy. Added exact count/CRC validation command.
- Checked captured counts, size, firmware check outcomes, links and whitespace; firmware unchanged.

## 0.4.1 — 2026-10-22

### Fixed

- Prefix optical SD paths with '/' as required by ESP32 FS; regression reproduced the original create failure.

### Validation

- Compile, verified upload, host Python checks and C++ storage regression pass. UTC clock acknowledged; button START/STOP saved 11 events/176 bytes with zero queue drops and write/read-back/accounting PASS. User considers physical-count discrepancy potentially manual and defers count-edge cases; exact 10-cycle accuracy remains inconclusive. Physical LED confirmation and actual-file host validation remain pending.

## 0.3.9 — 2026-10-22

### Added

- Button-controlled Stage 1 v0.3.8 bench report and serial: START/STOP observed, 14.099 s, 12 detections, 88 raw transitions. Physical LED confirmation and SD writing remain unverified; this sketch has no SD output. Documentation-only release.

## 0.4.0 — 2026-10-22

- Add buffered standalone optical SD event logging with per-session sequence/timestamp records, stop/drain/close/CRC read-back and separate error checks.
- Use user-set UTC date/time filenames with collision suffix; explicit UNSET boot/uptime fallback before clock setup. Preserve prior recordings.
- Add Serial clock command, standalone event validator, SD wiring and combined optical/storage test instructions.
- ESP32 build and host validator/encoding/session/filter checks pass. No upload or physical SD/optical acceptance performed.

## 0.3.8 — 2026-10-22

- Add standalone optical GPIO32 start/stop, per-run counter reset, frozen stop summary and idle gating.
- Built-in LED stays on during a test and pulses off per accepted detection; trailing clear does not pulse.
- Add button wiring, white-mark rotation and sensitivity-pot instructions; record this chat's code/build-only responsibility.
- ESP32 build and sanitized host session/filter checks pass; separate physical retest pending.

## 0.3.7 — 2026-10-22

- Reconcile current optical milestone and installed-firmware header with the already recorded exact-20 manual test.
- Check raw capture starts at zero and ends with 20 detections, 138 raw transitions, zero qualified rejections and stale state. No firmware or acceptance expansion.

## 0.3.6 — 2026-10-22

### Added

- Known-count manual optical bench evidence: 20 confirmed finger cover/uncover cycles yielded exactly 20 detections, zero rejected qualified detections and stable final counters. Raw transitions 138. Firmware remains 0.3.3; controlled rotation acceptance remains pending.

## 0.3.5 — 2026-10-22

### Added

- Updated Stage 1 v0.3.3 firmware rerun report and serial: 110 accepted detections, 1,413 raw transitions, zero rejected qualified detections, stable final counters. Physical cycle count remains ambiguous; no 20-cycle acceptance claim.
- Compile, verified upload and host optical checks passed. Documentation-only release; firmware remains 0.3.3.

## 0.3.4 — 2026-10-22

- Plan optional local Wi-Fi/browser status and start/stop, standalone test packaging, physical fallback and bounded command/telemetry handling.
- Define network load, retry/disconnect, storage-error and physical RF/power acceptance checks; BLE retained as an alternative.
- Official capability documentation reviewed; local links, version alignment and whitespace checked. Firmware unchanged; wireless unimplemented.

## 0.3.3 — 2026-10-22

- Qualify stable detected/clear levels so one detection cycle counts/flashes once; trailing clear only rearms after 1 ms stable, rejecting brief chatter.
- Added raw-transition diagnostics and configurable detection polarity; preserve candidate-edge timestamp. Document minimum pulse-width limitations.
- Host bounce/cycle regressions and ESP32 compilation pass; physical single-flash confirmation pending.

## 0.3.2 — 2026-10-22

- Reconciled optical report with 133 captured snapshots and corrected stale installed-firmware/milestone summary.
- No firmware changes or additional hardware acceptance; known-count optical passage and controlled-speed tests remain pending.
- Verified capture totals and whitespace.

## 0.3.1 — 2026-10-22

### Added

- Stage 1 v0.3.0 verified upload and finger cover/uncover bench report with raw serial: 23 accepted edges, 51 rejected edges, stable final counters. No known-count or calibrated RPM acceptance claimed.
- Firmware compile and host optical filter check passed. Documentation-only version; installed firmware remains 0.3.0.

## 0.3.0 — 2026-10-22

- Added standalone Stage1_Optical_Test folder: GPIO26 digital edge timestamps, counts, provisional pulse filtering, interval/RPM diagnostics and built-in LED.
- Added exact 3V3/GND/OUT wiring, module voltage checks, Arduino build/upload and bench procedures in README.txt, matching earlier stages.
- ESP32 compilation and sanitized host pulse-filter tests pass; optical hardware acceptance pending. User authorised advancing this standalone stage while Phase 0B acceptance remains open.

## 0.2.6 — 2026-10-22

- Reconcile the progress summary with the already captured passing RUN0003 short session, preserving RUN0002 as historical failure.
- Firmware remains 0.2.4; full hardware acceptance remains pending. Checked evidence arithmetic, links and whitespace.

## 0.2.5 — 2026-10-22

### Added

- Recorded v0.2.4 firmware upload and RUN0003 hardware statistics with raw serial evidence: 7,647 samples, zero missed/dropped slots, CRC read-back and all checks passing.
- Host regression checks and compile passed; 7.646714-second run qualifies a short session only. Full Stage 0B acceptance remains pending. Documentation-only release; installed firmware remains v0.2.4.

## 0.2.4 — 2026-10-22

- Service the current due synthetic slot once at stop, retaining earlier missed slots and queue drops.
- Separate write/read-back/accounting/continuity checks; read-back uses full-file CRC of submitted bytes instead of assuming lossless sample indices.
- Added exact-boundary, reported-duration, true-gap and golden CRC regressions. ESP32 build and host checks pass; hardware retest pending.

## 0.2.3 — 2026-10-22

- Record user-supplied firmware 0.2.2 RUN0002 bench statistics and update current progress beyond the earlier mount failure.
- Preserve failed continuity status: one missed slot, no drops, read-back outcome unresolved. Firmware unchanged; physical acceptance remains pending.
- Checked arithmetic, documentation links and whitespace; no binary inspection or new hardware run.

## 0.2.2 — 2026-10-22

### Fixed

- Declare State-dependent functions explicitly so Arduino preprocessing compiles the built-in LED firmware reliably.

### Validation

- ESP32 core 3.3.12 compile and host controls/validator checks pass. Physical ESP32-D0WD-V3 upload hashes verified at 115200 baud; startup reports SD mount/card failure after wiring recheck. Recording and LED acceptance remain pending.

## 0.2.1 — 2026-10-22

- Use the built-in recording LED instead of external GPIO25 wiring; support board-defined LED pin, GPIO2 fallback and configurable polarity.
- Updated standalone wiring instructions. ESP32 compilation, host checks and whitespace checks pass; physical LED mapping remains unverified.

## 0.2.0 — 2026-10-22

### Added

- Standalone Phase 0B Arduino recorder, wiring/upload/test instructions and Python recording validator.
- Debounced start/stop, state LED, sequential files, 1000 Hz six-channel synthetic records, 12 KiB queue, drain/close/read-back and Serial metrics.
- Host controls/naming and recording integrity checks.

### Validation and limits

- ESP32 core 3.3.12 compilation and host checks pass. No new dependencies.
- Physical 60-second/button/storage-full tests and stall qualification remain pending; no Stage 0B hardware acceptance.
- Simple raw files require Serial summary evidence for sample count/completion; pattern repeats after 65536 samples.

## 0.1.2 — 2026-10-22

### Changed

- Stage 0B will use a standalone root-level `Stage0B_Recorder/` folder with sketch, instructions, local helpers and validator, matching Stage 0A.
- Updated the implementation plan and project working contract to preserve this packaging requirement.

### Validation

- Documentation links, version alignment, plan-copy consistency and whitespace checked. No firmware implementation or hardware testing.

## 0.1.1 — 2026-10-22

### Added

- Detailed Stage 0B design, implementation sequence, accounting rules, timing targets, fault tests and hardware acceptance matrix.
- Linked the detailed plan from the documentation index and progress tracker.

### Findings and validation

- Arduino CLI is available; core/FQBN remain unverified.
- Imported Stage 0A sketch contains a stray `no` prefix; original preserved and correction planned in the new sketch.
- Documentation links, version alignment, plan-copy consistency and whitespace checked; no firmware changes or hardware tests.

## 0.1.0 — 2026-10-22

### Added

- Git baseline preserving the original handover and Stage 0A storage test.
- Documentation index, staged development plan, progress tracker and hardware/test references.
- Project version policy and mandatory progress, changelog and development-log updates.

### Validation

- Documentation links, required files, version/changelog alignment and whitespace checked locally.
- No firmware compilation or hardware rerun; Stage 0A results are supplied historical evidence.
