#!/usr/bin/env python3
"""Verify optical event sequence, monotonic timestamps and Serial-reported CRC."""
import argparse
from pathlib import Path
import struct
import sys
import zlib


def validate(path, expected_events, expected_crc):
    if expected_events < 0 or not 0 <= expected_crc <= 0xffffffff:
        raise ValueError('invalid expected count or CRC32')
    count = 0
    previous = None
    crc = 0
    with Path(path).open('rb') as recording:
        while data := recording.read(16):
            if len(data) != 16:
                raise ValueError('truncated event')
            sequence, timestamp = struct.unpack('<QQ', data)
            if count >= expected_events or sequence != count + 1:
                raise ValueError(f'event {count+1}: unexpected sequence {sequence}')
            if previous is not None and timestamp <= previous:
                raise ValueError(f'event {sequence}: timestamp did not increase')
            previous = timestamp
            crc = zlib.crc32(data, crc)
            count += 1
    if count != expected_events:
        raise ValueError(f'{count} events; expected {expected_events}')
    if crc != expected_crc:
        raise ValueError(f'CRC32 {crc:08x}; expected {expected_crc:08x}')
    return count


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('file')
    parser.add_argument('--expected-events', type=int, required=True)
    parser.add_argument('--expected-crc', type=lambda s: int(s, 16), required=True)
    args = parser.parse_args()
    try:
        count = validate(args.file, args.expected_events, args.expected_crc)
    except (ValueError, OSError) as error:
        print(f'FAIL: {error}', file=sys.stderr)
        return 1
    print(f'PASS: {count} sequential events, {count*16} bytes, increasing timestamps, CRC verified')
    return 0


if __name__ == '__main__':
    sys.exit(main())
