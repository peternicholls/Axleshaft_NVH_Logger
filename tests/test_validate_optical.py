import importlib.util
from pathlib import Path
import struct
import tempfile
import unittest
import zlib
import subprocess
import sys
SCRIPT = Path(__file__).resolve().parents[1] / 'Stage1_Optical_Test/validate_optical.py'
spec = importlib.util.spec_from_file_location('optical_validator',SCRIPT)
module = importlib.util.module_from_spec(spec)
spec.loader.exec_module(module)

class OpticalValidatorTests(unittest.TestCase):
    def test_integrity(self):
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / 'OPT0001.BIN'
            records = [struct.pack('<QQ', n+1,n*100000) for n in range(100)]
            for count in (0,1,20,100):
                data=b''.join(records[:count]); path.write_bytes(data)
                self.assertEqual(module.validate(path,count,zlib.crc32(data)),count)
            good=b''.join(records)
            for bad in (good[:-1],good[:-16],good+records[-1],records[0]+good, good[:16]+good[32:],struct.pack('<QQ',1,100001)+good[16:]):
                path.write_bytes(bad)
                with self.assertRaises(ValueError): module.validate(path,100,zlib.crc32(good))
            # Correct CRC cannot conceal bad ordering or repeated timestamps.
            for bad in (struct.pack('<QQ',2,0),struct.pack('<QQQQ',1,10,2,10)):
                path.write_bytes(bad)
                with self.assertRaises(ValueError): module.validate(path,len(bad)//16,zlib.crc32(bad))
            path.write_bytes(good)
            command=[sys.executable,str(SCRIPT),str(path),'--expected-events','100','--expected-crc',f'{zlib.crc32(good):08x}']
            self.assertEqual(subprocess.run(command,capture_output=True).returncode,0)
            path.write_bytes(b'bad')
            self.assertEqual(subprocess.run(command,capture_output=True).returncode,1)
