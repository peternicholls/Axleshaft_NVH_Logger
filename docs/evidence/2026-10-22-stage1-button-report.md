# Stage 1 button-controlled optical test

Date: 2026-10-22, Europe/London. Installed firmware banner 0.3.8. Compile and verified upload succeeded, Arduino-ESP32 3.3.12, target esp32:esp32:esp32, serial/upload 115200 baud. ESP32-D0WD-V3 revision 3.1, MAC 28:05:a5:35:1c:90. Build: 276,684 flash bytes, 22,740 static RAM bytes. C++17 optical session host checks passed with -Wall -Wextra -Werror -pedantic.

## Observations

- Initial running=0, counters zero.
- Physical button triggered START; running became 1.
- Optical input produced accepted detections while running.
- Physical button triggered STOP; running became 0 and counters remained frozen in subsequent snapshots.
- User replied “done”. Physical LED on/pulse/off behavior was requested but not explicitly confirmed separately.

| Metric | Observed |
|---|---:|
| Session duration | 14,099,000 us / 14.099 s |
| Accepted detections | 12 |
| Rejected qualified detections | 0 |
| Raw transitions | 88 |
| Last accepted timestamp since boot | 21,421,197 us |
| Last interval | 714,716 us |
| Minimum interval | 688,053 us |
| Maximum interval | 1,745,214 us |
| Average RPM display | 66.54 |
| Final instantaneous RPM | 0.00 |
| Final input level | 1 |
| Final running flag | 0 |

No specific count was required. RPM values represent detection spacing under an assumed single mark per revolution, not calibrated rotation measurements.

## Scope and limits

**Button start, optical activity and button stop observed successfully.** The firmware drives its LED on while running, with 30 ms off pulses after accepted detections and off while stopped. Physical LED behavior remains unconfirmed from the user's completion-only reply.

**No SD write or file-close test occurred.** This standalone sketch contains no SD initialization, recording or file output. The requested writing portion remains outstanding and cannot be inferred from serial activity. No recording-file statistics are available. No firmware changes were made during this upload/test; concurrently developed source edits were preserved. Full optical rotation and prior Stage 0B acceptance gates remain open.

[Raw serial capture](2026-10-22-stage1-button-serial.txt).
