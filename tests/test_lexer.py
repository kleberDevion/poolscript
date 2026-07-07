"""Testes do lexer."""
from poolscript.lexer import Lexer, PoolSyntaxError
import pytest


def types(src):
    return [t.type for t in Lexer(src).tokenize() if t.type != "NEWLINE"]


def test_basic_assignment():
    toks = types("x = 10")
    assert toks[:4] == ["IDENT", "OP", "INT", "EOF"]


def test_keywords_and_types():
    toks = types("str nome = \"Pool\"")
    assert toks[:5] == ["KW", "IDENT", "OP", "STR", "EOF"]


def test_fstring():
    toks = types('post(f"oi {x}")')
    assert "FSTRING" in toks


def test_indent_dedent():
    src = "if x:\n    post(1)\npost(2)\n"
    types_ = [t.type for t in Lexer(src).tokenize()]
    assert "INDENT" in types_
    assert "DEDENT" in types_


def test_unterminated_string():
    with pytest.raises(PoolSyntaxError):
        Lexer('x = "abc\n').tokenize()


def test_block_comment_skipped():
    toks = types('""" comentario """\nx = 1')
    assert toks[:4] == ["IDENT", "OP", "INT", "EOF"]


def test_null_aliases():
    for word in ("Null", "null", "None", "none"):
        toks = Lexer(f"x = {word}").tokenize()
        kinds = [t.type for t in toks]
        assert "NULL" in kinds


def test_bool_aliases():
    for word in ("True", "true", "False", "false"):
        toks = Lexer(f"x = {word}").tokenize()
        kinds = [t.type for t in toks]
        assert "BOOL" in kinds


def test_double_op_logical():
    toks = types("x and y && z")
    assert "KW" in toks and "OP" in toks
