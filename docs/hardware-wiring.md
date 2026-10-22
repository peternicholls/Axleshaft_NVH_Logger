# Hardware wiring

Original storage reference: [Stage 0A instructions](../Stage0A_SD_Test/README.txt). All allocations below follow the handover; only SD wiring has a reported hardware pass.

| GPIO | Function | Status |
|---|---|---|
| 13 | SD CS | Reported tested |
| 18 | SD SCK | Reported tested |
| 19 | SD MISO | Reported tested |
| 23 | SD MOSI | Reported tested |
| 32 | Remote button | Proposed; inspect module wiring |
| Built-in LED (GPIO2 fallback) | Phase 0B recording status | Board mapping/polarity to verify; no external LED |
| 25 | Future remote recording LED | Not used by current Phase 0B |
| 33 | Rotation LED | Reserved |
| 21 / 22 | MPU-6050 SDA / SCL | Proposed |
| 27 | MPU-6050 data ready | Proposed |
| 26 | Optical digital OUT | Stage 1 standalone sketch; electrical behaviour/physical detection unverified |

SD SPI clock: 1 MHz. Use verified 3.3 V logic/supply and common ground. Check the cradle voltage jumper. SD module 5 V input remains disconnected with the supplied 3.3 V arrangement; verify the actual module. Disconnect power before rewiring.

Proposed remote: button signal, two LED signals, common ground over four conductors. Confirm button S/GND behaviour before connection; provisional ESP32-side pull-up 4.7 kΩ, LED series resistors 330–470 Ω. Electrical behaviour and long-cable operation remain to be tested.

Stage 1 uses module VCC to verified 3V3, GND to common GND and digital OUT to GPIO26 only after confirming 3.3 V compatibility. Pin order must follow actual module labels. Built-in LED is used; GPIO33 remains reserved. See [standalone instructions](../Stage1_Optical_Test/README.txt).

Optical 0.3.8 also uses GPIO32 as a normally-open button to GND with INPUT_PULLUP, matching the recorder control wiring. Start resets session counts, stop freezes them. Built-in LED is on during the optical test with OFF pulses per detection; no external LED required. See standalone README for sensitivity-pot and white-mark procedure.

Optical 0.4.0 additionally uses the same SD connections and 1 MHz SPI as Stage 0A. No optical/button pin changes: GPIO26 input, GPIO32 button and built-in status LED. See [standalone instructions](../Stage1_Optical_Test/README.txt) for full wiring and clock/file validation procedure.
