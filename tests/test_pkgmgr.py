"""Testes do gerenciador de pacotes (`psl install/uninstall/list/registry`)."""
import json

import pytest

from poolscript import cli, pkgmgr


@pytest.fixture(autouse=True)
def poolscript_home(tmp_path, monkeypatch):
    home = tmp_path / ".poolscript"
    monkeypatch.setenv("POOLSCRIPT_HOME", str(home))
    return home


def _write_ps(tmp_path, name, body='action main() {\n    post("oi")\n}\nmain()'):
    f = tmp_path / name
    f.write_text(body, encoding="utf-8")
    return f


# ── instalação como comando global ──────────────────────────────────────────

def test_install_command_creates_shim_and_tracks(tmp_path, monkeypatch):
    monkeypatch.chdir(tmp_path)
    _write_ps(tmp_path, "hello.ps")

    rc, out, _ = _run(["install", "hello.ps"])
    assert rc == 0
    assert "hello" in out

    paths = pkgmgr._paths()
    assert (paths.commands / "hello.ps").is_file()
    assert (paths.bin / "hello.cmd").is_file()

    data = json.loads(paths.installed_file.read_text(encoding="utf-8"))
    assert "hello" in data["commands"]


def test_uninstall_command_removes_files(tmp_path, monkeypatch):
    monkeypatch.chdir(tmp_path)
    _write_ps(tmp_path, "hello.ps")
    _run(["install", "hello.ps"])

    rc, out, _ = _run(["uninstall", "hello"])
    assert rc == 0

    paths = pkgmgr._paths()
    assert not (paths.commands / "hello.ps").exists()
    assert not (paths.bin / "hello.cmd").exists()
    data = json.loads(paths.installed_file.read_text(encoding="utf-8"))
    assert "hello" not in data["commands"]


def test_uninstall_command_not_installed_errors():
    rc, _, err = _run(["uninstall", "ghost"])
    assert rc == 1
    assert "ghost" in err


# ── instalação como lib importável ──────────────────────────────────────────

def test_install_lib_is_importable_from_anywhere(tmp_path, monkeypatch):
    src_dir = tmp_path / "src_project"
    src_dir.mkdir()
    monkeypatch.chdir(src_dir)
    _write_ps(src_dir, "greetlib.ps", body='action greet() {\n    return "hi from lib"\n}')

    rc, out, _ = _run(["install", "greetlib.ps", "-asLib"])
    assert rc == 0
    assert "greetlib" in out

    other_dir = tmp_path / "unrelated_project"
    other_dir.mkdir()
    monkeypatch.chdir(other_dir)
    consumer = _write_ps(
        other_dir, "use.ps",
        body='import greetlib\npost(greetlib.greet())',
    )

    rc, out, _ = _run([str(consumer)])
    assert rc == 0
    assert "hi from lib" in out


def test_uninstall_lib_removes_file(tmp_path, monkeypatch):
    monkeypatch.chdir(tmp_path)
    _write_ps(tmp_path, "greetlib.ps", body='action greet() {\n    return "hi"\n}')
    _run(["install", "greetlib.ps", "-asLib"])

    rc, out, _ = _run(["uninstall", "greetlib", "-asLib"])
    assert rc == 0

    assert not (pkgmgr._paths().libs / "greetlib.ps").exists()


def test_uninstall_lib_without_flag_auto_detects(tmp_path, monkeypatch):
    """`psl uninstall <nome>` sem flag deve achar e remover a lib pool sozinho."""
    monkeypatch.chdir(tmp_path)
    _write_ps(tmp_path, "randomlib.ps", body='action r() {\n    return 4\n}')
    _run(["install", "randomlib.ps", "-asLib"])
    assert (pkgmgr._paths().libs / "randomlib.ps").exists()

    rc, out, _ = _run(["uninstall", "randomlib"])   # sem -asLib
    assert rc == 0
    assert "randomlib" in out
    assert not (pkgmgr._paths().libs / "randomlib.ps").exists()


def test_uninstall_ambiguous_requires_flag(tmp_path, monkeypatch):
    """Mesmo nome como comando E lib: sem flag não adivinha, pede desambiguação."""
    monkeypatch.chdir(tmp_path)
    _write_ps(tmp_path, "dup.ps")
    _run(["install", "dup.ps"])            # comando
    _run(["install", "dup.ps", "-asLib"])  # lib

    rc, _, err = _run(["uninstall", "dup"])   # sem flag -> ambíguo
    assert rc == 1
    assert "command" in err and "lib" in err

    # a flag desambigua: remove só a lib...
    rc, _, _ = _run(["uninstall", "dup", "-asLib"])
    assert rc == 0
    assert not (pkgmgr._paths().libs / "dup.ps").exists()

    # ...e agora, sem ambiguidade, o sem-flag remove o comando.
    rc, _, _ = _run(["uninstall", "dup"])
    assert rc == 0
    assert not (pkgmgr._paths().commands / "dup.ps").exists()


# ── instalação de lib Python ─────────────────────────────────────────────────

def test_install_py_calls_pip(monkeypatch):
    import subprocess

    calls = []

    class _FakeCompleted:
        returncode = 0

    def _fake_run(cmd, **kwargs):
        calls.append(cmd)
        return _FakeCompleted()

    monkeypatch.setattr(subprocess, "run", _fake_run)
    rc, out, _ = _run(["install", "requests", "-py"])
    assert rc == 0
    assert calls and calls[0][-2:] == ["install", "requests"]

    data = json.loads(pkgmgr._paths().installed_file.read_text(encoding="utf-8"))
    assert "requests" in data["py"]


# ── list ──────────────────────────────────────────────────────────────────────

def test_list_shows_installed_entries(tmp_path, monkeypatch):
    monkeypatch.chdir(tmp_path)
    _write_ps(tmp_path, "hello.ps")
    _run(["install", "hello.ps"])

    rc, out, _ = _run(["list"])
    assert rc == 0
    assert "hello" in out


# ── registro remoto ────────────────────────────────────────────────────────────

def test_registry_set_url_and_show():
    rc, _, _ = _run(["registry", "set-url", "https://example.com/index.json"])
    assert rc == 0
    rc, out, _ = _run(["registry", "show"])
    assert rc == 0
    assert "https://example.com/index.json" in out


def test_install_bare_name_resolves_via_registry(monkeypatch, tmp_path):
    _run(["registry", "set-url", "https://example.com/index.json"])

    index_payload = json.dumps({"greetlib": "https://example.com/pkgs/greetlib.ps"}).encode("utf-8")
    ps_payload = b'action greet() {\n    return "hi from registry"\n}'

    class _FakeResponse:
        def __init__(self, data):
            self._data = data

        def __enter__(self):
            return self

        def __exit__(self, *exc):
            return False

        def read(self):
            return self._data

    responses = {
        "https://example.com/index.json": _FakeResponse(index_payload),
        "https://example.com/pkgs/greetlib.ps": _FakeResponse(ps_payload),
    }

    import urllib.request
    monkeypatch.setattr(urllib.request, "urlopen", lambda url, timeout=10: responses[url])

    rc, out, _ = _run(["install", "greetlib", "-asLib"])
    assert rc == 0
    assert "greetlib" in out
    assert (pkgmgr._paths().libs / "greetlib.ps").read_text(encoding="utf-8") == ps_payload.decode("utf-8")


def test_install_unknown_name_in_registry_errors(monkeypatch):
    _run(["registry", "set-url", "https://example.com/index.json"])

    class _FakeResponse:
        def __enter__(self):
            return self

        def __exit__(self, *exc):
            return False

        def read(self):
            return b"{}"

    import urllib.request
    monkeypatch.setattr(urllib.request, "urlopen", lambda url, timeout=10: _FakeResponse())

    rc, _, err = _run(["install", "ghost"])
    assert rc == 1
    assert "ghost" in err


def _run(args):
    import io
    import sys
    from contextlib import redirect_stdout, redirect_stderr

    out, err = io.StringIO(), io.StringIO()
    with redirect_stdout(out), redirect_stderr(err):
        rc = cli.main(args)
    return rc, out.getvalue(), err.getvalue()
