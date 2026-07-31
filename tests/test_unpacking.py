"""Testes profundos de desemembramento de tuplas (unpacking), estilo Python.

Cobre:
  - parsing: forma do AST + garantias de não-regressão (chamadas, tupla-literal,
    comparação, atribuição de membro, atribuição simples, `return a, b` continuam
    parseando exatamente como antes).
  - interpretador: todas as formas de unpacking (múltiplo, swap, aninhado,
    star/rest, string, escopo, erros de aridade e de tipo).
"""
from __future__ import annotations

import pytest

from poolscript.interpreter import run_source, PoolRuntimeError
from poolscript.parser import (
    parse_source,
    PoolParseError,
    UnpackAssignment,
    UnpackTarget,
    Assignment,
    MemberAssignment,
    TupleLiteral,
    BinaryOp,
    ExpressionStmt,
    Call,
    ReturnStmt,
)
from poolscript.lexer import PoolSyntaxError
from poolscript.errors import OUTPUT_UNEXPECTED_VALUES, SOME_VALUE_UNEXPECTED


def run(src: str):
    return run_source(src, "<test>")


# ───────────────────────────── parser: forma do AST ─────────────────────────

def test_parses_simple_unpack_as_unpack_assignment():
    p = parse_source("a, b = 1, 2")
    stmt = p.statements[0]
    assert isinstance(stmt, UnpackAssignment)
    assert stmt.targets.elements == ["a", "b"]
    assert stmt.targets.star_index is None
    assert isinstance(stmt.value, TupleLiteral)
    assert len(stmt.value.items) == 2


def test_parses_unpack_from_single_expr_rhs_unwrapped():
    p = parse_source("a, b = minha_lista")
    stmt = p.statements[0]
    assert isinstance(stmt, UnpackAssignment)
    # RHS de valor único não deve ser empacotado numa TupleLiteral
    assert not isinstance(stmt.value, TupleLiteral)


def test_parses_nested_target():
    p = parse_source("a, (b, c) = 1, (2, 3)")
    stmt = p.statements[0]
    assert isinstance(stmt, UnpackAssignment)
    assert stmt.targets.elements[0] == "a"
    nested = stmt.targets.elements[1]
    assert isinstance(nested, UnpackTarget)
    assert nested.elements == ["b", "c"]


def test_parses_star_target():
    p = parse_source("a, *resto = [1, 2, 3, 4]")
    stmt = p.statements[0]
    assert isinstance(stmt, UnpackAssignment)
    assert stmt.targets.elements == ["a", "resto"]
    assert stmt.targets.star_index == 1


def test_parses_leading_star_target():
    p = parse_source("*inicio, z = [1, 2, 3, 4]")
    stmt = p.statements[0]
    assert isinstance(stmt, UnpackAssignment)
    assert stmt.targets.elements == ["inicio", "z"]
    assert stmt.targets.star_index == 0


def test_trailing_comma_single_target():
    p = parse_source("a, = [5]")
    stmt = p.statements[0]
    assert isinstance(stmt, UnpackAssignment)
    assert stmt.targets.elements == ["a"]
    assert stmt.targets.trailing_comma is True


def test_lone_star_without_comma_is_parse_error():
    with pytest.raises(PoolParseError):
        parse_source("*resto = [1, 2, 3]")


def test_two_stars_same_level_is_parse_error():
    with pytest.raises(PoolParseError):
        parse_source("a, *b, *c = [1, 2, 3]")


def test_two_stars_in_nested_level_is_parse_error():
    with pytest.raises(PoolParseError):
        parse_source("a, (*b, *c) = 1, [2, 3]")


# ───────────────────────── parser: garantias de não-regressão ───────────────

def test_plain_single_assignment_unaffected():
    p = parse_source("a = 1")
    assert isinstance(p.statements[0], Assignment)


def test_member_assignment_self_unaffected():
    p = parse_source("self.x = 1")
    assert isinstance(p.statements[0], MemberAssignment)


def test_member_assignment_obj_unaffected():
    p = parse_source("obj.y = 2")
    assert isinstance(p.statements[0], MemberAssignment)


def test_call_with_two_args_not_misdetected_as_unpack():
    p = parse_source("foo(a, b)")
    stmt = p.statements[0]
    assert isinstance(stmt, ExpressionStmt)
    assert isinstance(stmt.expression, Call)


def test_tuple_literal_comparison_unaffected():
    p = parse_source("(a, b) == (1, 2)")
    stmt = p.statements[0]
    assert isinstance(stmt, ExpressionStmt)
    assert isinstance(stmt.expression, BinaryOp)


def test_bare_tuple_literal_statement_unaffected():
    p = parse_source("(a, b)")
    stmt = p.statements[0]
    assert isinstance(stmt, ExpressionStmt)
    assert isinstance(stmt.expression, TupleLiteral)


def test_return_multi_value_tuple_unaffected():
    p = parse_source("action f() { return a, 409 }")
    action_block = p.statements[0].block
    ret = action_block.statements[0]
    assert isinstance(ret, ReturnStmt)
    assert isinstance(ret.value, TupleLiteral)


def test_augmented_assign_with_comma_falls_through_as_before():
    # Nunca foi válido; deve continuar dando erro (não deve virar UnpackAssignment).
    with pytest.raises((PoolParseError, PoolSyntaxError)):
        parse_source("a, b += 1, 2")


def test_redundant_parens_single_target_unaffected():
    # (a) = 5 nunca foi uma forma válida — não deve virar unpacking.
    with pytest.raises((PoolParseError, PoolSyntaxError)):
        parse_source("(a) = 5")


# ───────────────────────── interpretador: comportamento ─────────────────────

def test_basic_unpack():
    assert run("a, b = 1, 2\npost(a)\npost(b)") == ["1", "2"]


def test_unpack_from_list_variable():
    assert run("l = [1, 2, 3]\na, b, c = l\npost(a)\npost(b)\npost(c)") == ["1", "2", "3"]


def test_unpack_from_tuple_literal_variable():
    assert run("t = (1, 2, 3)\na, b, c = t\npost(a)\npost(b)\npost(c)") == ["1", "2", "3"]


def test_swap():
    assert run("a = 1\nb = 2\na, b = b, a\npost(a)\npost(b)") == ["2", "1"]


def test_swap_evaluates_rhs_before_any_write():
    # Garante que não há sobrescrita parcial durante o swap.
    out = run("a = 1\nb = 2\nc = 3\na, b, c = c, a, b\npost(a)\npost(b)\npost(c)")
    assert out == ["3", "1", "2"]


def test_nested_unpack():
    assert run("a, (b, c) = 1, (2, 3)\npost(a)\npost(b)\npost(c)") == ["1", "2", "3"]


def test_nested_unpack_with_star_inside():
    out = run("a, (b, c) = 1, (2, 3)\npost(a)")
    assert out == ["1"]
    out2 = run("a, (b, rest) = 1, (2, [3, 4])\npost(b)\npost(rest)")
    assert out2 == ["2", "[3, 4]"]


def test_star_rest_middle():
    out = run("a, *resto = [1, 2, 3, 4]\npost(a)\npost(resto)")
    assert out == ["1", "[2, 3, 4]"]
    assert isinstance(eval(out[1]), list)


def test_star_rest_is_real_list_type():
    interp_out = run("a, *resto = (1, 2, 3, 4)\npost(type(resto))")
    assert interp_out == ["list"]


def test_star_leading():
    out = run("*inicio, z = [1, 2, 3, 4]\npost(inicio)\npost(z)")
    assert out == ["[1, 2, 3]", "4"]


def test_star_middle():
    out = run("a, *meio, z = [1, 2, 3, 4, 5]\npost(a)\npost(meio)\npost(z)")
    assert out == ["1", "[2, 3, 4]", "5"]


def test_star_exact_zero_remaining():
    out = run("a, b, *resto = [1, 2]\npost(a)\npost(b)\npost(resto)")
    assert out == ["1", "2", "[]"]


def test_trailing_comma_single():
    assert run("a, = [5]\npost(a)") == ["5"]


def test_trailing_comma_too_many_raises():
    with pytest.raises(PoolRuntimeError) as exc:
        run("a, = [5, 6]")
    assert exc.value.code == OUTPUT_UNEXPECTED_VALUES


def test_string_rhs_unpack():
    assert run('a, b = "hi"\npost(a)\npost(b)') == ["h", "i"]


def test_not_enough_values_raises_with_code():
    with pytest.raises(PoolRuntimeError) as exc:
        run("a, b, c = [1, 2]")
    assert exc.value.code == OUTPUT_UNEXPECTED_VALUES


def test_too_many_values_raises_with_code():
    with pytest.raises(PoolRuntimeError) as exc:
        run("a, b = [1, 2, 3]")
    assert exc.value.code == OUTPUT_UNEXPECTED_VALUES


def test_star_not_enough_values_raises():
    with pytest.raises(PoolRuntimeError) as exc:
        run("a, b, *resto = [1]")
    assert exc.value.code == OUTPUT_UNEXPECTED_VALUES


def test_rhs_not_iterable_raises():
    with pytest.raises(PoolRuntimeError) as exc:
        run("a, b = 5")
    assert exc.value.code == SOME_VALUE_UNEXPECTED


def test_fresh_binding_then_reassignment_same_semantics():
    # Primeira ocorrência cria as variáveis; a segunda reatribui — igual à
    # Assignment simples (scope.set/define).
    out = run(
        "a, b = 1, 2\n"
        "post(a)\npost(b)\n"
        "a, b = 10, 20\n"
        "post(a)\npost(b)\n"
    )
    assert out == ["1", "2", "10", "20"]


def test_unpack_inside_action_scope():
    out = run(
        "action f() {\n"
        "    a, b = 1, 2\n"
        "    return a + b\n"
        "}\n"
        "post(f())\n"
    )
    assert out == ["3"]


def test_unpack_inside_nested_block_writes_to_correct_scope():
    out = run(
        "a = 0\n"
        "b = 0\n"
        "if (True) {\n"
        "    a, b = 7, 8\n"
        "}\n"
        "post(a)\npost(b)\n"
    )
    assert out == ["7", "8"]


def test_swap_of_three_via_unpack():
    out = run("a, b, c = 1, 2, 3\nc, a, b = a, b, c\npost(a)\npost(b)\npost(c)")
    assert out == ["2", "3", "1"]


def test_generator_rhs_unpack():
    # Iteráveis do tipo PoolGenerator (yield) também devem poder ser desempacotados.
    out = run(
        "action gen() {\n"
        "    yield 1\n"
        "    yield 2\n"
        "}\n"
        "a, b = gen()\n"
        "post(a)\npost(b)\n"
    )
    assert out == ["1", "2"]


# ─────────────────── regressão: nada mais quebrou no interpretador ──────────

def test_existing_for_each_still_works():
    assert run("for each i in [1, 2, 3]:\n    post(i)\n") == ["1", "2", "3"]


def test_existing_member_assignment_still_works():
    out = run(
        "Entity Ponto() {\n"
        "    action __init__(self, x) {\n"
        "        self.x = x\n"
        "    }\n"
        "}\n"
        "p = Ponto(5)\n"
        "post(p.x)\n"
    )
    assert out == ["5"]


def test_existing_return_tuple_still_works():
    out = run(
        "action f() {\n"
        "    return 1, 2\n"
        "}\n"
        "r = f()\n"
        "post(r)\n"
    )
    assert out == ["(1, 2)"]
