"""Acesso a chave de dict por atributo: `d.chave` equivale a `d["chave"]`,
para LER e ESCREVER (inclusive `+=`). Método de dict (`.get`, `.keys`, ...)
tem prioridade; chave inexistente dá o mesmo `KeyError` do acesso por índice.
Saída idêntica nos dois motores (paridade).
"""
import os
import subprocess
import sys
from pathlib import Path

import pytest

RAIZ = Path(__file__).resolve().parent.parent
POOL_BIN = RAIZ / "pool"
NL = chr(10)

SCRIPT = NL.join([
    'd = { "nome": "ana", "n": 1 }',
    "post(d.nome)",             # leitura por atributo
    'd.nome = "bia"',           # escrita por atributo
    "post(d.nome)",
    'post(d["nome"])',          # consistente com o índice
    "d.n += 5",                 # aumentada
    "post(d.n)",
    'post(d.get("nome"))',      # método ainda ganha
]) + NL


def _run(cmd, tmp_path):
    entry = tmp_path / "d.ps"
    entry.write_text(SCRIPT, encoding="utf-8")
    env = dict(os.environ)
    env["PYTHONPATH"] = str(RAIZ / "src")
    env["GUZER_HEADLESS"] = "1"
    return subprocess.run(cmd + [str(entry)], capture_output=True, text=True, env=env)


def test_interp_dict_atributo(tmp_path):
    r = _run([sys.executable, "-m", "poolscript"], tmp_path)
    assert r.returncode == 0, r.stdout + r.stderr
    linhas = r.stdout.strip().splitlines()
    assert linhas == ["ana", "bia", "bia", "6", "bia"], r.stdout


@pytest.mark.skipif(not POOL_BIN.exists(), reason="binário ./pool não compilado")
def test_paridade_dict_atributo(tmp_path):
    a = _run([sys.executable, "-m", "poolscript"], tmp_path).stdout
    b = _run([str(POOL_BIN)], tmp_path).stdout
    assert a == b, "divergência:" + NL + "INTERP:" + NL + a + NL + "VM C:" + NL + b
    assert a.strip().splitlines() == ["ana", "bia", "bia", "6", "bia"]
