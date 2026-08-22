"""Builtins — paridade INTERP/VM no 'Grupo B' (engines alinhados nos dois sentidos).

Decisão do usuário ("alinha os dois igual"):
  - int() sem argumento -> 0        (VM passou a aceitar, como o interp)
  - sum(lista, start)   -> soma+start (VM passou a aceitar, como o interp)
  - round(n, casas<0)   -> casas clampadas a 0 (interp restringido, como a VM)
  - min/max sem kwargs  -> só posicional (interp restringido, como a VM)
"""
import os
import subprocess
import sys
from pathlib import Path

import pytest

RAIZ = Path(__file__).resolve().parent.parent
POOL_BIN = RAIZ / "pool"
assert POOL_BIN.exists(), "binário pool não compilado — rode ./rebuild_vm.sh (VM em C não se pula: skip = falso verde)"
NL = chr(10)

CASOS = [
    ("post(int())", "0"),
    ("post(sum([1, 2, 3], 10))", "16"),
    ("post(sum([1.5], 1))", "2.5"),
    ("post(round(1234.5, -2))", "1234.0"),
    # regressão dos casos normais
    ("post(round(3.567, 2), round(2.5))", "3.57 2"),
    ("post(min([3, 1, 2]), max(5, 2, 8))", "1 8"),
    ("post(sum([1, 2, 3]))", "6"),
]


def _run(cmd, src, tmp_path):
    entry = tmp_path / "s.ps"
    entry.write_text(src + NL, encoding="utf-8")
    env = dict(os.environ)
    env["PYTHONPATH"] = str(RAIZ / "src")
    env["GUZER_HEADLESS"] = "1"
    return subprocess.run(cmd + [str(entry)], capture_output=True, text=True, env=env).stdout.strip()


@pytest.mark.parametrize("src,esperado", CASOS)
def test_interp_builtins_grupoB(src, esperado, tmp_path):
    assert _run([sys.executable, "-m", "poolscript"], src, tmp_path) == esperado


@pytest.mark.parametrize("src,esperado", CASOS)
def test_paridade_builtins_grupoB(src, esperado, tmp_path):
    oi = _run([sys.executable, "-m", "poolscript"], src, tmp_path)
    ov = _run([str(POOL_BIN)], src, tmp_path)
    assert oi == ov == esperado, "INT:" + oi + " VM:" + ov


def test_min_max_sem_kwargs_erram_nos_dois(tmp_path):
    # kwargs do Python (default=/key=) não existem: os dois motores erram
    src = "post(min([], default=0))"
    for cmd in ([sys.executable, "-m", "poolscript"], [str(POOL_BIN)]):
        entry = tmp_path / "e.ps"
        entry.write_text(src + NL, encoding="utf-8")
        env = dict(os.environ)
        env["PYTHONPATH"] = str(RAIZ / "src")
        env["GUZER_HEADLESS"] = "1"
        r = subprocess.run(cmd + [str(entry)], capture_output=True, text=True, env=env)
        assert r.returncode != 0
