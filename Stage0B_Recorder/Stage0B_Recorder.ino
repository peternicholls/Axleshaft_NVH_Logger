/* Polestar NVH Logger 0.2.4 — standalone Phase 0B. See README.txt. */
#include <Arduino.h>
#include <SPI.h>
#include <SD.h>
#include <esp_timer.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include "ButtonTest.h"
#include "RecordingChecks.h"
using button_test::Button;
// Generic ESP32 Dev Module has no LED_BUILTIN definition. Common WROOM
// development boards use GPIO2; adjust these for the actual board if needed.
#ifdef LED_BUILTIN
constexpr uint8_t RECORDING_LED_PIN = LED_BUILTIN;
#else
constexpr uint8_t RECORDING_LED_PIN = 2;
#endif
constexpr bool RECORDING_LED_ACTIVE_LOW = false;
static void recordingLed(bool on) {
  digitalWrite(RECORDING_LED_PIN, (on != RECORDING_LED_ACTIVE_LOW) ? HIGH : LOW);
}
constexpr unsigned CAPACITY = 1024;
struct Sample { int16_t channel[6]; };
static_assert(sizeof(Sample) == 12, "sample size");
enum class State { Starting, Idle, Recording, Saving, Error };
// Explicit declarations prevent Arduino from placing these before State.
static State getState();
static void setState(State value);
portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;
State state = State::Starting;
QueueHandle_t queueHandle;
StaticQueue_t queueControl;
uint8_t queueMemory[CAPACITY * sizeof(Sample)];
esp_timer_handle_t timerHandle;
uint64_t startUs, cutoffUs, nextSlot, generated, missed, dropped;
unsigned peak;
Button button;

static State getState() {
  portENTER_CRITICAL(&mux); State value = state; portEXIT_CRITICAL(&mux); return value;
}
static void setState(State value) {
  portENTER_CRITICAL(&mux); state = value; portEXIT_CRITICAL(&mux);
}
// Called only with mux held. Actual synthetic generation, never backfilling.
static void generateSlotLocked(uint64_t slot) {
  missed += slot - nextSlot;
  nextSlot = slot + 1;
  Sample sample;
  for (unsigned c = 0; c < 6; ++c)
    sample.channel[c] = int32_t((slot * (2*c+1) + 997*c) & 65535) - 32768;
  if (xQueueSend(queueHandle, &sample, 0) == pdTRUE) {
    ++generated;
    unsigned used = uxQueueMessagesWaiting(queueHandle);
    if (used > peak) peak = used;
  } else ++dropped;
}
static void stopLocked(uint64_t now) {
  cutoffUs = now;
  uint64_t eligible = recording_checks::eligible(startUs, now);
  // The button may precede the periodic callback within the current slot.
  // Service that current synthetic slot once; retain all earlier misses.
  if (eligible > nextSlot) generateSlotLocked(recording_checks::stopSlot(startUs, now));
  state = State::Saving;
}
static void sampleTimer(void *) {
  portENTER_CRITICAL(&mux);
  uint64_t now = esp_timer_get_time();
  if (state == State::Recording && now >= startUs) {
    uint64_t slot = (now - startUs) / 1000;
    if (slot >= nextSlot) {
      generateSlotLocked(slot);
      if (dropped) { cutoffUs = now + 1; state = State::Saving; }
    }
  }
  portEXIT_CRITICAL(&mux);
}
static void error(const char *message) {
  setState(State::Error);
  Serial.printf("ERROR: %s. Partial file retained; reset to retry.\n", message);
}
static void storageTask(void *) {
  pinMode(13, OUTPUT); digitalWrite(13, HIGH);
  SPI.begin(18, 19, 23, 13);
  if (!SD.begin(13, SPI, 1000000) || SD.cardType() == CARD_NONE) {
    error("SD mount/card failed"); vTaskDelete(nullptr); return;
  }
  unsigned highest = 0;
  File root = SD.open("/");
  if (!root || !root.isDirectory()) {
    error("directory scan failed"); vTaskDelete(nullptr); return;
  }
  File entry = root.openNextFile();
  while (entry) {
    unsigned n = button_test::runNumber(entry.name());
    if (n > highest) highest = n;
    entry.close(); entry = root.openNextFile();
  }
  root.close();
  Serial.printf("Card bytes=%llu; highest run=%u; ready, release then press.\n",
                (unsigned long long)SD.cardSize(), highest);
  setState(State::Idle);
  for (;;) {
    if (getState() != State::Starting) { vTaskDelay(pdMS_TO_TICKS(1)); continue; }
    char name[13];
    if (!button_test::nextName(highest, name, sizeof(name)) || SD.exists(name)) {
      error("filename exhausted or already exists"); continue;
    }
    File file = SD.open(name, FILE_WRITE);
    if (!file) { error("file create failed; card may be full"); continue; }
    ++highest;
    xQueueReset(queueHandle);
    portENTER_CRITICAL(&mux);
    generated = missed = dropped = nextSlot = 0; peak = 0;
    startUs = uint64_t(esp_timer_get_time()) + 1000;
    cutoffUs = 0; state = State::Recording;
    portEXIT_CRITICAL(&mux);
    Serial.printf("START %s — press to stop\n", name);
    uint8_t block[504];  // 42 complete records; no padding in file.
    size_t used = 0;
    uint64_t persisted = 0, bytes = 0;
    uint32_t maxWrite = 0;
    uint32_t writtenCrc = 0xffffffffU;
    bool ok = true;
    while (ok) {
      Sample sample;
      if (xQueueReceive(queueHandle, &sample, pdMS_TO_TICKS(2)) == pdTRUE) {
        for (unsigned c = 0; c < 6; ++c) {
          uint16_t value = uint16_t(sample.channel[c]);
          block[used++] = uint8_t(value); block[used++] = uint8_t(value >> 8);
        }
      }
      bool draining = getState() == State::Saving;
      bool empty = uxQueueMessagesWaiting(queueHandle) == 0;
      if (used == sizeof(block) || (draining && empty && used)) {
        uint64_t before = esp_timer_get_time();
        size_t count = file.write(block, used);
        uint32_t duration = uint64_t(esp_timer_get_time()) - before;
        if (duration > maxWrite) maxWrite = duration;
        bytes += count;
        if (count != used) { ok = false; break; }
        writtenCrc = recording_checks::crcUpdate(writtenCrc, block, used);
        persisted += used / 12; used = 0;
      }
      if (draining && empty && used == 0) break;
      if (getState() == State::Error) { ok = false; break; }
    }
    if (!ok) {
      portENTER_CRITICAL(&mux);
      if (state == State::Recording) stopLocked(esp_timer_get_time());
      state = State::Error;
      portEXIT_CRITICAL(&mux);
    }
    uint64_t closing = esp_timer_get_time();
    file.flush(); file.close();
    uint64_t closeUs = uint64_t(esp_timer_get_time()) - closing;
    // Compare stored bytes with bytes actually submitted, independently of gaps.
    bool verified = ok;
    File check = SD.open(name, FILE_READ);
    if (!check || check.size() != bytes) verified = false;
    uint64_t readBytes = 0;
    uint32_t readCrc = 0xffffffffU;
    while (verified && readBytes < bytes) {
      size_t wanted = bytes - readBytes < sizeof(block) ? size_t(bytes-readBytes) : sizeof(block);
      if (check.read(block, wanted) != wanted) { verified = false; break; }
      readCrc = recording_checks::crcUpdate(readCrc, block, wanted);
      readBytes += wanted;
      vTaskDelay(1);
    }
    if (readCrc != writtenCrc) verified = false;
    check.close();
    bool accounting = nextSlot == generated + missed + dropped && generated == persisted;
    Serial.printf("CHECKS write=%s readback=%s accounting=%s continuity=%s crc32=%08lx\n",
      ok ? "PASS" : "FAIL", !ok ? "NOT_CHECKED" : (verified ? "PASS" : "FAIL"),
      accounting ? "PASS" : "FAIL", (missed == 0 && dropped == 0) ? "PASS" : "FAIL",
      (unsigned long)(writtenCrc ^ 0xffffffffU));
    Serial.printf("SUMMARY %s duration_us=%llu generated=%llu persisted=%llu bytes=%llu missed=%llu dropped=%llu max_write_us=%lu peak_records=%u peak_bytes=%u flush_close_us=%llu\n",
      name, (unsigned long long)(cutoffUs > startUs ? cutoffUs-startUs : 0),
      (unsigned long long)generated, (unsigned long long)persisted, (unsigned long long)bytes,
      (unsigned long long)missed, (unsigned long long)dropped, (unsigned long)maxWrite,
      peak, peak*12, (unsigned long long)closeUs);
    if (verified && accounting && missed == 0 && dropped == 0) {
      Serial.printf("PASS %s expected_samples=%llu — closed and CRC read-back verified\n", name,
                    (unsigned long long)persisted);
      setState(State::Idle);
    } else error("storage, sample continuity or read-back failed");
  }
}
void setup() {
  Serial.begin(115200);
  pinMode(32, INPUT_PULLUP); pinMode(RECORDING_LED_PIN, OUTPUT); pinMode(33, OUTPUT);
  recordingLed(false); digitalWrite(33, LOW);
  delay(1600);
  Serial.println("Phase 0B 0.2.4; 1000 Hz, six int16 channels, 12 KiB queue. No auto-stop.");
  queueHandle = xQueueCreateStatic(CAPACITY, sizeof(Sample), queueMemory, &queueControl);
  esp_timer_create_args_t args = {};
  args.callback = sampleTimer; args.name = "samples"; args.skip_unhandled_events = true;
  if (!queueHandle || esp_timer_create(&args, &timerHandle) != ESP_OK ||
      esp_timer_start_periodic(timerHandle, 1000) != ESP_OK) { error("timer/queue setup failed"); return; }
  if (xTaskCreate(storageTask, "storage", 6144, nullptr, 1, nullptr) != pdPASS)
    error("storage task creation failed");
}
void loop() {
  static State previous = State::Starting;
  State current = getState();
  if (current != previous && (current == State::Idle || current == State::Error))
    button.requireRelease();
  previous = current;
  if (button.update(digitalRead(32) == LOW, millis())) {
    portENTER_CRITICAL(&mux);
    if (state == State::Idle) state = State::Starting;
    else if (state == State::Recording) stopLocked(esp_timer_get_time());
    portEXIT_CRITICAL(&mux);
  }
  current = getState();
  bool led = current == State::Recording ||
    ((current == State::Starting || current == State::Saving) && (millis()/500)%2 == 0) ||
    (current == State::Error && (millis()/125)%2 == 0);
  recordingLed(led);
  delay(1);
}
