# RPM ripple: short report and diagnosis procedure

26 October 2026 | Version 0.6.2 | Preliminary; no root cause established

## What the existing files show

A repeating faster-interval pattern exists in the binary timestamps. It is not invented by the GUI. UP74 has shorter intervals at positions 5, 10 and 15 (ending at detections 6, 11 and 16); UP484 has a five-event component plus slow drift. The short UP471 run provides weaker evidence of the same pattern.

| Run | Overall RPM* | Raw RPM SD | Raw RPM CV |
|---|---:|---:|---:|
| UP74 | 144.55 | 1.00 | 0.69% |
| UP471 | 910.79 | 1.69 | 0.19% |
| UP484 | 282.27 | 1.29 | 0.46% |

*One detection per revolution assumed. SD/CV describe unsmoothed interval-derived RPM; overall RPM is the time-weighted rate. These are speed observations, not measured timestamp errors. Captures span only 6.64, 3.03 and 19.34 seconds respectively. Voltage settings and pulley ratio were not recorded.

Setup reported: nominal 12 V DC motor, speed adjusted by voltage, pulley/belt drive to the measured shaft. Motor/pulley/belt details and waveform are unknown. The likely categories to distinguish are real drive variation, optical threshold/edge selection and firmware capture latency. A fixed-frequency disturbance can alias when observed only once per revolution; a five-event pattern alone does not identify mechanics or a timer.

## How much does it matter for the car?

The observed spread is modest, but its cause matters. Real speed variation is expected to be handled by order tracking. Basic acceleration traces, RMS and broad spectra are less dependent on exact shaft phase; angular averaging and higher-order phase comparisons need much better timing assurance.

If event times contain error dt, phase error is approximately **6 x RPM x dt(seconds) x order**, in degrees. At an illustrative 1000 shaft RPM, 1 ms timing error is 6 degrees at first order and 36 degrees at sixth order. These are examples, not measured errors or asserted car operating conditions. Our observed RPM variation cannot be substituted for dt.

Adding six-axis sampling creates 12000 raw payload bytes/s at 1000 Hz, plus timing/format overhead. Hardware sampling need not change, but extra I2C, interrupt, locking and SD work can worsen capture latency or cause FIFO/queue loss. Compare timing before/after under the same known input; preserve independent FIFO servicing. Sensor filtering adds another response delay and must be recorded, not silently subtracted.

## Present assessment

The existing data is adequate to justify investigation, not to assign blame or claim synchronisation accuracy. Existing code saves a candidate edge timestamp rather than a simple 250 us tick; qualification/chatter and interrupt delay still need tests. Keep raw data and avoid smoothing the issue away. Before car phase/order claims, define a timing-error budget from the highest intended RPM/order and permitted phase error, then verify it with accelerometer acquisition running.

Evidence: [detailed analysis and source identities](../evidence/2026-10-26-rpm-ripple-investigation.md), [raw-interval figure](../../output/analysis/rpm-ripple/rpm-ripple.pdf). Technical basis: [Espressif timer scheduling](https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-reference/system/esp_timer.html), [NI order-analysis/resampling](https://www.ni.com/en/shop/labview/vibration-analysis-and-signal-processing-in-labview.html). Phase/error and payload figures above are derived calculations.

<!-- PAGE BREAK -->

# Diagnosis: data record and analysis

Your next experiment is already designed: the GY receives a tap intended to align with the optical leading edge. This checklist records the evidence for analysis; it does not change your rig or prescribe another test setup.

## 1. Identify each recording

Keep the original filename and add a descriptive run label. Record firmware/version or UNKNOWN, actual test mode, duration/count if known, and whether IMU acquisition was enabled. Note voltage, supply/controller, pulley/belt details, sensor settings and any changes only where available. Missing historic settings stay UNKNOWN. Unset dates do not prevent analysis of relative timestamps.

## 2. Keep the data the firmware actually provides

Retain raw optical event numbers/timestamps and raw six-axis IMU samples with sample indices and timing anchors when the acquisition firmware supports them. Include configuration/filter settings and overflow/reset/error counters if recorded. List missing fields; do not invent timestamps or treat FIFO/SD read completion as the sample time. The full combined capture path remains to be implemented and qualified.

## 3. Preserve provenance and completion evidence

Save the binary files unchanged, source hashes and the Serial summary/checks where available. For offline captures, keep manual run-order/filename notes and mark unavailable completion counters explicitly. The companion run-sheet CSV is an optional record template. A calculated CRC/structural pass alone is not proof of a clean capture or correct physical count.

## 4. Analyse the aligned tap dataset

First validate record sequence, file integrity, sample continuity and timing epochs. Then plot the unsmoothed optical intervals and IMU waveform together, using a documented sample-time mapping. Match tap cycles to optical events, retain ambiguous/unmatched cases, and report tap-to-edge onset/peak lag, spread and drift separately. Show the detection/feature rule used; avoid forcing the peaks to zero lag or silently removing faster intervals.

## 5. Separate what each reference can establish

The table is an interpretation guide for references present in a dataset, not a request to add equipment or redesign the next experiment.

| Evidence available | Interpretation supported |
|---|---|
| Independent shaft reference has matching ripple | Real motor/drive variation supported |
| Shaft stable, optical OUT varies | Optical/electrical edge generation implicated |
| Optical OUT stable, stored event times vary | Capture/timestamp path implicated |
| Aligned tap and optical times only | Combined lag/repeatability; individual delays may remain confounded |

A known geometry aligns the intended events; measured response can still include mechanical transmission, filtering, sampling and timestamp error. State which contributions the actual saved evidence identifies, and leave unseparated terms unresolved.

## 6. Report the result and acquisition load

Report raw RPM mean, SD/CV, repeat interval, tap lag/spread/drift and uncertainty; explain exclusions. Where matched optical-only and IMU-enabled datasets exist, compare continuity, FIFO/queue losses and measured service latency. Changed motor conditions cannot establish CPU-load effects. Missing diagnostics leave conclusions UNVERIFIED.

Status: analysis checklist prepared for your existing experiment; aligned-tap data not yet supplied. No hardware tests or uploads performed here.
