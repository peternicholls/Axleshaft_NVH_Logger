# Basic rotation reader — local software checks

Date 2026-10-26, repository/software 0.6.0. No firmware changes, ESP32 connection, upload or physical operation. These recordings have relative event microseconds but no verified wall-clock date. No original Serial count/CRC references were supplied for these six files; structural pass is not completion or physical revolution acceptance.

## Real inputs inspected

| File | Events | Bytes | Calculated CRC32 | Source SHA-256 |
|---|---:|---:|---|---|
| OPT_UNSET_31c690b3_UP100_0001.BIN | 0 | 0 | 00000000 | e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855 |
| OPT_UNSET_31c690b3_UP114_0001.BIN | 60 | 960 | 536abd7b | 51c13bc96807899e189a98ecacc85bb0badd0959f8b9c9b12c786baf06fd23ba |
| OPT_UNSET_cd77abcc_UP19_0001.BIN | 346 | 5536 | f0bed4ef | f06eb46f304773fe9f19f9e11728d6e59952bd66ebae45732d026712ca8c58b7 |
| OPT_UNSET_cd77abcc_UP74_0001.BIN | 17 | 272 | a2c9b496 | 280aa9f466e3039167b9ed3d88f8a703c88c2f408dc97428cc22eea8b1489400 |
| OPT_UNSET_e3fab4b9_UP471_0001.BIN | 47 | 752 | 049eab66 | 5b55c8b925c67c82d6833e1134b0a3851a152994cfc6597fc9e286e1ccc655eb |
| OPT_UNSET_e3fab4b9_UP484_0001.BIN | 92 | 1472 | 6587ae40 | db6bc170ce5dbbfd37716a6c87b9ceaaf2af7fafdd58df6cf407484ef7aff137 |

All six opened in the GUI with matching event counts. The empty run shows no invented RPM. Original files remain under output/runs, ignored as raw recordings; this evidence records identities without changing them.

## Verification

Local runtime: Python 3.14.9, Node 26.12.1 (syntax checks only), browser Chrome 149.0.0.0. Desktop viewport 1440x1000; mobile emulation 390x844. No browser/device compatibility beyond these local checks is claimed.

- `python3 -m unittest discover -s tests -v`: 11 test methods pass (nine reader-backend tests plus two existing validator groups). Includes retained 11-event reference fixture, synthetic 600 RPM, malformed/truncated/duplicate/timestamp cases, count/CRC mismatch, empty/single input, limits, read-only inputs, HTTP path/origin confinement and local-only startup.
- `python3 -m py_compile analysis-bench/reader.py tests/test_reader.py` and Node syntax checks on app.js/browser harness pass. No external Python/JS dependencies added.
- Real-browser harness `tests/browser_reader_checks.js`: 22 checks pass, including settings, raw/median values, review thresholds, selected ranges, event selection/focus, optional count/CRC references, incorrect CRC, CSV download, empty/diagnostic views, mobile controls and no JavaScript errors.
- Browser-exported CSV independently compared with all 346 original sequence/timestamp fields. Reader version, source CRC/hash, checked reference values and analysis settings retained. No row deletion from review flags.
- Synthetic UI check: 100000 us interval at two detections/revolution gives 300 RPM. A 1000 us spike remains raw 30000 RPM while centred window-three median is 300 RPM; one short interval is flagged. Synthetic responses are UI-only fixtures, not hardware evidence; Python tests exercise real bytes separately.
- Maximum-size synthetic UI response: 65536 events / 1 MiB and median window101 re-rendered in about 126 ms on this development Mac/browser. 700 plotted points labelled as downsampled; table/export preserve all records. This is an observation, not a universal performance guarantee.
- Desktop (1440 px) and mobile (390 px) views checked with screenshots. Chart/table scroll within their containers; no horizontal page overflow.
- Independent read-only review identified a stale-file attribution bug under invalid settings. Fixed by rendering each selected file with cached valid settings plus an explicit warning. Browser regression switches files with an invalid window and verifies old events cannot appear under a new name; reviewer confirmed fix by inspection. No remaining blocking review findings.
- Documentation links, version alignment and whitespace checked.

## Visual evidence

[Desktop view](2026-10-26-reader-desktop.png), [mobile view](2026-10-26-reader-mobile.png). Local intermediate browser artifacts and derived CSV are ignored under .playwright-mcp/.

## Remaining scope and limits

Optical-events-v1 only, max 1 MiB per file; no accelerometer, session comparison, automatic correction, saved settings, plot export or live hardware control. Review flags use a tunable global-median heuristic and may flag legitimate speed changes/pauses. Smoothing is display-only; raw values remain visible. Matching user-supplied count/CRC references is not proof of reference authenticity or physical detection accuracy. Actual raw events with unsafe JavaScript numeric precision are rejected explicitly.
