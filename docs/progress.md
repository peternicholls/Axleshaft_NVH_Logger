# Progress

Last updated: 2026-10-22. Repository version: 0.2.2.

## Current position

Stage 0A: accepted in the supplied handover; not rerun in this repository session.
Stage 0B: standalone 1 kHz firmware, instructions and validator implemented. ESP32 compilation and host checks pass; bench acceptance pending.

| Milestone | Status | Evidence / remaining gate |
|---|---|---|
| Repository baseline | Complete | Original files committed as f9b294a |
| Documentation and delivery rules | Complete | Version 0.1.0 documentation checks; see development log |
| Stage 0A storage | Historical pass | Handover: 262,144 bytes verified; 2.93 s write; 58 ms maximum observed block write |
| Stage 0B planning | Complete | Version 0.1.2; design, test matrix and standalone Stage0B_Recorder folder contract |
| Stage 0B recorder | Implemented, bench pending | Core 3.3.12 compile passes; host controls/validator checks pass; ESP32 upload verified; current startup blocked by SD mount/card failure |
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
- Firmware compile and host checks passed; upload hashes verified at 115200 baud. Startup after wiring recheck reports SD mount/card failed; recording acceptance remains pending.

Phase 0B indicator now uses the built-in LED (board definition or GPIO2 fallback), with configurable polarity; no separate recording LED required. Actual board mapping remains to be verified.
