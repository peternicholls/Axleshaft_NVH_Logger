# Progress

Last updated: 2026-10-22. Repository version: 0.2.6 (installed firmware: 0.2.4).

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
| Stage 1 optical | Gated | Requires Stage 0B acceptance, then standalone sensor evidence |
| Stage 2 IMU | Gated | Requires Stage 0B acceptance, then standalone acquisition evidence |
| Stage 3 integration | Gated | Requires accepted optical and IMU stages |
| Vehicle trials | Not started | Requires integrated bench acceptance and secured mounting/wiring |

## Immediate next work

1. Follow the standalone Stage0B_Recorder instructions to confirm wiring and upload to the actual ESP32 port.
2. Run the 60-second button recording and several short sessions; retain Serial summaries and host validator results.
3. Test disposable-card insufficient-space behaviour and log results before opening Stage 1/2.

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
