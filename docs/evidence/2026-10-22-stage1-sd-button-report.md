# Stage 1 SD/button/clock bench report

2026-10-22, Europe/London. Firmware 0.4.1. Source baseline 52dfb76c5e6f9e1ef607ae98493c0e7f1ae7ce2f plus the absolute-path fix in this delivery. ESP32-D0WD-V3 revision 3.1, MAC 28:05:a5:35:1c:90; Arduino CLI 1.5.1, ESP32 core 3.3.12, target esp32:esp32:esp32. Compile: 349,805 flash bytes, 28,552 static RAM bytes. Upload 115200 baud; flash hashes verified. Firmware application SHA-256 `10cfd2fc369d43c6ee2d162610aaf3243fee8507bcca7e29b6fdcba74fd63174`.

## Fix and startup

Original 0.4.0 mounted SD and acknowledged TIME, but failed creating the file. Inspection of installed FS/src/vfs_api.cpp confirmed paths must begin with '/'. Filename helper lacked that slash. Updated regression expectations failed before the fix and passed after adding '/'; basename/format unchanged. Corrected firmware 0.4.1 compiled, uploaded and restarted. Host sent TIME 1792680309 and firmware acknowledged UTC clock set. Host-supplied time is not independently measured clock synchronization. Set time again after future resets.

## Session result

**Storage/button test PASS; exact physical count inconclusive.** Requested 10 finger cover/uncover cycles; firmware accepted and saved 11. User subsequently stated the discrepancy might have been their fault and deferred count-edge cases until later. Do not classify this as a confirmed sensor defect or a 10-cycle acceptance pass.

| Metric | Result |
|---|---:|
| SD recording | /OPT_2026-10-22_14-45-28_0001.BIN |
| Filename time zone | UTC |
| Duration | 16,847,223 us / 16.847223 s |
| Accepted detections | 11 |
| Persisted events | 11 |
| Bytes | 176 |
| Raw transitions | 85 |
| Rejected qualified detections | 0 |
| Queue drops | 0 |
| Peak queue | 1 event / 16 bytes |
| Queue capacity | 256 events / 4,096 bytes |
| Maximum write call | 22 us |
| Flush and close | 53,464 us / 53.464 ms |
| CRC32 | d0aa5ebe |
| Write / read-back / accounting | PASS / PASS / PASS |
| Minimum accepted interval | 47,260 us |
| Maximum accepted interval | 1,473,034 us |
| Last accepted interval | 681,691 us |
| Average RPM display | 77.83 |

Physical button START opened a timestamped file; STOP drained, closed and read-back verified the file. Final state returned to Idle with counters unchanged. The 47.260 ms interval may reflect an extra physical cycle or an extra optical detection; origin unverified. RPM is event-spacing under an assumed one-event-per-revolution, not calibrated rotation.

Physical LED on/pulse/off was requested but not separately confirmed by the completion-only reply. No computer copy of the SD file was available, so independent sequence/timestamp/CRC validator execution on this recording remains pending. All 11 accepted events were persisted with zero queue loss and firmware CRC read-back PASS. Host Python validator tests and C++ optical storage regression checks passed. Exact-count edge cases are explicitly deferred by the user; controlled rotation and remaining Stage 0B/full acceptance gates remain open.

Raw captures: [initial 0.4.0 failure](2026-10-22-stage1-v040-sd-serial.txt), [corrected 0.4.1 session](2026-10-22-stage1-v041-sd-serial.txt).

## Follow-up review and user clarification — repository 0.4.2

Reviewed raw 0.4.1 Serial capture: 11 accepted/persisted events, 176 bytes, zero queue drops and CRC32 d0aa5ebe agree with this report; write/readback/accounting all explicitly PASS. User clarified that this was informal finger passing, accounting for the imprecise physical count. Treat it as a storage/button test, not a calibrated 10-revolution test or confirmed optical defect.

User confirmed the binary remains on the ESP32's SD card. No computer-side file is currently available for independent inspection. This chat performs no hardware operation/upload. After the separate workflow copies the file to a computer, validate with:

```sh
python3 Stage1_Optical_Test/validate_optical.py /path/to/OPT_2026-10-22_14-45-28_0001.BIN --expected-events 11 --expected-crc d0aa5ebe
```

Retain the original file and capture. Expected file size is 176 bytes. File SHA-256, independent sequence/timestamp checks and host CRC verification remain pending; no claim of computer-side validation is made.

## Independent recording validation — repository 0.4.3

The actual [recording](OPT_2026-10-22_14-45-28_0001.BIN) is now available in evidence. Ran the existing standalone validator against expected_events=11 and expected_crc=d0aa5ebe:

```text
PASS: 11 sequential events, 176 bytes, increasing timestamps, CRC verified
```

Recording SHA-256: `66cb58696b546e97e47509218deb2af37229fd2af8b96ac94c5b0a73e61677e2`.

Decoded sequence is exactly 1..11. All timestamps are within the captured 16847223 us session duration. Inter-event minimum 47260 us, maximum 1473034 us, final interval 681691 us and derived mean event-spacing RPM 77.833792 match the Serial report (display rounds to 77.83). This confirms storage/transfer integrity and sequence continuity for accepted events; it does not prove physical revolution count or speed in this informal finger test. The short interval occurs between records 7 and 8; no physical cause is inferred.

| Sequence | Elapsed microseconds from START |
|---|---:|
| 1 | 1329668 |
| 2 | 2289436 |
| 3 | 3762470 |
| 4 | 4575245 |
| 5 | 5439029 |
| 6 | 6203687 |
| 7 | 6872315 |
| 8 | 6919575 |
| 9 | 7661991 |
| 10 | 8356711 |
| 11 | 9038402 |

This supersedes the earlier pending computer-copy/host-validation status for this recording only. Firmware unchanged; no new hardware operation. Physical LED, controlled white-mark rotation, speed-range and remaining storage qualification gates remain open.
