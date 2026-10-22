# Development log

## 2026-10-22 — 0.1.0: repository foundation

Initial baseline commit: f9b294a. Preserved the supplied handover, Stage 0A sketch and its README without edits. Established `main`; ignored local runtime/editor artifacts and raw recordings. No remote configured.

Added documentation index, stage-gated development plan, progress tracker, wiring/test references and format/timing design placeholders. Established a mandatory version, changelog, progress and evidence update for each deliverable in the repository working contract.

Validation: local relative Markdown links, required documentation files, version/changelog alignment, original-file preservation and Git whitespace checks passed. Firmware has not been compiled, uploaded or rerun. Historical Stage 0A hardware results remain attributed to the handover. Stage 0B implementation and all subsequent hardware gates remain pending.

## 2026-10-22 — 0.1.1: Stage 0B planning

Created `docs/stage0b-plan.md` with a local planning copy under ignored `.omx/plans/`. Defined producer/storage separation, bounded buffering, exact loss accounting, deterministic records, control states, ordered deliverables and host/build/bench gates. Recorded the imported sketch's `no/*` prefix and available Arduino CLI without modifying firmware or assuming a valid installed target.

Validation: relative documentation links, 0.1.1 bookkeeping alignment, local plan-copy consistency and whitespace checks passed. No firmware compilation, API qualification, upload or hardware measurements performed. Stage 0B implementation and acceptance remain pending.

## 2026-10-22 — 0.1.2: standalone stage packaging

Updated Stage 0B planning to follow the user's Stage 0A folder convention. `Stage0B_Recorder/` will contain the sketch, README.txt, required local helpers and standalone validator, with no sibling runtime dependencies. Tests remain repository-level and use the bundled validator directly. No empty stage folder or placeholder firmware created.

Validation: documentation links, version alignment, local plan-copy consistency and whitespace checks passed. Implementation and hardware acceptance remain pending.

## 2026-10-22 — 0.2.0: standalone Phase 0B implementation

Implemented debounced GPIO32 start/stop, GPIO25 state indication, reserved GPIO33, persistent highest-plus-one RUN naming, 1000 Hz six-int16 synthetic samples and static 12 KiB FreeRTOS queue. Storage task batches records, drains on stop, flushes/closes and fully reads back. Serial reports count/duration, bytes, missed/dropped slots, write latency, queue peak and close time. Added standalone instructions and Python validator; preserved Stage 0A source unchanged.

Scope followed the user's final full Phase 0B requirements after a temporary button-only narrowing. Kept raw 12-byte records and external summary evidence; no header/footer or automatic stop. Disclosed repeating-pattern and interruption limits. No new dependencies installed.

Evidence: Arduino CLI 1.5.1, ESP32 core 3.3.12, FQBN esp32:esp32:esp32 compile with all warnings succeeds (331802 flash bytes, 35152 static RAM bytes). Python validator checks valid empty/short/multi-batch sessions and corruption/truncation/extra/missing samples; C++ controls/naming checks pass with -Wall -Wextra -Werror -pedantic and address/undefined sanitizers. Documentation and whitespace checks pass. Official ESP timer documentation and installed core headers reviewed. Native helper delegation was unavailable due to model availability; implementation completed directly.

No ESP32 USB device detected, no upload or hardware acceptance. Timing under actual writes, physical controls/LEDs, 60-second recording, storage-full behaviour and injected-stall/error qualification remain unverified. Stage 1/2 remain gated.

## 2026-10-22 — 0.2.1: built-in recording indicator

User confirmed no separate LED is fitted. Replaced GPIO25 with LED_BUILTIN where available or GPIO2 for the generic target, with configurable active-low polarity. Generic ESP32 variant does not define LED_BUILTIN; actual board mapping remains unconfirmed. Updated standalone and project wiring instructions. ESP32 compile, existing host controls/validator checks and whitespace checks pass. No physical LED test or upload performed.

## 2026-10-22 — 0.2.2: upload retry and Arduino declaration fix

Reproduced Arduino-generated declarations appearing before State after the built-in LED helper was added. Explicit getState/setState declarations fix compilation without changing recorder behavior. Python validator and C++ controls checks pass; git whitespace check passes. Compile target esp32:esp32:esp32, CLI 1.5.1, core 3.3.12. Physical board ESP32-D0WD-V3 revision 3.1, MAC 28:05:a5:35:1c:90 on /dev/cu.usbserial-410. Upload at 115200 baud verified flash hashes. Startup after user wiring check still reports SD mount/card failed. No recording started; card identity, physical LED operation and full Stage 0B acceptance remain unverified.

## 2026-10-22 — 0.2.3: reported RUN0002 evidence

Logged user-supplied firmware 0.2.2 statistics in docs/evidence/2026-10-22-phase0b-run0002.md. Reported button start/stop and persistence succeeded, with one missed eligible slot and no queue drops; firmware latched ERROR. Combined diagnostics do not establish read-back outcome. Larger reported card capacity supersedes the tiny-card assumption for this run only. No firmware changes, binary inspection or new hardware test. Checked count/size/slot arithmetic, documentation links and whitespace. Hardware acceptance remains open.

## 2026-10-22 — 0.2.4: stop-boundary and diagnostic correction

Code inspection found stopLocked counted a pending current slot as missed even if its periodic callback had not yet run. Stop now generates that current synthetic slot once under the same producer lock; older skipped slots remain counted. This is a plausible explanation of the reported single miss, not a confirmed diagnosis without its timing trace. No historical samples are backfilled and no loss tolerance was relaxed.

Read-back now computes CRC32 across all reopened bytes and compares against CRC32 of successfully submitted writes, independently of sample indices. Added separate write/readback/accounting/continuity statuses; final PASS still requires all checks. Added portable eligible-slot/CRC helper and host tests for before-start/exact-deadline boundaries, reported run duration, retained earlier gaps, chunked CRC and a standard golden vector. No dependencies added; raw format unchanged.

Verification: ESP32 core 3.3.12 compile with all warnings, Python validator checks and both C++ host test executables with warnings-as-errors and address/undefined sanitizers pass. No upload or hardware retest performed. Physical 60-second run, LED polarity and insufficient-space qualification remain open.

## 2026-10-22 — 0.2.5: v0.2.4 hardware recording evidence

Compiled and uploaded source fef35d1 to ESP32-D0WD-V3 revision 3.1 at 115200 baud; flash hashes verified. SD mounted. Physical button recorded RUN0003.BIN for 7,646,714 us: generated/persisted 7,647 samples, 91,764 bytes, missed=0, dropped=0. Write/read-back/accounting/continuity checks all PASS; CRC32 f6e7c3d2. Maximum write 44,064 us, peak queue 44 records/528 bytes, flush-close 44,584 us. Full statistics and raw serial saved under docs/evidence/2026-10-22-stage0b-v0.2.4-*. Python validator and both C++ host checks passed. Actual recording host validation, 60-second test and full bench matrix remain pending. Documentation version 0.2.5; installed firmware remains 0.2.4.

## 2026-10-22 — 0.2.6: reconcile evidence and progress

Read both RUN0002 report and RUN0003 v0.2.4 statistics/raw Serial capture. Updated the stale milestone summary to identify RUN0003 as the latest passing short run. Verified 7647*12=91764 and ceil(7646714/1000)=7647. Retained all remaining gates and original evidence. Documentation links and whitespace checked; no firmware modification, binary inspection or new hardware run.

## 2026-10-22 — 0.3.0: standalone optical test

User explicitly requested optical next. Added independently copyable Stage1_Optical_Test folder with code and README.txt matching the earlier stage convention. GPIO26 digital falling-edge input, module verified 3V3/common GND, built-in LED, GPIO33 reserved. Captures monotonic edge times in the interrupt; reports count, rejected edges, intervals, instantaneous/run-wide RPM and stale state outside the interrupt. Provisional 1000 us rejection threshold is configurable and unqualified. No SD or IMU integration and no new dependencies.

Verification: ESP32 core 3.3.12 compile with all warnings; host tests with warnings-as-errors and address/undefined sanitizers cover first edge, threshold boundary, rejection reference, interval extrema and 64-bit timestamps; documentation links/whitespace checked. No upload or physical optical testing. Actual module pin order, output voltage, polarity, sensing distance and reliable speed range remain unverified. Phase 0B gates remain open by explicit sequence override.

## 2026-10-22 — 0.3.1: Stage 1 input-response bench evidence

Built and uploaded Stage 1 v0.3.0 (source 1f92fd2), verified flash hashes on ESP32-D0WD-V3 revision 3.1. User covered/uncovered sensor with a finger; no known-count mark or rotation test. Captured 23 accepted and 51 rejected falling edges, intervals 231,431–688,384 us; final level 1, stale 1 and counters stable. Saved raw serial and full report in docs/evidence/2026-10-22-stage1-v0.3.0-*. Host optical filter check and firmware compile passed. Gap, voltage, LED observation, passage accuracy and controlled RPM unverified; Stage 1 acceptance remains pending. Documentation release 0.3.1; installed firmware 0.3.0.
