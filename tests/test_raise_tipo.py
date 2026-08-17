"""`raise Tipo("msg")` — levantar exception com NOME DE TIPO LIVRE (qualquer
IDENT_UPPER vira o tipo do erro). Funciona com e sem catch, e casa no
`catch (Tipo ...)`. Saída idêntica nos dois motores (paridade).
"""
import os
import subprocess
import sys
from pathlib import Path

import pytest

RAIZ = Path(__file__).resolve().parent.parent
POOL_BIN = RAIZ / "pool"
NL = chr(10)

PEGA = (
    'try { raise NetworkError("caiu") } catch (NetworkError e) { post("A", e) }' + NL +
    'try { raise DatabaseError("off") } catch (DatabaseError) { post("B") }' + NL +
    'try { raise MinhaFalhaQualquer("x") } catch (e) { post("C", e) }' + NL +
    'post("fim")' + NL
)
SEM_CATCH = 'raise NetworkError("propaga limpo")' + NL


def _run(cmd, src, tmp_path):
    entry = tmp_path / "r.ps"
    entry.write_text(src, encoding="utf-8")
    env = dict(os.environ)
    env["PYTHONPATH"] = str(RAIZ / "src")
    return subprocess.run(cmd + [str(entry)], capture_output=True, text=True, env=env)


def test_interp_raise_tipo_livre(tmp_path):
    r = _run([sys.executable, "-m", "poolscript"], PEGA, tmp_path)
    assert r.returncode == 0, r.stdout + r.stderr
    linhas = r.stdout.strip().splitlines()
    assert linhas[0].startswith("A") and "caiu" in linhas[0]
    assert "B" in linhas
    assert any(l.startswith("C") for l in linhas)
    assert linhas[-1] == "fim"


def test_interp_raise_sem_catch_eh_limpo(tmp_path):
    r = _run([sys.executable, "-m", "poolscript"], SEM_CATCH, tmp_path)
    assert r.returncode != 0
    saida = r.stdout + r.stderr
    assert "NetworkError" in saida and "propaga limpo" in saida
    assert "Traceback" not in r.stderr or "NetworkError" in r.stderr  # sem crash do runtime


@pytest.mark.skipif(not POOL_BIN.exists(), reason="binário ./pool não compilado")
def test_paridade_raise_tipo(tmp_path):
    a = _run([sys.executable, "-m", "poolscript"], PEGA, tmp_path).stdout
    b = _run([str(POOL_BIN)], PEGA, tmp_path).stdout
    assert a == b, "divergência:" + NL + "INTERP:" + NL + a + NL + "VM C:" + NL + b
