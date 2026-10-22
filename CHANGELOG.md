# Changelog

Versions describe repository deliverables, not hardware qualification. The imported Stage 0A firmware has no established firmware version.

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
