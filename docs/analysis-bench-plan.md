# Analysis bench software — planned

Planning version 0.4.4, 2026-10-22. No bench application implemented.

## Purpose and delivery

A local computer application for opening, checking, inspecting and comparing logger recordings. Start offline with the optical files already available; expand as IMU and synchronised formats become established. This work can begin independently of unfinished hardware qualification, but it must not imply that the sensors or combined logger are accepted.

Prefer a Python analysis engine, following the handover, with a simple local graphical workbench. Select the UI framework at implementation time against the actual environment; no new dependencies or service subscriptions are authorised by this planning entry. Keep processing reusable from a command line and tests. The bench should operate without internet or a live ESP32. Proposed project location: `analysis-bench/`, with its own README, startup instructions, code, tests and small deliberate fixtures.

## First increment: optical recording inspection

1. Import an optical `.BIN` plus its Serial capture or manually supplied expected count/CRC. Identify the format explicitly: the current headerless file cannot identify itself reliably from extension or size alone.
2. Reuse the existing standalone validator as the integrity oracle. Do not create a second set of conflicting format rules. Keep existing stage folders independently usable.
3. Show source filename/path, SHA-256, format, expected/actual count and size, CRC result, completion evidence and firmware/configuration when available. Preserve unknown values as unknown; show a failed or incomplete import distinctly.
4. Present an event table (sequence, elapsed time, interval, calculated RPM), interval/RPM plots and min/max/mean statistics. Allow zoom, range selection and event inspection. Label derived RPM with the assumed detections per revolution; default one and record any user change.
5. Highlight suspect intervals for inspection without deleting events, smoothing away gaps or diagnosing a mechanical fault. Handle zero/one-event files without invented speed estimates. Plot elapsed monotonic time; UTC filenames are labels, not proof of clock synchronisation.
6. Compare two or more sessions with clear legends and explicit alignment choices. Do not automatically merge unrelated clocks or equate finger motion with axle rotation.
7. Export decoded CSV, plots and a concise validation/analysis report, with source hash, selected range, assumptions, software version and parameters. Keep source recordings read-only; write derived artifacts separately.

The existing 11-event recording and its Serial report are an initial regression fixture, not a calibrated RPM reference. Preserve their original bytes and reported checks.

## Later increments

| Increment | Scope | Prerequisite |
|---|---|---|
| IMU inspection | Raw six-axis traces, scale conversion, RMS/peak, clipping and continuity diagnostics, FFT and spectrogram | Documented real IMU configuration, rate/timing and format; distinguish current synthetic data |
| Synchronous analysis | Optical/IMU time alignment, shaft-angle interpolation, order tracking and angular averaging | Validated common timebase and timing anchors; expose interpolation/drift uncertainty |
| Bench replay | Reproducible replay of saved sessions, parameter comparisons and saved analysis settings | Verified import/processing engine |
| Optional live monitoring | Read status/telemetry from planned wireless interface and keep acquisition independent | Wireless API and performance qualification; no raw streaming assumption |

Keep offline analysis useful before live monitoring exists. The bench does not initially control firmware upload, operate hardware, replace SD logging or implement wireless start/stop; the separate wireless plan covers those controls.

## Verification and acceptance

- Import the retained optical fixture: 11 events, 176 bytes, CRC d0aa5ebe, matching recording SHA-256 and report. Minimum/maximum/final intervals must be 47260/1473034/681691 us.
- Generated optical fixtures with 100000 us spacing must show 600 RPM at one detection/revolution, and 300 RPM at two. Cover empty/single-event input, missing/duplicate sequence, non-increasing timestamps, truncation, wrong count and wrong CRC.
- Failed integrity must remain visible in plots/reports; any explicit diagnostic view of failed data is clearly labelled. Never silently repair the source or promote it to a passing recording.
- CSV values must round-trip against decoded source fields; exported reports include source hash, parameters, format and version. Repeat processing with the same inputs/settings must reproduce numeric outputs.
- Demonstrate import, zoom/selection, comparison and export through the chosen UI on the target computer, with clear errors for unsupported formats and missing metadata.
- Define supported file-size/duration and numerical error budgets before implementing later signal-processing stages; check bounded memory, cancelable imports and responsiveness against those limits.
- Before IMU/order features are accepted, validate against known synthetic frequencies/amplitudes/rotational orders, explicit units, filter/window choices, timing gaps and drift. Do not claim these features from optical-only tests.

## Evidence and workflow

Record software version/commit, environment, fixture hashes, test outputs and UI checks in development evidence. Keep progress/version/changelog current per the project workflow. No firmware changes or physical acceptance are part of this planning deliverable.

References: [current formats](data-format.md), [verified optical recording report](evidence/2026-10-22-stage1-sd-button-report.md), [wireless control plan](wireless-control-plan.md), [overall development plan](development-plan.md).

## First delivered reader — 0.6.0, 2026-10-26

Implemented [basic optical GUI](../analysis-bench/README.md) using stdlib Python plus local browser assets, reusing the standalone optical validator. Includes counts, relative timing, count/RPM plots, event table/range, adjustable ratio/window/review thresholds, integrity/reference checks and CSV export. No accelerometer or wall-clock dependency. Initial scope is one file at a time and 1 MiB (65536 events), with explicit errors beyond that; the earlier 10 MiB target is deferred pending broader qualification. Comparison, saved sessions/settings and plot exports remain future work. [Local software evidence](evidence/2026-10-26-reader-local-checks.md) does not qualify physical rotation.
