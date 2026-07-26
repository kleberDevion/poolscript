"""Download binário do request: .content / .save / .decode / stream / content_type."""
import io

import pytest

from poolscript.stdlib import request_lib as R

PAYLOAD = b"MZ\x90\x00" + bytes(range(256)) * 40  # ~10KB, contém bytes não-UTF8


class _FakeResp:
    def __init__(self, payload, headers, url):
        self._buf = io.BytesIO(payload)
        self.status = 200
        self.headers = headers
        self._url = url

    def read(self, n=-1):
        return self._buf.read() if (n is None or n < 0) else self._buf.read(n)

    def geturl(self):
        return self._url

    def __enter__(self):
        return self

    def __exit__(self, *exc):
        return False


def _patch(monkeypatch, payload=PAYLOAD, headers=None):
    hdr = headers or {
        "Content-Type": "application/octet-stream",
        "Content-Disposition": 'attachment; filename="baixado.bin"',
        "Content-Length": str(len(payload)),
    }
    import urllib.request
    monkeypatch.setattr(
        urllib.request, "urlopen",
        lambda req, timeout=30: _FakeResp(payload, hdr, "http://x/baixado.bin"),
    )


def test_content_bytes_intact(monkeypatch):
    _patch(monkeypatch)
    r = R.get("http://x/f")
    assert r.content == PAYLOAD          # bytes crus, sem corromper
    assert r.size == len(PAYLOAD)


def test_text_and_json_backward_compat(monkeypatch):
    _patch(monkeypatch, payload=b'{"a": 1}', headers={"Content-Type": "application/json"})
    r = R.get("http://x/f")
    assert isinstance(r.text, str)       # .text antigo continua
    assert r.json() == {"a": 1}


def test_decode_encoding(monkeypatch):
    _patch(monkeypatch, payload="olá".encode("latin-1"), headers={"Content-Type": "text/plain"})
    r = R.get("http://x/f")
    assert r.decode("latin-1") == "olá"


def test_save_returns_poolfile_into_folder(monkeypatch, tmp_path):
    _patch(monkeypatch)
    pf = (R.get("http://x/f", stream=True)
          .content_type("application/octet-stream")
          .save(str(tmp_path) + "/"))       # pasta -> nome vem do Content-Disposition
    assert pf.name == "baixado.bin"
    assert pf.bytes() == PAYLOAD
    assert (tmp_path / "baixado.bin").is_file()


def test_content_type_mismatch_raises(monkeypatch):
    _patch(monkeypatch)
    with pytest.raises(ValueError):
        R.get("http://x/f").content_type("application/json")


def test_max_size_exceeded_raises(monkeypatch):
    _patch(monkeypatch)
    with pytest.raises(MemoryError):
        R.get("http://x/f", stream=True, max_size="1kb")   # 10KB > 1KB


def test_parse_size_units():
    assert R._parse_size("100mb") == 100 * 1024 * 1024
    assert R._parse_size("2gb") == 2 * 1024 ** 3
    assert R._parse_size(500) == 500
