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

## 2026-10-22 — 0.3.2: review optical report and capture

Read optical report and checked all captured snapshots programmatically: 133 total, final 23 accepted/51 rejected, 123 final snapshots unchanged. Updated stale progress header and optical milestone to the actual input-response result. No passage count or rotation accuracy can be derived from the finger test; 51 rejections indicate close electrical transitions, with cause unproven. Next test: fixed setup and known-count mark passages, then controlled rotation. Firmware unchanged; no new hardware run. Capture arithmetic and whitespace checks passed.

## 2026-10-22 — 0.3.3: qualify one optical detection cycle

Replaced direct falling-edge counts with CHANGE-edge candidate capture and stable detected/clear qualification. Clear transition never counts; 1 ms stable clear is needed before another stable detection can count. A 250 us ESP timer qualifies independently of Serial loop, retaining candidate timestamp. Boot-covered cannot produce an initial event. Preserved interval filter and added raw-transition diagnostic; documented polarity and pulse-width tradeoffs. User's double-flash observation is not proven to have a particular electrical cause; this rejects brief edge chatter without blindly halving counts.

Host cycle/bounce/boot tests pass with warnings-as-errors and address/undefined sanitizers; ESP32 compile and whitespace checks pass. No dependencies, SD or IMU changes. Physical single-flash test and speed-range qualification remain pending; no upload performed.

## 2026-10-22 — 0.3.4: wireless planning

User proposed the fitted ESP32 radio for monitoring and recording start/stop. Reviewed supplied photo and official Espressif ESP32/Wi-Fi documentation; added a SoftAP/browser-first proposal with BLE alternative, explicit idempotent commands, physical fallback, bounded telemetry and acquisition isolation. Added standalone StageWireless_Test packaging proposal and measurable network/recording/disconnect/range gates. No code, dependencies, pin changes or hardware tests. Verified documentation links, version alignment and whitespace; current acceptance gaps remain open.

## 2026-10-22 — 0.3.5: updated optical firmware rerun

Uploaded firmware banner 0.3.3 from repository revision 61d2fd0 (0.3.4). Compile and flash hash verification passed; host optical checks passed. Captured 110 accepted detections, 1,413 raw transitions, zero rejected qualified detections; final counters stable. User answered “yes” to combined count/LED prompt; actual physical cycle count cannot be inferred. No verified 20-cycle pass claimed. Report and raw serial under docs/evidence/2026-10-22-stage1-v0.3.3-rerun-*. Controlled optical acceptance remains pending; firmware unchanged by documentation release.

## 2026-10-22 — 0.3.6: exact 20-cycle optical bench pass

Reset installed Stage 1 firmware 0.3.3 and captured zero initial counters. User completed requested exactly 20 finger cover/uncover cycles. Accepted=20, rejected qualified=0, raw transitions=138; final level=1, stale=1, counters stable. Intervals 391,662–632,032 us; latest 418,269 us. Saved full report and raw serial under docs/evidence/2026-10-22-stage1-exact20-*. No firmware edit/upload needed. Exact-count bench check passes; physical LED flash count, measured setup and calibrated rotation remain unverified. Whitespace and report/capture count consistency checks passed.

## 2026-10-22 — 0.3.7: reconcile exact-count evidence

Read updated-code rerun and exact-20 reports, checked exact-20 raw capture initial/final counters, and reconciled stale current milestone and installed firmware header. Manual detection count passes for this test; flash count was not independently confirmed and controlled rotation remains open. Firmware unchanged. Capture assertions and whitespace checks passed; no new hardware run.

## 2026-10-22 — 0.3.8: button-controlled rotating-mark test

Added local Button.h using the existing Stage 0B debounce behaviour without sibling runtime dependencies. OpticalSession owns running state, reset/frozen counts and LED presentation. GPIO32 first press starts fresh, second stops with summary. ISR/qualification ignore idle input; new start over a mark requires clear before counting. Built-in indicator is steady on while running and off for 30 ms per accepted detection, off idle. README now includes exact button wiring, white mark, sensitivity adjustment, known revolutions and pulse-width considerations. No SD or IMU changes; no dependencies added.

User clarified this chat only prepares programming, never uploads. Persisted that working agreement. ESP32 core 3.3.12 compile with all warnings and sanitized host session/filter tests pass, covering idle gating, reset, frozen stop, boot-covered, held button, one off pulse and no trailing pulse. Documentation/whitespace checks pass. No upload or hardware operation performed; new physical behaviour and rotating-speed accuracy remain unverified.

## 2026-10-22 — 0.3.9: button-controlled optical upload and bench test

Compiled/uploaded firmware 0.3.8 with verified flash hashes; host optical session checks passed. Serial captured button START, optical activity, STOP after 14,099,000 us, accepted=12, rejected=0, raw=88, then running=0 with frozen counters. User replied done without separate LED confirmation. Saved report/raw capture under docs/evidence/2026-10-22-stage1-button-*. No SD-writing code exists in this sketch: writing/file-close requirement remains outstanding. Installed firmware unchanged at 0.3.8; docs release 0.3.9.

## 2026-10-22 — 0.4.0: optical SD event logging and dated files

User requested SD writes in the standalone optical test and useful date/time filenames. Added accepted-event queue (256 x 16 bytes), dedicated SD task, 512-byte batching, stop/drain/close/read-back CRC, explicit write/readback/accounting diagnostics and partial-file retention on errors. Retained CS13/SCK18/MISO19/MOSI23 and 1 MHz, GPIO32 controls and off-pulse LED. Files contain sequence and monotonic elapsed timestamps, no IMU or synthetic data. Names use user-set UTC and collision suffix; TIME command while idle sets wall time. Unset clock is explicitly named with boot ID/uptime; no historical dates invented. Local helpers/validator keep the folder independently usable; no dependencies added.

Validation: ESP32 core 3.3.12 build with all warnings; host Python checks valid empty/short/multi-batch records, truncation, extra/missing/duplicate records, timestamp order, corruption/CRC and CLI status. Sanitized C++ checks encoding, date/fallback names, collision suffix bounds and golden CRC; existing session/filter checks pass. Documentation and whitespace checked. No upload, clock setting or hardware operations performed. Physical SD-full, storage stalls, known-revolution file verification, LED/controls and naming across restart remain pending. This optical/storage test does not accept earlier Stage 0B gates or integrate IMU acquisition.

## 2026-10-22 — 0.4.1: SD absolute-path fix and clocked recording

0.4.0 upload and clock set succeeded but file create failed. Installed FS source rejects paths without '/'. Changed helper plus regression expectations; test failed before fix and passed after. Firmware 0.4.1 compiled/uploaded with verified hashes; reset, mounted SD, sent current TIME and confirmed acknowledgement. Button session wrote OPT_2026-10-22_14-45-28_0001.BIN: 16,847,223 us, accepted=persisted=11, bytes=176, raw=85, rejected=0, queue drops=0, peak=1/16 bytes, max write=22 us, flush-close=53,464 us, CRC d0aa5ebe. Write/read-back/accounting PASS and returned Idle. User reported possible manual error for requested 10 cycles and deferred count edge cases; count accuracy inconclusive. Full report/raw captures saved. Host Python and C++ storage checks/whitespace pass; actual-file host validation and LED observation remain pending.

## 2026-10-22 — 0.4.2: review optical storage evidence

Read SD/button report and 0.4.1 raw capture; checked accepted=persisted=11, 176 bytes, zero drops and three PASS checks with CRC d0aa5ebe. User clarified informal finger passing and confirmed binary is still on ESP32 SD. No binary contents or file hash inspected; host validation awaits separate copy. Preserved original findings and appended clarification/validator command. Capture arithmetic, documentation links and whitespace checked. Firmware unchanged; no upload or hardware operation.

## 2026-10-22 — 0.4.3: independent SD recording inspection

Validated actual evidence binary with validate_optical.py --expected-events 11 --expected-crc d0aa5ebe: PASS. Independently decoded sequence/time records and checked all timestamps fall within captured duration; min 47260 us, max 1473034 us and last 681691 us match Serial; event-spacing mean rounds to 77.83 RPM. File SHA-256 66cb58696b546e97e47509218deb2af37229fd2af8b96ac94c5b0a73e61677e2. Retained the 176-byte binary explicitly despite the general raw-recording ignore rule and documented decoded events. Host copy/integrity gate for this recording now passed; physical rotation accuracy remains open. Firmware unchanged, no hardware operations. Existing validator, timing assertions, links, version alignment and whitespace checked.

## 2026-10-22 — 0.4.4: analysis bench planning

User requested analysis bench software as a planning item. Added a focused plan and roadmap/index links, covering offline import with integrity evidence, optical timing/RPM UI, session comparisons, parameterised exports and later IMU/synchronous analysis. Proposed Python processing engine and local UI, with framework/dependencies left for implementation evaluation. Defined existing-file and synthetic-fixture acceptance, source immutability and unknown-metadata handling. Reconciled stale progress header against existing evidence. Documentation links, version alignment and whitespace pass. No application, dependencies, firmware changes or hardware operation.

## 2026-10-23 — 0.4.5: next-phase work plan

Reviewed current evidence/progress and planned a qualification-and-analysis phase rather than treating incomplete hardware gates as passed. Sequenced run sheet, offline optical engine/UI, separate physical evidence, acceptance review and conditional standalone GY-521 entry. Documented exact intended Stage 2 pins, need for official register verification and independent sensor/timing qualification. Wireless deferred to separate qualification. Saved tracked plan and local .omx/plans copy; no firmware/UI implementation, dependencies, upload or hardware operation. Links, version alignment, plan-copy consistency and whitespace checks passed.

## 2026-10-23 — 0.4.6: GY-521-first synchronisation rig plan

User steered next work to GY-521 setup, then a motor/shaft with white mark and once-per-revolution click. Replaced the analysis-first sequence with standalone sensor bring-up/FIFO acquisition, mechanical tap rig, combined anchor-based timing and analysis. Distinguished mechanically transmitted acceleration from audible click, physical phase offset from digital timing, and filter delay from sample timestamps. Reviewed official MPU register-map sample-rate/filter documentation. Updated sequence authority while retaining older gates. Links, version alignment, local plan copy and whitespace pass. No firmware, new dependencies, hardware operations or qualification results.

## 2026-10-23 — 0.4.7: illustrated experiment procedure

User requested a way to determine/verify delays and response times. Prepared NVH-TIME-01 with editable protocol, reproducible ReportLab vector builder and seven-page PDF. Four figures show event timing, motor/marked-shaft mechanical tap, signal wiring and acceptance flow. Five tests separate electrical input/qualification, physical optics, IMU/filter/FIFO, rotating-phase/drift and SD/control response. Added explicit Level A/B reference capabilities, confounding terms, correction/uncertainty rules, required firmware instrumentation and run worksheet. Nominal microsecond timer resolution is not treated as end-to-end accuracy; contact time is not equated with acceleration onset.

Used existing bundled Python/ReportLab/pypdf and installed Poppler; no packages installed. All seven pages rendered and visually inspected, PDF section/figure/worksheet/source extraction checked, builder syntax and example phase arithmetic verified, links/version/whitespace checked. Layout iteration removed an orphan sources page. No firmware modifications, uploads, motor operation or physical measurements. Current optical/IMU qualification gaps remain open.

## 2026-10-23 — 0.4.8: GY-521 coding outline

User asked to start outlining accelerometer coding. Created docs/stage2-coding-outline.md with standalone packaging, exact proposed pins, file/interface ownership, bounded bus/frame operations, five slices (bring-up, FIFO, anchors, storage, inspection), local versus physical tests and start/stop/error accounting. Reuse earlier button/CRC/naming patterns locally; freeze a new explicit timing-aware format before encoding. No stub firmware or new dependencies created, no build/hardware claim. Documentation links, version alignment and whitespace checked.

## 2026-10-23 — 0.5.0: design fixes and GY-521 I1 coding

User requested gaps resolved, then coding. Reviewed outline against register-map sections and installed Arduino-ESP32 Wire APIs. Fixed design ambiguities around 14-byte register versus 12-byte FIFO decoding, identity independent of AD0 address, finite reads/timeouts, 100 kHz throughput limits, stop boundary, loss/timing association and terminal error capacity. Future FIFO/anchor/format details remain explicit gates, not physical results. Added docs/stage2-design-review.md and updated outline.

Implemented Stage2_IMU_Test with local raw decoders, fixed configuration/read-back driver, bounded real Wire adapter and ~10 Hz fresh diagnostic snapshots. Reset/settle pauses are explicit, generated interrupts/FIFO disabled, raw signed data plus range conversions printed, config health checked every 5 seconds, no fresh snapshot for 1 second latches error. Preserved prior stages, no new dependencies, no uploads or hardware access. README includes exact 3V3/SDA21/SCL22/AD0/INT27 wiring and limits.

Host tests exercise both layouts, signed bounds, both bus addresses with WHO_AM_I=0x68, unsupported IDs, reset/readback/write failures, config drift, missing fresh data, repeated START, finite adapter bounds and partial output preservation. Sanitizers pass; Clang static analysis reports zero diagnostics. ESP32 core 3.3.12 compile with all warnings passes cleanly (296836 program bytes / 23548 static RAM bytes; durable local-check evidence linked from progress); independent copied-folder build passes. Python validator regressions pass. Read-only independent verifier found no blocker, corrected README working-directory wording and identified remaining sketch-clock coverage gap. One-second/5-second loop timing and actual sensor response require further evidence; no continuous-acquisition acceptance. Documentation/version/whitespace checked.

## 2026-10-23 — 0.5.1: wheel-balancer concept

Created a separately indexed side-project concept at user request. Defined RAM-only acquisition, spindle angle feedback, trial-mass calibration, single-plane learning then two-plane wheel correction, pictorial inner/outer placement and guarded indexing. Reviewed primary balancing references and HSE/machinery safety sources. Recorded wheel envelope and powertrain as unknown; no component purchases, dependencies, firmware edits, uploads or hardware operation. Independent emergency-stop/guard/standstill design and all measurement/positioning qualification remain pending. Checked local Markdown links, repository version alignment and git diff whitespace.

## 2026-10-23 — 0.5.2: CAD and circuit-board workstreams

Added user-required CAD and PCB development to the wheel-balancer scope. Defined assemblies, tolerances/load assumptions, schematics, harnesses, PCB outputs, source/export revision matching, mechanical/electrical interface review and fabrication gates. Kept independent safety engineering and measured as-built/board acceptance explicit. No tools selected, dependencies installed, design sources created, fabrication or hardware operation. Local Markdown links, version alignment and whitespace pass.

## 2026-10-23 — 0.5.3: drive, sensing and explicit research

Updated the wheel-balancer README following angle/drive discussion and the request to spell out research. Recorded sensor and drive alternatives, provisional spindle-mounted absolute magnetic encoder and belt-driven servo, common drive-cycle behaviour and unresolved motor sizing. Added 14 explicit research areas with questions and closure evidence; no technical choice promoted to an approved design. Linked references consulted in the preceding discussion; no fresh research campaign, dependencies, firmware, fabrication or physical operations. Local Markdown links, version alignment and whitespace checks pass.

## 2026-10-23 — 0.5.4: completed research archive

Transferred the corrected user-supplied research from its embedded completed report in the original ChatGPT conversation. Retained unchanged Markdown/PDF exports, provenance, byte counts and SHA-256 manifest; supporting B&K PDF from earlier transfer retained and labelled separately. Verified source/destination equality, PDF signature, Markdown title/ending, PDF text extraction, links/version/whitespace. Exported citation tokens may require original conversation; no independent fact-check, visual PDF audit, hardware operation or gate closure. Failed sensor-datasheet downloads not represented as available.
