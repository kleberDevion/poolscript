"""Testes dos operadores bitwise: ^ | & ~ << >>."""
import pytest
from poolscript.interpreter import Interpreter, PoolRuntimeError
from poolscript.errors import SOME_VALUE_UNEXPECTED
from poolscript.parser import parse_source


def run(src):
    interp = Interpreter(source=src)
    interp.run(parse_source(src))
    return interp.output


# ── cada operador, valor correto ─────────────────────────────────────────────
def test_xor():
    assert run("post(5 ^ 3)") == ["6"]


def test_bitor():
    assert run("post(5 | 2)") == ["7"]


def test_bitand():
    assert run("post(6 & 3)") == ["2"]


def test_lshift():
    assert run("post(1 << 4)") == ["16"]


def test_rshift():
    assert run("post(256 >> 4)") == ["16"]


def test_bitnot():
    assert run("post(~5)") == ["-6"]


def test_bitnot_zero():
    assert run("post(~0)") == ["-1"]


# ── precedência: | ^ & << >> ficam entre comparação e soma, igual Python ────
def test_precedence_shift_vs_add():
    # 2 + 3 << 1  ==  (2 + 3) << 1  ==  10  (shift mais fraco que soma)
    assert run("post(2 + 3 << 1)") == ["10"]


def test_precedence_xor_vs_compare():
    # 1 == 1 | 0  ==  1 == (1 | 0)  ==  True  (bitwise mais forte que comparação)
    assert run("post(1 == 1 | 0)") == ["True"]


def test_precedence_and_vs_or_vs_xor():
    # 8 & 4 ^ 2 | 1  ==  ((8 & 4) ^ 2) | 1  ==  (0 ^ 2) | 1  ==  3
    assert run("post(8 & 4 ^ 2 | 1)") == ["3"]


def test_precedence_shift_vs_bitand():
    # 1 << 2 & 6  ==  (1 << 2) & 6  ==  4 & 6  ==  4
    assert run("post(1 << 2 & 6)") == ["4"]


def test_parens_override_precedence():
    assert run("post((2 + 3) << 1)") == ["10"]
    assert run("post(2 + (3 << 1))") == ["8"]


# ── variáveis, não só literais ───────────────────────────────────────────────
def test_xor_with_variables():
    assert run("int a = 12\nint b = 10\npost(a ^ b)") == ["6"]


def test_shift_with_variable_count():
    assert run("int n = 1\nint k = 3\npost(n << k)") == ["8"]


# ── bool é rejeitado (não deve silenciosamente virar 0/1) ───────────────────
def test_xor_rejects_bool():
    with pytest.raises(PoolRuntimeError) as exc_info:
        run("post(True ^ False)")
    assert exc_info.value.code == SOME_VALUE_UNEXPECTED


def test_bitand_rejects_bool():
    with pytest.raises(PoolRuntimeError) as exc_info:
        run("post(True & 1)")
    assert exc_info.value.code == SOME_VALUE_UNEXPECTED


def test_bitnot_rejects_bool():
    with pytest.raises(PoolRuntimeError) as exc_info:
        run("post(~True)")
    assert exc_info.value.code == SOME_VALUE_UNEXPECTED


# ── tipos inválidos (str, flo, list) são rejeitados com erro claro ──────────
def test_xor_rejects_string():
    with pytest.raises(PoolRuntimeError) as exc_info:
        run('post("a" ^ 1)')
    assert exc_info.value.code == SOME_VALUE_UNEXPECTED


def test_bitor_rejects_float():
    with pytest.raises(PoolRuntimeError) as exc_info:
        run("post(1.5 | 1)")
    assert exc_info.value.code == SOME_VALUE_UNEXPECTED


def test_bitnot_rejects_string():
    with pytest.raises(PoolRuntimeError) as exc_info:
        run('post(~"x")')
    assert exc_info.value.code == SOME_VALUE_UNEXPECTED


# ── deslocamento negativo (Python levanta ValueError nativo — precisa
#    virar PoolRuntimeError, não vazar como exceção Python crua) ────────────
def test_lshift_negative_count_raises_clean_error():
    with pytest.raises(PoolRuntimeError) as exc_info:
        run("post(1 << -1)")
    assert exc_info.value.code == SOME_VALUE_UNEXPECTED


def test_rshift_negative_count_raises_clean_error():
    with pytest.raises(PoolRuntimeError) as exc_info:
        run("post(1 >> -1)")
    assert exc_info.value.code == SOME_VALUE_UNEXPECTED


# ── não quebra o '|' já usado em match patterns (contexto diferente) ────────
def test_match_or_pattern_still_works():
    src = """
match 2:
    case 1 | 2 | 3:
        post("bateu")
    case _:
        post("nao bateu")
"""
    assert run(src) == ["bateu"]


# ── não quebra o count/in que reusa o mesmo nível de precedência ────────────
def test_count_in_still_works_after_precedence_change():
    src = "list nums = [1, 7, 7, 17, 7]\npost(count int(7) in nums)"
    assert run(src) == ["3"]
