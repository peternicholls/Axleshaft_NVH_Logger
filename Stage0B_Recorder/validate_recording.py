#!/usr/bin/env python3
"""Validate Phase 0B raw six-channel records using the Serial PASS sample count."""
import argparse
import struct
import sys
from pathlib import Path


def validate(path, expected_samples):
    if expected_samples < 0:
        raise ValueError('expected sample count must be nonnegative')
    index = 0
    with Path(path).open('rb') as recording:
        while data := recording.read(12):
            if len(data) != 12:
                raise ValueError('partial sample at end of file')
            if index >= expected_samples:
                raise ValueError('more samples than expected')
            expected = tuple(((index * (2*c+1) + 997*c) % 65536) - 32768 for c in range(6))
            if struct.unpack('<6h', data) != expected:
                raise ValueError(f'sample {index}: corruption or sequence gap')
            index += 1
    if index != expected_samples:
        raise ValueError(f'{index} samples; expected {expected_samples}')
    return index


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('file')
    parser.add_argument('--expected-samples', type=int, required=True)
    args = parser.parse_args()
    try:
        count = validate(args.file, args.expected_samples)
    except (OSError, ValueError) as error:
        print(f'FAIL: {error}', file=sys.stderr)
        return 1
    print(f'PASS: {count} sequential samples, {count * 12} bytes')
    return 0


if __name__ == '__main__':
    sys.exit(main())
