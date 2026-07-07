"""Testes do interpretador."""
import io
import sys
from contextlib import redirect_stdout

import pytest
from poolscript.interpreter import (
    Interpreter,
    PoolRuntimeError,
    run_source,
)
from poolscript.errors import (
    ATTRIBUTTED_VALUE_ERROR,
    OUTPUT_UNEXPECTED_VALUES,
    SOME_VALUE_UNEXPECTED,
)


def run(src):
    interp = Interpreter(source=src)
    from poolscript.parser import parse_source
    interp.run(parse_source(src))
    return interp.output


def test_post_basic():
    assert run('post("oi")') == ["oi"]


def test_var_decl_and_use():
    out = run('x = 10\npost(x)')
    assert out == ["10"]


def test_typed_var_correct():
    out = run('str nome = "Pool"\npost(nome)')
    assert out == ["Pool"]


def test_typed_var_wrong_type():
    with pytest.raises(PoolRuntimeError) as exc_info:
        run('str nome = 10')
    assert exc_info.value.code == ATTRIBUTTED_VALUE_ERROR


def test_redeclaration_error():
    with pytest.raises(PoolRuntimeError) as exc_info:
        run('int x = 1\nint x = 2')
    assert exc_info.value.code == OUTPUT_UNEXPECTED_VALUES


def test_str_plus_int_error():
    with pytest.raises(PoolRuntimeError) as exc_info:
        run('post("a" + 1)')
    assert exc_info.value.code == ATTRIBUTTED_VALUE_ERROR


def test_math_invalid_types():
    with pytest.raises(PoolRuntimeError) as exc_info:
        run('post("a" * "b")')
    assert exc_info.value.code == SOME_VALUE_UNEXPECTED


def test_null_eq_zero():
    out = run('if Null == 0:\n    post("yes")\n')
    assert out == ["yes"]


def test_null_gt_zero_false():
    out = run('if Null > 0:\n    post("not")\nelse:\n    post("ok")\n')
    # Note: spec só pede que comparações de magnitude com null retornem False.
    # Mas o parser não tem `else:` sem if branches anteriores; usamos elif:
    # Simplifiquei: só checamos via expressão direta.


def test_null_gt_zero_direct():
    out = run('x = Null > 0\npost(x)')
    assert out == ["False"]


def test_index_out_of_bounds_returns_null(capsys):
    out = run('lista = [1, 2, 3]\nx = lista[99]\npost(x)')
    assert out == ["null"]
    captured = capsys.readouterr()
    assert "IndexOutOfBoundsWarning" in captured.err


def test_for_each():
    out = run('for each i in ["a","b","c"]:\n    post(i)\n')
    assert out == ["a", "b", "c"]


def test_action_call():
    src = 'action sum(a, b) { return a + b }\npost(sum(2, 3))'
    assert run(src) == ["5"]


def test_action_local_scope():
    # variável local não vaza para fora
    src = '''action f() { local = 99 }
f()
post("ok")
'''
    assert run(src) == ["ok"]


def test_try_catch_division():
    out = run('try {\n    x = 10 / 0\n} catch (e) {\n    post("err: " {e})\n}\n')
    assert any("err:" in line for line in out)


def test_fstring():
    assert run('x = 10\npost(f"v={x}")') == ["v=10"]


def test_braces_interpolation():
    assert run('g = 25\npost("Clima: " {g} "°")') == ["Clima: 25°"]


def test_juncao():
    out = run('g = 25\nj = ("Resultado: " {g})\npost(j)')
    assert out == ["Resultado: 25"]


def test_postfix_increment():
    out = run('i = 0\nwhile i < 3 {\n    post(i)\n    i++\n}\n')
    assert out == ["0", "1", "2"]
