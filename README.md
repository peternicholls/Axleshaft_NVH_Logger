# Polestar NVH Logger

Standalone ESP32 vibration and driveshaft rotation logger for a 2021 Polestar 2 Long Range Dual Motor.

Project version: see [VERSION](VERSION). Changes: [CHANGELOG.md](CHANGELOG.md).

Stage 0A passed the hardware test reported in the [original handover](handover_v1.md). Stage 0B firmware is implemented and compile-checked; hardware acceptance remains pending; the integrated logger is not operational.

## Documentation

- [Progress and current evidence](docs/progress.md)
- [Development plan and stage gates](docs/development-plan.md)
- [Planned wireless monitoring and controls](docs/wireless-control-plan.md)
- [Planned analysis bench software](docs/analysis-bench-plan.md)
- [Stage 0B implementation plan](docs/stage0b-plan.md)
- [Development workflow and version policy](docs/development-workflow.md)
- [Hardware wiring](docs/hardware-wiring.md)
- [Test procedures](docs/test-procedures.md)
- [Data format](docs/data-format.md)
- [Timing and synchronisation](docs/timing-and-synchronisation.md)
- [Development log](docs/development-log.md)

## Working storage reference

The supplied [Stage 0A sketch](Stage0A_SD_Test/Stage0A_SD_Test.ino) remains in its original directory. Follow its [build, upload and wiring instructions](Stage0A_SD_Test/README.txt). Arduino IDE uses the Espressif ESP32 board package, ESP32 Dev Module where appropriate, and Serial Monitor at 115200 baud. The exact board package version used for the original pass is unknown and must be recorded on the next run.

Stage deliverables use individual root-level folders containing code and instructions, matching `Stage0A_SD_Test/`. Stage 0B will use `Stage0B_Recorder/` with its sketch, `README.txt`, local firmware helpers and standalone recording validator. Project-wide analysis tools belong under `analysis/`, automated checks under `tests/`, and deliberate fixtures under `sample-data/`. Create directories when they contain actual deliverables.

## Phase 0B

Open [Stage0B_Recorder/Stage0B_Recorder.ino](Stage0B_Recorder/Stage0B_Recorder.ino) and follow [its standalone instructions](Stage0B_Recorder/README.txt). The folder includes its validator and requires no sibling runtime files.

## Stage 1 optical bench test

Open [Stage1_Optical_Test.ino](Stage1_Optical_Test/Stage1_Optical_Test.ino) and follow [the standalone wiring and test instructions](Stage1_Optical_Test/README.txt). Module VCC uses verified 3V3, GND uses common ground, and digital OUT goes to GPIO26 after verifying its voltage. Built-in LED indicates accepted edges. No recording integration yet.

Optical firmware 0.4.0 adds buffered SD event files, UTC date/time filenames (or explicit UNSET boot/uptime fallback), close/CRC checks and a standalone host validator. See the optical README for exact SD pins and Serial clock setup. Code preparation only; physical combined optical/storage acceptance remains pending.
