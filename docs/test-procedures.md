# Test procedures

## Stage 0A historical baseline

Supplied handover reports T0001.BIN, 262,144 bytes, complete reopen/byte verification, SDSC approximately 0.01 GiB, 2.93 s writing and maximum observed 512-byte write of 58 ms. No rerun has been performed. Follow the original sketch instructions for a new run and retain serial results.

## Stage 0B acceptance checklist — all pending

- [ ] Pin/module checks and toolchain versions recorded.
- [ ] One start and one stop per debounced press, including held/bouncing button cases.
- [ ] LED off idle, on recording, slow flash saving, rapid flash error; indication matches actual file state.
- [ ] Sequential files survive reboot without overwriting; name exhaustion handled.
- [ ] Deterministic samples and sequence numbers validate independently.
- [ ] Numerical scheduling tolerance is documented before testing; sample count agrees with acquisition duration within that tolerance.
- [ ] Stop drains outstanding buffers and closes; file reopens and validates.
- [ ] Write latency, buffer high-water mark and sample-loss counters are recorded under sustained and induced-stall conditions.
- [ ] Storage-full, short-write and unavailable-card cases produce explicit errors and preserve diagnostic evidence.
- [ ] Truncated/corrupted files and missing/duplicate records are rejected by the host validator.
- [ ] No unexplained missing, duplicate or corrupted records.

Begin with short recordings on the tiny development card. Exercise storage exhaustion on disposable card contents after preserving wanted recordings. Define expected error behaviour before injecting failures. Do not remove cards during powered recording as a routine stop method.

Record each outcome and remaining failures in progress and development log. Stage 1/2 remain gated until all Stage 0B criteria pass on hardware.

## Phase 0B executable instructions

Follow [the standalone README](../Stage0B_Recorder/README.txt) for the current firmware and validation commands. Record ~60 seconds, stop with GPIO32, require full read-back PASS and zero missed/dropped slots; compare persisted count against duration_us/1000 within one boundary slot. Repeat short sessions, reboot numbering and insufficient-space tests. All physical checklist boxes above remain pending. Current file format deliberately uses raw six-channel records and Serial evidence instead of an embedded footer.

Host checks: `python3 -m unittest discover -s tests -v`; compile/run `tests/test_controls.cpp` using a C++17 compiler with warnings and address/undefined sanitizers. Firmware build: `arduino-cli compile --fqbn esp32:esp32:esp32 --warnings all Stage0B_Recorder`.

## Optical SD test — 0.4.0 (physical execution pending)

Use [Stage 1 instructions](../Stage1_Optical_Test/README.txt) to set UTC over Serial, record a known 20-revolution session, stop and require accepted=persisted=20, 320 bytes, zero queue drops and all checks PASS. Copy the file and run validate_optical.py with Serial count/CRC. Repeat dated-file collisions/restarts, controlled speeds and disposable-card insufficient-space cases; preserve Serial and recording hashes. Code-preparation chat does not upload or operate hardware.

## Timing and response experiments — procedure prepared, not executed

Follow [NVH-TIME-01](experiments/timing-protocol.md) and the [illustrated PDF](../output/pdf/NVH-TIME-01-experiment-protocol.pdf). Run prerequisites first: Stage 2 and combined instrumentation are not yet implemented. Record reference capabilities and numerical tolerances before acceptance runs. Preserve candidate and qualified-event timestamps separately and never substitute SD/FIFO read time for sample time. Independent references are required for claims about isolated/absolute component delay; the basic motor/tap rig supports combined repeatability first. No upload/hardware operation is performed in this chat.
