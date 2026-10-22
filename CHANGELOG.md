# Changelog

Versions describe repository deliverables, not hardware qualification. The imported Stage 0A firmware has no established firmware version.

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
