# Next phase: qualify the bench and inspect recordings

Plan version 0.4.5, 2026-10-23. Planning only: no firmware or application implementation in this deliverable.

## Requirements and starting evidence

Prepare a dependable recording/inspection workflow before moving to real six-axis acquisition. Keep each firmware stage independently usable, preserve original data and version every coherent deliverable. This chat writes code, instructions and local checks; the separate upload/capture workflow executes hardware tests.

Current evidence: [optical SD report](evidence/2026-10-22-stage1-sd-button-report.md) establishes host-validated storage of 11 accepted events, 176 bytes and CRC d0aa5ebe. It was an informal finger test, not controlled rotation. [Phase 0B statistics](evidence/2026-10-22-stage0b-v0.2.4-statistics.md) establish only a 7.65-second synthetic recording, not the planned 60-second gate. [Progress](progress.md) records the open physical tests. No new qualification is inferred from planning.

## Recommended order

| Work item | Deliverable / intended location | Dependency and completion evidence |
|---|---|---|
| N1: execution pack | A short run sheet in `docs/test-procedures.md`, referencing the existing stage READMEs | Preserve separate firmware versions and record which stage is installed; no fresh upload here |
| N2: offline import engine | `analysis-bench/` with README and tested Python import/validation/export code | Existing optical format and fixture; reuse standalone validation rules without requiring firmware changes |
| N3: first analysis UI | Local event/interval/RPM view and file/Serial metadata import under `analysis-bench/` | N2; select a UI approach using available runtimes, no new dependency without explicit request |
| N4: physical qualification | New Serial captures, binaries and reports in `docs/evidence/` from separate workflow | Can run alongside N2/N3; matrix below; failures stay open and lead to bounded fixes |
| N5: acceptance review | Update `docs/progress.md` against every gate and publish a precise remaining-gap list | N2–N4 results; do not mark stages accepted based on short/finger-only tests |
| N6: Stage 2 entry | Detailed standalone GY-521 implementation specification, then code only when prerequisites pass | Recorder Stage 0B acceptance required; no IMU/optical integration in this phase |

N2/N3 are the next substantive code work available in this chat. Offline analysis can proceed while physical qualification is outstanding. Wireless implementation follows later as a separately measured change; [its plan](wireless-control-plan.md) remains on the roadmap.

## First analysis bench scope

Follow [analysis-bench-plan.md](analysis-bench-plan.md), limited initially to current optical files:

- Explicit optical format selection; file plus Serial evidence or expected count/CRC input. Show filename, source SHA-256, expected/actual size/count, CRC, firmware identity and unknown metadata.
- Read-only source handling; table of sequence, elapsed microseconds, interval and derived RPM. Explain the detections-per-revolution assumption and distinguish finger spacing from verified shaft speed.
- One interval/RPM plot with zoom/range selection, plus decoded CSV and an evidence summary export. Session comparison, IMU processing, FFT/order analysis and live telemetry remain later increments.
- Validation failures stay visible; no silent repairs, fabricated timestamps or automatic physical fault diagnosis. Offline operation requires no ESP32 connection.

Before UI selection, inspect installed libraries/runtimes and propose the smallest viable local approach. Existing validators remain independently runnable in their stage folders. Implementation must avoid letting bench code and firmware validators drift; lock format/CRC rules with shared fixtures and cross-checks.

### Software acceptance

1. Actual retained optical fixture imports as 11 events/176 bytes, CRC d0aa5ebe; source SHA-256 remains unchanged.
2. Decoded minimum/maximum/final intervals equal 47260/1473034/681691 us; empty and single-event sessions show no invented RPM.
3. Synthetic 100000 us event intervals yield 600 RPM for one detection/revolution and 300 for two; every plot/export records the assumption.
4. Missing/duplicate records, non-increasing timestamps, truncation, wrong count/CRC and unsupported format are rejected or explicitly labelled diagnostic-only.
5. Exported CSV exactly preserves event sequence/timestamps; summary includes source hash, inputs and software version. Export never overwrites the source.
6. Run import, selection and export through the UI locally; inspect behaviour/errors with the actual fixture and malformed fixtures. Record host test and UI evidence separately.
7. Initial supported file size: up to 10 MiB optical files. Use bounded parsing; larger files receive an actionable limit message. Record import time and responsiveness on the development Mac and set an evidence-based performance gate before claiming larger-file support.

## Separate hardware qualification run sheet

| Stage / test | Required evidence | Pass condition |
|---|---|---|
| Optical white mark | Fixed mount, one mark, measured gap, lighting/pot setting, independent physical count, saved Serial/file | Exactly 20 complete manual revolutions produce 20 accepted/persisted records, zero queue drops, firmware and host validation PASS; repeat three sessions |
| Optical speed | Independently measured RPM at low/mid/high points across a declared intended range; mark/clear pulse-width evidence | No unexplained missing/extra indices; measured RPM agreement within reference uncertainty; freeze numeric tolerance after documenting reference accuracy, before tests |
| Indicator/controls | Explicit observation, not inferred from completion replies | One off pulse per detection at observable speed; idle off, running on, saving/error indications; held/boot-held button behaviour and frozen stop counts confirmed |
| Stage 0B sustained | Three approximately 60-second runs with raw files/Serial, at least one run after restart | Zero missed/dropped slots; count agrees with recorded acquisition duration to one boundary slot; file size/pattern validators PASS |
| Names and clock | Several short optical sessions, same-second collision where practical, restart with and without clock, prior file hashes | UTC names after clock set, explicit UNSET otherwise, no overwrite, every saved file matches its own summary |
| Insufficient storage | Preserve wanted recordings; disposable card contents/controlled lack of space, partial output and Serial | Explicit error, acquisition stops, earlier files unchanged, no false saved/PASS; document recovery by reset |
| Stalls and buffering | Controlled write-delay tests according to existing Stage 0B plan, with fault injection clearly enabled/disabled | Sufficient-buffer stalls show no loss; exhaustion fails observably, counters reconcile; no silent overwrites |

Use [test-procedures.md](test-procedures.md) and the standalone READMEs for exact wiring and commands. A nominal speed range or tolerance is not invented here: lack of an independent reference/pulse-width measurement leaves that optical gate pending. User-deferred finger-count edge cases are not reclassified as defects.

## Stage 2 after recorder acceptance

Create `Stage2_IMU_Test/` with matching sketch, README.txt, all local helpers and host validator, following earlier stage packaging. Planned connections: GY-521 SDA21, SCL22, data-ready INT27, verified 3V3/common ground; retain button32 and existing SD CS13/SCK18/MISO19/MOSI23 at 1 MHz. Verify actual module labels, electrical behaviour and pull-ups before hardware use.

First increment: device identity/register read-back and stationary/orientation checks. Next: hardware/FIFO-based six-axis acquisition targeting 1000 Hz, raw signed 16-bit samples, explicit configuration, timing anchors and overflow/reset accounting, with SD serviced separately. Proposed ±8 g / ±500 degrees per second and filter bandwidths remain subject to official device register documentation and measured qualification before implementation. Do not timestamp FIFO samples as though acquired at I2C-read time. Use a documented new format rather than interpreting old synthetic files as IMU data.

Optical speed acceptance may proceed in parallel with standalone IMU only after the recorder prerequisite passes; combined logging requires both sensors independently qualified and a separate timing design review. No Stage 2 implementation is started by this planning request.

## Risks and mitigations

- Misreading short/finger success as full qualification: preserve evidence authority and gate status in each report.
- UI/library overhead or extra dependencies: inspect environment first, keep processing headless/testable and approval requirement intact for new dependencies.
- Unknown pulse widths/speed reference: measure before reducing stable-level filtering or asserting RPM accuracy.
- Data-format ambiguity: require format selection and Serial metadata; retain unknowns, hashes and original bytes.
- Physical workflow unavailable: complete software checks and prepare run sheets; leave hardware gates pending without uploading or operating hardware here.

## Delivery and review

Implement each bounded slice with its progress, VERSION, changelog and development-log updates in one coherent Lore commit. Preserve unrelated changes, run relevant host/build/UI checks and integrate verified slices into main. No release tags or remote publication are needed. Final phase report must separate completed software, captured hardware evidence and all unpassed gates.
