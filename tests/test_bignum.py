"""bignum — inteiro de precisão arbitrária. O C VM promove int64 pra bignum
(GMP) no overflow; o INTERP já é ilimitado (Python). O teste roda a MESMA
conta gigante nos dois motores e exige saída IDÊNTICA (paridade real).

Cobre: overflow de +/-/*, potência por laço, %, comparação, ==, negação,
type()==int, e um LCG (que o C VM antes truncava em 64 bits, divergindo do
INTERP).
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
    "post(9223372036854775807 + 1)",
    "post(1000000000000 * 1000000000000)",
    "post(9223372036854775807 * 2 - 1)",
    "a = 1",
    "for each i in range(40) { a = a * 10 }",
    "post(a)",
    "post(type(a))",
    "post(a > 999999999999999999)",
    "post(a == a)",
    "post(a % 7)",
    "post(-a)",
    "b = 12345",
    "for each i in range(25) { b = 1103515245 * b + 12345 }",
    "post(b % 26)",
    "post(b)",
]) + NL


def _run(cmd, tmp_path):
    entry = tmp_path / "b.ps"
    entry.write_text(SCRIPT, encoding="utf-8")
    env = dict(os.environ)
    env["PYTHONPATH"] = str(RAIZ / "src")
    return subprocess.run(cmd + [str(entry)], capture_output=True, text=True, env=env)


def test_interp_bignum(tmp_path):
    r = _run([sys.executable, "-m", "poolscript"], tmp_path)
    assert r.returncode == 0, r.stdout + r.stderr
    linhas = r.stdout.strip().splitlines()
    assert linhas[0] == "9223372036854775808"        # overflow do int64
    assert linhas[6] == "int"                          # type(bignum) == int
    assert linhas[7] == "True" and linhas[8] == "True"  # comparação e ==


@pytest.mark.skipif(not POOL_BIN.exists(), reason="binário ./pool não compilado")
def test_paridade_bignum(tmp_path):
    """A conta gigante tem que dar EXATAMENTE a mesma coisa nos dois motores."""
    a = _run([sys.executable, "-m", "poolscript"], tmp_path).stdout
    b = _run([str(POOL_BIN)], tmp_path).stdout
    assert a == b, "divergência bignum:" + NL + "INTERP:" + NL + a + NL + "VM C:" + NL + b
    assert a.strip().splitlines()[0] == "9223372036854775808"
