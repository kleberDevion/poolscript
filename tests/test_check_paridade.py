"""`pool --check` — o modo "só analisa, não roda" que o editor/LSP consome.

A VM em C já tinha (`cmd_check` em vm/main.c); o interpretador não — quem
chamasse `python -m poolscript --check arq.ps` recebia "arquivo não
encontrado: --check" e, pior, um script chamado `--check` seria EXECUTADO.
Agora os dois respondem o MESMO JSON de uma linha: `{"ok":true}` ou
`{"ok":false,"tipo":...,"msg":...,"linha":N,"coluna":N}` (o texto da `msg`
é de cada motor, como nos demais erros; o resto tem que bater).
"""
import json
import os
import subprocess
import sys
from pathlib import Path

import pytest

RAIZ = Path(__file__).resolve().parent.parent
POOL = RAIZ / "pool"
SRC = RAIZ / "src"

# VM em C NÃO se pula: sem o binário o teste FALHA (skip = falso verde).
assert POOL.exists(), "binário 'pool' não compilado — rode ./rebuild_vm.sh."

INTERP = [sys.executable, "-m", "poolscript"]
VM = [str(POOL)]


def _check(cmd, arquivo=None, stdin=None):
    env = dict(os.environ, PYTHONPATH=str(SRC))
    argv = cmd + ["--check"] + ([str(arquivo)] if arquivo else [])
    p = subprocess.run(argv, capture_output=True, text=True, env=env,
                       input=stdin, timeout=30)
    assert p.returncode == 0, f"--check não deve sair com erro: {p.stderr}"
    return json.loads(p.stdout.strip())


OK = 'action f(x):\n    return x + 1\npost(f(1))\n'
RUIM = 'action f(x:\n    return x\n'


def test_arquivo_valido(tmp_path):
    arq = tmp_path / "ok.ps"
    arq.write_text(OK, encoding="utf-8")
    assert _check(INTERP, arq) == {"ok": True}
    assert _check(VM, arq) == {"ok": True}


def test_arquivo_com_erro_mesma_posicao(tmp_path):
    arq = tmp_path / "ruim.ps"
    arq.write_text(RUIM, encoding="utf-8")
    a, b = _check(INTERP, arq), _check(VM, arq)
    for r in (a, b):
        assert r["ok"] is False
        assert r["tipo"] == "SyntaxError"
        assert r["msg"]                      # mensagem não vazia (texto é de cada motor)
    assert (a["linha"], a["coluna"]) == (b["linha"], b["coluna"]), (a, b)


def test_le_da_entrada_padrao(tmp_path):
    assert _check(INTERP, stdin=OK) == {"ok": True}
    assert _check(VM, stdin=OK) == {"ok": True}
    assert _check(INTERP, stdin=RUIM)["ok"] is False
    assert _check(VM, stdin=RUIM)["ok"] is False


def test_nao_executa_o_script(tmp_path):
    """--check ANALISA; se rodasse, o post apareceria na saída."""
    arq = tmp_path / "efeito.ps"
    arq.write_text('post("NAO DEVIA RODAR")\n', encoding="utf-8")
    env = dict(os.environ, PYTHONPATH=str(SRC))
    for cmd in (INTERP, VM):
        p = subprocess.run(cmd + ["--check", str(arq)], capture_output=True,
                           text=True, env=env, timeout=30)
        assert "NAO DEVIA RODAR" not in p.stdout, p.stdout
        assert json.loads(p.stdout.strip()) == {"ok": True}


def test_arquivo_inexistente(tmp_path):
    alvo = tmp_path / "nao_existe.ps"
    for cmd in (INTERP, VM):
        r = _check(cmd, alvo)
        assert r["ok"] is False
        assert r["tipo"] == "IOError"


def test_check_no_help_dos_dois(tmp_path):
    env = dict(os.environ, PYTHONPATH=str(SRC))
    for cmd in (INTERP, VM):
        p = subprocess.run(cmd + ["--help"], capture_output=True, text=True,
                           env=env, timeout=30)
        assert "--check" in p.stdout, f"--check fora do help: {cmd}"
