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
