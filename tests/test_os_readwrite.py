"""os.readFile / os.writeFile — roundtrip de texto e criação da pasta pai,
igual nos dois motores."""
import os
import subprocess
import sys
from pathlib import Path

import pytest

RAIZ = Path(__file__).resolve().parent.parent
POOL = RAIZ / "pool"
SRC = RAIZ / "src"

# escreve num subdiretório inexistente (writeFile cria a pasta pai) e lê de volta
PROG = """import os
p = os.writeFile("__DIR__/sub/nota.txt", "linha1\\nábc")
post(p)
post(os.readFile("__DIR__/sub/nota.txt"))
"""


def _run(cmd, tmp_path, nome):
    ps = tmp_path / nome
    ps.write_text(PROG.replace("__DIR__", str(tmp_path)), encoding="utf-8")
    env = dict(os.environ, PYTHONPATH=str(SRC))
    return subprocess.run(cmd + [str(ps)], capture_output=True, text=True,
                          env=env, timeout=15)


def _confere(r, tmp_path):
    linhas = r.stdout.splitlines()
    assert linhas[0] == str(tmp_path / "sub" / "nota.txt"), r.stdout
    assert linhas[1] == "linha1"
    assert linhas[2] == "ábc"


def test_interp(tmp_path):
    _confere(_run([sys.executable, "-m", "poolscript"], tmp_path, "a.ps"), tmp_path)


@pytest.mark.skipif(not POOL.exists(), reason="binário pool não compilado")
def test_vm(tmp_path):
    _confere(_run([str(POOL)], tmp_path, "b.ps"), tmp_path)
