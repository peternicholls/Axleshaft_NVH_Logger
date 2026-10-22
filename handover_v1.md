# PROJECT HANDOVER
# Polestar 2 — Driveshaft Vibration & Rotational Analysis Logger

**Status:** Active development  
**Date:** 22 October 2026  
**Development environment:** ESP32 / Arduino framework  
**Current milestone:** Stage 0A completed successfully; Stage 0B next  
**Purpose:** Handover from ChatGPT engineering discussions to a Codex development project.

---

## 1. Project overview

We are developing a standalone, synchronised vibration and rotational-speed data logger to investigate drivetrain vibration on a 2021 Polestar 2 Long Range Dual Motor.

The immediate investigation concerns a replacement front-right driveshaft and its rubber vibration damper. Vibration developed after an initially smooth period following installation.

The instrument must measure vibration at a suspension component while simultaneously recording the angular rotation of the associated driveshaft.

**Primary objective:** Identify relationships between vibration frequency, amplitude, driveshaft rotational speed and angular position.

We want to distinguish rotationally synchronous vibration from other excitation sources and investigate possible imbalance, eccentricity, CV-joint-related effects and resonances.

The instrument is intended to be reusable for other vehicle NVH investigations.

### Key measurement requirements

- Record three-axis acceleration and three-axis angular velocity.
- Target 1,000 six-axis samples per second.
- Detect and timestamp individual driveshaft revolutions.
- Preserve raw measurements rather than only calculated results.
- Synchronise rotation events with accelerometer samples.
- Record autonomously to an SD card.
- Provide remote recording controls inside the vehicle.
- Support subsequent FFT, spectrogram and rotational-order analysis.
- Record timing errors, missed samples and acquisition overruns.

The development approach is deliberately incremental. Each subsystem must pass independent tests before integration.

---

## 2. Confirmed hardware inventory

The following components are already owned and available.

| Component | Description | Role |
|---|---|---|
| ESP32 development board | ESP32-WROOM-type board fitted to pin-header expansion base | Main processor |
| GY-521 | MPU-6050 accelerometer/gyroscope | Vibration acquisition |
| MH-SD Card Module | Full-size SD card SPI interface | Data storage |
| SD card | Small SDSC test card | Initial development |
| Reflective optical sensor | Narrow, three-pin TCRT5000-style module | Rotational index detection |
| IR-08H optical module | Larger adjustable infrared obstacle sensor | Alternative rotation detector |
| KY-004-style push-button module | Three-pin S/+/- momentary button | Recording start/stop |
| LEDs | External indicator LEDs | Recording and rotation indications |
| Breadboards and Dupont wiring | Available | Development and testing |
| ESP32 expansion cradle | Exposes GPIO connections | Electrical integration |

Additional hardware includes a GY-271 magnetometer, but it is not part of the current preferred design.

**Important:** Some modules have been identified from photographs rather than confirmed schematics. Their voltage behaviour, pin functionality and operating characteristics should be verified during testing.

### Hardware already tested

The ESP32 and SD card module have been wired successfully and have passed a write/read verification test.

---

## 3. Physical architecture

The ESP32 will eventually be mounted locally near the GY-521 beneath the vehicle.

This arrangement minimises the length of the timing-sensitive I²C connections and allows the SD card and processor to remain together.

### Under the vehicle

- ESP32 development board.
- SD card module.
- GY-521 attached rigidly to a suitable stationary suspension component.
- Optical rotation sensor mounted close to the wheel-side CV joint.
- Short local sensor connections.

### Inside the vehicle

A small wired remote control containing:

- Start/stop push button.
- Recording status LED.
- Rotation detection LED.

A four-conductor cable, approximately 3–5 metres long, connects this remote to the ESP32.

USB can run into the cabin for power, diagnostics and programming, but routine recording must not depend on a continuously connected computer.

The final mechanical mounting and enclosure designs have not yet been completed.

---

## 4. GPIO allocation

The following pin allocation has been agreed provisionally.

| GPIO | Function | Interface |
|---|---|---|
| GPIO21 | MPU-6050 SDA | I²C |
| GPIO22 | MPU-6050 SCL | I²C |
| GPIO27 | MPU-6050 data ready | Digital input |
| GPIO26 | Optical rotation signal | Digital interrupt/input |
| GPIO13 | SD card chip select | SPI |
| GPIO18 | SD card clock | SPI |
| GPIO19 | SD card MISO | SPI |
| GPIO23 | SD card MOSI | SPI |
| GPIO32 | Remote push button | Digital input |
| GPIO25 | Recording LED | Digital output |
| GPIO33 | Rotation LED | Digital output |

The existing SPI configuration has already been tested successfully.

### Electrical provisions

- Sensors and ESP32 logic operate at 3.3 V.
- Use the confirmed 3.3 V supply rail from the ESP32.
- The expansion cradle has a voltage-selection jumper; its position must be checked.
- The SD module's 5 V input remains disconnected when using its 3.3 V supply input, subject to module verification.
- All devices share a common electrical ground.
- Recording LEDs require appropriate current-limiting resistors, provisionally 330–470 Ω.
- The remote button input should have an ESP32-side pull-up, provisionally 4.7 kΩ, plus firmware debouncing.
- The three-pin push-button module's internal wiring should be checked before adopting the proposed two-wire S/GND connection.

The final remote arrangement requires four conductors: one button signal, two LED signals and common ground.

---

## 5. Development sequence

Development is divided into four primary stages.

### STAGE 0 — Storage and recording controls

**0A: SD card integration — COMPLETED**

Test requirements:

- Initialise SPI and detect card.
- Read card type and capacity.
- Create a numbered binary file.
- Write test data.
- Close the file.
- Reopen and verify every byte.
- Report write timings and failures.

**Confirmed successful result:**

| Test | Result |
|---|---|
| Filename | T0001.BIN |
| Bytes written | 262,144 |
| File closed successfully | Yes |
| File reopened successfully | Yes |
| Complete byte verification | Passed |
| Card type | SDSC |
| Approximate capacity | 0.01 GiB |
| Write time | 2.93 seconds |
| Longest measured 512-byte write | 58 milliseconds |

The card is intentionally very small and is being used for development only.

The average write throughput was approximately 87 KiB/s.

The 58 ms maximum observed write delay is an important result. It demonstrates that individual SD writes may be much slower than the average throughput suggests.

This result is not a guaranteed upper latency bound.

**0B: Recording controls and sustained logging — NEXT TASK**

Extend the already working SD-card firmware to implement:

1. Start/stop recording using GPIO32.
2. Recording LED on GPIO25.
3. Automatically numbered recording files.
4. Buffered continuous binary writing.
5. Simulated sensor data at approximately the eventual 1 kHz acquisition rate.
6. Accurate sample counting.
7. Graceful recording shutdown.
8. SD write latency statistics.
9. Storage-full and write-error detection.
10. Clear Serial Monitor diagnostics.

The button should toggle recording on a debounced press, not on each iteration of the main loop.

Stopping must flush outstanding buffered data and close the recording file before indicating that the operation is complete.

The firmware must not overwrite earlier recordings.

Initial recordings should be short enough for the small test card.

Use the already verified SD initialization and SPI settings rather than replacing working functionality unnecessarily.

**Stage 0B acceptance criteria:**

- Button starts recording exactly once per press.
- Button stops recording exactly once per press.
- Recording LED accurately reflects recording and saving states.
- Sequential files are created without overwriting.
- Synthetic sample rate and sample count agree with elapsed acquisition time within the defined scheduling tolerance.
- Files close and reopen correctly.
- File contents and record sequence numbers validate.
- Write stalls and buffer utilisation are measured.
- Storage exhaustion is handled predictably.
- No unexplained missing, duplicate or corrupted sample records.

---

### STAGE 1 — Optical revolution sensor

This stage can take place before or after Stage 2.

**Preferred sensor:** Existing narrow three-pin reflective infrared module.

Connections:

| Optical module | ESP32 |
|---|---|
| VCC | 3V3, subject to verification |
| GND | GND |
| OUT | GPIO26 |

#### Physical proposal

Mount the sensor on a stationary component above or adjacent to the wheel-side CV joint.

Use a white reference mark on the black CV boot, preferably on a smooth circumferential section.

The intention is to generate one distinctive optical pulse per driveshaft revolution.

Potential difficulties:

- Reflectivity differences between white paint and rubber.
- Changing sensor distance due to boot runout.
- Rubber boot flexibility.
- Surface curvature.
- Ambient light interference.
- Contamination.
- False or multiple transitions.

An accessible, dimensionally stable rotating metal section may be a better reference surface if boot detection proves unreliable.

A larger IR-08H module is available as an alternative if the short sensing distance of the narrow sensor proves unsuitable.

#### Standalone testing

Create a simple optical-sensor test sketch which:

- Reads the sensor output.
- Detects signal transitions.
- Counts revolutions.
- Timestamps accepted pulses using a high-resolution monotonic clock.
- Calculates instantaneous and averaged RPM.
- Displays detection information over USB serial.
- Flashes an LED when rotation is detected.
- Allows sensitivity and target position to be adjusted.

Start with manual rotation and progress to different controlled rotational speeds.

Measure and record pulse interval consistency.

The pulse rejection/debounce interval must be selected for optical noise without discarding legitimate pulses at the highest test RPM.

**Acceptance:** One reliable index event per actual shaft revolution over the intended operating speed range, with no unexplained missing or additional pulses.

---

### STAGE 2 — GY-521 vibration acquisition

This stage can occur before or after Stage 1.

#### Sensor

GY-521 using the MPU-6050 six-axis inertial measurement device.

Initial proposed configuration:

| Parameter | Proposed setting |
|---|---|
| Sample rate | 1,000 Hz |
| Accelerometer range | ±8 g |
| Gyroscope range | ±500°/s |
| Accelerometer low-pass bandwidth | Approximately 94 Hz |
| Gyroscope low-pass bandwidth | Approximately 98 Hz |
| Data format | Raw signed 16-bit measurements |
| Channels | Ax, Ay, Az, Gx, Gy, Gz |

The settings should be verified against the actual MPU-6050 register configuration.

#### Acquisition approach

Use hardware-configured sampling, the MPU-6050 FIFO and its data-ready mechanism.

Avoid a simple `delay(1)` sampling loop.

The FIFO is 1,024 bytes, corresponding to approximately 85 complete 12-byte six-axis samples.

At 1,000 samples/second, that represents around 85 milliseconds.

This creates an important engineering issue:

The SD card has already demonstrated one write operation lasting 58 ms. A future write could take longer.

Therefore, buffering sensor measurements in ESP32 RAM is necessary but not sufficient by itself: the firmware must also ensure that the MPU-6050 FIFO is serviced frequently enough while storage operations occur.

Investigate an appropriate acquisition/storage separation using ESP32 FreeRTOS tasks or another robust scheduling method.

Do not assume that merely adding a large RAM buffer eliminates hardware FIFO overruns.

#### Standalone testing

- Confirm device identity and I²C communication.
- Verify acceleration measurements and orientation.
- Configure full-scale ranges and filtering.
- Confirm nominal sampling frequency.
- Read complete FIFO sample frames.
- Record raw samples.
- Check continuity of sample sequences.
- Detect FIFO overflow and reset events.
- Generate acceleration time traces and FFT results.
- Establish baseline sensor noise.

**Acceptance:** Continuous six-axis acquisition at the target rate, with accurate sample accounting and explicit reporting of any losses or overruns.

---

### STAGE 3 — Synchronised vibration and rotation logging

Integrate the independently verified accelerometer and optical detector.

This stage is central to the purpose of the instrument.

#### Synchronisation problem

The MPU-6050 has its own sampling clock.

The ESP32 has a separate clock used to timestamp optical pulses.

Samples retrieved from the sensor FIFO were acquired before the ESP32 read them.

**Do not treat the I²C read time as the actual sample acquisition time.**

A robust timing model is required to relate the IMU sample sequence to the ESP32's monotonic timestamp domain.

Investigate timestamping data-ready events, estimating actual sample intervals, detecting accumulated drift and establishing a mapping between sensor sample indices and ESP32 time.

Optical pulse events must be timestamped independently of SD writes.

#### Combined data requirements

Store:

- Sample indices.
- Raw six-axis IMU values.
- Timing anchors and acquisition configuration.
- Optical index timestamps.
- Recording start/stop events.
- Buffer overruns and missing-sample counts.
- Sensor configuration and calibration metadata.
- Storage errors.
- Firmware and file-format versions.

Use a documented, versioned binary format.

The file format should support offline conversion to CSV and scientific analysis tools.

A common monotonic timebase must be recoverable from the stored information.

#### Final acceptance

A combined recording must allow us to determine:

- Driveshaft rotational speed over time.
- Approximate angular phase between consecutive index events.
- Vibration amplitude and frequency.
- First and higher rotational orders.
- Repeatability of vibration peaks relative to shaft angle.
- Changes in vibration during acceleration and deceleration.

---

## 6. Recording controls and LED behaviour

The final user interface is a remote control in the cabin.

### Button

One momentary push button:

- First press: start recording.
- Second press: stop recording.
- Debouncing handled in firmware.

A long-press event marker may be introduced later but is not required for Stage 0B.

### Recording LED

| Condition | Indication |
|---|---|
| Idle | Off |
| Recording | Steady on |
| Stopping/flushing | Slow flashing |
| Storage or acquisition error | Rapid flashing |
| Successfully stopped | Off |

### Rotation LED

The second LED indicates accepted optical rotation events.

The event timestamp must be captured immediately; LED indication is a separate presentation operation.

At higher speeds, individual LED flashes may merge visually. That is acceptable provided event counting and timestamps remain accurate.

Indicator operations must not block acquisition.

---

## 7. Data storage architecture

The recorder should ultimately generate one file per recording session.

Suggested naming convention:

`RUN0001.BIN`  
`RUN0002.BIN`  
`RUN0003.BIN`

Numbering must persist across restarts without overwriting existing recordings.

### Preliminary data rates

Each six-axis sample contains:

- Three accelerometer readings.
- Three gyroscope readings.
- Six signed 16-bit integers.

Total raw sensor payload:

12 bytes per sample.

At 1,000 samples per second:

12,000 bytes/second.

Approximately 720 KB per minute, or 21.6 MB for 30 minutes, before metadata.

Additional event records and timestamps increase the storage requirement.

A larger FAT32 SD card will be introduced before extended tests.

The small SDSC card is intentionally retained for short bench testing.

### Buffer design

Initially investigate RAM buffering in the region of 8–16 KB, with metrics for:

- Current buffer occupancy.
- Peak occupancy.
- Block write time.
- Worst observed write latency.
- Total bytes committed.
- Recording duration.
- Overflow count.
- Error count.

Buffer size must ultimately be determined by measured acquisition and storage behaviour rather than assumed sufficient.

### File integrity

The recorder should minimise the consequences of interruption by:

- Using bounded and documented buffering.
- Closing recordings cleanly.
- Recording explicit errors.
- Avoiding file overwrites.
- Performing validation after recording.
- Reporting incomplete or interrupted sessions when detectable.

Periodic flushing may be considered, balancing power-loss exposure against storage latency.

---

## 8. Data analysis requirements

Analysis will primarily take place on a computer rather than inside the ESP32.

Python is the preferred initial analysis environment.

Required outputs:

1. Acceleration versus time for X, Y and Z.
2. Gyroscope measurements versus time.
3. Vibration RMS and peak amplitudes.
4. FFT frequency spectra.
5. Time-frequency spectrograms.
6. Revolution intervals and calculated RPM.
7. Shaft rotational-order plots.
8. Angularly synchronised waveform averaging.
9. Comparison between recording sessions.

At known driveshaft rotational speed:

First-order frequency = RPM / 60.

Higher orders occur at integer multiples of shaft rotational frequency.

One optical index per revolution provides the reference required for order tracking and approximate angular synchronisation.

The instrument must preserve enough data to revisit the analysis without repeating a vehicle test.

### Diagnostic principle

Do not assume that a first-order vibration peak conclusively proves imbalance.

Compare behaviour across rotational speed, sensor position, drive direction and operating conditions.

The logger should collect evidence without baking a specific fault diagnosis into its firmware.

---

## 9. Proposed repository structure

Codex should first inspect any existing project files, then adapt this structure rather than unnecessarily replacing working code.

```text
polestar-nvh-logger/
├── README.md
├── docs/
│   ├── project-specification.md
│   ├── hardware-wiring.md
│   ├── development-log.md
│   ├── test-procedures.md
│   ├── data-format.md
│   └── timing-and-synchronisation.md
├── firmware/
│   ├── stage0a-sd-test/
│   ├── stage0b-recorder/
│   ├── stage1-optical/
│   ├── stage2-imu/
│   └── integrated-logger/
├── analysis/
│   ├── bin_to_csv.py
│   ├── validate_recording.py
│   ├── vibration_analysis.py
│   └── order_tracking.py
├── tests/
└── sample-data/
```

The existing Stage 0A Arduino sketch was named:

`Stage0A_SD_Test.ino`

It is the established working reference for SD card integration.

The sketch and the earlier engineering planning document may need to be copied into the Codex workspace if they are not already available there.

Do not assume any file has been imported simply because it is mentioned in this handover.

---

## 10. Engineering priorities and constraints

### Reliability before sophistication

The first priority is continuous, correctly timed data acquisition.

Visualisation and advanced diagnostics come later.

### No blocking operations in critical acquisition paths

Avoid delays, excessive serial printing or long filesystem operations that could compromise sensor acquisition.

### Preserve raw data

Use raw integer IMU values and separate scale metadata rather than irreversibly converting or filtering readings during acquisition.

### Explicit errors

Failures must be observable and reported.

Do not silently discard samples or invent replacement readings.

### Modular firmware

Separate storage, controls, IMU acquisition, optical events and timing logic.

The intention is to retain independently testable subsystems.

### Incremental verification

Do not start writing the final integrated firmware before completing the individual test stages.

### Avoid unnecessary hardware purchases

The initial design is deliberately based on available components.

Only recommend hardware changes when experimental results establish a need.

### Maintain a technical development log

Record firmware versions, hardware configurations, measured performance, observed problems and test outcomes.

---

## 11. Current project state

**Completed:**

- Hardware inventory established.
- ESP32 development board and expansion cradle identified.
- GY-521 sensor selected.
- Optical sensing approach selected.
- SD storage architecture established.
- Cabin remote architecture agreed.
- GPIO allocation planned.
- Initial engineering planning document produced.
- SD card physically integrated.
- Stage 0A SD card test completed and verified.

**Not yet completed:**

- Button input testing.
- External LED testing.
- Sustained simulated data recording.
- SD buffer stress testing.
- Optical sensor mounting and detection testing.
- GY-521 standalone acquisition testing.
- Common-clock synchronisation implementation.
- Combined recording.
- Final data analysis software.
- Vehicle measurement trials.

No claim should be made that the integrated logger is operational yet.

---

## 12. Immediate instructions to Codex

**Begin with Stage 0B.**

1. Inspect the existing workspace and determine whether the successful Stage 0A Arduino sketch is available.
2. Preserve its known-working SD configuration.
3. Implement external push-button start/stop control on GPIO32.
4. Add recording status LED handling on GPIO25.
5. Reserve GPIO33 for rotation indication.
6. Implement sequentially numbered binary recording files.
7. Generate deterministic synthetic six-axis records at the expected acquisition rate.
8. Implement buffered SD writes.
9. Measure actual write latency and buffer utilisation.
10. Implement controlled stop, flush and file closure.
11. Add serial diagnostics and recording validation.
12. Document wiring, configuration, test instructions and acceptance criteria.

The synthetic-data test should exercise the actual proposed recording pipeline, including buffering and file handling.

Where practical, include record sequence numbers or equivalent validation information so skipped and corrupted records can be identified after the test.

Do not start Stage 1 or Stage 2 until Stage 0B has been verified.

### First expected Codex deliverables

- Working Stage 0B ESP32 firmware.
- Updated wiring reference.
- Build and upload instructions.
- Defined binary test-record format.
- A simple computer-side recording validator.
- Test checklist and expected results.
- Development log documenting the implementation and known limitations.

**Long-term outcome:** A standalone, reliable, synchronised drivetrain NVH data acquisition system that records acceleration and exact revolution-reference events, permitting repeatable engineering analysis of the Polestar 2 driveshaft and associated components.

---

**End of handover — Version 1.0**