"""Builtins — paridade INTERP/VM no 'Grupo A' (VM alinhado ao interp).

O VM em C passou a aceitar o que o interp já aceitava:
  - len(null) -> 0 (era erro no VM)
  - range("3") -> parseia a string numérica (era erro no VM)
  - chr(true) -> aceita bool como int (era erro no VM)
"""
import os
import subprocess
import sys
from pathlib import Path

import pytest

RAIZ = Path(__file__).resolve().parent.parent
POOL_BIN = RAIZ / "pool"
NL = chr(10)

CASOS = [
    ("post(len(null))", "0"),
    ('post(range("3"))', "[0, 1, 2]"),
    ('post(range("2", "5"))', "[2, 3, 4]"),
    ("post(chr(false) == chr(0))", "True"),
    # regressão dos casos normais
    ('post(len("olá"), len([1, 2]))', "3 2"),
    ("post(range(3), range(1, 7, 2))", "[0, 1, 2] [1, 3, 5]"),
    ("post(chr(65))", "A"),
]


def _run(cmd, src, tmp_path):
    entry = tmp_path / "s.ps"
    entry.write_text(src + NL, encoding="utf-8")
    env = dict(os.environ)
    env["PYTHONPATH"] = str(RAIZ / "src")
    env["GUZER_HEADLESS"] = "1"
    return subprocess.run(cmd + [str(entry)], capture_output=True, text=True, env=env).stdout.strip()


@pytest.mark.parametrize("src,esperado", CASOS)
def test_interp_builtins_grupoA(src, esperado, tmp_path):
    assert _run([sys.executable, "-m", "poolscript"], src, tmp_path) == esperado


@pytest.mark.skipif(not POOL_BIN.exists(), reason="binário ./pool não compilado")
@pytest.mark.parametrize("src,esperado", CASOS)
def test_paridade_builtins_grupoA(src, esperado, tmp_path):
    oi = _run([sys.executable, "-m", "poolscript"], src, tmp_path)
    ov = _run([str(POOL_BIN)], src, tmp_path)
    assert oi == ov == esperado, "INT:" + oi + " VM:" + ov
