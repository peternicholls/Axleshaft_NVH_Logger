# Progress

Last updated: 2026-10-22. Repository version: 0.1.1.

## Current position

Stage 0A: accepted in the supplied handover; not rerun in this repository session.
Stage 0B: detailed [implementation plan](stage0b-plan.md) complete; implementation has not started.

| Milestone | Status | Evidence / remaining gate |
|---|---|---|
| Repository baseline | Complete | Original files committed as f9b294a |
| Documentation and delivery rules | Complete | Version 0.1.0 documentation checks; see development log |
| Stage 0A storage | Historical pass | Handover: 262,144 bytes verified; 2.93 s write; 58 ms maximum observed block write |
| Stage 0B planning | Complete | Version 0.1.1; design, work sequence and measurable test matrix |
| Stage 0B recorder | Next | Firmware, validator, host checks, build and bench acceptance pending |
| Stage 1 optical | Gated | Requires Stage 0B acceptance, then standalone sensor evidence |
| Stage 2 IMU | Gated | Requires Stage 0B acceptance, then standalone acquisition evidence |
| Stage 3 integration | Gated | Requires accepted optical and IMU stages |
| Vehicle trials | Not started | Requires integrated bench acceptance and secured mounting/wiring |

## Immediate next work

1. Record available Arduino toolchain and exact board/package configuration.
2. Define the Stage 0B record layout, deterministic pattern and numerical scheduling tolerance before implementation.
3. Implement and verify the Stage 0B pipeline using the known working SPI settings.
4. Collect bench evidence against every acceptance item before opening subsequent stages.

## Open evidence and decisions

- Imported Stage 0A sketch line 1 contains `no/*`; preserve historical input and correct only in the new Stage 0B sketch. Its exact imported contents have not been build-qualified.
- Arduino CLI is available; installed core/FQBN inspection remains pending.
- Exact ESP32 variant and Arduino-ESP32 package version remain unconfirmed.
- Button module wiring and output voltage behaviour require inspection.
- Small SDSC capacity is approximately 0.01 GiB according to supplied results.
- The 58 ms observed write is not an upper latency bound.
- No new build, upload or hardware measurement has been performed.
