# Stage 1 exact 20-cycle bench test

Date: 2026-10-22, Europe/London. Installed firmware banner 0.3.3, uploaded in the preceding test from source revision 61d2fd09427936a967fc6d68cd0bad9d59da30ef. No firmware changes or upload in this run; hardware reset began a new capture at zero counters. Documentation release 0.3.6.

**PASS: 20 user-confirmed manual cover/uncover cycles produced exactly 20 accepted detections.**

User was instructed to begin clear, perform exactly 20 complete finger cover/uncover cycles, then leave clear, and replied “done”. Serial began level=1 with zero counters; level 0 and 1 occurred during movement. Final counters remained unchanged after completion. This is a manual finger bench test, not calibrated rotation acceptance.

| Measurement | Result |
|---|---:|
| Expected manual cycles | 20 |
| Accepted detections | 20 |
| Count difference | 0 |
| Rejected qualified detections | 0 |
| Raw transitions, both polarities including chatter | 138 |
| Final input level | 1 (clear) |
| Last accepted timestamp since boot | 29,487,788 us |
| Last accepted interval | 418,269 us |
| Minimum accepted interval | 391,662 us |
| Maximum accepted interval | 632,032 us |
| Average RPM display | 133.44 |
| Final instantaneous RPM display | 0.00 |
| Final stale flag | 1 |

GPIO26 active LOW. Detection and clear must each remain stable for 1,000 us; task qualification timer period 250 us. Raw transitions include chatter and both cover/clear edges. Zero qualified rejections does not imply no raw chatter. RPM assumes one detection per revolution; here it represents finger-cycle spacing only. No calibrated speed claim.

Counters stayed at 20 accepted/138 raw after motion stopped. Physical LED flash count was not separately confirmed for this run. Sensor gap, output/supply voltage, lighting, target surface and actual rotation speed were not measured. Controlled mark/revolution accuracy across intended RPM, pulse-width qualification and full Stage 1 acceptance remain pending. Outstanding Stage 0B checks remain open.

[Raw serial capture](2026-10-22-stage1-exact20-serial.txt).
