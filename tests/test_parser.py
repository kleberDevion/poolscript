"""Testes do parser."""
from poolscript.parser import parse_source, PoolParseError
from poolscript.lexer import PoolSyntaxError
import pytest


def test_var_decl():
    p = parse_source("int x = 10")
    assert len(p.statements) == 1


def test_if_elif_else():
    src = """
if (x == 1) {
    post("a")
} elif x == 2:
    post("b")
else {
    post("c")
}
"""
    p = parse_source(src)
    assert len(p.statements) == 1


def test_action_decl():
    src = "action sum(a, b) { return a + b }"
    p = parse_source(src)
    assert len(p.statements) == 1


def test_for_each():
    src = "for each i in [1, 2, 3]:\n    post(i)\n"
    p = parse_source(src)
    assert len(p.statements) == 1


def test_try_catch():
    src = "try {\n    x = 1\n} catch (e) {\n    post(e)\n}\n"
    p = parse_source(src)
    assert len(p.statements) == 1


def test_import_modes():
    assert parse_source("import os")
    assert parse_source("from os import getenv")
    assert parse_source("PUSH os GET getenv")


def test_invalid_syntax():
    with pytest.raises((PoolSyntaxError, PoolParseError)):
        parse_source("if {\n}\n")


def test_string_interpolation_braces():
    # Forma 1 do spec
    p = parse_source('post("Clima: " {grau} "°")')
    assert len(p.statements) == 1
