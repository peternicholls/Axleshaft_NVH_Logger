# Wireless monitoring and recording control — proposed

Planning deliverable 0.3.4, 2026-10-22. No wireless firmware implemented or tested.

## Hardware and approach

The supplied photo shows an ESP-32 module and PCB antenna. The existing upload evidence identifies ESP32-D0WD-V3; the photo alone does not establish antenna performance or board pin behaviour. The ESP32 family supports 2.4 GHz Wi-Fi and Bluetooth/BLE. Arduino-ESP32 supports SoftAP operation and serving a local HTTP interface: see [ESP32 datasheet](https://www.espressif.com/sites/default/files/documentation/esp32_datasheet_en.pdf) and [official Wi-Fi API](https://docs.espressif.com/projects/arduino-esp32/en/latest/api/wifi.html).

Preferred first implementation: ESP32 creates a password-protected local Wi-Fi network; a phone/tablet/laptop joins and opens a small browser page hosted on the board. Serve assets locally, with no router, internet, cloud or app installation required. Display the SSID and actual local URL in Serial and the standalone instructions. Use installed Arduino-ESP32 WiFi/WebServer facilities after verifying APIs against the installed core; add no dependencies.

This is a design choice pending qualification, not an RF range claim. BLE remains an alternative if measured Wi-Fi overhead or range proves unsuitable; it needs a compatible client and separate validation. Joining an existing hotspot introduces an additional network dependency, so defer station mode initially. No need for Wi-Fi and BLE simultaneously in the first test.

## User interface

- Explicit Start and Stop buttons, plus confirmed recorder state: idle, starting, recording, saving, error.
- Current filename/session, elapsed duration, generated/persisted count, bytes, missed/dropped counts, buffer occupancy/peak, worst write latency, free space estimate, firmware version and last error.
- Optical accepted/rejected counts, RPM and last-event age when an optical source is actually available; show unavailable in the initial synthetic-only wireless test.
- One client initially; poll compact status at 2 Hz. Show disconnected/stale status after 3 s without a response. Never present a stale state as current.
- Stop means request accepted first, then saving, then confirmed closed/read-back result. Do not label request receipt as saved.

Keep the GPIO32 button, built-in indicator, Serial diagnostics and autonomous SD logging. Wireless supplements them and may eventually reduce cabin wiring if the physical installation passes range tests. Do not remove the physical controls during initial qualification.

## Command and failure semantics

Use explicit start/stop operations rather than a remotely retried toggle. A single recorder state machine consumes bounded commands from both button and network. Assign command IDs and recorder boot/session identity; deduplicate retries and reject obsolete session commands. Starting an already active session must not create another file; stopping an idle recorder must not start one. A status page refresh must have no recording side effect. Report busy/rejected/error outcomes explicitly.

Link loss, browser close and client reconnection must not stop or restart acquisition. Recording continues autonomously until an explicit valid stop, physical button action or recorder error. On reconnect, read actual state before issuing commands. Boot starts idle and never replays an old browser command. Network failure leaves local recording controls operational; recording errors remain visible through existing indicators and Serial.

Use a configured, non-default AP passphrase, limit initial client count, require an unpredictable per-boot control token for modifying POST requests, and reject requests from other origins. Never embed real credentials in committed source or expose control on the public internet. Document credential configuration in the eventual standalone README. These controls are a bounded local design, not a general security certification.

## Protect acquisition

Network callbacks must not touch SD or block acquisition. Copy bounded status snapshots under short locks; queue commands to the recorder owner. Keep handlers, request size, command queue, timeouts and telemetry rate bounded. Measure CPU/task stack/free heap and queue pressure. Do not stream raw samples, download files or update firmware while recording in the initial implementation. Defer live plots and OTA until separately justified.

Wi-Fi affects scheduling, RAM, supply load and RF conditions. Qualify on/off behaviour rather than assuming task separation is enough. Disable radio cleanly for the comparison baseline; expose whether wireless is enabled in diagnostics. Never alter existing SPI pin allocation or sample accounting to hide network-induced losses.

## Sequence and standalone packaging

1. Finish pending recorder continuity/storage checks and optical single-flash bench checks as independent evidence. Wireless planning does not accept either stage.
2. Build an isolated root-level `StageWireless_Test/` folder containing matching `.ino`, README.txt, all local helpers and any local webpage assets, using the Stage 0A/0B convention. Start with synthetic recording/status; no sensor integration required.
3. Factor or reuse proven recorder behaviour through deliberate, regression-checked changes; preserve the independently usable earlier stage folders. Implement command state/duplicate/reconnect tests before adding transport.
4. Add the bounded SoftAP/web interface and credential configuration; compile with recorded board/core settings.
5. Run host/browser and physical test matrix below. Integrate with the final sensor logger only after subsystem qualification and a separate timing review.

No implementation is authorised merely by this planning entry. Future implementation should follow the normal version, changelog, progress and evidence workflow.

## Acceptance gates — all pending

| Check | Required result |
|---|---|
| Basic client | Join local AP and operate page without internet on intended phone/browser; URL/instructions verified |
| Controls | 20 start/stop sessions, mixing button and browser; retries/double taps produce one operation and one incremented file per session |
| Ordering | Duplicate, delayed, stale-session and busy commands give deterministic responses; never overwrite or replay starts after reboot |
| UI | State converges within 2 s on a healthy link; saving shown until verified completion; stale indicator within 3 s of failed status replies |
| Acquisition | Three approximately 60 s sessions radio-off, three radio-on at 2 Hz status polling and commanded stops; zero unexplained misses/drops and all files host-validated |
| Stress | Reconnects, malformed/oversized requests and bounded request bursts do not starve producer or corrupt files; reject excess work and record diagnostics |
| Loss/recovery | Disconnect browser mid-recording for at least 30 s, reconnect and stop; one continuous file, no unsolicited state change; repeat with physical button stop while disconnected |
| Storage errors | Insufficient space and missing-card cases preserve diagnostics and local control; remote state agrees with actual error, never falsely reports saved |
| Range/power | Measure cabin-to-proposed-mount connectivity with final enclosure/antenna orientation, representative vehicle obstruction and supply; no reset/brownout or loss of local logging |
| Evidence | Record board/core/commit, client/browser, setup, files/hashes, command outcomes, timing/loss/buffer/heap metrics, Serial and validator results |

Antenna clearance and metal shielding under the vehicle require experimental placement. Do initial bench and stationary vehicle range checks before relying on cabin wireless control. The current photo and desktop build cannot qualify this deployment.
