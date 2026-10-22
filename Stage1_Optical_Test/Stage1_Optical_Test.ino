/* Polestar NVH Logger 0.3.8 — standalone optical sensor bench test.
   Read README.txt before wiring. No SD, IMU or Phase 0B code required. */
#include <Arduino.h>
#include <esp_timer.h>
#include "OpticalSession.h"
#include "Button.h"
constexpr uint8_t BUTTON_PIN = 32;
button_test::Button button;
bool ready = false;
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
  uint64_t timestamp = esp_timer_get_time();
  portENTER_CRITICAL_ISR(&pulseMux);
  session.edge(digitalRead(OPTICAL_PIN) == DETECTED_LEVEL, timestamp);
  portEXIT_CRITICAL_ISR(&pulseMux);
}
static void qualifyDetection(void *) {
  portENTER_CRITICAL(&pulseMux);
  session.qualify(esp_timer_get_time(), STABLE_LEVEL_US, MIN_PULSE_INTERVAL_US);
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
  Serial.println("\n=== STAGE 1 OPTICAL TEST 0.3.8 ===");
  Serial.printf("OUT=GPIO26; stable detection cycle, clear to rearm; minimum_us=%lu\n",
                (unsigned long)MIN_PULSE_INTERVAL_US);
  Serial.println("One mark/revolution assumed. Counts are accepted edges, not verified revolutions.");
  Serial.println("IDLE: release GPIO32 button, press to start; press again to stop. LED on testing, off pulses per detection.");
  attachInterrupt(digitalPinToInterrupt(OPTICAL_PIN), opticalEdge, CHANGE);
  esp_timer_create_args_t args = {};
  args.callback = qualifyDetection; args.name = "optical_qualify";
  if (esp_timer_create(&args, &qualificationTimer) != ESP_OK ||
      esp_timer_start_periodic(qualificationTimer, 250) != ESP_OK) {
    detachInterrupt(digitalPinToInterrupt(OPTICAL_PIN));
    Serial.println("ERROR: qualification timer failed; reset to retry.");
  } else ready = true;
}
void loop() {
  bool toggled = ready && button.update(digitalRead(BUTTON_PIN) == LOW, millis());
  OpticalSession snapshot;
  portENTER_CRITICAL(&pulseMux);
  uint64_t now = esp_timer_get_time();
  if (toggled) {
    if (session.running) session.stop(now);
    else session.start(digitalRead(OPTICAL_PIN) == DETECTED_LEVEL, now);
  }
  snapshot = session;
  portEXIT_CRITICAL(&pulseMux);
  digitalWrite(INDICATOR_PIN, (snapshot.ledOn(now) != INDICATOR_ACTIVE_LOW) ? HIGH : LOW);
  if (toggled) {
    if (snapshot.running) Serial.println("START: counters reset; clear mark before detection if starting covered.");
    else Serial.printf("STOP duration_us=%llu accepted=%llu rejected=%llu raw_transitions=%llu\n",
      (unsigned long long)(snapshot.stopUs-snapshot.startUs),
      (unsigned long long)snapshot.pulses.accepted,
      (unsigned long long)snapshot.pulses.rejected,
      (unsigned long long)snapshot.rawTransitions);
  }
  static uint32_t reportedMs = 0;
  if (toggled || uint32_t(millis() - reportedMs) >= 1000) {
    reportedMs = millis();
    const PulseCounter &pulses = snapshot.pulses;
    bool stale = !snapshot.running || !pulses.accepted || now-pulses.lastUs >= STOP_TIMEOUT_US;
    double rpm = !stale && pulses.intervalUs ? 60000000.0 / pulses.intervalUs : 0;
    double average = pulses.accepted > 1 ?
      60000000.0 * (pulses.accepted-1) / (pulses.lastUs-pulses.firstUs) : 0;
    Serial.printf("running=%d raw_transitions=%llu level=%d accepted=%llu rejected=%llu last_us=%llu interval_us=%llu min_us=%llu max_us=%llu rpm=%.2f average_rpm=%.2f stale=%d\n",
      snapshot.running, (unsigned long long)snapshot.rawTransitions, digitalRead(OPTICAL_PIN),
      (unsigned long long)pulses.accepted, (unsigned long long)pulses.rejected,
      (unsigned long long)pulses.lastUs, (unsigned long long)pulses.intervalUs,
      (unsigned long long)pulses.minUs, (unsigned long long)pulses.maxUs, rpm, average, stale);
  }
  delay(1);
}
