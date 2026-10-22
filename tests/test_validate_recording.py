import importlib.util
from pathlib import Path
import struct
import tempfile
import unittest

script = Path(__file__).resolve().parents[1] / 'Stage0B_Recorder/validate_recording.py'
spec = importlib.util.spec_from_file_location('validator', script)
validator = importlib.util.module_from_spec(spec)
spec.loader.exec_module(validator)

class ValidatorTests(unittest.TestCase):
    def test_sessions_and_failures(self):
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / 'RUN0001.BIN'
            records = [struct.pack('<6h', *[(((n*(2*c+1)+997*c)%65536)-32768) for c in range(6)]) for n in range(1000)]
            self.assertEqual(struct.unpack('<6h', records[0]), (-32768,-31771,-30774,-29777,-28780,-27783))
            for count in (0,1,42,1000):
                path.write_bytes(b''.join(records[:count]))
                self.assertEqual(validator.validate(path,count),count)
            data = b''.join(records)
            for bad in (data[:-1], data[:-12], data+records[0], data[:12]+data[24:], records[0]+data, b'bad'+data[3:]):
                path.write_bytes(bad)
                with self.assertRaises(ValueError): validator.validate(path,1000)
            with self.assertRaises(ValueError): validator.validate(path,-1)
