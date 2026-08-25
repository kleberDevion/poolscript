"""Inteiro grande (bignum) na VM em C: conta certa em vez de número errado.

Achados da caça de 2026-08-25 (notas/caca-bugs-2026-08-25.md). O padrão era o
pior possível — a VM **não errava**, ela respondia ERRADO e seguia:

  1 << 63                 -> -9223372036854775808   (deu a volta no int64)
  1 << 64                 -> 1                      (UB do C: shift % 64)
  abs(-9223372036854775808) -> negativo
  int("<32 dígitos>")     -> lixo negativo
  bignum | 1              -> "'|' exige int"   (o motor chamando de não-int
                                                um valor cujo type() é "int")

Comparado com o interpretador em cada caso.
"""
import os
import subprocess
import sys
from pathlib import Path

import pytest

RAIZ = Path(__file__).resolve().parent.parent
POOL = RAIZ / "pool"
SRC = RAIZ / "src"

assert POOL.exists(), "binário 'pool' não compilado — rode ./rebuild_vm.sh."


def _saida(cmd, tmp_path, fonte):
    ps = tmp_path / "t.ps"
    ps.write_text(fonte, encoding="utf-8")
    env = dict(os.environ, PYTHONPATH=str(SRC))
    r = subprocess.run(cmd + [str(ps)], capture_output=True, text=True,
                       env=env, timeout=30)
    assert r.returncode == 0, r.stderr
    return r.stdout.strip()


def ambos(tmp_path, fonte):
    vm = _saida([str(POOL)], tmp_path, fonte)
    interp = _saida([sys.executable, "-m", "poolscript"], tmp_path, fonte)
    assert vm == interp, f"VM {vm!r} != interp {interp!r}\n{fonte}"
    return vm


@pytest.mark.parametrize("expr,esperado", [
    ("1 << 62", "4611686018427387904"),
    ("1 << 63", "9223372036854775808"),      # estourava pra negativo
    ("1 << 64", "18446744073709551616"),     # virava 1
    ("3 << 70", "3541774862152233910272"),   # virava 192
    ("1 << 200", "1606938044258990275541962092341162602522202993782792835301376"),
    ("(1 << 64) >> 64", "1"),
    ("9223372036854775808 >> 1", "4611686018427387904"),
    ("-8 >> 1", "-4"),
    ("1 >> 100", "0"),
    ("-1 >> 100", "-1"),
])
def test_shift(tmp_path, expr, esperado):
    assert ambos(tmp_path, f"post({expr})\n") == esperado


@pytest.mark.parametrize("expr", [
    "18446744073709551616 | 1",
    "18446744073709551616 & 1",
    "18446744073709551616 ^ 1",
    "18446744073709551616 + 1",
    "18446744073709551616 * 2",
])
def test_bitwise_e_aritmetica_aceitam_bignum(tmp_path, expr):
    """`type()` diz que é int; os operadores tinham que concordar."""
    ambos(tmp_path, f"post({expr})\n")


def test_abs_de_int64_min(tmp_path):
    assert ambos(tmp_path, "post(abs(-9223372036854775808))\n") == "9223372036854775808"


def test_int_de_texto_grande(tmp_path):
    assert ambos(tmp_path, 'post(int("12345678901234567890123456789012"))\n') \
        == "12345678901234567890123456789012"


def test_int_de_texto_normal_nao_regrediu(tmp_path):
    assert ambos(tmp_path, 'post(int("42"), int("-7"), int(" 8 "))\n') == "42 -7 8"


def test_builtins_numericos_com_bignum(tmp_path):
    """sum/sorted/max/min/round/flo/int/str tratando bignum como int."""
    fonte = ("x = 18446744073709551616\n"
             "post(sum([x, 1]))\n"
             "post(sorted([x, 1]))\n"
             "post(max(x, 1), min(x, 1))\n"
             "post(round(x))\n"
             "post(int(x))\n"
             "post(str(x))\n")
    ambos(tmp_path, fonte)


def test_ordem_entre_listas_e_tuplas(tmp_path):
    """`<`/`>=` entre duas listas: o sorted() já comparava, o operador não."""
    assert ambos(tmp_path, "post([1] < [2], [1,2] >= [1,2], (1,2) < (1,3))\n") \
        == "True True True"


def test_type_continua_dizendo_int(tmp_path):
    assert ambos(tmp_path, "x = 18446744073709551616\npost(type(x), x.type())\n") == "int int"
