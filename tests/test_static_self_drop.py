"""@static com `self` na assinatura: chamada ESTÁTICA (`Classe.metodo(x)`)
dropa o `self` e liga o posicional no 1º parâmetro REAL — não no self.
Regressão do caso `idSession(self, webToken=str)`, onde o posicional caía no
self e `webToken` ficava com o default (o tipo `str`), estourando lá adentro
com "não sei criar bytes de objeto". Vale igual nos dois motores.
"""
import os
import subprocess
import sys
from pathlib import Path

import pytest

RAIZ = Path(__file__).resolve().parent.parent
POOL = RAIZ / "pool"
SRC = RAIZ / "src"

CLASSE = """public class C()
{
    @static
    public reaction f(self, a, b=10)
    {
        return a + b
    }
}
"""


def _run(cmd, tmp_path, corpo):
    ps = tmp_path / "t.ps"
    ps.write_text(CLASSE + corpo, encoding="utf-8")
    env = dict(os.environ, PYTHONPATH=str(SRC))
    return subprocess.run(cmd + [str(ps)], capture_output=True, text=True,
                          env=env, timeout=15)


def _engines():
    yield [sys.executable, "-m", "poolscript"]
    # VM em C NÃO se pula: se o pool não existe, o teste FALHA (skip = falso verde).
    assert POOL.exists(), "binário 'pool' não compilado — rode ./rebuild_vm.sh."
    yield [str(POOL)]


@pytest.mark.parametrize("corpo,esperado", [
    # estático posicional: o arg cai no `a`, NÃO no self
    ("post(C.f(5))",        "15"),
    ("post(C.f(5, 20))",    "25"),
    # estático nomeado
    ("post(C.f(a=7))",      "17"),
    ("post(C.f(a=7, b=1))", "8"),
    # via instância continua ligando self = instância
    ("post(C().f(5))",      "15"),
    ("post(C().f(a=7))",    "17"),
])
def test_liga_no_param_real(tmp_path, corpo, esperado):
    for cmd in _engines():
        r = _run(cmd, tmp_path, corpo)
        assert r.stdout.strip() == esperado, (cmd, corpo, r.stdout, r.stderr)


@pytest.mark.parametrize("corpo,trecho", [
    ("post(C.f())",        "faltando argumento: 'a'"),
    ("post(C.f(1, 2, 3))", "esperava até 2 argumentos, recebeu 3"),
])
def test_erro_limpo(tmp_path, corpo, trecho):
    for cmd in _engines():
        r = _run(cmd, tmp_path, corpo)
        assert r.returncode != 0, (cmd, corpo, r.stdout)
        assert trecho in (r.stdout + r.stderr), (cmd, corpo, r.stdout + r.stderr)


# @static passado como VALOR (map/filter/callback) também dropa o self —
# antes a VM mandava o elemento pro self e estourava runtime.
CB = """public class C()
{
    @static
    public reaction dobro(self, x)
    {
        return x * 2
    }
}
post(map([1, 2, 3], C.dobro))
"""


def test_static_como_callback(tmp_path):
    ps = tmp_path / "cb.ps"
    ps.write_text(CB, encoding="utf-8")
    env = dict(os.environ, PYTHONPATH=str(SRC))
    for cmd in _engines():
        r = subprocess.run(cmd + [str(ps)], capture_output=True, text=True,
                           env=env, timeout=15)
        assert r.stdout.strip() == "[2, 4, 6]", (cmd, r.stdout, r.stderr)
