Polestar 2 NVH Logger - Stage 0A: SD-card verification
======================================================

Target: conventional ESP32-WROOM-32 DevKit, Arduino IDE with the Espressif esp32 board package.

Hardware (MH-SD Card Module):
  GND  -> ESP32 GND
  3V3  -> ESP32 3V3
  5V   -> NOT CONNECTED
  CS   -> ESP32 GPIO13
  MOSI -> ESP32 GPIO23
  SCK  -> ESP32 GPIO18
  MISO -> ESP32 GPIO19
  Other GND pin need not be connected.

IMPORTANT: the ESP32 breakout/cradle has a voltage-selection jumper. Obtain
3.3 V from a pin measured or verified as 3V3. Do not assume every exposed
V-rail is 3.3 V. Disconnect power before moving wires.

1. Format the card FAT32, ideally on a <=32 GB card (backup files first).
2. Install Arduino IDE plus the Espressif 'esp32' board package if needed.
3. Open Stage0A_SD_Test/Stage0A_SD_Test.ino in Arduino IDE.
4. Select ESP32 Dev Module (if applicable), select USB port, upload.
5. Open Serial Monitor at 115200 baud; press RESET to rerun if needed.
6. Look for 'PASS: 262144 bytes written, closed, reopened and byte-verified.'
7. Remove the SD card only after the test has finished and it is powered down.
   On the computer, verify T0001.BIN (or next available number) is 262,144 bytes.

On a failure, send the full Serial Monitor text and a picture of the six wires.

Scope: Only storage. No GY-521, optical pickup, remote button or LEDs attached.
Follow-on Stage 0B: add button on GPIO32; status LEDs GPIO25 and GPIO33.
