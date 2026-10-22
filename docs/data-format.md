# Data format

## Existing Stage 0A test

T0001.BIN through T9999.BIN contain 262,144 pattern bytes with no header. For zero-based absolute byte offset `i`, expected byte is `(i * 17 + (i >> 8) + 0x5A) & 0xFF`. This is a storage test, not a sensor recording format.

## Stage 0B — design pending

Before implementation define byte order, magic, format version, header/record lengths, six signed 16-bit synthetic channels, sequence numbers, timestamps, deterministic generation, completion/error records and corruption checks. Specify overflow, truncation, unsupported-version and incomplete-session handling. Document exact field offsets and validator rules here once selected.

Repository version is not the binary format version. Integrated format must later carry raw values, scale/configuration metadata, timing anchors, optical events, errors and firmware identity. No RUN file format is implemented or accepted yet.
