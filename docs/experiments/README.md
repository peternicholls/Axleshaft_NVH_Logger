# Experiment protocols

- [NVH-TIME-01: editable timing procedure](timing-protocol.md)
- [NVH-TIME-01: illustrated printable PDF](../../output/pdf/NVH-TIME-01-experiment-protocol.pdf)

The timing protocol separates optical, firmware, mechanical, IMU/filter and storage delay. It contains a delay/reference ledger, wiring/rig/timeline diagrams, five test sequences, uncertainty rules and a run worksheet. Level A measures combined repeatability; Level B requires independent references to isolate absolute delays. Neither level is executed or accepted by creating the document.

## Rebuild the PDF

The source is timing-protocol.md; vector diagrams and PDF layout are in [build_timing_protocol.py](../tools/build_timing_protocol.py). Run from any directory with an existing Python runtime containing ReportLab:

```sh
python3 docs/tools/build_timing_protocol.py
```

The script resolves paths from its own location. The command above assumes the repository root; otherwise use its absolute path. This deliverable used the existing bundled Python/ReportLab runtime, with no dependency installation. Re-render all PDF pages with Poppler and visually check them after any source/layout update. Preserve protocol revisions and evidence references; a changed procedure is not a changed measured result.
