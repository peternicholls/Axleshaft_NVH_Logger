# Reader focus/library and logger numbering — 0.6.1

2026-10-26. Software checks only. No firmware upload, device operation or fresh physical qualification. Source hashes of all six output/runs binaries match the recorded 0.6.0 manifest.

## Reader checks

- Python suite: 16 test methods pass, including original format/API tests and metadata persistence, root/hash binding, strict inputs, concurrent server requests, atomic-write failure/corrupt-state preservation and local POST origin checks.
- Node pure math tests pass: raw RPM population variance/SD/CV, empty/single intervals, both-endpoint boundary rule, ratio scaling, stable accumulation, clamping and fixed-width motion.
- Hand-calculated fixture [600,300] RPM gives mean450, population variance22500, SD150, CV33.333...%, and time-weighted rate400. Two detections/revolution halves mean/SD/rate and quarters variance. A focus beginning at100000 us includes only the complete next interval; crossing-left intervals are excluded.
- Original browser harness: 22 checks pass on isolated port8766.
- New browser harness: 17 checks pass, including default RPM, independently calculated real-run focus statistics, unchanged raw statistics under smoothing, overview selection/translation/resizing, keyboard sliders, wide2100px SVG mapping, label/mark persistence, hidden restoration and mobile layout. No JavaScript errors.
- Independent review found and verified fixes for wide SVG pointer coordinates and active recording ARIA/highlight after metadata repaint; no remaining blocker.
- Desktop1440 and mobile390 screenshots inspected. Test metadata used a separate .playwright-mcp state file and restored its labels/marks; normal user state was not modified by browser QA. Both reader ports use identical source assets.
- Python/JS syntax, launcher shell syntax, version alignment, document links and whitespace checked. No dependencies installed.

Screenshots: [desktop](2026-10-26-reader-focus-desktop.png), [mobile](2026-10-26-reader-focus-mobile.png).

## Optical logger naming checks

- Shared uint32 device run index is committed/read back through bundled NVS before opening a recording. Same counter advances for timed and untimed starts. Unset example /OPT_0002.BIN; timed example /OPT_2026-10-26_14-30-00_0004.BIN.
- Sanitized C++ tests pass for mixed-mode sequence1..7, restart reservation model, failed reservation, collision/open-failure gaps, bounds/exhaustion, legacy migration, lower-case recognition and unchanged CRC/event encoding. Existing optical/session host checks pass.
- Installed Preferences/NVS sources inspected: distinguish missing first-use key from actual read/type errors; putUInt commits before successful length return. Exact-status NVS reread verifies the reservation.
- ESP32 core3.3.12 target esp32:esp32:esp32 compile --warnings all passes without warnings: 359417 program bytes, 28568 static RAM bytes.
- Independent review found no blocking numbering/persistence code defect. Hardware NVS/reset/power-loss/card-swap behaviour is UNVERIFIED. Migration is a lower-bound seed from remaining files; deleted legacy history cannot be reconstructed. Failed starts/collisions can consume indices; earlier files are never overwritten. Collision attempts are bounded at100.

## Limits

Population stats describe raw interval RPM, not calibrated shaft-speed accuracy. Flagged intervals stay included; malformed recordings do not receive valid-looking focused statistics. Renaming is a display label only, not disk filename editing. One reader process per state file is supported; multiple independent servers should use separate state files. Current firmware changes require upload/physical validation through the separate workflow. Older acquisition and optical qualification gates stay open.
