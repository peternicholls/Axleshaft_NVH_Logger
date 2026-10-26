# Rotation bench reader

Basic local GUI for optical event recordings. No accelerometer, internet, cloud account or extra packages required. Python 3.9+ and a modern browser are sufficient. Keep this folder in the repository: it reuses `Stage1_Optical_Test/validate_optical.py` as its format validator.

## Start

From the repository root:

```sh
python3 analysis-bench/reader.py --open
```

Or double-click **StartReader.command** on macOS. The local reader opens at [http://127.0.0.1:8765](http://127.0.0.1:8765). Stop its Terminal process with Ctrl+C. If that port is occupied, use `--port 8766`. Nothing connects to or uploads the ESP32.

The reader lists `.BIN` recordings under `output/runs`, including nested folders. Copy more files there and choose **Refresh recordings**. Raw BIN files remain ignored by Git unless deliberately retained as evidence. A clean checkout contains the folder instructions but no raw runs: copy your files there, or inspect the retained fixture with:

```sh
python3 analysis-bench/reader.py --runs-dir docs/evidence --open
```

Use `--runs-dir /absolute/path/to/recordings` for a different folder. Only the configured folder is exposed; the server binds to loopback only. It does not change source files.

## What the GUI shows

- Recording list and detected revolution count, assuming one detection/revolution by default.
- RPM plot by default, with optional median overlay and a revolution-count view.
- Clickable events, raw sequence numbers, elapsed time, intervals and review flags; 100 table rows per page.
- Selected time range, flagged-only table view and derived CSV export.
- File size, calculated CRC32 and SHA-256, structural checks, and optional expected count/CRC checks from Serial evidence.

The current reader explicitly assumes **optical-events-v1**: 16-byte little-endian records containing sequence and elapsed microseconds. It does not auto-identify other headerless formats. Files are limited to **1 MiB / 65,536 complete events**. This first-reader limit is smaller than the longer-term bench proposal. Larger files get an explicit error. Large plots are reduced to labelled min/max RPM buckets; table and export retain all records in the selected range.

## No date/time code is needed

The UNSET files lack wall-clock dates but contain relative microsecond event timestamps. These support time intervals and derived RPM. The GUI does not infer recording date from file modification time, the filename or computer connection. Each recording keeps its own elapsed time domain.

Event span means first-to-last detection, not full recording duration. There may have been waiting before the first or after the last event. The first detection has no interval/RPM; empty files show no speed estimate.

## Tune analysis without repairing the data

| Control | Meaning |
|---|---|
| Detections / revolution | Integer 1–100. Scales inferred revolution count and RPM; does not change raw counts |
| RPM median window | Odd 1–101 events, centred with shorter windows at ends; 1 is raw. Raw RPM remains displayed alongside the median |
| Unusual interval ratio | Global positive-interval median divided/multiplied by the ratio. Flags unusually fast/slow intervals; default 3 |
| Short interval below | Millisecond threshold for a review flag, default 0 (disabled). Does not remove or merge detections |
| Display range | Elapsed seconds since test start, supporting microsecond precision. Limits plot/table/export; headline file statistics cover the full file |

Flags are **heuristics**, not diagnoses. Real acceleration, pauses and variable speeds may be flagged; a global median can be unsuitable for mixed-speed runs. Tune the settings and inspect raw events instead of treating the flag count as a lost/extra revolution count. Filtering the table never removes events from the plot or underlying source. CSV exports all complete records in the selected time range, even if the table is flagged-only.

The CSV contains reader version, source identity/CRC, checked reference values, integrity state, all analysis settings, raw sequence/timestamp, interval, raw/median RPM and review flags. Empty-file exports contain column headers only. This release has no session comparison, plot export, saved analysis settings, binary editing/deletion or live controls.

## Poor or incomplete recordings

Structural faults (sequence gaps/duplicates, non-increasing times or a partial trailing record) are prominently marked **DIAGNOSTIC ONLY**. The complete-record prefix is still available for inspection. Unsafe numeric precision or oversized files are rejected rather than rounded silently.

A structure pass is not proof of a clean stop, correct physical detection or storage completeness: current files have no embedded completion footer. Calculated CRC alone is a fingerprint, not an independent check. Supply BOTH original expected count and CRC from the Serial summary for a reference match. Missing references remain unverified; mismatches remain visible. An empty run can be structurally valid without proving the sensor worked.

## Verification

```sh
python3 -m unittest discover -s tests -v
node --check analysis-bench/web/app.js
```

Browser interaction checks are in `tests/browser_reader_checks.js`, a Playwright page-function harness used with the existing browser tool. They assume the six supplied recordings and the isolated QA server at port 8766, write only derived artifacts to `.playwright-mcp/`, and install no framework. The Python tests independently cover decoding, reference mismatches, malformed/empty data, read-only sources, limits and HTTP path confinement.

Local browser tests cover settings, raw/median values, ranges, selection/focus, count/CRC mismatch, empty/diagnostic views, export and responsive layout. This is software evidence only. It does not qualify the shaft sensor or any hardware experiment.

## Movable time focus and RPM spread (0.6.1)

Drag left-to-right across the overview to select an interval. Drag the selected band to move it without changing width, or drag its edges to resize it. The two range sliders provide equivalent keyboard controls. Numeric From/To fields and Full range remain available. Plot, table, CSV range and focused statistics follow the selection; the headline file statistics stay whole-file.

The focused statistics use **raw** interval-derived RPM, including review-flagged values. Only intervals with both endpoints inside the selected window are included; a partial interval at the left edge is excluded. Empty windows show no estimate. One complete interval has zero population spread but does not establish physical stability. Structurally invalid recordings remain diagnostic and do not receive valid-looking focus statistics.

- Arithmetic mean and median of interval RPM.
- Population variance = sum((RPM - mean)^2) / N, in RPM squared.
- Standard deviation = square root of variance, in RPM.
- Coefficient of variation = SD / mean x 100, as a percentage.
- Minimum and maximum RPM.
- Overall rate = revolutions represented by complete intervals / their summed duration. This time-weighted rate differs from the arithmetic mean when speed varies.

The calculation uses stable incremental variance accumulation. Median smoothing affects the plotted overlay only; it never changes these raw statistics. Detections/revolution scales RPM and SD linearly and variance quadratically.

## Rename, mark and hide recordings

**Rename label** stores a display name, **Mark recording** adds/removes a star, and **Hide recording** removes a run from the default library. Enable **Show hidden recordings**, select it and choose **Restore recording** to bring it back. Blank display name restores the original filename. Original filenames remain in file details and exports.

These actions modify separate reader metadata, never BIN bytes or filenames. Metadata is bound to the source folder, relative name and SHA-256; a changed file does not silently inherit an old label. Changes persist across reader restarts in `analysis-bench/.reader-state.json`, which is ignored by Git. Keep/backup that file if you need to retain organisation. Use `--state-file /path/to/state.json` for a separate library/test instance. Run one reader process per state file; concurrent requests in one server are locked, but independent processes should use different state files.

Malformed saved state is preserved and reported rather than silently reset. A failed save leaves the previous metadata file intact and displays an error. Display labels are limited to 80 characters and CSV formula-leading label/name characters are escaped as text on export. Hidden files are still present on disk and can be restored; this is not a deletion feature.

Additional checks:

```sh
node tests/test_rotation_math.js
```

Both browser harnesses use the isolated QA server at port 8766 so metadata tests cannot change your normal library:

```sh
python3 analysis-bench/reader.py --port 8766 --state-file .playwright-mcp/reader-focus-test-state.json
```

Create `.playwright-mcp` first if absent. `tests/browser_reader_focus_checks.js` covers focus dragging/sliders, spread calculations, RPM default and metadata persistence/restore. The previous harness provides the original reader regressions.
