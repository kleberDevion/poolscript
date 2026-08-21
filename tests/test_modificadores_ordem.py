"""Modificadores de action/reaction em QUALQUER ordem — nos 2 motores.

A ordem quem decide é o usuário: `int async reaction`, `async int reaction`,
`public async reaction`, `private reaction`... todos válidos. E `int x = 5`
continua sendo declaração de variável (não action).
"""
import os
import subprocess
import sys
from pathlib import Path

import pytest

RAIZ = Path(__file__).resolve().parent.parent
POOL = RAIZ / "pool"
SRC = RAIZ / "src"

CASOS = [
    ("int async reaction foo():\n    return 7\npost(foo())", "7"),
    ("async int reaction foo():\n    return 7\npost(foo())", "7"),
    ("public async reaction foo():\n    return 8\npost(foo())", "8"),
    ("private async reaction foo():\n    return 9\npost(foo())", "9"),
    ("int reaction foo():\n    return 11\npost(foo())", "11"),
    ("async reaction foo():\n    return 3\npost(foo())", "3"),
    ("public action foo():\n    return 12\npost(foo())", "12"),
    # não pode virar action: continua declaração de variável
    ("int x = 5\npost(x)", "5"),
]


def _interp(src, tmp_path):
    p = tmp_path / "a.ps"
    p.write_text(src, encoding="utf-8")
    env = dict(os.environ, PYTHONPATH=str(SRC))
    r = subprocess.run([sys.executable, "-m", "poolscript", str(p)],
                       capture_output=True, text=True, env=env, timeout=15)
    return r.stdout.strip()


def _vm(src, tmp_path):
    p = tmp_path / "b.ps"
    p.write_text(src, encoding="utf-8")
    env = dict(os.environ, PYTHONPATH=str(SRC))
    r = subprocess.run([str(POOL), str(p)], capture_output=True, text=True, env=env, timeout=15)
    return r.stdout.strip()


@pytest.mark.parametrize("src,esperado", CASOS)
def test_ordem_livre_interp(src, esperado, tmp_path):
    assert _interp(src, tmp_path) == esperado


@pytest.mark.skipif(not POOL.exists(), reason="binário pool não compilado")
@pytest.mark.parametrize("src,esperado", CASOS)
def test_ordem_livre_vm(src, esperado, tmp_path):
    assert _vm(src, tmp_path) == esperado
