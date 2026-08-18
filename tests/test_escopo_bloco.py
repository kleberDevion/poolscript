"""Escopo de bloco — paridade INTERP/VM.

Variável NOVA nascida dentro de um bloco (if/elif/else, while, for each,
count each, using, try/catch/finally, match/case, run_selfwith_) não vaza pro
escopo de fora — igual ao interpretador. Reatribuir uma variável que já existe
fora (write-through) continua funcionando, e cada iteração de laço começa com o
corpo "limpo" (escopo por-iteração).

Regressão da divergência em que só a VM em C vazava (escopo de função).
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
    return r.stdout, r.returncode


def _interp(src, tmp_path):
    return _run([sys.executable, "-m", "poolscript"], src, tmp_path)


def _vm(src, tmp_path):
    return _run([str(POOL_BIN)], src, tmp_path)


# (fonte, stdout esperado, deve_dar_erro)
CASOS = [
    # variável de bloco não vaza
    ("if true:" + NL + "    d = 5" + NL + "post(d)" + NL, "", True),
    # reatribuir a de fora (write-through) funciona
    ("x = 1" + NL + "if true:" + NL + "    x = 2" + NL + "post(x)" + NL, "2", False),
    # for: var do laço não vaza
    ("for each i in [1,2,3]:" + NL + "    pass" + NL + "post(i)" + NL, "", True),
    # for: corpo é por-iteração (prev da volta anterior não existe)
    ("for each i in [1,2,3]:" + NL + "    if i > 1:" + NL
     + "        post(prev)" + NL + "    prev = i" + NL, "", True),
    # for: acumulador externo (write-through)
    ("t = 0" + NL + "for each i in [1,2,3]:" + NL + "    t = t + i" + NL
     + "post(t)" + NL, "6", False),
    # while: corpo não vaza
    ("w = 0" + NL + "while w < 3:" + NL + "    dentro = w" + NL
     + "    w += 1" + NL + "post(dentro)" + NL, "", True),
    # break limpa o que foi criado no laço
    ("for each i in [1,2,3]:" + NL + "    achou = i" + NL + "    if i == 2:" + NL
     + "        break" + NL + "post(achou)" + NL, "", True),
    # catch var é do bloco
    ('try:' + NL + '    raise Erro("x")' + NL + "catch (e):" + NL
     + "    ok = 1" + NL + "post(e)" + NL, "", True),
    # função: bloco não vaza, mas parâmetro sobrevive
    ("action f(a):" + NL + "    if a > 0:" + NL + "        tmp = a * 2" + NL
     + "        return tmp" + NL + "    return a" + NL + "post(f(3))" + NL, "6", False),
]


@pytest.mark.parametrize("src,esperado,erro", CASOS)
def test_interp_escopo_bloco(src, esperado, erro, tmp_path):
    out, rc = _interp(src, tmp_path)
    assert out.strip() == esperado, out
    assert (rc != 0) == erro, "returncode inesperado: " + str(rc)


@pytest.mark.skipif(not POOL_BIN.exists(), reason="binário ./pool não compilado")
@pytest.mark.parametrize("src,esperado,erro", CASOS)
def test_paridade_escopo_bloco(src, esperado, erro, tmp_path):
    oi, ri = _interp(src, tmp_path)
    ov, rv = _vm(src, tmp_path)
    assert oi.strip() == ov.strip(), "stdout diverge:" + NL + "INT:" + oi + NL + "VM:" + ov
    assert (ri != 0) == (rv != 0), "erro? diverge: interp=" + str(ri) + " vm=" + str(rv)
    assert ov.strip() == esperado
