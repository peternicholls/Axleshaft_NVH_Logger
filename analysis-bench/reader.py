#!/usr/bin/env python3
"""Read-only, loopback optical recording reader. Uses relative event time only."""
import argparse
import hashlib
import importlib.util
import json
from pathlib import Path
import re
import struct
import tempfile
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
from urllib.parse import parse_qs, urlsplit
import zlib
import webbrowser

ROOT = Path(__file__).resolve().parents[1]
MAX_BYTES = 1024 * 1024
MAX_SAFE_INTEGER = 2**53 - 1
_spec = importlib.util.spec_from_file_location(
    'optical_validator', ROOT / 'Stage1_Optical_Test' / 'validate_optical.py')
_validator = importlib.util.module_from_spec(_spec)
_spec.loader.exec_module(_validator)


class ReaderError(Exception):
    def __init__(self, status, message):
        super().__init__(message)
        self.status = status


def recording_path(root, name):
    """Resolve a recording without allowing the configured root to be escaped."""
    relative = Path(name)
    if not name or relative.is_absolute() or '..' in relative.parts or '\\' in name:
        raise ReaderError(400, 'Recording name must be a relative path within the runs folder.')
    path = (root / relative).resolve()
    if not path.is_relative_to(root) or path.suffix.lower() != '.bin':
        raise ReaderError(400, 'Only BIN recordings within the runs folder are available.')
    if not path.is_file():
        raise ReaderError(404, 'Recording not found.')
    return path


def list_runs(root):
    runs = []
    for path in sorted(root.rglob('*')):
        try:
            resolved = path.resolve()
            if path.suffix.lower() == '.bin' and resolved.is_relative_to(root) and resolved.is_file():
                runs.append({'name': path.relative_to(root).as_posix(), 'size': resolved.stat().st_size})
        except OSError:
            continue
    return {'runs': runs}


def read_run(root, name, expected_events=None, expected_crc=None):
    path = recording_path(root, name)
    if path.stat().st_size > MAX_BYTES:
        raise ReaderError(413, 'Recording exceeds the 1 MiB reader limit.')
    with path.open('rb') as source:
        data = source.read(MAX_BYTES + 1)
    if len(data) > MAX_BYTES:
        raise ReaderError(413, 'Recording exceeds the 1 MiB reader limit.')
    events = [list(record) for record in struct.iter_unpack('<QQ', data[:len(data)//16*16])]
    if any(value > MAX_SAFE_INTEGER for event in events for value in event):
        raise ReaderError(400, 'An event exceeds JavaScript safe integer precision; this file cannot be displayed reliably.')
    crc = zlib.crc32(data)
    structure = 'pass'
    message = 'Sequential events and increasing relative timestamps checked. Completion needs both independent event count and CRC references.'
    completion = 'unverified'
    # Snapshot ensures the established validator checks exactly the displayed bytes.
    # Matching the observed count/CRC proves structure, never capture completeness.
    with tempfile.NamedTemporaryFile(suffix='.BIN') as snapshot:
        snapshot.write(data)
        snapshot.flush()
        try:
            _validator.validate(snapshot.name, len(events), crc)
        except ValueError as error:
            structure = 'fail'
            message = f'DIAGNOSTIC ONLY: {error}. Complete record prefix displayed; recording is invalid.'
        if expected_events is not None or expected_crc is not None:
            mismatch = (expected_events is not None and expected_events != len(events)) or (
                expected_crc is not None and expected_crc != crc)
            if mismatch:
                completion = 'reference-mismatch'
                message += ' Supplied reference does not match the file.'
            elif expected_events is not None and expected_crc is not None:
                try:
                    _validator.validate(snapshot.name, expected_events, expected_crc)
                    completion = 'reference-match'
                    message = 'Structure and supplied event-count/CRC references match. This does not verify optical detection accuracy.'
                except ValueError:
                    completion = 'reference-mismatch'
            else:
                message += ' Single supplied reference matches; completion remains unverified.'
    times = [event[1] for event in events]
    intervals = [b-a for a,b in zip(times, times[1:])]
    span = times[-1]-times[0] if times else None
    valid_intervals = structure == 'pass' and bool(intervals)
    stats = {
        'first_us': times[0] if times else None, 'last_us': times[-1] if times else None,
        'span_us': span, 'min_interval_us': min(intervals) if valid_intervals else None,
        'max_interval_us': max(intervals) if valid_intervals else None,
        'mean_interval_us': span / len(intervals) if valid_intervals else None,
        'mean_rpm': 60_000_000 * len(intervals) / span if valid_intervals else None,
    }
    return {'name': name, 'size': len(data), 'event_count': len(events),
            'crc32': f'{crc:08x}', 'sha256': hashlib.sha256(data).hexdigest(),
            'wall_time': None, 'events': events, 'stats': stats,
            'validation': {'structure': structure, 'completion': completion, 'message': message}}


def make_handler(root, web_dir=None):
    root = Path(root).resolve()
    web_dir = Path(web_dir or Path(__file__).parent / 'web').resolve()

    class Handler(BaseHTTPRequestHandler):
        def send_content(self, status, content, content_type):
            self.send_response(status)
            self.send_header('Content-Type', content_type)
            self.send_header('Content-Length', str(len(content)))
            self.send_header('Cache-Control', 'no-store')
            self.send_header('X-Content-Type-Options', 'nosniff')
            self.send_header('Content-Security-Policy', "default-src 'self'; object-src 'none'; frame-ancestors 'none'")
            self.end_headers()
            self.wfile.write(content)

        def send_json(self, status, value):
            self.send_content(status, json.dumps(value, allow_nan=False).encode(), 'application/json; charset=utf-8')

        def do_GET(self):
            try:
                port = self.server.server_address[1]
                allowed = {f'127.0.0.1:{port}', f'localhost:{port}'}
                host = self.headers.get('Host', '')
                if host not in allowed:
                    raise ReaderError(400, 'Use the loopback address printed by the reader.')
                origin = self.headers.get('Origin')
                if origin is not None and origin != f'http://{host}':
                    raise ReaderError(400, 'Cross-origin requests are not permitted.')
                if self.headers.get('Sec-Fetch-Site') == 'cross-site':
                    raise ReaderError(400, 'Cross-site requests are not permitted.')
                url = urlsplit(self.path)
                if url.scheme or url.netloc:
                    raise ReaderError(400, 'Only local paths are accepted.')
                try:
                    query = parse_qs(url.query, keep_blank_values=True, strict_parsing=True)
                except ValueError:
                    raise ReaderError(400, 'Malformed query parameters.')
                if url.path == '/api/runs':
                    if query:
                        raise ReaderError(400, 'The runs list does not accept query parameters.')
                    self.send_json(200, list_runs(root))
                elif url.path == '/api/run':
                    if set(query) - {'name', 'expected_events', 'expected_crc'} or any(len(v) != 1 for v in query.values()):
                        raise ReaderError(400, 'Unknown or repeated query parameters.')
                    name = query.get('name', [''])[0]
                    count = query.get('expected_events', [None])[0]
                    crc = query.get('expected_crc', [None])[0]
                    if count is not None and (not re.fullmatch(r'\d{1,5}', count) or int(count) > 65536):
                        raise ReaderError(400, 'Expected events must be an integer from 0 to 65536.')
                    if crc is not None and not re.fullmatch(r'(?:0[xX])?[0-9a-fA-F]{1,8}', crc):
                        raise ReaderError(400, 'Expected CRC must contain up to eight hexadecimal digits.')
                    self.send_json(200, read_run(root, name, int(count) if count is not None else None,
                                                 int(crc, 16) if crc is not None else None))
                else:
                    files = {'/': ('index.html', 'text/html'), '/style.css': ('style.css', 'text/css'),
                             '/app.js': ('app.js', 'text/javascript'), '/favicon.svg': ('favicon.svg', 'image/svg+xml')}
                    if url.path not in files or query:
                        raise ReaderError(404, 'Page not found.')
                    filename, mime = files[url.path]
                    self.send_content(200, (web_dir / filename).read_bytes(), mime + '; charset=utf-8')
            except ReaderError as error:
                self.send_json(error.status, {'error': str(error)})
            except FileNotFoundError:
                self.send_json(404, {'error': 'File not found.'})
            except (OSError, ValueError):
                self.send_json(400, {'error': 'Could not read recording; check file access and query parameters.'})
    return Handler


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--runs-dir', type=Path, default=ROOT / 'output' / 'runs')
    parser.add_argument('--port', type=int, default=8765)
    parser.add_argument('--open', action='store_true', help='Open the local reader in your browser')
    args = parser.parse_args()
    if not args.runs_dir.is_dir():
        parser.error('--runs-dir must be an existing directory')
    if not 0 <= args.port <= 65535:
        parser.error('--port must be between 0 and 65535')
    server = ThreadingHTTPServer(('127.0.0.1', args.port), make_handler(args.runs_dir))
    local_url = f'http://127.0.0.1:{server.server_address[1]}'
    print(f'Optical reader: {local_url}', flush=True)
    print(f'Read-only recordings: {args.runs_dir.resolve()}', flush=True)
    if args.open:
        webbrowser.open(local_url)
    try:
        server.serve_forever()
    except KeyboardInterrupt:
        pass
    finally:
        server.server_close()


if __name__ == '__main__':
    main()
