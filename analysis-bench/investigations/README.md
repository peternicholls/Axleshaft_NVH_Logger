# RPM ripple investigation

Reproduce from the repository root using an existing Python runtime with NumPy and ReportLab:

```sh
python3 analysis-bench/investigations/rpm_ripple.py
python3 -m unittest discover -s tests -p test_rpm_ripple.py
python3 docs/tools/build_rpm_ripple_report.py
```

No dependencies were installed for this deliverable. The local bundled Python runtime supplied NumPy 2.3.5 and ReportLab. The reader itself still uses the standard library.

Default inputs are three byte-identical snapshots in `docs/evidence/2026-10-26-rpm-ripple-inputs/`; source identities and assumptions are in the [investigation](../../docs/evidence/2026-10-26-rpm-ripple-investigation.md). Outputs are JSON, complete raw-interval CSVs and vector PDF/SVG under `output/analysis/rpm-ripple/`. `--runs-dir`, `--output-dir` and `--files` select alternatives. This exploratory tool rejects files above 4096 events rather than truncating them.

The executed companion notebook `rpm_ripple.ipynb` retains numeric output and an SVG figure. Its three Python cells were executed sequentially using the same existing runtime; notebook structure was checked with JSON/AST because Jupyter validation packages were unavailable. Linear detrending and lag-five examination are exploratory; quadratic detrending is a sensitivity check, not a causal test. No filtering or repair of original records is performed.

[Short report](../../docs/experiments/rpm-ripple-diagnosis.md) and [run-sheet template](../../output/analysis/rpm-ripple/run-sheet.csv) support analysis of the user's already designed test. Hardware acceptance and root cause remain unresolved.
