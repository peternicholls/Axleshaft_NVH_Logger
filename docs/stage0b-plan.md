# Stage 0B implementation plan

Planning version: 0.1.1, 2026-10-22. Status: ready for implementation; no Stage 0B acceptance claimed.

## Requirements and inspected baseline

Deliver a standalone button-controlled synthetic recorder, its independent validator, and reproducible bench evidence before optical or IMU development. Scope follows [development-plan.md](development-plan.md) and [test-procedures.md](test-procedures.md).

- Retain CS=13, SCK=18, MISO=19, MOSI=23 and 1 MHz SPI from `Stage0A_SD_Test/Stage0A_SD_Test.ino:17–22`; reuse its initialisation sequence at lines 65–69.
- The imported sketch begins with literal `no/*` at line 1. Its historical hardware pass does not establish that this exact imported file builds. Preserve the original and remove the stray prefix in the new Stage 0B sketch only.
- `arduino-cli` is available at `/opt/homebrew/bin/arduino-cli`. Installed core version, board FQBN and connected hardware remain to be checked.
- Controls and electrical checks follow [hardware-wiring.md](hardware-wiring.md). Storage baseline and pending acceptance are in [progress.md](progress.md).

## Proposed design

Use a periodic synthetic producer, a bounded RAM queue, and a dedicated storage owner. Controls/LED updates must continue during SD stalls. Use existing Arduino-ESP32/FreeRTOS facilities without adding dependencies; verify current official API documentation against the installed core before selecting timer/task APIs. Keep SD access, file naming, finalisation and read-back in the storage task. Keep critical sections short and never hold a queue lock during I/O. A queue is preferred over a bespoke lock-free ring for the first implementation; benchmark before adding complexity. A single loop that produces samples and writes synchronously would hide stalls and is rejected.

Start with a fixed 16 KiB sample-buffer budget; document queue and task-stack overhead separately. The buffer is a measured design choice, not a bound on SD latency. No pinning or task priority assumptions until the actual ESP32/core configuration is confirmed.

Use a 64-bit monotonic microsecond domain. Nominal deadlines are 1,000 µs apart. Each record carries the scheduled slot index, actual producer timestamp and six deterministic signed 16-bit channels. Derive channel `c` (0–5) from slot `n` as `((n * (2*c+1) + 997*c) mod 65536) - 32768`, with explicitly sized arithmetic. This permits independent validation without pretending these are real IMU samples.

Late wakeups produce at most one current-slot record: count earlier elapsed slots as missed, without fabricating a backlog of on-time samples. Record queue-full drops separately. At stop, fix a producer cutoff, quiesce it and count all eligible deadlines before draining. Define eligible deadlines as `start + k*1000 < cutoff`, for integer k >= 0. Require `eligible = enqueued + missed_deadlines + queue_drops` and, on a clean stop, `enqueued = persisted`. Define boundary rounding in the format specification and test exact-boundary cases.

States: IDLE → STARTING → RECORDING → STOPPING → IDLE, with any failure entering ERROR. A press requires 30 ms stable low after release; a held button cannot repeat. Require stable release after boot so boot-held input cannot start a session. Ignore presses while STARTING/STOPPING. Error is latched until reset for Stage 0B. LED: off IDLE, slow 1 Hz full-cycle flash STARTING/STOPPING, steady RECORDING, rapid 4 Hz flash ERROR. Begin acquisition only after file/header creation succeeds; end indication only after clean finalisation and read-back checks.

Choose a documented little-endian format version 1 with magic, lengths, rate, repository/build identity, monotonic start, deterministic-pattern identifier, sample records and a completion footer with counts, cutoff, storage metrics and checksum. Serialise explicit fields, never native struct padding. Specify exact offsets, checksum coverage/algorithm, partial final block handling, maximum lengths and unsupported-version behaviour before coding. Keep file-format version separate from repository version. Batch whole records into block-oriented writes; do not insert undocumented SD padding. Track time for write calls and flush/close separately.

Scan existing RUN0001.BIN–RUN9999.BIN before creation with one storage owner; refuse exhausted names and never truncate an existing path. Re-check absence immediately before open. Detect short writes regardless of free-space estimates. On failure halt production, close best-effort and retain the partial file. A missing/invalid footer is incomplete; a footer reporting losses is not a passing recording. On healthy storage, drain and finalise with explicit counters even after an acquisition failure. No claim of power-loss atomicity or guaranteed durability from flush/close.

## Implementation sequence and deliverables

| ID | Deliverable and intended files | Dependency / verification |
|---|---|---|
| B1 | Capture CLI/core/FQBN and electrical preflight in `docs/development-log.md`; create `firmware/stage0b-recorder/Stage0B_Recorder/` | Verify installed APIs and board; preserve original reference |
| B2 | Freeze byte layout/accounting in `docs/data-format.md`; implement `analysis/validate_recording.py`, `tests/test_validate_recording.py` | Independent generated fixtures; stdlib only; reject malformed lengths/versions/checksums/footer/sequence/pattern |
| B3 | Implement portable control, synthetic scheduling and accounting helpers plus host regression tests under `tests/`; firmware files in B1 directory | Bounce, hold, boot-held, timestamp boundaries, missed slots, overflow and stop race tests |
| B4 | Integrate producer, fixed queue, storage owner, naming, state machine, read-back and metrics in the Stage 0B sketch and local headers | B2/B3; compile actual ESP32 target; host-injected stalls/errors through a narrow storage test seam |
| B5 | Add build/upload commands, wiring and bench run sheet to `firmware/stage0b-recorder/README.md` and `docs/test-procedures.md` | B4; run host checks and actual firmware compile; record tool versions and gaps |
| B6 | Execute bench matrix; save serial/validator summaries in a dated `docs/evidence/` record and update progress | Hardware and B5 required; all criteria below must pass before Stage 1/2 |

For each completed slice update progress, VERSION, changelog and development log together under [development-workflow.md](development-workflow.md). Use patch increments for tooling/fixes and a minor increment when the recorder capability is delivered. Keep hardware acceptance pending until B6 passes. Commit verified slices with Lore trailers; integrate usable slices into main. Do not tag a hardware-qualified release from compilation alone.

## Acceptance and verification matrix

These numerical values are initial acceptance targets, chosen before measurement. If evidence requires changing one, document the reason and repeat affected tests; do not silently relax it.

| Test | Passing result |
|---|---|
| Host validator | Valid empty/short/multi-block sessions pass; unknown version, invalid lengths, byte corruption, missing/duplicate/out-of-order slots, wrong pattern, truncation, missing footer and trailing garbage fail with nonzero status and diagnostic |
| Controls | 20 bench start/stop pairs produce exactly 20 closed files; 2 s holds and boot-held button produce no repeats or unintended start; bounce scenarios pass host tests |
| Normal sessions | Three 10 s and three 60 s runs, subject to measured card space; zero missed deadlines, drops, short writes or corrupted records; all reopen and validate |
| Rate/accounting | Slot period exactly 1,000 µs in metadata; eligible count agrees with elapsed start/cutoff within one boundary slot; accounting identities hold exactly; actual timestamp mean rate within 1% of 1 kHz on 60 s runs and maximum producer lateness <= 2 ms |
| LED / graceful stop | All documented state indications observed; no IDLE until queue drained, footer written, flush/close and reopen checks completed; measure stop latency without assuming an SD upper bound |
| Storage stalls | Inject 60 ms and 250 ms delays in storage path while acquisition continues; at least 10 injections each, queue high-water recorded and no loss if capacity permits; inject a delay exceeding measured buffer coverage and require explicit failure/loss accounting, no silent overwrite |
| Filename persistence | Reboot between runs; pre-existing files retain byte-for-byte hashes; occupied names skipped; all 9,999 occupied names fail without truncation (host seam permitted for exhaustion) |
| Storage failure | Inject zero/short writes and open failures in host tests; confirm missing-card and disposable-card-full behaviour on bench; ERROR latched, production stopped, partial recording retained and validator rejects incomplete content |
| Metrics | Persist/report enqueued, persisted, missed, dropped, bytes written, duration, queue peak, write maximum and flush/close duration; attempted/accepted/persisted counts have distinct definitions |
| Build | Recorded core/FQBN compile succeeds; warnings reviewed; host tests and Git whitespace checks pass; no unapproved dependencies |

Use a disposable card or preserve wanted recordings before fullness tests. Confirm room before each duration test and transfer evidence between runs if necessary. A shorter substitute cannot satisfy the 60 s rate gate; if the small card cannot support it, leave that gate open pending suitable storage. Use the same production pipeline for injected delays, with test-only fault controls disabled by default and clearly reported when enabled.

## Risks and mitigations

- Scheduling or shared-state races: explicit producer quiescence handshake, single storage owner, exact accounting and boundary/race tests.
- SD latency exceeds buffer capacity: record occupancy, detect overflow, latch failure; no inferred upper bound from the earlier 58 ms result.
- Filesystem errors prevent writing an error footer: keep partial file and serial diagnostics; host validator fails closed.
- Independent host code accidentally mirrors a firmware error: fixed hand-calculated golden records and mutation tests alongside generated fixtures; verify cross-language bytes.
- Card wear/space and wiring remain unknown: short initial sessions, preserve existing files, electrical preflight and recorded card identity.
- Hardware unavailable: complete host/build deliverables and leave B6 explicitly pending; no Stage 1/2 implementation.

## Completion evidence

Record commit/version, CLI/core/FQBN, hardware/card identity, supply/wiring, run duration, serial capture reference, recording filename/hash, validator output, counters and pass/fail per matrix row. Raw recordings remain ignored unless deliberately selected as small fixtures. Link evidence from progress and test procedures. This plan authorises no claim that the pipeline or timing targets have already been achieved.
