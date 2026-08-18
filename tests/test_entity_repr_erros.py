"""Paridade INTERP/VM no repr de instância e nas mensagens de erro de chamada.

Alinha a VM em C ao interpretador (autoridade):
  - repr de instância: `<Nome {campos}>` (era `<Nome>` no C);
  - método sem `self`, argumentos de menos/de mais: mesma mensagem;
  - `base` sem `(`: mesmo SyntaxError.
"""
import os
import subprocess
import sys
from pathlib import Path

import pytest

RAIZ = Path(__file__).resolve().parent.parent
POOL_BIN = RAIZ / "pool"
NL = chr(10)


def _run(cmd, src, tmp_path):
    entry = tmp_path / "s.ps"
    entry.write_text(src, encoding="utf-8")
    env = dict(os.environ)
    env["PYTHONPATH"] = str(RAIZ / "src")
    env["GUZER_HEADLESS"] = "1"
    r = subprocess.run(cmd + [str(entry)], capture_output=True, text=True, env=env)
    # junta stdout e a última linha de erro relevante (sem ANSI)
    saida = r.stdout.strip()
    return saida, r.returncode


REPR = (
    "Entity U():" + NL + "    nome: str" + NL + "    idade: int" + NL
    + 'post(U("ana", 30))' + NL
)
REPR_VAZIA = (
    "Entity R():" + NL + "    action m(self):" + NL + "        return 1" + NL
    + "post(R())" + NL
)


@pytest.mark.skipif(not POOL_BIN.exists(), reason="binário ./pool não compilado")
@pytest.mark.parametrize("src,esperado", [
    (REPR, "<U {'nome': 'ana', 'idade': 30}>"),
    (REPR_VAZIA, "<R {}>"),
])
def test_repr_instancia_paridade(src, esperado, tmp_path):
    oi, _ = _run([sys.executable, "-m", "poolscript"], src, tmp_path)
    ov, _ = _run([str(POOL_BIN)], src, tmp_path)
    assert ov == esperado, ov
    assert oi == esperado, oi


# (fonte, trecho que deve aparecer na mensagem de erro dos DOIS motores)
ERROS = [
    ("Entity M():" + NL + "    action f():" + NL + "        return 1" + NL + "post(M().f())" + NL,
     "deve ter 'self' como primeiro parâmetro"),
    ("action f(a, b):" + NL + "    return a" + NL + "post(f(1))" + NL,
     "faltando argumento: 'b'"),
    ("action f(a):" + NL + "    return a" + NL + "post(f(1, 2))" + NL,
     "esperava até 1 argumentos, recebeu 2"),
]


@pytest.mark.skipif(not POOL_BIN.exists(), reason="binário ./pool não compilado")
@pytest.mark.parametrize("src,trecho", ERROS)
def test_mensagem_erro_paridade(src, trecho, tmp_path):
    # roda os dois; ambos devem falhar e conter o mesmo trecho na saída de erro
    for cmd in ([sys.executable, "-m", "poolscript"], [str(POOL_BIN)]):
        entry = tmp_path / "e.ps"
        entry.write_text(src, encoding="utf-8")
        env = dict(os.environ)
        env["PYTHONPATH"] = str(RAIZ / "src")
        env["GUZER_HEADLESS"] = "1"
        r = subprocess.run(cmd + [str(entry)], capture_output=True, text=True, env=env)
        assert r.returncode != 0
        assert trecho in (r.stdout + r.stderr), (cmd, r.stdout + r.stderr)
