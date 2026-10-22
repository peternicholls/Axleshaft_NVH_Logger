# Polestar NVH Logger

Standalone ESP32 vibration and driveshaft rotation logger for a 2021 Polestar 2 Long Range Dual Motor.

Project version: see [VERSION](VERSION). Changes: [CHANGELOG.md](CHANGELOG.md).

Stage 0A passed the hardware test reported in the [original handover](handover_v1.md). Stage 0B is next; the integrated logger is not operational.

## Documentation

- [Progress and current evidence](docs/progress.md)
- [Development plan and stage gates](docs/development-plan.md)
- [Stage 0B implementation plan](docs/stage0b-plan.md)
- [Development workflow and version policy](docs/development-workflow.md)
- [Hardware wiring](docs/hardware-wiring.md)
- [Test procedures](docs/test-procedures.md)
- [Data format](docs/data-format.md)
- [Timing and synchronisation](docs/timing-and-synchronisation.md)
- [Development log](docs/development-log.md)

## Working storage reference

The supplied [Stage 0A sketch](Stage0A_SD_Test/Stage0A_SD_Test.ino) remains in its original directory. Follow its [build, upload and wiring instructions](Stage0A_SD_Test/README.txt). Arduino IDE uses the Espressif ESP32 board package, ESP32 Dev Module where appropriate, and Serial Monitor at 115200 baud. The exact board package version used for the original pass is unknown and must be recorded on the next run.

Future firmware belongs under `firmware/`, computer-side tools under `analysis/`, automated checks under `tests/`, and deliberately selected fixtures under `sample-data/`. Create these directories when they contain actual deliverables.
