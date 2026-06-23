"""Testes da CLI."""
import io
import sys
from contextlib import redirect_stdout, redirect_stderr
from pathlib import Path

import pytest
from poolscript import cli, __version__


def run_cli(args):
    out, err = io.StringIO(), io.StringIO()
    with redirect_stdout(out), redirect_stderr(err):
        rc = cli.main(args)
    return rc, out.getvalue(), err.getvalue()


def test_version():
    rc, out, _ = run_cli(["--version"])
    assert rc == 0
    assert __version__ in out


def test_help():
    rc, out, _ = run_cli(["--help"])
    assert rc == 0
    assert "PoolScript" in out
    assert "poolscript@proton.me" in out


def test_doc():
    rc, out, _ = run_cli(["//doc"])
    assert rc == 0
    assert "http" in out


def test_install_stub():
    rc, out, _ = run_cli(["install", "minha_lib"])
    assert rc == 0
    assert "minha_lib" in out


def test_update_stub():
    rc, out, _ = run_cli(["-up", "release"])
    assert rc == 0


def test_run_file(tmp_path):
    f = tmp_path / "t.ps"
    f.write_text('post("hello cli")', encoding="utf-8")
    rc, out, _ = run_cli([str(f)])
    assert rc == 0
    assert "hello cli" in out


def test_run_missing_file():
    rc, _, err = run_cli(["nope.ps"])
    assert rc == 2
    assert "não encontrado" in err


def test_build_runs_all_ps(tmp_path, monkeypatch):
    monkeypatch.chdir(tmp_path)
    (tmp_path / "a.ps").write_text('post("a")', encoding="utf-8")
    (tmp_path / "b.ps").write_text('post("b")', encoding="utf-8")
    rc, out, _ = run_cli(["build"])
    assert rc == 0
    assert "a" in out and "b" in out


def test_no_args_prints_help():
    rc, out, _ = run_cli([])
    assert rc == 0
    assert "USO" in out
