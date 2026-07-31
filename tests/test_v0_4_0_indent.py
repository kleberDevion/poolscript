"""Testes da Rodada 3: indentação por ':' (Python-like) com 4 espaços estritos."""
import pytest
from poolscript.lexer import Lexer, PoolSyntaxError
from poolscript.parser import parse_source
from poolscript.interpreter import Interpreter


def run(src: str) -> list:
    """Roda código PoolScript e retorna stdout como lista de linhas."""
    interp = Interpreter(source=src)
    interp.run(parse_source(src, "<test>"))
    return interp.output


# ── Aceitação: 4 espaços ────────────────────────────────────────────
def test_if_colon_4spaces_ok():
    src = (
        "int x = 10\n"
        "if x > 5:\n"
        "    post(\"big\")\n"
    )
    assert run(src) == ["big"]


def test_while_colon_4spaces_ok():
    src = (
        "int i = 0\n"
        "while i < 3:\n"
        "    post(i)\n"
        "    i = i + 1\n"
    )
    assert run(src) == ["0", "1", "2"]


def test_action_colon_nested_4spaces():
    src = (
        "action greet(name):\n"
        "    if name == \"oi\":\n"
        "        post(\"hello\")\n"
        "    else:\n"
        "        post(\"bye\")\n"
        "greet(\"oi\")\n"
        "greet(\"x\")\n"
    )
    assert run(src) == ["hello", "bye"]


# ── Rejeição: indent inválido ────────────────────────────────────────
def test_indent_2_spaces_fails():
    src = (
        "if 1 == 1:\n"
        "  post(\"ruim\")\n"
    )
    with pytest.raises(PoolSyntaxError) as ei:
        Lexer(src, "<t>").tokenize()
    assert "múltiplo de 4" in str(ei.value)


def test_indent_3_spaces_fails():
    src = (
        "if 1 == 1:\n"
        "   post(\"ruim\")\n"
    )
    with pytest.raises(PoolSyntaxError):
        Lexer(src, "<t>").tokenize()


def test_indent_tab_fails():
    src = "if 1 == 1:\n\tpost(\"ruim\")\n"
    with pytest.raises(PoolSyntaxError) as ei:
        Lexer(src, "<t>").tokenize()
    assert "TAB" in str(ei.value)


def test_indent_jump_2_levels_fails():
    src = (
        "if 1 == 1:\n"
        "        post(\"pulou nivel\")\n"
    )
    with pytest.raises(PoolSyntaxError) as ei:
        Lexer(src, "<t>").tokenize()
    assert "esperado exatamente 4" in str(ei.value)


# ── Híbrido livre por bloco ──────────────────────────────────────────
def test_brace_outer_brace_inner():
    # Dentro de '{...}' a indentação é ignorada — então blocos internos
    # também precisam usar '{...}'. Misturar ':' dentro de '{' não é suportado.
    src = (
        "if 1 == 1 {\n"
        "    if 2 == 2 { post(\"misto\") }\n"
        "}\n"
    )
    assert run(src) == ["misto"]


def test_colon_outer_brace_inner():
    src = (
        "if 1 == 1:\n"
        "    if 2 == 2 { post(\"misto2\") }\n"
    )
    assert run(src) == ["misto2"]


# ── Warning (não impede execução) ────────────────────────────────────
def test_mix_no_longer_warns(capsys):
    # o aviso de "mistura" foi removido — era falso-positivo (pegava ':' de
    # dict e CSS). Misturar {} e : no mesmo arquivo agora é silencioso.
    src = (
        "if 1 == 1 { post(\"a\") }\n"
        "if 1 == 1:\n"
        "    post(\"b\")\n"
    )
    parse_source(src, "<mix>")
    err = capsys.readouterr().err
    assert "mistura" not in err


def test_pure_brace_no_warning(capsys):
    parse_source("if 1 == 1 { post(\"a\") }\n", "<t>")
    assert "mistura" not in capsys.readouterr().err


def test_pure_colon_no_warning(capsys):
    parse_source("if 1 == 1:\n    post(\"a\")\n", "<t>")
    assert "mistura" not in capsys.readouterr().err


# ── for each por : ──────────────────────────────────────────────────
def test_foreach_colon():
    src = (
        "for each x in [1, 2, 3]:\n"
        "    post(x)\n"
    )
    assert run(src) == ["1", "2", "3"]


# ── try/catch por : ──────────────────────────────────────────────────
def test_try_catch_colon():
    src = (
        "try:\n"
        "    int x = 1 / 0\n"
        "catch (e):\n"
        "    post(\"pego\")\n"
    )
    assert run(src) == ["pego"]
