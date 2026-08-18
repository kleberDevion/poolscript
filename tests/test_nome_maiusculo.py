"""Nome MAIÚSCULO é variável normal — paridade INTERP/VM.

O interp recusava atribuir a um nome maiúsculo sem tipo (`NOME = "x"`) e
recusava reatribuir; a VM em C aceitava. Agora os dois aceitam (maiúsculo é um
nome como qualquer outro: com ou sem tipo, reatribuível).
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
    ('NOME = "x"' + NL + "post(NOME)" + NL, "x"),
    ('NOME = "a"' + NL + 'NOME = "b"' + NL + "post(NOME)" + NL, "b"),
    ("MAX = 100" + NL + "MAX = MAX + 1" + NL + "post(MAX)" + NL, "101"),
    ('str NOME = "oi"' + NL + "post(NOME)" + NL, "oi"),
    ("MAX = 1" + NL + "MAX += 5" + NL + "post(MAX)" + NL, "6"),
]


def _run(cmd, src, tmp_path):
    entry = tmp_path / "s.ps"
    entry.write_text(src, encoding="utf-8")
    env = dict(os.environ)
    env["PYTHONPATH"] = str(RAIZ / "src")
    env["GUZER_HEADLESS"] = "1"
    return subprocess.run(cmd + [str(entry)], capture_output=True, text=True, env=env).stdout.strip()


@pytest.mark.parametrize("src,esperado", CASOS)
def test_interp_nome_maiusculo(src, esperado, tmp_path):
    assert _run([sys.executable, "-m", "poolscript"], src, tmp_path) == esperado


@pytest.mark.skipif(not POOL_BIN.exists(), reason="binário ./pool não compilado")
@pytest.mark.parametrize("src,esperado", CASOS)
def test_paridade_nome_maiusculo(src, esperado, tmp_path):
    oi = _run([sys.executable, "-m", "poolscript"], src, tmp_path)
    ov = _run([str(POOL_BIN)], src, tmp_path)
    assert oi == ov == esperado, "INT:" + oi + " VM:" + ov
