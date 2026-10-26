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
- Cumulative revolution plot or raw RPM with an optional median overlay.
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

The CSV contains reader version, source identity/CRC, checked reference values, integrity state, all analysis settings, raw sequence/timestamp, interval, raw/median RPM and review flags. Empty-file exports contain column headers only. This release has no session comparison, plot export, saved settings, editing/deletion or live controls.

## Poor or incomplete recordings

Structural faults (sequence gaps/duplicates, non-increasing times or a partial trailing record) are prominently marked **DIAGNOSTIC ONLY**. The complete-record prefix is still available for inspection. Unsafe numeric precision or oversized files are rejected rather than rounded silently.

A structure pass is not proof of a clean stop, correct physical detection or storage completeness: current files have no embedded completion footer. Calculated CRC alone is a fingerprint, not an independent check. Supply BOTH original expected count and CRC from the Serial summary for a reference match. Missing references remain unverified; mismatches remain visible. An empty run can be structurally valid without proving the sensor worked.

## Verification

```sh
python3 -m unittest discover -s tests -v
node --check analysis-bench/web/app.js
```

Browser interaction checks are in `tests/browser_reader_checks.js`, a Playwright page-function harness used with the existing browser tool. They assume the six supplied recordings and a server at port 8765, write only derived artifacts to `.playwright-mcp/`, and install no framework. The Python tests independently cover decoding, reference mismatches, malformed/empty data, read-only sources, limits and HTTP path confinement.

Local browser tests cover settings, raw/median values, ranges, selection/focus, count/CRC mismatch, empty/diagnostic views, export and responsive layout. This is software evidence only. It does not qualify the shaft sensor or any hardware experiment.
