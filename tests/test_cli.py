"""Testes da CLI."""
import io
import sys
from contextlib import redirect_stdout, redirect_stderr
from pathlib import Path

import pytest
from poolscript import cli, __version__


def run_cli(args, stdin_text: str | None = None):
    out, err = io.StringIO(), io.StringIO()
    prev_stdin = sys.stdin
    if stdin_text is not None:
        sys.stdin = io.StringIO(stdin_text)
    try:
        with redirect_stdout(out), redirect_stderr(err):
            rc = cli.main(args)
    finally:
        sys.stdin = prev_stdin
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


def test_install_py_flag(tmp_path, monkeypatch):
    # `psl install <nome> -py` chama `pip install` de verdade — não deve
    # depender de rede/PyPI real no teste. Mocka subprocess.run.
    import subprocess

    monkeypatch.setenv("POOLSCRIPT_HOME", str(tmp_path / ".poolscript"))

    class _FakeCompleted:
        returncode = 0

    monkeypatch.setattr(subprocess, "run", lambda *a, **k: _FakeCompleted())
    rc, out, _ = run_cli(["install", "minha_lib", "-py"])
    assert rc == 0
    assert "minha_lib" in out


def test_install_bare_name_without_registry_fails(tmp_path, monkeypatch):
    # Sem `-py` e sem registro configurado, um nome que não termina em
    # `.ps` não deve mais cair silenciosamente no pip (comportamento antigo).
    monkeypatch.setenv("POOLSCRIPT_HOME", str(tmp_path / ".poolscript"))
    rc, _, err = run_cli(["install", "minha_lib"])
    assert rc == 1
    assert "registro" in err


def test_update_no_args_is_usage_error():
    rc, _, err = run_cli(["-up"])
    assert rc == 1
    assert "psl -up release" in err


def test_update_up_to_date(monkeypatch):
    import subprocess

    class _Result:
        def __init__(self, stdout="", returncode=0, stderr=""):
            self.stdout, self.returncode, self.stderr = stdout, returncode, stderr

    def fake_run(cmd, cwd=None, capture_output=None, text=None):
        sub = cmd[1:]
        if sub[:2] == ["fetch", "--quiet"]:
            return _Result()
        if sub[:2] == ["rev-parse", "--abbrev-ref"]:
            return _Result(stdout="main\n")
        if sub == ["rev-parse", "HEAD"] or sub == ["rev-parse", "origin/main"]:
            return _Result(stdout="same-sha\n")
        raise AssertionError(f"chamada git inesperada: {sub}")

    monkeypatch.setattr(subprocess, "run", fake_run)
    rc, out, _ = run_cli(["-up", "release"])
    assert rc == 0
    assert "mais recente" in out


def test_update_pulls_when_behind(monkeypatch):
    import subprocess

    class _Result:
        def __init__(self, stdout="", returncode=0, stderr=""):
            self.stdout, self.returncode, self.stderr = stdout, returncode, stderr

    def fake_run(cmd, cwd=None, capture_output=None, text=None):
        sub = cmd[1:]
        if sub[:2] == ["fetch", "--quiet"]:
            return _Result()
        if sub[:2] == ["rev-parse", "--abbrev-ref"]:
            return _Result(stdout="main\n")
        if sub == ["rev-parse", "HEAD"]:
            return _Result(stdout="aaaa\n")
        if sub == ["rev-parse", "origin/main"]:
            return _Result(stdout="bbbb\n")
        if sub[0] == "rev-list":
            return _Result(stdout="0\t3\n")
        if sub[0] == "merge":
            return _Result(returncode=0)
        raise AssertionError(f"chamada git inesperada: {sub}")

    monkeypatch.setattr(subprocess, "run", fake_run)
    rc, out, _ = run_cli(["-up", "release"])
    assert rc == 0
    assert "Atualizado" in out
    assert "->" in out  # sem caractere não-ASCII que quebra em consoles cp1252


def test_update_ff_only_conflict_does_not_touch_repo(monkeypatch):
    import subprocess

    class _Result:
        def __init__(self, stdout="", returncode=0, stderr=""):
            self.stdout, self.returncode, self.stderr = stdout, returncode, stderr

    def fake_run(cmd, cwd=None, capture_output=None, text=None):
        sub = cmd[1:]
        if sub[:2] == ["fetch", "--quiet"]:
            return _Result()
        if sub[:2] == ["rev-parse", "--abbrev-ref"]:
            return _Result(stdout="main\n")
        if sub == ["rev-parse", "HEAD"]:
            return _Result(stdout="aaaa\n")
        if sub == ["rev-parse", "origin/main"]:
            return _Result(stdout="bbbb\n")
        if sub[0] == "rev-list":
            return _Result(stdout="0\t3\n")
        if sub[0] == "merge":
            return _Result(returncode=1, stderr="local changes would be overwritten")
        raise AssertionError(f"chamada git inesperada: {sub}")

    monkeypatch.setattr(subprocess, "run", fake_run)
    rc, _, err = run_cli(["-up", "release"])
    assert rc == 1
    assert "divergiu" in err or "mudanças locais" in err


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


def test_no_args_opens_repl():
    # `pool` sem argumentos abre o REPL interativo (igual ao `python` sem
    # argumentos) — não imprime a tela de ajuda. Com stdin vazio (EOF
    # imediato), o REPL encerra sozinho com rc=0 após mostrar o banner.
    rc, out, _ = run_cli([], stdin_text="")
    assert rc == 0
    assert "REPL" in out


def test_fast_main_no_args_opens_repl(monkeypatch):
    # Regressão: fast_main() (o entry point REAL de `pool`/`psl`, não main())
    # curto-circuitava "sem argumento nenhum" junto com --version e nunca
    # chegava no REPL. main() sozinho já funcionava certo (ver teste acima),
    # por isso esse bug só aparecia rodando o comando `pool` de verdade.
    monkeypatch.setattr(sys, "argv", ["pool"])
    monkeypatch.setattr(sys, "stdin", io.StringIO(""))
    out, err = io.StringIO(), io.StringIO()
    with redirect_stdout(out), redirect_stderr(err):
        with pytest.raises(SystemExit) as exc_info:
            cli.fast_main()
    assert exc_info.value.code == 0
    assert "REPL" in out.getvalue()
