"""Método de instância SEM `self` como 1º parâmetro: a VM injetava a instância
no 1º parâmetro real e o argumento do usuário virava o 2º -> cuspia o confuso
"argumentos demais" (mesmo passando UM só argumento). Agora dá o MESMO erro
claro do interpretador ("deve ter 'self' como primeiro parâmetro") em todos os
caminhos de chamada (posicional, nomeado, callback). Com `self`, funciona.
"""
import os
import subprocess
import sys
from pathlib import Path

import pytest

RAIZ = Path(__file__).resolve().parent.parent
POOL = RAIZ / "pool"
SRC = RAIZ / "src"

CLAREZA = "deve ter 'self' como primeiro parâmetro"


def _engines():
    yield [sys.executable, "-m", "poolscript"]
    # VM em C NÃO se pula: se o pool não existe, o teste FALHA (skip = falso verde).
    assert POOL.exists(), "binário 'pool' não compilado — rode ./rebuild_vm.sh."
    yield [str(POOL)]


def _run(cmd, tmp_path, prog):
    ps = tmp_path / "t.ps"
    ps.write_text(prog, encoding="utf-8")
    env = dict(os.environ, PYTHONPATH=str(SRC))
    return subprocess.run(cmd + [str(ps)], capture_output=True, text=True,
                          env=env, timeout=15)


SEM_SELF = [
    # posicional com dict (o caso reportado)
    'class B() { reaction m(x) { return x } }\npost(B().m({"nome": "valor"}))\n',
    # nomeado
    'class B() { reaction m(x) { return x } }\npost(B().m(x={"nome": "valor"}))\n',
    # como callback (chama_valor)
    'class B() { reaction m(x) { return x } }\nb = B()\npost(map([1, 2, 3], b.m))\n',
]


@pytest.mark.parametrize("prog", SEM_SELF)
def test_sem_self_erro_claro(tmp_path, prog):
    for cmd in _engines():
        r = _run(cmd, tmp_path, prog)
        assert r.returncode != 0, (cmd, r.stdout)
        saida = r.stdout + r.stderr
        assert CLAREZA in saida, (cmd, saida)
        # o erro confuso NÃO deve mais aparecer
        assert "argumentos demais" not in saida, (cmd, saida)


def test_com_self_funciona(tmp_path):
    prog = 'class B() { reaction m(self, x) { return x } }\npost(B().m({"nome": "valor"}))\n'
    for cmd in _engines():
        r = _run(cmd, tmp_path, prog)
        assert r.stdout.strip() == "{'nome': 'valor'}", (cmd, r.stdout, r.stderr)
