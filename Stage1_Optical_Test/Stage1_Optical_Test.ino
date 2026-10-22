/* Polestar NVH Logger 0.3.0 — standalone optical sensor bench test.
   Read README.txt before wiring. No SD, IMU or Phase 0B code required. */
#include <Arduino.h>
#include <esp_timer.h>
#include "PulseCounter.h"
constexpr uint8_t OPTICAL_PIN = 26;
// Verify on Serial which transition corresponds to the reference mark.
constexpr int OPTICAL_EDGE = FALLING;
constexpr uint32_t MIN_PULSE_INTERVAL_US = 1000; // provisional; not validated
constexpr uint64_t STOP_TIMEOUT_US = 2000000;
#ifdef LED_BUILTIN
constexpr uint8_t INDICATOR_PIN = LED_BUILTIN;
#else
constexpr uint8_t INDICATOR_PIN = 2;
#endif
constexpr bool INDICATOR_ACTIVE_LOW = false;
portMUX_TYPE pulseMux = portMUX_INITIALIZER_UNLOCKED;
PulseCounter pulses;
static void opticalEdge() {
  uint64_t timestamp = esp_timer_get_time();
  portENTER_CRITICAL_ISR(&pulseMux);
  pulses.edge(timestamp, MIN_PULSE_INTERVAL_US);
  portEXIT_CRITICAL_ISR(&pulseMux);
}
void setup() {
  Serial.begin(115200);
  pinMode(OPTICAL_PIN, INPUT_PULLUP);
  pinMode(INDICATOR_PIN, OUTPUT);
  digitalWrite(INDICATOR_PIN, INDICATOR_ACTIVE_LOW ? HIGH : LOW);
  pinMode(33, OUTPUT); digitalWrite(33, LOW); // Future external rotation LED.
  delay(1600);
  Serial.println("\n=== STAGE 1 OPTICAL TEST 0.3.0 ===");
  Serial.printf("OUT=GPIO26, VCC=verified 3V3, GND=GND; FALLING edge; minimum_us=%lu\n",
                (unsigned long)MIN_PULSE_INTERVAL_US);
  Serial.println("One mark/revolution assumed. Counts are accepted edges, not verified revolutions.");
  attachInterrupt(digitalPinToInterrupt(OPTICAL_PIN), opticalEdge, OPTICAL_EDGE);
}
void loop() {
  PulseCounter snapshot;
  portENTER_CRITICAL(&pulseMux);
  snapshot = pulses;
  portEXIT_CRITICAL(&pulseMux);
  uint64_t now = esp_timer_get_time();
  bool recent = snapshot.accepted && now - snapshot.lastUs < 30000;
  digitalWrite(INDICATOR_PIN, (recent != INDICATOR_ACTIVE_LOW) ? HIGH : LOW);
  static uint32_t reportedMs = 0;
  if (uint32_t(millis() - reportedMs) >= 1000) {
    reportedMs = millis();
    bool stopped = !snapshot.accepted || now - snapshot.lastUs >= STOP_TIMEOUT_US;
    double rpm = !stopped && snapshot.intervalUs ? 60000000.0 / snapshot.intervalUs : 0;
    double average = snapshot.accepted > 1 ?
      60000000.0 * (snapshot.accepted - 1) / (snapshot.lastUs - snapshot.firstUs) : 0;
    Serial.printf("level=%d accepted=%llu rejected=%llu last_us=%llu interval_us=%llu min_us=%llu max_us=%llu rpm=%.2f average_rpm=%.2f stale=%d\n",
      digitalRead(OPTICAL_PIN), (unsigned long long)snapshot.accepted,
      (unsigned long long)snapshot.rejected, (unsigned long long)snapshot.lastUs,
      (unsigned long long)snapshot.intervalUs, (unsigned long long)snapshot.minUs,
      (unsigned long long)snapshot.maxUs, rpm, average, stopped);
  }
  delay(1);
}
