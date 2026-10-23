# Stage 2 I1 local verification — 0.5.0

2026-10-23. Software-only evidence; no ESP32 upload, sensor connection or physical operation performed in this chat. No physical identity, measurement or watchdog acceptance is implied.

## Results

- Arduino CLI 1.5.1 / Arduino-ESP32 3.3.12, FQBN esp32:esp32:esp32: final sketch compiles with --warnings all and no warnings. Program storage 296836 / 1310720 bytes, static RAM 23548 / 327680 bytes.
- Final complete Stage2_IMU_Test folder copied to an independent temporary directory and compiled successfully, proving no sibling runtime dependency.
- tests/test_imu_bringup.cpp passes with C++17, -Wall -Wextra -Werror -pedantic and address/undefined sanitizers. Actual driver, decoders and Wire adapter are exercised with fake bus/port fault injection.
- Clang --analyze on the host driver/adapter test: zero diagnostics.
- python3 -m unittest discover -s tests -v: both existing Python recording-validator test groups pass.
- Independent read-only verifier found no blocking I1 defect; independently reran driver tests. Corrected README command working directory and documented remaining sketch scheduler coverage.
- Documentation links, version alignment, I2C payload arithmetic, prior-stage source preservation and git diff --check pass.

## Reproduction

```sh
arduino-cli compile --fqbn esp32:esp32:esp32 --warnings all Stage2_IMU_Test
c++ -std=c++17 -Wall -Wextra -Werror -pedantic -fsanitize=address,undefined -I tests/imu_stubs tests/test_imu_bringup.cpp -o /tmp/polestar-imu-bringup-tests
/tmp/polestar-imu-bringup-tests
c++ --analyze -std=c++17 -Wall -Wextra -I tests/imu_stubs tests/test_imu_bringup.cpp -o /tmp/polestar-imu-analysis.plist
python3 -m unittest discover -s tests -v
```

## Limits

No hardware readings exist yet for this sketch. Whole-sketch one-second freshness watchdog and five-second health-loop timing have no host-clock harness coverage. Physical LED/pin mapping, module voltage, identity/register dump, I2C/orientation/gyro response and long diagnostic stability require separate evidence. Short taps can be missed by the slow snapshots. Continuous FIFO acquisition, data-ready timing anchors, SD writes, validation format and optical synchronisation are not implemented in I1.
