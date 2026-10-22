no/*
  Polestar 2 NVH Logger - Stage 0A
  ESP32 + MH-SD full-size SD SPI module: stand-alone storage verification.

  This sketch deliberately does NOT use the button, LEDs, GY-521 or optical sensor.
  Tests mount, card identification, unique file creation, 256 KiB block writing,
  full read-back verification and basic write-latency measurements.

  Target: conventional ESP32-WROOM-32 DevKit using Arduino-ESP32 SD library.
  Wire SD 3V3 to ESP32 3V3; SD 5V pin must stay disconnected.
*/

#include <Arduino.h>
#include <SPI.h>
#include <SD.h>

constexpr uint8_t PIN_SD_CS   = 13;
constexpr uint8_t PIN_SD_SCK  = 18;
constexpr uint8_t PIN_SD_MISO = 19;
constexpr uint8_t PIN_SD_MOSI = 23;

constexpr uint32_t SPI_CLOCK_HZ = 1000000;  // Start conservatively: 1 MHz.
constexpr size_t BLOCK_BYTES = 512;
constexpr size_t NUM_BLOCKS = 512;         // 256 KiB total.
constexpr size_t TOTAL_BYTES = BLOCK_BYTES * NUM_BLOCKS;

uint8_t buffer[BLOCK_BYTES];

static uint8_t expectedByte(size_t absoluteOffset) {
  // Known deterministic data pattern, varying with byte position and block.
  return static_cast<uint8_t>((absoluteOffset * 17U + (absoluteOffset >> 8U) + 0x5AU) & 0xFFU);
}

static void fillBlock(size_t blockIndex) {
  const size_t base = blockIndex * BLOCK_BYTES;
  for (size_t i = 0; i < BLOCK_BYTES; ++i) {
    buffer[i] = expectedByte(base + i);
  }
}

static const char *cardTypeName(uint8_t type) {
  switch (type) {
    case CARD_MMC: return "MMC";
    case CARD_SD: return "SDSC";
    case CARD_SDHC: return "SDHC/SDXC";
    case CARD_NONE: return "NONE";
    default: return "UNKNOWN";
  }
}

static bool findUnusedFilename(char *outName, size_t outSize) {
  for (unsigned i = 1; i <= 9999; ++i) {
    snprintf(outName, outSize, "/T%04u.BIN", i); // 8.3 safe.
    if (!SD.exists(outName)) return true;
  }
  return false;
}

static void testSD() {
  Serial.println();
  Serial.println("=== STAGE 0A: SD CARD TEST ===");
  Serial.println("SPI: SCK=18 MISO=19 MOSI=23 CS=13");
  Serial.println("SD module supply: 3V3 only (not 5V)");

  pinMode(PIN_SD_CS, OUTPUT);
  digitalWrite(PIN_SD_CS, HIGH);
  SPI.begin(PIN_SD_SCK, PIN_SD_MISO, PIN_SD_MOSI, PIN_SD_CS);

  if (!SD.begin(PIN_SD_CS, SPI, SPI_CLOCK_HZ)) {
    Serial.println("FAIL: SD mount failed. Check power, wiring, card and FAT32 format.");
    return;
  }

  const uint8_t type = SD.cardType();
  Serial.printf("Card type: %s\n", cardTypeName(type));
  if (type == CARD_NONE) {
    Serial.println("FAIL: no card detected.");
    SD.end();
    return;
  }
  Serial.printf("Capacity: %.2f GiB\n", double(SD.cardSize()) / (1024.0 * 1024.0 * 1024.0));

  char filename[18] = {};
  if (!findUnusedFilename(filename, sizeof(filename))) {
    Serial.println("FAIL: no free test filename T0001.BIN through T9999.BIN.");
    SD.end();
    return;
  }
  Serial.printf("Creating: %s (will not overwrite an existing file)\n", filename);

  File file = SD.open(filename, FILE_WRITE);
  if (!file) {
    Serial.println("FAIL: cannot open test file for writing.");
    SD.end();
    return;
  }

  bool writeOK = true;
  uint32_t maxWriteUs = 0;
  uint32_t totalWriteUs = 0;
  for (size_t block = 0; block < NUM_BLOCKS; ++block) {
    fillBlock(block);
    uint32_t t0 = micros();
    size_t written = file.write(buffer, BLOCK_BYTES);
    uint32_t duration = static_cast<uint32_t>(micros() - t0);
    if (duration > maxWriteUs) maxWriteUs = duration;
    totalWriteUs += duration;
    if (written != BLOCK_BYTES) {
      Serial.printf("FAIL: short write at block %u (%u / %u bytes).\n",
                    unsigned(block), unsigned(written), unsigned(BLOCK_BYTES));
      writeOK = false;
      break;
    }
  }

  const uint32_t closeStart = micros();
  file.flush();
  file.close();
  const uint32_t closeUs = static_cast<uint32_t>(micros() - closeStart);

  Serial.printf("Write-call time: %lu ms total; longest 512-byte call: %lu us\n",
                static_cast<unsigned long>(totalWriteUs / 1000U),
                static_cast<unsigned long>(maxWriteUs));
  Serial.printf("Final flush + close: %lu ms\n",
                static_cast<unsigned long>(closeUs / 1000U));
  Serial.println("Note: these write latencies are a bench baseline, not a guaranteed worst case.");

  if (!writeOK) {
    Serial.println("FAIL: write test failed; partial file left on card for diagnosis.");
    SD.end();
    return;
  }

  File check = SD.open(filename, FILE_READ);
  if (!check) {
    Serial.println("FAIL: cannot reopen file for verification.");
    SD.end();
    return;
  }

  if (check.size() != TOTAL_BYTES) {
    Serial.printf("FAIL: file size %lu; expected %u.\n",
                  static_cast<unsigned long>(check.size()), unsigned(TOTAL_BYTES));
    check.close();
    SD.end();
    return;
  }

  bool readOK = true;
  for (size_t block = 0; block < NUM_BLOCKS && readOK; ++block) {
    const size_t count = check.read(buffer, BLOCK_BYTES);
    if (count != BLOCK_BYTES) {
      Serial.printf("FAIL: short read at block %u.\n", unsigned(block));
      readOK = false;
      break;
    }
    for (size_t i = 0; i < BLOCK_BYTES; ++i) {
      if (buffer[i] != expectedByte(block * BLOCK_BYTES + i)) {
        Serial.printf("FAIL: data mismatch at byte %u.\n", unsigned(block * BLOCK_BYTES + i));
        readOK = false;
        break;
      }
    }
  }
  check.close();
  SD.end();

  if (readOK) {
    Serial.printf("PASS: %u bytes written, closed, reopened and byte-verified.\n", unsigned(TOTAL_BYTES));
    Serial.printf("Keep %s for computer-side verification.\n", filename);
  } else {
    Serial.println("FAIL: read-back verification failed.");
  }
}

void setup() {
  Serial.begin(115200);
  delay(1600); // Allow a USB serial monitor to attach; not required for SD operations.
  testSD();
  Serial.println("Test finished. Press RESET to run again with a new filename.");
}

void loop() {
  delay(1000);
}
