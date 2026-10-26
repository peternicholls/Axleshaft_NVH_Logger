"""Reader format, confinement and HTTP regressions; no third-party packages."""
import hashlib
import http.client
import importlib.util
import json
from pathlib import Path
import struct
import tempfile
import threading
import unittest
from unittest.mock import patch, MagicMock
import zlib

ROOT = Path(__file__).resolve().parents[1]
_spec = importlib.util.spec_from_file_location('reader', ROOT / 'analysis-bench/reader.py')
reader = importlib.util.module_from_spec(_spec)
_spec.loader.exec_module(reader)


def records(count, spacing=100000):
    return b''.join(struct.pack('<QQ', n+1, n*spacing) for n in range(count))


class ReaderTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name).resolve()
        self.path = self.root / 'OPT_UNSET_TEST.BIN'
        self.path.write_bytes(records(11))

    def read(self, **kwargs):
        return reader.read_run(self.root, self.path.name, **kwargs)

    def test_real_evidence_and_references(self):
        data = (ROOT / 'docs/evidence/OPT_2026-10-22_14-45-28_0001.BIN').read_bytes()
        self.path.write_bytes(data)
        result = self.read()
        self.assertEqual(result['event_count'], 11)
        self.assertEqual(result['size'], 176)
        self.assertEqual(result['crc32'], 'd0aa5ebe')
        self.assertEqual(result['sha256'], hashlib.sha256(data).hexdigest())
        self.assertIsNone(result['wall_time'])
        self.assertEqual(result['validation']['structure'], 'pass')
        self.assertEqual(result['validation']['completion'], 'unverified')
        self.assertEqual(self.read(expected_events=11)['validation']['completion'], 'unverified')
        self.assertEqual(self.read(expected_events=11, expected_crc=0xd0aa5ebe)['validation']['completion'], 'reference-match')
        for args in ({'expected_events': 12}, {'expected_crc': 0}, {'expected_events': 11, 'expected_crc': 0}):
            self.assertEqual(self.read(**args)['validation']['completion'], 'reference-mismatch')

    def test_600_rpm_and_no_wall_time(self):
        result = self.read()
        self.assertEqual(result['stats']['mean_rpm'], 600)
        self.assertEqual(result['stats']['span_us'], 1000000)
        self.assertEqual(result['stats']['mean_interval_us'], 100000)
        self.assertEqual(result['events'][0], [1, 0])
        self.assertIsNone(result['wall_time'])

    def test_empty_and_single(self):
        for count in (0, 1):
            self.path.write_bytes(records(count))
            result = self.read()
            self.assertEqual(result['event_count'], count)
            self.assertEqual(result['validation']['structure'], 'pass')
            self.assertEqual(result['validation']['completion'], 'unverified')
            self.assertIsNone(result['stats']['mean_rpm'])
            self.assertIsNone(result['stats']['mean_interval_us'])
        self.assertEqual(self.read()['stats']['span_us'], 0)

    def test_structural_failures_preserve_diagnostics(self):
        for data in (records(2)+b'x', struct.pack('<QQ', 2, 0),
                     struct.pack('<QQQQ', 1, 10, 2, 10),
                     struct.pack('<QQQQ', 1, 10, 2, 9)):
            self.path.write_bytes(data)
            result = self.read()
            self.assertEqual(result['validation']['structure'], 'fail')
            self.assertIn('DIAGNOSTIC ONLY', result['validation']['message'])
            self.assertEqual(len(result['events']), len(data)//16)
            self.assertIsNone(result['stats']['mean_rpm'])
            crc = zlib.crc32(data)
            result = self.read(expected_events=len(data)//16, expected_crc=crc)
            self.assertEqual(result['validation']['completion'], 'reference-mismatch')

    def test_limits(self):
        self.path.write_bytes(b'x'*(reader.MAX_BYTES+1))
        with self.assertRaises(reader.ReaderError) as error:
            self.read()
        self.assertEqual(error.exception.status, 413)
        self.path.write_bytes(struct.pack('<QQ', 1, 2**53))
        with self.assertRaises(reader.ReaderError):
            self.read()
        self.path.write_bytes(records(65536))
        self.assertEqual(self.read()['event_count'], 65536)

    def test_confinement_and_recursive_list(self):
        nested = self.root / 'nested'
        nested.mkdir()
        (nested/'quoted"name.bin').write_bytes(b'')
        (nested/'skip.txt').write_text('ignore')
        with tempfile.TemporaryDirectory() as outside:
            external = Path(outside) / 'escape.bin'
            external.write_bytes(records(2))
            (self.root/'escape.bin').symlink_to(external)
            (self.root/'UPPER_ESCAPE.BIN').symlink_to(external)
            result = reader.list_runs(self.root)
            self.assertEqual([x['name'] for x in result['runs']], [self.path.name, 'nested/quoted"name.bin'])
            for name in ('../escape.bin', str(external), 'escape.bin', 'UPPER_ESCAPE.BIN', 'nested/../../escape.bin', 'x\\y.bin', 'nested/skip.txt'):
                with self.assertRaises(reader.ReaderError):
                    reader.read_run(self.root, name)
        with self.assertRaises(reader.ReaderError) as error:
            reader.read_run(self.root, 'missing.bin')
        self.assertEqual(error.exception.status, 404)

    def test_open_flag_opens_only_local_url(self):
        server = MagicMock()
        server.server_address = ('127.0.0.1', 54321)
        server.serve_forever.side_effect = KeyboardInterrupt
        with patch.object(reader, 'ThreadingHTTPServer', return_value=server) as constructor, \
                patch.object(reader.webbrowser, 'open') as browser, \
                patch('sys.argv', ['reader.py', '--runs-dir', str(self.root), '--port', '0', '--open']):
            reader.main()
        self.assertEqual(constructor.call_args.args[0], ('127.0.0.1', 0))
        browser.assert_called_once_with('http://127.0.0.1:54321')
        server.server_close.assert_called_once()

    def test_existing_runs_are_read_only(self):
        for path in (ROOT / 'output/runs').glob('*.BIN'):
            before = path.read_bytes()
            result = reader.read_run(path.parent.resolve(), path.name)
            self.assertIsNone(result['wall_time'])
            self.assertEqual(path.read_bytes(), before)

    def test_http(self):
        web = self.root / 'web'
        web.mkdir()
        (web/'index.html').write_text('<title>Reader</title>')
        server = reader.ThreadingHTTPServer(('127.0.0.1', 0), reader.make_handler(self.root, web))
        thread = threading.Thread(target=server.serve_forever, daemon=True)
        thread.start()
        self.addCleanup(server.server_close)
        self.addCleanup(server.shutdown)
        def request(path, headers=None, method='GET'):
            connection = http.client.HTTPConnection('127.0.0.1', server.server_address[1])
            connection.request(method, path, headers=headers or {})
            response = connection.getresponse()
            data = response.read()
            result = response.status, response.headers, data
            connection.close()
            return result
        status, headers, data = request('/api/runs')
        self.assertEqual(status, 200)
        self.assertEqual(headers['Cache-Control'], 'no-store')
        self.assertEqual(len(json.loads(data)['runs']), 1)
        status, _, data = request('/api/run?name='+self.path.name)
        self.assertEqual(status, 200)
        self.assertEqual(json.loads(data)['event_count'], 11)
        status, _, data = request('/api/run?name='+self.path.name+'&expected_events=11&expected_crc='+f'{zlib.crc32(self.path.read_bytes()):08x}')
        self.assertEqual(json.loads(data)['validation']['completion'], 'reference-match')
        self.assertEqual(request('/')[0], 200)
        for path in ('/api/run', '/api/run?name=../outside.bin', '/api/run?name=a&name=b',
                     '/api/run?name=x&expected_events=-1', '/api/run?name=x&expected_events=65537',
                     '/api/run?name=x&expected_crc=zz', '/api/run?name=x&foo=1', '/api/run?name',
                     '/api/runs?foo=1'):
            status, _, data = request(path)
            self.assertEqual(status, 400, path)
            self.assertIn('error', json.loads(data))
        self.assertEqual(request('/../reader.py')[0], 404)
        self.assertEqual(request('/api/run?name=missing.bin')[0], 404)
        self.assertEqual(request('/api/runs', {'Origin': 'https://example.com'})[0], 400)
        self.assertEqual(request('/api/runs', {'Host': 'evil.example'})[0], 400)
        self.assertEqual(request('/api/runs', {'Sec-Fetch-Site': 'cross-site'})[0], 400)
        self.assertEqual(request('/api/runs', method='POST')[0], 501)
        self.path.write_bytes(b'x'*(reader.MAX_BYTES+1))
        self.assertEqual(request('/api/run?name='+self.path.name)[0], 413)


if __name__ == '__main__':
    unittest.main()
