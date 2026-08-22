"""Precedência de import: uma LIB instalada (~/.poolscript/libs, ou
$POOLSCRIPT_HOME/libs) SEMPRE ganha de um arquivo local com o mesmo nome.

Motivou o teste: o usuário tinha `import random` num arquivo chamado
`random.psl`; o arquivo local ofuscava a lib `random` instalada e o import
pegava o próprio arquivo (auto-import), dando erro sem sentido. O nome do
arquivo nunca pode atrapalhar uma lib — vale nos DOIS motores, idêntico.
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


def _cenario(tmp_path):
    """LIB instalada e ARQUIVO local com o MESMO nome, resultados diferentes."""
    home = tmp_path / "home"; (home / "libs").mkdir(parents=True)
    proj = tmp_path / "proj"; proj.mkdir()
    (home / "libs" / "minhalib.ps").write_text(
        '#!lib' + NL + 'action quem() { return "LIB" }' + NL, encoding="utf-8")
    (proj / "minhalib.psl").write_text(
        'action quem() { return "ARQUIVO_LOCAL" }' + NL, encoding="utf-8")
    entry = proj / "uso.ps"
    entry.write_text('import minhalib' + NL + 'post(minhalib.quem())' + NL, encoding="utf-8")
    return home, entry


def _env(home):
    e = dict(os.environ)
    e["POOLSCRIPT_HOME"] = str(home)
    e["PYTHONPATH"] = str(RAIZ / "src")
    return e


def test_interp_lib_ganha_de_arquivo_local(tmp_path):
    home, entry = _cenario(tmp_path)
    r = subprocess.run([sys.executable, "-m", "poolscript", str(entry)],
                       capture_output=True, text=True, env=_env(home))
    assert "LIB" in r.stdout and "ARQUIVO_LOCAL" not in r.stdout, r.stdout + r.stderr


def test_vm_lib_ganha_de_arquivo_local(tmp_path):
    home, entry = _cenario(tmp_path)
    r = subprocess.run([str(POOL_BIN), str(entry)],
                       capture_output=True, text=True, env=_env(home))
    assert "LIB" in r.stdout and "ARQUIVO_LOCAL" not in r.stdout, r.stdout + r.stderr


def test_dois_motores_batem(tmp_path):
    """A saída tem que ser IDÊNTICA nos dois motores (paridade)."""
    home, entry = _cenario(tmp_path)
    a = subprocess.run([sys.executable, "-m", "poolscript", str(entry)],
                       capture_output=True, text=True, env=_env(home)).stdout
    b = subprocess.run([str(POOL_BIN), str(entry)],
                       capture_output=True, text=True, env=_env(home)).stdout
    assert a == b == "LIB" + NL
