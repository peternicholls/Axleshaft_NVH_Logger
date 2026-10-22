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
