# Marked-shaft and GY-521 timing experiment

Protocol NVH-TIME-01 | Revision 1 | Repository 0.4.7 | 2026-10-23
Status: procedure prepared, not executed. Stage 2/combined timing firmware and diagnostic instrumentation are still to be implemented. Existing optical 0.4.1 cannot perform the full protocol.

## 1. What we are measuring

Measure delays separately before applying any correction. A white mark, optical output, software event, mechanical contact and acceleration peak are different events. One comparison cannot uniquely identify all their delays. An audible click is not a calibrated timing reference, and the GY-521 is not a microphone.

Two capability levels:

- Level A, existing rig: optical events and IMU data can establish relative lag, repeatability and drift of the complete chain. Report combined delay, not absolute sensor response.
- Level B, instrumented rig: an independently timed electrical capture, physical mark reference, contact reference and/or characterised reference accelerometer can isolate additional terms. Use available instruments if suitable; no purchase or new hardware is required to begin Level A. Missing references leave the corresponding delay unverified.

Illustrations in the printable PDF show the rig, wiring, event timeline and test decision flow. They are schematic and not to scale.

## 2. Delay ledger and required references

| Quantity | Measurement | What it does NOT establish |
|---|---|---|
| Optical physical response | Module OUT minus independently timed mark passage at the same position | GPIO or firmware capture alone cannot measure mark-to-output latency |
| GPIO timestamp delay | Logged edge timestamp versus measured OUT edge, after aligning instrument/ESP32 clocks | Timer resolution is not capture accuracy; account for alignment and probe/marker delay |
| Qualification response | Qualified-event processing time minus candidate-edge timestamp | Stored optical time is the candidate time; a 1 ms stability test delays acceptance, not necessarily stored timestamp |
| Mechanical phase | Contact reference minus mark reference, or measured shaft angle/period | Assuming mark and tap coincide is not a measurement |
| Observed IMU response | Chosen IMU onset/peak time minus contact time or optical time | Includes mechanical propagation, filtering, sampling and timing-map errors |
| IMU-specific response | IMU trace versus co-mounted, characterised reference acceleration | Contact closure alone is not acceleration onset at the sensor |
| FIFO service | FIFO service intervals, occupancy, drain duration and overflow/reset counters | I2C read completion is not sample acquisition time |
| SD persistence | Queue dwell, write duration, final flush/close and host integrity check | These do not shift sample acquisition timestamps or prove power-loss durability |

Definitions: t_out = electrical optical edge; t_edge = ESP32 candidate timestamp; t_qual = acceptance-processing time; t_contact = measured first mechanical contact; t_onset and t_peak = IMU features on the reconstructed sample timeline. Report signed differences with positive meaning later. Record onset and peak separately; do not call them interchangeable response times. The chosen onset rule also needs adequate signal-to-noise ratio; a weak tap that fails the rule is not automatically a missing sensor sample.

## 3. Rig and wiring

Fix motor, shaft/disc, optical sensor and tapping arrangement. One white mark and one adjustable cam/tab per revolution. Tap a stationary bracket carrying the rigidly mounted IMU. Keep electronics stationary, rotating parts secured, and fingers/cables away during powered tests; switch motor power off for pot, gap or cam adjustments. Record a photo and orientation axes. Separate motor vibration by running motor-on/tap-disabled controls.

| Connection | ESP32 pin / note |
|---|---|
| GY-521 VCC / GND | Verified compatible 3V3 / common GND; verify actual module |
| GY-521 SDA / SCL / INT | GPIO21 / GPIO22 / GPIO27 |
| GY-521 AD0 | Document fixed level and actual address |
| Optical VCC / GND / DO | Verified compatible 3V3 / GND / GPIO26; check mark polarity |
| Button | Normally open GPIO32 to GND, input pull-up |
| Indicator | Built-in LED, GPIO2 fallback; not a timing reference |
| SD CS / SCK / MISO / MOSI | GPIO13 / GPIO18 / GPIO19 / GPIO23; retain 1 MHz SPI |
| SD supply | Verified 3V3, common GND; module 5V input disconnected |

Optional diagnostic assignments, RESERVED ONLY AFTER board/firmware verification: GPIO14 test-waveform output; GPIO16 timestamp-marker output; GPIO34 dry-contact reference input with external 4.7k pull-up to 3V3 (GPIO34 has no internal pull-up). These features are not implemented. Keep GPIO33 reserved for rotation indication. Never connect motor supply to a GPIO or use the contact circuit unless it is isolated from motor power.

For electrical loopback only, disconnect sensor DO and connect the 3.3 V test output GPIO14 to GPIO26 through a series resistor (for example 1k). Restore sensor wiring before physical runs. Do not connect two outputs together. A logic analyser/scope observes OUT, INT and marker; use compatible probe voltage and a shared reference ground. An analyser clock is a separate clock: record its resolution, accuracy and alignment method.

## 4. Firmware/readiness checklist

Before full execution, implement and locally verify:

1. Standalone GY-521 identity/configuration read-back, stationary orientation and tap response; FIFO six-axis acquisition near 1000 Hz with data-ready anchors, complete frames and explicit epoch/reset/overflow accounting.
2. Combined versioned records with raw sample indices, optical candidate timestamps and sequence, qualification-processing times, raw/rejected counts, start/stop, SD/write/queue metrics and configuration. Preserve actual clock domains; never fabricate missing samples.
3. Record t_qual independently of t_edge. Current optical firmware saves only candidate elapsed timestamps, not this acceptance delay.
4. For optional Level B, bounded diagnostic edge marker and separately captured reference/contact channels with debounce/reference edge rules. Measure instrumentation overhead on versus off; do not Serial-print in interrupt paths.
5. Host validator, source hashes, complete-file checks and reproducible time mapping. Test missing/duplicate anchors and resets fail closed or mark uncertain segments.

Do not execute later tests with pretend firmware support. Code/build work happens in this chat. Uploads, instrument connections and physical capture are separate workflow actions.

## 5. Before every run

Assign run ID, operator/date, source revision and firmware hash. Preserve original files. Record supply, card, wiring/photo, motor arrangement, sensor gap/pot position, mark width/angle, cam contact angle, IMU orientation/mount, ranges, rate-divider/filter registers, interrupt configuration, optical stable time/minimum interval and clock source. Date filenames are labels, not synchronisation anchors.

Warm up for 2 minutes as a consistent procedural condition, not a guaranteed stabilisation time. Capture at least 30 s stationary baseline. Determine noise using a fixed axis and baseline interval; note mean and standard deviation. Keep that axis/feature definition fixed through compared runs. Freeze thresholds, speed range and error budget on the worksheet before collecting comparison data. A pilot may set them, but is excluded from acceptance statistics.

## 6. Test A: electrical capture and qualification

Purpose: isolate firmware input handling from optical physics and mechanics.

1. With motor off, configure the loopback described above. Generate known 3.3 V pulses; do not use the sensor output concurrently. Start slowly, then use widths at 0.5x, 1x, 2x and 5x the configured stability time for active AND clear states. Vary phase relative to the qualification timer.
2. Log at least 1000 cycles per condition; repeat three runs at each required operating-rate condition, first without SD traffic and then with production SD logging. Boundary-width trials are characterisation, not an expectation of guaranteed acceptance.
3. Compare generated electrical edges and logged candidate/qualified events. Report missed/extra counts, t_qual-t_edge median/p95/max and any input servicing delay. The provisional 1 ms stability requirement plus 250 us task timer does not guarantee a 1.25 ms upper bound under load.
4. A generator and input timed by the same ESP32 test internal consistency only. For externally verified timing, capture source edge/INT/diagnostic markers with a suitable independent instrument. Align domains using repeated markers bracketing the run, fit offset and scale, check hold-out marker residuals and include marker overhead. No suitable instrument means absolute timestamp latency remains unverified.
5. Save the waveform and table. Repeat with diagnostic marking disabled to check count/rate/load effects. Restore sensor DO afterwards.

Never infer optical module response from this bypass test.

## 7. Test B: mark passage and optical reliability

1. Fix the sensor gap/pot with motor off. Verify clear and white-mark levels; record active polarity. Obtain active/clear dwell widths from a scope/analyser if available, or label them unmeasured.
2. Do 20 manually counted complete revolutions, start/end clear, three sessions. Require exactly 20 accepted events each, no unreported loss, host validation PASS. Observe the off pulse separately; merged flashes at speed are not lost counts.
3. Use three steady speeds selected from the independently measured usable range. Capture at least 100 complete revolutions per speed for three runs each. Compare to independent revolution/RPM reference where available; a motor setting alone is not a speed reference.
4. Observe stationary clear for 30 s, stationary mark for 30 s, and motor-on/tap-disabled baseline. Require no false accumulating counts. Freeze the pot/gap between compared runs.
5. To measure optical physical delay, observe the same mark crossing with a separately characterised fast optical reference at matched geometry. Its response delay and spatial offset remain in the uncertainty budget. Without this reference, report output reliability only, not mark-to-output latency.

Shorten the stability setting only after measuring pulse widths and repeating this test. More rejected raw chatter is not automatically more missed revolutions.

## 8. Test C: IMU sampling, filter response and FIFO

1. Run stationary acquisition for 60 s, three times. Check raw axes/gravity response, sample/frame accounting, data-ready count/mapping, actual period, noise, FIFO service gaps and every reset/overflow. Distinguish hardware-generated samples from ISR notifications; an interrupt count alone may not establish sample identity.
2. Fit each uninterrupted timing epoch as t_available(n) = a + b*n using correctly associated data-ready anchors. Report b, rate 1000000/b, frequency error relative to a documented reference, anchor residual median/p95/max and fit drift. If sample-anchor association is ambiguous, stop accuracy claims for that segment. This is availability time, not unfiltered mechanical measurement time.
3. With rotation off, deliver at least 30 separated light taps to the same stationary bracket. Exclude only predeclared invalid taps (clipping, unresolved rebound, lost data) and report exclusions. A contact reference records first contact; mechanical propagation to the IMU remains part of observed lag.
4. Define onset as first sample on a chosen axis exceeding baseline mean by 5 standard deviations in absolute value for two successive samples. Use a predeclared post-event window and polarity-independent peak rule. Record the rule's own delay and amplitude dependence; do not silently change it between runs. Ringing peaks belong to the same tap window.
5. Compare two documented filter configurations in an A-B-A order with rate/ranges held fixed and their registers read back. Retain the datasheet bandwidth/delay table for reference; impulse onset/peak shift is not an exact calibration of the tabulated filter delay. Confirm the selected rate-divider setting still gives the intended sample rate.
6. IMU-only absolute response requires an independently timed, co-mounted reference acceleration measurement with known bandwidth/delay. Without one, label contact-to-IMU lag as combined mechanical/filter/sample response, not pure MPU-6050 latency.

## 9. Test D: combined rotating tap and phase

1. Align mark and cam approximately; measure and record actual offset. Use one mark and one contact per revolution. Do not assume the optical edge and contact are simultaneous.
2. Begin clear; start capture, reach steady speed and record at least 100 complete revolutions at each of three speeds, three runs per speed. Include motor-on/no-tap and stationary controls. Flag acceleration clipping and speed changes. Keep a consistent revolution association; if phase crosses a revolution boundary, unwrap against event sequence rather than reporting a sudden whole-period delay jump.
3. Associate each optical event with one tap window, ending before the next revolution's window. A manual pilot defines the window; freeze it before acceptance data. Mark missing/ambiguous responses; do not force nearest peaks to match.
4. Plot raw acceleration, sample-availability timeline, optical events and contact references where available. Report signed optical-to-onset and optical-to-peak lag, median, p5/p95, min/max and robust spread (MAD). Preserve per-revolution values.
5. Move the cam/tap phase by measured 30 degrees and repeat at the same steady speeds; then return to the original angle (A-B-A). Record angle uncertainty. Expected lag change is (30/360)*T, using measured local period T; at 600 RPM this is about 8.333 ms. This is an illustrative calculation, not a requested or validated rig speed.
6. A geometric phase change produces lag proportional to T; a fixed timing delay contributes a roughly fixed offset. Fit lag = c + k*T across runs only where contact behaviour and speed are stable. Check residuals and repeats. c remains combined fixed delay unless component references independently separate it.
7. Run at least one 5-minute steady recording to inspect clock/phase drift, buffer service and start-to-end lag. Convert timing drift to ppm only with a stated reference/model; mechanical drift can also move the tap peak.

## 10. Test E: SD load and command response

At one validated speed, repeat identical experiments with memory-only diagnostic capture, normal SD writes, then declared injected storage delays. Keep capture/analysis identical; bounded RAM-only runs must not overflow. Compare raw timing and anchor residuals, not write completion times.

Record queue dwell enqueue-to-dequeue, write start/end, max/percentile latency, occupancy, FIFO service maxima and qualification delay. Inject 60 ms and 250 ms delays at least 10 times each, then a deliberately excessive delay to prove overflow is visible. The excessive-delay run is an expected failure, never a valid accuracy dataset. Disable injection for normal acceptance and record the setting.

For controls, timestamp raw button candidate, debounced press, acquisition start/cutoff, last dequeue, flush/close end and verification complete. Report button-to-start, button-to-cutoff and cutoff-to-verified durations separately. A 30 ms debounce plus polling is not SD saving latency. Repeat 20 start/stop cycles and include the declared no-space case using disposable contents. Retain originals; do not erase wanted data. Raw input measurement requires firmware instrumentation/reference that existing logs do not provide.

## 11. Uncertainty, corrections and decision rules

Build a delay budget with units and provenance for each term: reference instrument resolution/accuracy, domain alignment residuals, interrupt/locking/marker delay, sample spacing/anchor fit, optical geometry/threshold, cam angle/contact jitter, structural propagation and filter/feature delay. At 1000 Hz, sample spacing is about 1 ms; a microsecond timestamp does not make the acceleration measurement microsecond-accurate. Keep known biases separate from random jitter.

For angular uncertainty u_theta in degrees, timing contribution is approximately T*u_theta/360. A contact's measured repeatability is another contribution; do not call it zero. Do not assume half-sample uncertainty or sub-sample interpolation accuracy without validating the estimator/reference.

For each corrected lag, show raw lag, each independently supported correction and remaining interval of uncertainty. Example: corrected = raw - measured geometry contribution - characterised reference delay; never subtract an assumed filter delay just to make zero. Use conservative sums for bounds; use root-sum-square only for independently justified statistical uncertainties with declared coverage. Compare A/B difference uncertainty, not only individual error bars.

Mandatory integrity gate: zero unexplained missing/duplicate events or samples, no overflow/reset inside an accepted segment, no clipping, known physical reference/count and host file checks PASS. Failures invalidate that accuracy segment, but remain useful diagnostics.

Record numerical timing/lag-repeatability tolerances BEFORE acceptance runs, after reference/rig pilot characterisation. A gate not assigned a defensible tolerance/reference is UNVERIFIED. Report PASS only for the claim actually measured: integrity, repeatability, relative phase, or independently verified absolute delay. The protocol itself supplies no measured delay results.

## 12. Run worksheet and evidence pack

Complete one worksheet per condition. Use NOT MEASURED / NOT AVAILABLE rather than blank assumptions.

| Field | Entry |
|---|---|
| Run ID / date / operator / test A-E | |
| Firmware source/hash / format / validator version | |
| Setup photo / wiring / sensor model / board | |
| Gap / mark width / pot setting / polarity | |
| Measured RPM/reference / angle/uncertainty | |
| IMU axis/mount / ranges / filter / rate registers | |
| Stable time / qualification timer / pulse rule | |
| Clock domains / reference instrument / alignment | |
| Onset/peak window / thresholds / exclusions | |
| Numerical target / uncertainty budget frozen | |
| Samples/events expected / observed / lost | |
| Duration / SD mode / injected delays / queue peak | |
| Lag median/p95/max / drift / uncertainty | |
| File names / SHA-256 / CRC / host validation | |
| Claims PASS / FAIL / UNVERIFIED and reason | |

Archive raw binary and Serial, instrument trace with timebase metadata, setup photograph, register dump, decoded CSV, per-event lag table, plots, source hashes and validation output. Keep every rejected trial and reason. Store reports under docs/evidence with links from progress; never replace historical captures with corrected values. This chat only prepares programming/procedures and inspects saved evidence.

Sources: [MPU-6000/6050 register map, sections on sample divider, DLPF, interrupts and FIFO](https://invensense.tdk.com/wp-content/uploads/2015/02/MPU-6000-Register-Map.pdf); [Espressif ESP Timer reference](https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-reference/system/esp_timer.html). Check installed-core APIs and actual module before firmware work. ESP Timer callback delays depend on scheduling; nominal timer resolution/period is not a maximum response guarantee.
