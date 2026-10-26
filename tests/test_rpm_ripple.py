"""Optional numeric-investigation tests; core reader remains stdlib-only."""
import importlib.util
from pathlib import Path
import math
import struct
import tempfile
import unittest
ROOT=Path(__file__).resolve().parents[1]
spec=importlib.util.spec_from_file_location('ripple',ROOT/'analysis-bench/investigations/rpm_ripple.py')
ripple=importlib.util.module_from_spec(spec);spec.loader.exec_module(ripple)

@unittest.skipUnless(importlib.util.find_spec('numpy'),'Existing NumPy runtime required for offline investigation only')
class RippleTests(unittest.TestCase):
    def setUp(self):
        self.temp=tempfile.TemporaryDirectory();self.addCleanup(self.temp.cleanup)
        self.path=Path(self.temp.name)/'run.BIN'
    def write(self,intervals):
        t=0;events=[(1,0)]
        for i,dt in enumerate(intervals):t+=dt;events.append((i+2,t))
        self.path.write_bytes(b''.join(struct.pack('<QQ',*r)for r in events))
    def test_known_five_event_pattern(self):
        self.write([round(200000+2000*math.sin(2*math.pi*i/5))for i in range(100)])
        result=ripple.inspect(self.path)
        self.assertGreater(result['lag5_linear_correlation'],.98)
        self.assertAlmostEqual(result['five_event_approx_seconds'],1,places=6)
        self.assertEqual(len(result['records']),100)
    def test_constant_and_linear_trend_not_false_ripple(self):
        for values in ([200000]*100,[200000+100*i for i in range(100)]):
            self.write(values)
            self.assertIsNone(ripple.inspect(self.path)['lag5_linear_correlation'])
    def test_malformed_and_too_short(self):
        self.path.write_bytes(b'bad')
        with self.assertRaises(ValueError):ripple.inspect(self.path)
        self.write([200000]*3)
        with self.assertRaises(ValueError):ripple.inspect(self.path)
    def test_limits(self):
        self.path.write_bytes(b'\0'*(4096*16+1))
        with self.assertRaisesRegex(ValueError,'limit'):ripple.inspect(self.path)
        self.path.write_bytes(b''.join(struct.pack('<QQ',i+1,2**53+i*100000)for i in range(7)))
        with self.assertRaisesRegex(ValueError,'precision'):ripple.inspect(self.path)
    def test_real_sources(self):
        expected=[('OPT_UNSET_cd77abcc_UP74_0001.BIN',17,.715282176645792),('OPT_UNSET_e3fab4b9_UP471_0001.BIN',47,.31925657613928704),('OPT_UNSET_e3fab4b9_UP484_0001.BIN',92,.7029669041385401)]
        for name,count,corr in expected:
            path=ripple.ARCHIVED_INPUTS/name
            self.assertTrue(path.exists(), 'Archived fixture required')
            result=ripple.inspect(path)
            self.assertEqual(result['events'],count)
            self.assertAlmostEqual(result['lag5_linear_correlation'],corr,places=10)
            self.assertEqual(sum(r['interval_us']for r in result['records']),result['last_event_us']-result['first_event_us'])
