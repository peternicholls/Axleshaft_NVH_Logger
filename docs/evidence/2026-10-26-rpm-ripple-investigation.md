# Repeating RPM ripple — preliminary investigation

2026-10-26 | Repository 0.6.2 | Data/code analysis only; cause not established

## Finding

The regular-looking faster detections are present in raw event timings, not created by the reader plot. UP74 has visibly shorter intervals at interval numbers 5, 10 and 15. UP484 also has a recurring five-event component plus slower drift. The very short UP471 recording does **not** establish the same pattern convincingly.

This is not yet evidence that the motor is faulty or that the ESP32 is at fault. At least three paths need separating: actual motor/belt/shaft speed variation, optical edge generation/qualification, and firmware timestamp capture.

## User-confirmed setup and unknowns

- Nominal 12 V DC motor; speed adjusted by voltage. Actual voltages were not recorded. Motor model/type, supply waveform and whether the effective voltage is PWM-derived are unverified.
- Motor drives the measured sensor shaft through pulleys/belt. Motor-shaft speed is therefore not necessarily measured-shaft speed.
- Pulley diameters/tooth counts, ratio, belt type/length and slip are unknown. No motor-revolution/belt-travel frequency can be assigned from this data.
- These headerless files have no firmware build/configuration metadata or supplied matching Serial capture. Their UNSET names are consistent with the earlier logger naming, but do not prove the exact firmware version or identical sensor gap/pot/mark settings. Current code and historical 0.4.1 timestamp logic were both inspected; relevant helper logic is unchanged.

## Raw evidence

Selected whole recordings are the three lowest-variation nonempty supplied runs, matching the apparent steady-speed captures. No points were removed or smoothed. All three have contiguous sequence and increasing timestamps. Calculated CRC/hash match the earlier local manifest; this is structural/provenance evidence, not an independent capture-completion reference.

| Run suffix | Events / intervals | Event span | Rate, one event/turn assumed | Lag-5 correlation* | Paired intervals | 5 mean intervals |
|---|---:|---:|---:|---:|---:|---:|
| UP74 | 17 / 16 | 6.641 s | 144.548 RPM | 0.715 | 11 | 2.075 s |
| UP471 | 47 / 46 | 3.030 s | 910.787 RPM | 0.319 | 41 | 0.329 s |
| UP484 | 92 / 91 | 19.343 s | 282.269 RPM | 0.703 | 86 | 1.063 s |

*Pearson correlation between a linearly detrended interval sequence and the same sequence shifted by five events. Five was chosen exploratorily after viewing the data; these are descriptive associations, not a significance test or an automatically established period. Drift can create positive correlations at several lags. UP74 covers only about three five-interval cycles; UP471 spans only three seconds. A quadratic-detrend sensitivity check gives lag-5 correlations 0.899 / 0.295 / 0.640 respectively, so trend choice matters. Five-event phase profiles group interval numbers modulo five, not calibrated shaft angle.

In UP74 the three conspicuously short intervals are 409388, 410958 and 409332 us, compared with an overall mean interval415085.75 us. In UP484 the fifth phase has lower mean residual but there is substantial slow variation as well. In UP471, several lag choices have similarly modest correlation; it is not evidence of a shared strong five-turn ripple.

[Source-backed figure](../../output/analysis/rpm-ripple/rpm-ripple.png), [vector PDF](../../output/analysis/rpm-ripple/rpm-ripple.pdf), [computed metrics/records](../../output/analysis/rpm-ripple/analysis.json). Each row uses its own vertical scale. Negative interval residual means a shorter interval/faster inferred RPM. Detrending is a diagnostic transform only; raw fields remain in the CSV/JSON.

## Code-path assessment

- The reader computes raw RPM as 60000000 / interval_us / detections_per_revolution. These small files are below plot downsampling thresholds; the initial plot uses no median smoothing. Direct binary interval calculation reproduces the pattern, excluding a graph-created ripple.
- Both inspected logger revisions timestamp a GPIO transition using esp_timer_get_time inside the ISR after acquiring pulseMux, then store the candidate transition time if that detection later qualifies. There is no deliberate rounding of stored event time to the 250 us qualification callback.
- Intervals exactly divisible by250 us number 0/16, 0/46 and 1/91. This does not fit simple 250 us quantisation of every stored interval; it does not rule out delayed ISR entry, locking, cache/interrupt effects or other timing error.
- CHANGE edges overwrite the current candidate time; detection and clear need 1 ms stable state. Chatter/threshold variation can therefore change which transition becomes the accepted candidate. No raw edge trace exists in these files, so this cannot be checked retrospectively.
- Qualification runs on a 250 us ESP timer task. Serial output is nominally1 s, LED off duration30 ms, and storage batches flush from RAM after approximately100 ms or when full. None of these periods alone proves a matching data artefact. SD byte preservation does not prove the event timestamp was unbiased.
- The inspected critical section includes qualification/counter work and zero-wait queue send, while GPIO timestamping waits for that same lock. Its duration and hardware interrupt latency are not measured. Do not assign a fixed maximum error from timer resolution.

Espressif documents serialized task-dispatched timer callbacks and scheduling-dependent delay: [ESP Timer](https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-reference/system/esp_timer.html). Installed-core behaviour still needs hardware traces; current online documentation may describe a newer SDK than Arduino core3.3.12. No firmware change or performance fix is claimed here.

## Why pulley drive matters

A motor, drive pulley, belt or shaft disturbance may repeat with motor turns, shaft turns or belt travel. Their relationships require the unknown drive ratio and belt geometry. A five-detection repeat at different speeds has different elapsed-time lengths: about2.075 s and1.063 s in the clearer examples. That motivates checking revolution order as well as elapsed frequency, but is not enough to prefer mechanics conclusively.

A fixed-frequency electrical/processor disturbance sampled once per turn can alias into an apparent rotation-count pattern. At shaft sample rate f_r, frequencies separated by integer multiples of f_r are indistinguishable in the once-per-turn record. Changing speed is useful, but a short run or one speed cannot separate these cases. Mark angle/runout/sensitivity can also modulate threshold timing without a true speed change.

## Discriminating test plan (separate hardware workflow)

1. **Repeatable baseline:** freeze gap/pot/mark/belt arrangement. Record three 60-second runs at each of three speed settings. Measure and save actual voltage at the motor and supply/controller mode, warm-up condition, physical arrangement and any independent speed reference. Keep the complete original files and Serial checks. Do not compare unknown voltages as if they were matched conditions.
2. **Small speed sweep:** repeat at measured slightly lower/higher settings, with about±5–10% speed change where the rig supports it. Analyse both seconds and detection count. Measure pulley tooth counts/diameters and belt type/length when practical. No drive-order attribution until those are known.
3. **Known electrical input:** motor off, sensor DO disconnected, feed independently verified stable 3.3 V pulses to GPIO26 at approximately2.4,4.7 and15.2 Hz, with active/clear dwell comfortably above1 ms. Use an available independent source/instrument; an ESP32 software timer driving its own input is not an independent timing reference. Follow existing protocol wiring precautions, never drive against the sensor output. Compare expected edges and recorded times, not only counts. This diagnostic input generation/instrumentation is not implemented by this report.
4. **Separate software loads:** with the verified input or stable physical setup, compare normal logging with Serial disabled, then SD versus bounded RAM-only capture, changing one factor per trial. Retain raw candidate and qualification-processing times, transition counts and lock/ISR diagnostic markers where supported. The existing file format lacks these traces; a separate bounded diagnostic record path is needed. Do not print/write SD in an ISR. Compare instrumentation enabled/disabled to quantify its own effect.
5. **Independent simultaneous references:** observe optical OUT electrically and, if available, an independent shaft-speed/mark reference. Match the actual qualified candidate among all transitions. A stable optical OUT with varying stored times implicates the capture path; varying OUT with stable independent shaft timing implicates optics/electrical thresholding; matching variation in independent shaft timing supports real drive variation.

If the input-bypass test becomes clean, that alone does not prove the motor: the bypass also removes the optical module. If it remains irregular with a verified stable input, the logger/capture path needs investigation. Absence/presence is meaningful only within the tested load, pulse shape and instrument uncertainty.

## Instrumentation next increment

Before any root-cause fix, design a bounded diagnostic trace with candidate sequence/time/level, qualification time, number of intervening transitions and errors, plus optional calibrated ISR marker/reference capture. Keep the production raw format unchanged or explicitly version the diagnostic file. Saturated trace buffers must fail visibly. Existing NVH-TIME-01 covers the timing-reference principles; do not treat this plan as implemented instrumentation or executed bench evidence.

## Reproduction and verification

[Runnable analysis](../../analysis-bench/investigations/rpm_ripple.py) uses existing NumPy and ReportLab, not the dependency-free reader runtime. No packages were installed. Run with a Python environment already containing those libraries:

```sh
python3 analysis-bench/investigations/rpm_ripple.py
python3 -m unittest discover -s tests -p test_rpm_ripple.py -v
```

This investigation used the preinstalled Codex bundled Python environment. Core reader tests remain stdlib-only; optional numeric tests skip where NumPy is unavailable. Re-running the figure additionally needs ReportLab. Original output/runs files remain unchanged/ignored; three tiny byte-identical input snapshots are retained under docs/evidence/2026-10-26-rpm-ripple-inputs for reproducibility, with identities in analysis.json. The script defaults to those snapshots; --runs-dir selects another explicit source.

Five numeric regression checks cover a known five-event synthetic pattern, constant/linear drift without false numeric correlation, malformed/short files and actual source metrics and explicit input-size/numeric-precision limits. The exploratory tool rejects more than4096 events instead of truncating; it does not impose this smaller limit on the reader GUI. Source interval sums reconcile with last-minus-first event time. All six source hashes remain unchanged. Figure rendered/visually checked. No experiment, upload, motor operation or causal identification has occurred.

## Next planned experiment — user clarification

The user has already designed an aligned mechanical-tap test: the GY receives the tap at the intended optical leading edge. No further rig-design advice is requested. This is the next planned dataset; its eventual raw optical/IMU timing will be analysed for lag, repeatability and drift. No new measured result or exact timing accuracy is inferred from the intended alignment. The alternate diagnostic controls above remain an investigation menu, not instructions to redesign the user's experiment.
