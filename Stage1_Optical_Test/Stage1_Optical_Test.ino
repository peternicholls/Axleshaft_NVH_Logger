/* Polestar NVH Logger 0.4.0 — standalone optical sensor bench test.
   Read README.txt before wiring. Optical SD event logging; no IMU or sibling runtime files. */
#include <Arduino.h>
#include <esp_timer.h>
#include <SPI.h>
#include <SD.h>
#include <time.h>
#include <sys/time.h>
#include <esp_system.h>
#include <stdlib.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include "OpticalStorage.h"
#include "OpticalSession.h"
#include "Button.h"
constexpr uint8_t BUTTON_PIN = 32;
button_test::Button button;
enum class LogState { Starting, Idle, Running, Saving, Error };
LogState logState = LogState::Starting; // protected by pulseMux
constexpr unsigned EVENT_CAPACITY = 256;
struct OpticalEvent { uint64_t sequence, elapsedUs; };
QueueHandle_t eventQueue;
StaticQueue_t queueControl;
uint8_t queueMemory[EVENT_CAPACITY * sizeof(OpticalEvent)];
uint64_t queueDrops = 0;
unsigned peakQueue = 0;
static void storageTask(void *);
constexpr uint8_t OPTICAL_PIN = 26;
// Detection polarity must match the module's stable output over the mark.
constexpr int DETECTED_LEVEL = LOW;
constexpr uint32_t STABLE_LEVEL_US = 1000;
esp_timer_handle_t qualificationTimer;
constexpr uint32_t MIN_PULSE_INTERVAL_US = 1000; // provisional; not validated
constexpr uint64_t STOP_TIMEOUT_US = 2000000;
#ifdef LED_BUILTIN
constexpr uint8_t INDICATOR_PIN = LED_BUILTIN;
#else
constexpr uint8_t INDICATOR_PIN = 2;
#endif
constexpr bool INDICATOR_ACTIVE_LOW = false;
portMUX_TYPE pulseMux = portMUX_INITIALIZER_UNLOCKED;
OpticalSession session;
static void opticalEdge() {
  portENTER_CRITICAL_ISR(&pulseMux);
  uint64_t timestamp = esp_timer_get_time();
  session.edge(digitalRead(OPTICAL_PIN) == DETECTED_LEVEL, timestamp);
  portEXIT_CRITICAL_ISR(&pulseMux);
}
static void qualifyDetection(void *) {
  portENTER_CRITICAL(&pulseMux);
  uint64_t before = session.pulses.accepted;
  session.qualify(esp_timer_get_time(), STABLE_LEVEL_US, MIN_PULSE_INTERVAL_US);
  if (session.pulses.accepted != before) {
    OpticalEvent event{session.pulses.accepted, session.pulses.lastUs-session.startUs};
    if (xQueueSend(eventQueue, &event, 0) == pdTRUE) {
      unsigned used = uxQueueMessagesWaiting(eventQueue);
      if (used > peakQueue) peakQueue = used;
    } else {
      ++queueDrops; session.stop(esp_timer_get_time()); logState = LogState::Saving;
    }
  }
  portEXIT_CRITICAL(&pulseMux);
}
void setup() {
  Serial.begin(115200);
  pinMode(OPTICAL_PIN, INPUT_PULLUP);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(INDICATOR_PIN, OUTPUT);
  digitalWrite(INDICATOR_PIN, INDICATOR_ACTIVE_LOW ? HIGH : LOW);
  pinMode(33, OUTPUT); digitalWrite(33, LOW); // Future external rotation LED.
  delay(1600);
  Serial.println("\n=== STAGE 1 OPTICAL TEST 0.4.0 ===");
  Serial.printf("OUT=GPIO26; stable detection cycle, clear to rearm; minimum_us=%lu\n",
                (unsigned long)MIN_PULSE_INTERVAL_US);
  Serial.println("One mark/revolution assumed. Counts are accepted edges, not verified revolutions.");
  Serial.println("Initialising SD. Set UTC clock with TIME <Unix-seconds> while idle; then release GPIO32 and press to start.");
  eventQueue = xQueueCreateStatic(EVENT_CAPACITY, sizeof(OpticalEvent), queueMemory, &queueControl);
  if (!eventQueue) {
    portENTER_CRITICAL(&pulseMux); logState = LogState::Error; portEXIT_CRITICAL(&pulseMux);
    Serial.println("ERROR: event queue setup failed; reset required."); return;
  }
  attachInterrupt(digitalPinToInterrupt(OPTICAL_PIN), opticalEdge, CHANGE);
  esp_timer_create_args_t args = {};
  args.callback = qualifyDetection; args.name = "optical_qualify";
  if (esp_timer_create(&args, &qualificationTimer) != ESP_OK ||
      esp_timer_start_periodic(qualificationTimer, 250) != ESP_OK) {
    detachInterrupt(digitalPinToInterrupt(OPTICAL_PIN));
    Serial.println("ERROR: qualification timer failed; reset to retry.");
    portENTER_CRITICAL(&pulseMux); logState = LogState::Error; portEXIT_CRITICAL(&pulseMux);
  } else if (xTaskCreate(storageTask, "optical_sd", 6144, nullptr, 1, nullptr) != pdPASS) {
    portENTER_CRITICAL(&pulseMux); logState = LogState::Error; portEXIT_CRITICAL(&pulseMux);
    Serial.println("ERROR: storage task creation failed; reset required.");
  }
}
// Bounded, nonblocking Serial clock input; never infer wall time from build time.
static void clockCommand() {
  static char line[32]; static unsigned used = 0; static bool overflow = false;
  unsigned budget = 32;
  while (budget-- && Serial.available()) {
    char c = Serial.read();
    if (c == '\r') continue;
    if (c != '\n') {
      if (used+1 < sizeof(line)) line[used++] = c; else overflow = true;
      continue;
    }
    line[used] = 0;
    bool valid = !overflow && used > 5 && strncmp(line,"TIME ",5) == 0;
    for (unsigned i=5; i<used; ++i) if (line[i]<'0' || line[i]>'9') valid = false;
    unsigned long long seconds = valid ? strtoull(line+5,nullptr,10) : 0;
    portENTER_CRITICAL(&pulseMux); bool idle = logState == LogState::Idle; portEXIT_CRITICAL(&pulseMux);
    if (valid && seconds >= 1577836800ULL && seconds < 4102444800ULL && idle) {
      struct timeval value = {}; value.tv_sec = seconds;
      if (settimeofday(&value,nullptr) == 0) Serial.println("UTC clock set. Filenames use UTC, not local daylight-saving time.");
      else Serial.println("ERROR: clock set failed");
    } else Serial.println("Clock command rejected: use TIME <Unix-seconds> (2020..2099) while idle.");
    used=0; overflow=false;
  }
}
void loop() {
  clockCommand();
  bool pressed = button.update(digitalRead(BUTTON_PIN) == LOW, millis());
  bool toggled = false;
  LogState stateSnapshot;
  OpticalSession snapshot;
  portENTER_CRITICAL(&pulseMux);
  uint64_t now = esp_timer_get_time();
  if (pressed) {
    if (logState == LogState::Idle) { logState = LogState::Starting; toggled = true; }
    else if (logState == LogState::Running) {
      session.stop(now); logState = LogState::Saving; toggled = true;
    }
  }
  stateSnapshot = logState;
  snapshot = session;
  portEXIT_CRITICAL(&pulseMux);
  bool led = snapshot.ledOn(now);
  if (stateSnapshot == LogState::Starting || stateSnapshot == LogState::Saving)
    led = (millis()/500)%2 == 0;
  if (stateSnapshot == LogState::Error) led = (millis()/125)%2 == 0;
  digitalWrite(INDICATOR_PIN, (led != INDICATOR_ACTIVE_LOW) ? HIGH : LOW);
  if (toggled) Serial.println(stateSnapshot == LogState::Starting ? "START requested; opening file" : "STOP requested; draining and saving");
  static uint32_t reportedMs = 0;
  if (toggled || uint32_t(millis() - reportedMs) >= 1000) {
    reportedMs = millis();
    const PulseCounter &pulses = snapshot.pulses;
    bool stale = !snapshot.running || !pulses.accepted || now-pulses.lastUs >= STOP_TIMEOUT_US;
    double rpm = !stale && pulses.intervalUs ? 60000000.0 / pulses.intervalUs : 0;
    double average = pulses.accepted > 1 ?
      60000000.0 * (pulses.accepted-1) / (pulses.lastUs-pulses.firstUs) : 0;
    Serial.printf("state=%d running=%d raw_transitions=%llu level=%d accepted=%llu rejected=%llu last_us=%llu interval_us=%llu min_us=%llu max_us=%llu rpm=%.2f average_rpm=%.2f stale=%d\n",
      int(stateSnapshot), snapshot.running, (unsigned long long)snapshot.rawTransitions, digitalRead(OPTICAL_PIN),
      (unsigned long long)pulses.accepted, (unsigned long long)pulses.rejected,
      (unsigned long long)pulses.lastUs, (unsigned long long)pulses.intervalUs,
      (unsigned long long)pulses.minUs, (unsigned long long)pulses.maxUs, rpm, average, stale);
  }
  delay(1);
}

static void storageError(const char *message) {
  portENTER_CRITICAL(&pulseMux);
  if (session.running) session.stop(esp_timer_get_time());
  logState = LogState::Error;
  portEXIT_CRITICAL(&pulseMux);
  Serial.printf("ERROR: %s; file retained if created; reset required.\n", message);
}
static void storageTask(void *) {
  pinMode(13, OUTPUT); digitalWrite(13, HIGH);
  SPI.begin(18,19,23,13);
  if (!SD.begin(13,SPI,1000000) || SD.cardType() == CARD_NONE) {
    storageError("SD mount/card failed"); vTaskDelete(nullptr); return;
  }
  const uint32_t bootId = esp_random();
  portENTER_CRITICAL(&pulseMux); logState = LogState::Idle; portEXIT_CRITICAL(&pulseMux);
  Serial.println("SD ready; release then press button. TIME <Unix-seconds> sets UTC filenames; otherwise UNSET boot/uptime names.");
  for (;;) {
    portENTER_CRITICAL(&pulseMux); bool starting = logState == LogState::Starting; portEXIT_CRITICAL(&pulseMux);
    if (!starting) { vTaskDelay(1); continue; }
    char label[64], filename[96];
    time_t utcNow = time(nullptr);
    if (utcNow >= 1577836800 && uint64_t(utcNow) < 4102444800ULL) {
      struct tm utc; gmtime_r(&utcNow, &utc);
      strftime(label, sizeof(label), "%Y-%m-%d_%H-%M-%S", &utc);
    } else snprintf(label, sizeof(label), "UNSET_%08lx_UP%llu",
        (unsigned long)bootId, (unsigned long long)(esp_timer_get_time()/1000000));
    bool found = false;
    for (unsigned suffix=1; suffix<=9999; ++suffix) {
      if (!optical_storage::eventFilename(label,suffix,filename,sizeof(filename))) break;
      if (!SD.exists(filename)) { found = true; break; }
    }
    if (!found) { storageError("filename collision suffixes exhausted"); continue; }
    File file = SD.open(filename, FILE_WRITE);
    if (!file) { storageError("file create failed"); continue; }
    xQueueReset(eventQueue);
    portENTER_CRITICAL(&pulseMux);
    queueDrops = 0; peakQueue = 0;
    session.start(digitalRead(OPTICAL_PIN) == DETECTED_LEVEL, esp_timer_get_time());
    logState = LogState::Running;
    portEXIT_CRITICAL(&pulseMux);
    Serial.printf("START %s; one 16-byte record per accepted detection\n", filename);
    uint8_t block[512]; size_t used = 0;
    uint64_t persisted = 0, bytes = 0;
    uint32_t crc = 0xffffffffU, maxWriteUs = 0, lastWriteMs = millis();
    bool ok = true;
    while (ok) {
      OpticalEvent event;
      if (xQueueReceive(eventQueue, &event, pdMS_TO_TICKS(2)) == pdTRUE) {
        optical_storage::encodeEvent(block+used, event.sequence, event.elapsedUs); used += 16;
      }
      portENTER_CRITICAL(&pulseMux); bool saving = logState == LogState::Saving; portEXIT_CRITICAL(&pulseMux);
      bool empty = uxQueueMessagesWaiting(eventQueue) == 0;
      if (used == sizeof(block) || (used && (saving || uint32_t(millis()-lastWriteMs) >= 100))) {
        uint64_t before = esp_timer_get_time();
        size_t count = file.write(block, used);
        uint32_t elapsed = uint64_t(esp_timer_get_time())-before;
        if (elapsed > maxWriteUs) maxWriteUs = elapsed;
        bytes += count;
        if (count != used) { ok = false; break; }
        crc = optical_storage::crcUpdate(crc, block, used);
        persisted += used/16; used = 0; lastWriteMs = millis();
      }
      if (saving && empty && used == 0) break;
    }
    if (!ok) storageError("short SD write (full card or storage failure)");
    uint64_t closing = esp_timer_get_time();
    file.flush(); file.close();
    uint64_t closeUs = uint64_t(esp_timer_get_time())-closing;
    bool verified = ok;
    File check = SD.open(filename, FILE_READ);
    if (!check || check.size() != bytes) verified = false;
    uint64_t readBytes = 0; uint32_t readCrc = 0xffffffffU;
    while (verified && readBytes < bytes) {
      size_t wanted = bytes-readBytes < sizeof(block) ? size_t(bytes-readBytes) : sizeof(block);
      if (check.read(block,wanted) != wanted) { verified = false; break; }
      readCrc = optical_storage::crcUpdate(readCrc,block,wanted); readBytes += wanted;
      vTaskDelay(1);
    }
    check.close(); verified = verified && readCrc == crc;
    portENTER_CRITICAL(&pulseMux);
    OpticalSession frozen = session; uint64_t drops = queueDrops; unsigned peak = peakQueue;
    portEXIT_CRITICAL(&pulseMux);
    bool accounting = persisted == frozen.pulses.accepted && drops == 0;
    Serial.printf("SUMMARY %s duration_us=%llu accepted=%llu persisted=%llu bytes=%llu raw=%llu rejected=%llu queue_drops=%llu peak_records=%u peak_bytes=%u max_write_us=%lu flush_close_us=%llu crc32=%08lx\n",
      filename, (unsigned long long)(frozen.stopUs-frozen.startUs),
      (unsigned long long)frozen.pulses.accepted, (unsigned long long)persisted,
      (unsigned long long)bytes, (unsigned long long)frozen.rawTransitions,
      (unsigned long long)frozen.pulses.rejected, (unsigned long long)drops,
      peak, peak*16, (unsigned long)maxWriteUs, (unsigned long long)closeUs,
      (unsigned long)(crc ^ 0xffffffffU));
    Serial.printf("CHECKS write=%s readback=%s accounting=%s\n", ok?"PASS":"FAIL",
      !ok?"NOT_CHECKED":(verified?"PASS":"FAIL"), accounting?"PASS":"FAIL");
    if (verified && accounting) {
      Serial.printf("PASS %s expected_events=%llu crc32=%08lx; closed and read-back verified\n",
        filename, (unsigned long long)persisted, (unsigned long)(crc ^ 0xffffffffU));
      portENTER_CRITICAL(&pulseMux); logState = LogState::Idle; portEXIT_CRITICAL(&pulseMux);
    } else storageError("optical storage verification/accounting failed");
  }
}
