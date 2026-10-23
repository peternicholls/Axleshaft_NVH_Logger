# Progress

Last updated: 2026-10-23. Repository version: 0.5.2 (latest captured installed firmware: optical 0.4.1).

## Current position

Stage 0A: accepted in the supplied handover; not rerun in this repository session.
Stage 0B: standalone 1 kHz firmware, instructions and validator implemented. ESP32 compilation and host checks pass; bench acceptance pending.

| Milestone | Status | Evidence / remaining gate |
|---|---|---|
| Repository baseline | Complete | Original files committed as f9b294a |
| Documentation and delivery rules | Complete | Version 0.1.0 documentation checks; see development log |
| Stage 0A storage | Historical pass | Handover: 262,144 bytes verified; 2.93 s write; 58 ms maximum observed block write |
| Stage 0B planning | Complete | Version 0.1.2; design, test matrix and standalone Stage0B_Recorder folder contract |
| Stage 0B recorder | Implemented, bench pending | Core 3.3.12 compile passes; host controls/validator checks pass; ESP32 upload verified; [RUN0003 short-run capture](evidence/2026-10-22-stage0b-v0.2.4-statistics.md): 7647 samples, zero misses/drops, all four checks PASS; full acceptance pending |
| Stage 1 optical | Known-count manual test passed; rotation acceptance pending | [Exact 20-cycle report](evidence/2026-10-22-stage1-exact20-report.md): 20 cycles / 20 detections; physical LED count and controlled RPM unverified |
| Stage 2 IMU | I1 bring-up code ready; physical evidence pending | Standalone folder compiles; driver/frame/bus tests and static analysis pass; FIFO/anchors/SD not implemented |
| Stage 3 integration | Gated | Requires accepted optical and IMU stages |
| Vehicle trials | Not started | Requires integrated bench acceptance and secured mounting/wiring |

## Immediate next work

Use [Stage2_IMU_Test instructions](../Stage2_IMU_Test/README.txt) in the separate upload/capture workflow to obtain identity/configuration and slow raw orientation/motion evidence. This chat remains programming-only. Bring-up is I1 only; prepare I2 FIFO acquisition next with the throughput/fault policies in the [design review](stage2-design-review.md), then I3 anchors and I4 storage. No combined timing/continuous-rate acceptance yet. Older recorder/optical gates remain open.

## Open evidence and decisions

- Imported Stage 0A sketch line 1 contains `no/*`; preserve historical input and correct only in the new Stage 0B sketch. Its exact imported contents have not been build-qualified.
- Arduino CLI 1.5.1, installed ESP32 core 3.3.12, compile target esp32:esp32:esp32. Physical board: ESP32-D0WD-V3 revision 3.1, MAC 28:05:a5:35:1c:90.
- Connected chip confirmed; Arduino-ESP32 core 3.3.12.
- Button module wiring and output voltage behaviour require inspection.
- Small SDSC capacity is approximately 0.01 GiB according to supplied results.
- The 58 ms observed write is not an upper latency bound.
- Firmware compile and host checks passed; upload hashes verified at 115200 baud. Later user-reported RUN0002 recording demonstrates storage available on that run; one missed slot leaves acceptance pending.

Phase 0B indicator now uses the built-in LED (board definition or GPIO2 fallback), with configurable polarity; no separate recording LED required. Actual board mapping remains to be verified.

## Latest reported bench evidence

[Firmware 0.2.2 RUN0002](evidence/2026-10-22-phase0b-run0002.md): 5.994734 s, 5995 eligible slots, 5994 generated/persisted, 71928 bytes, one missed slot and zero drops. Button start/stop reported successful. Read-back outcome unresolved by combined error; host validation, LED polarity, 60-second and insufficient-space tests remain pending. Current card reports 31323062272 bytes; do not treat it as the original tiny SDSC. Repository 0.2.3 is a documentation update, not a new tested firmware build.

## 0.2.4 correction awaiting bench retest

Service the current pending synthetic slot at stop instead of unconditionally counting it missed; genuine earlier gaps remain errors. Split write/read-back/accounting/continuity diagnostics and compare full-file CRC with submitted bytes independently of sequence gaps. Boundary/CRC host regressions and ESP32 compilation pass. This addresses a code-level explanation for RUN0002, not a confirmed diagnosis of that run. Upload and physical retest remain pending.

## Latest bench evidence — 2026-10-22

[Full RUN0003 statistics](evidence/2026-10-22-stage0b-v0.2.4-statistics.md): v0.2.4 upload verified; SD mounted; 7.646714 s, 7,647 generated/persisted samples, 91,764 bytes, zero misses/drops, write/read-back/accounting/continuity PASS. Short-session success only. Actual recording host validation, 60-second sessions and remaining acceptance matrix are open.

## Latest captured result — RUN0003 supersedes RUN0002 as the current run

Firmware 0.2.4 passed the captured 7.646714-second button-controlled session: 7647 eligible/generated/persisted samples, 91764 bytes, write/read-back/accounting/continuity all PASS. Peak queue 528 bytes (4.30%); longest observed write 44.064 ms. See the linked report and raw Serial capture above. RUN0002 remains preserved as historical failed evidence.

Next gates: independent validation of the actual recording, approximately 60-second recording, repeated button sessions and reboot numbering preservation, physical LED/polarity, and storage-full/stall qualification. No per-sample timing evidence exists. Firmware unchanged; this update reconciles the summary with the already committed evidence.

## Stage 1 standalone development — 0.3.0

User requested optical work next, overriding the development-order gate for this standalone test only. Stage1_Optical_Test contains sketch, local filter helper and README.txt with exact pins, electrical checks, build/upload and manual/controlled-speed procedures. Optical count/interval/RPM firmware and host filter tests compile/pass; no upload or optical hardware result collected. Stage 0B acceptance remains open; Stage 2 and combined logging remain unimplemented.

## Stage 1 bench observation — 2026-10-22

[Report and statistics](evidence/2026-10-22-stage1-v0.3.0-report.md): v0.3.0 upload hashes verified; finger cover/uncover produced 23 accepted/51 rejected edges, then stable counters. Input response observed only. Known-count passages, measured setup and controlled-speed accuracy remain pending. Installed firmware remains 0.3.0.

## Evidence review — 0.3.2

Reconciled optical report against all 133 Serial snapshots: final 23 accepted and 51 rejected edges, with 123 final snapshots retaining those counters. No new measurement or firmware change. Prior no-upload statements describe earlier milestones; latest captured installed firmware is optical 0.3.0. Next optical check is a known count of mark passages (for example 20), with fixed measured gap/lighting and stationary false-count observation before controlled rotation. Do not equate rejected electrical transitions with lost revolutions or tune the filter without these measurements.

## Optical single-flash correction — 0.3.3

User observed leading/trailing flashes. Stable-cycle qualification now counts only detected state, rearming only after stable clear. Both levels need 1000 us dwell; task timer checks at 250 us, preserving candidate transition timestamps. Raw transitions are shown separately. Host tests cover boot-covered, one event per hold, trailing bounce and rearming; compile passes. Latest captured installed firmware remains 0.3.0; upload and physical retest of 0.3.3 pending. Long chatter or incorrect polarity still require measured setup rather than assuming a verified fix.

## Wireless proposal — 0.3.4

Added [wireless plan](wireless-control-plan.md) at user request: local password-protected Wi-Fi/browser monitoring and explicit start/stop, physical fallback, bounded networking and disconnect-safe autonomous recording. Includes independent stage packaging and host/browser/60-second/range/storage acceptance gates. Planning only; no firmware changes or additional acceptance. Current optical 0.3.3 physical retest and earlier recorder gates remain open.

## Updated optical rerun — 2026-10-22

[Updated-code report](evidence/2026-10-22-stage1-v0.3.3-rerun-report.md): uploaded firmware 0.3.3 from repository 0.3.4, verified flash; 110 accepted, 1,413 raw transitions, zero rejected qualified detections, stable final counters. Actual cover/uncover count ambiguous; no 20-cycle acceptance claimed. Full optical accuracy remains pending.

## Exact-count manual optical check — 2026-10-22

[20-cycle report](evidence/2026-10-22-stage1-exact20-report.md): **PASS**, 20 user-confirmed finger cover/uncover cycles produced 20 accepted detections, zero qualified rejections, 138 raw transitions; counters stable after stopping. Firmware remains 0.3.3. Controlled rotation/RPM and full optical acceptance remain pending.

## Rotating white-mark session controls — 0.3.8

Prepared GPIO32 debounced start/stop for the standalone optical test; each start resets counters, stop freezes them and prints a summary. Built-in LED stays on during testing with a brief OFF pulse per accepted mark, off when idle/stopped. Added button wiring and sensitivity-pot/white-mark procedure to the standalone README. Code/build only in this chat; upload and physical capture belong to the separate workflow. Host tests and compile pass; latest captured hardware remains 0.3.3. New physical controls/indicator and rotating-mark qualification remain pending.

## Button-controlled optical bench — 2026-10-22

[Report](evidence/2026-10-22-stage1-button-report.md): v0.3.8 upload verified; physical button START/STOP observed, 14.099 s, 12 detections, zero qualified rejections, 88 raw transitions. Post-stop counters frozen. LED physical confirmation and requested SD writing remain open; standalone sketch does not write SD. Firmware remains 0.3.8.

## Optical SD event logging — 0.4.0

Prepared standalone Stage 1 optical+SD test by explicit user request (no IMU integration). Accepted optical timestamps/sequence numbers pass through a 4 KiB queue to a dedicated storage task. Button starts a new dated file; stop drains, closes and CRC-verifies. Serial TIME sets UTC naming; unset clock uses labelled boot/uptime fallback. Added standalone validator and SD wiring/bench instructions. ESP32 compile and host integrity/session/filter checks pass. No upload or hardware operation here; optical+storage load, 20-revolution file validation, SD-full behaviour and timestamp naming need physical qualification in the separate workflow. Earlier Stage 0B gates remain open.

## Clocked optical storage bench — 2026-10-22

[Report](evidence/2026-10-22-stage1-sd-button-report.md): v0.4.1 fixes missing '/' in SD paths. Verified upload and UTC clock acknowledgement; button START/STOP saved 11 events/176 bytes with zero queue drops, write/read-back/accounting PASS. Requested 10-cycle comparison is inconclusive because user reported possible manual error and deferred edge cases. Physical LED confirmation and independent actual-file validator remain open. Full optical/Stage 0B acceptance is not implied.

## Optical SD evidence review — 0.4.2

Firmware 0.4.1 capture confirms 11 accepted/persisted events, 176 bytes, zero queue drops, CRC32 d0aa5ebe and write/readback/accounting PASS. User clarified informal finger passages; no confirmed optical miscount diagnosis. Binary remains on ESP32 SD, so host file/sequence/timestamp/CRC validation and file hash are pending copy through the separate hardware workflow. See [updated storage report](evidence/2026-10-22-stage1-sd-button-report.md) for exact validator command. Firmware unchanged; no hardware operations here.

## Independent optical file validation — 0.4.3

Actual evidence file OPT_2026-10-22_14-45-28_0001.BIN passes the host validator: 11 events, 176 bytes, contiguous sequence 1..11, increasing timestamps, CRC d0aa5ebe matching Serial. Intervals agree with captured min/max/final statistics; recording SHA-256 and decoded events are preserved in the [storage report](evidence/2026-10-22-stage1-sd-button-report.md). This closes the pending host-validation gate for this recording. Physical rotation accuracy remains unqualified; firmware remains 0.4.1. No upload or hardware operation.

## Analysis bench planning — 0.4.4

Added [analysis bench software](analysis-bench-plan.md) to the roadmap: offline optical import/validation, event/interval/RPM inspection, comparison and exports first; IMU traces, spectra and order tracking later when timing/format prerequisites exist. Reuse existing validators and retain source bytes/metadata. No application implementation or new dependencies; firmware unchanged. Also reconciled the stale progress header with the captured 0.4.1 firmware evidence.

## Next-phase planning — 0.4.5, 2026-10-23

Added sequenced bench qualification and first offline analysis work, software/physical acceptance criteria, risks and conditional Stage 2 entry. This is a plan only; no code, dependencies, hardware operation or acceptance change. Latest captured optical firmware remains 0.4.1.

## Revised next-phase planning — 0.4.6

User wants GY-521 first, then marked rotating shaft and mechanical click/tap for optical/IMU synchronisation. Revised the plan, including mechanical coupling, precise intended pins, separate acquisition/combined gates, timing/filter/physical delay limits, known-angle phase-change experiment and evidence criteria. Analysis-first ordering superseded. No new code, hardware operation or acceptance; latest captured firmware stays 0.4.1.

## Illustrated timing protocol — 0.4.7

Prepared [NVH-TIME-01](experiments/timing-protocol.md) and a seven-page [printable PDF](../output/pdf/NVH-TIME-01-experiment-protocol.pdf), with four vector figures, five test sequences, delay/reference ledger, uncertainty rules and a run worksheet. Covers optical input/qualification, GY-521 sampling/filter response, marked-shaft contact phase, drift, SD/FIFO and button response. Clearly distinguishes combined repeatability from independently verified component latency. Stage 2/combined instrumentation still pending; no experiments executed. PDF text checks, all-page visual review, source syntax, arithmetic, links and version/whitespace checks passed. No dependencies installed or firmware changed.

## GY-521 coding outline — 0.4.8

Added [implementation breakdown](stage2-coding-outline.md): standalone Stage2_IMU_Test folder, bring-up first, then FIFO, timing observations, SD owner and host inspection. Defined bounded bus errors, raw-frame types, lifecycle and format decisions to freeze. I1 sensor bring-up is next. Outline only; no firmware placeholders, compilation claim or physical operation. Latest captured optical firmware remains 0.4.1.

## Reviewed design and first GY-521 code — 0.5.0

Resolved snapshot-versus-FIFO layout, address/identity, bounded I2C operations, bus throughput, stop/fault policy and timing/format gates in the outline and design review. Added independent Stage2_IMU_Test bring-up firmware, README and real-driver host tests: fixed configuration/read-back, fresh ~10 Hz register snapshots, raw and uncalibrated scaled axes. ESP32 core 3.3.12 build without warnings, copied-folder build, sanitized host tests, Python regressions and Clang static analysis pass; independent review found no blocker. No upload or physical sensor operation. One-second freshness and five-second health-loop scheduling lack sketch-level host coverage and remain explicit verification gaps. Installed physical evidence remains optical 0.4.1.

Local I1 build/test details: [2026-10-23 verification record](evidence/2026-10-23-stage2-i1-local-checks.md). This is software evidence only.

## Wheel-balancer side project — 0.5.1

Added [separate concept](../side-projects/wheel-balancer/README.md) with no SD requirement, motor/encoder/vibration/display controls, calibrated correction estimates, two-plane target and guarded indexing. Sources checked for balancing principles and machinery safeguards. Target-wheel envelope, mechanics, safety design, components, implementation and all physical evidence remain open. Documentation links/version/whitespace pass; logger stage acceptance unchanged.

## Wheel-balancer CAD and PCB scope — 0.5.2

Expanded the [side-project concept](../side-projects/wheel-balancer/README.md) with required mechanical CAD and electronics/PCB development, manufacturing outputs, revision control and shared fit/wiring interfaces. Defined review gates before fabrication and separate as-built/board qualification. Requirements and design work remain pending; no CAD, circuit design, firmware or hardware implementation delivered. Local links/version/whitespace checks pass.
