"""Palavra reservada não pode ser usada como nome (igual o Python recusa `if = 1`)."""
import pytest
from poolscript.parser import parse_source, PoolParseError


def parse(src):
    return parse_source(src, "<test>")


def rejects(src) -> str:
    """Parseia esperando recusa; devolve a mensagem do erro."""
    with pytest.raises(PoolParseError) as exc:
        parse(src)
    return str(exc.value)


# ── `base` (reservada contextual: chamada de superclasse) ───────────────────
# Antes ela passava SILENCIOSAMENTE em vários pontos e só quebrava depois,
# na chamada `base(...)`, com erro de "variável não definida".
def test_base_as_assignment():
    assert "palavra reservada" in rejects("base = 5")


def test_base_as_typed_decl():
    assert "palavra reservada" in rejects("int base = 5")


def test_base_as_param():
    assert "palavra reservada" in rejects("action f(base) { return 1 }")


def test_base_as_loop_var():
    assert "palavra reservada" in rejects("for each base in [1] {\n post(1)\n}")


def test_base_as_catch_var():
    assert "palavra reservada" in rejects("try {\n post(1)\n} catch (base) {\n post(2)\n}")


def test_base_as_action_name():
    assert "palavra reservada" in rejects("action base() { return 1 }")


def test_base_as_entity_name():
    assert "palavra reservada" in rejects("Entity base() {\n action m(self) { return 1 }\n}")


def test_base_as_global():
    assert "palavra reservada" in rejects("action f() {\n global base\n}")


def test_base_in_unpacking():
    assert "palavra reservada" in rejects("a, base = 1, 2")


# ── keywords normais ────────────────────────────────────────────────────────
@pytest.mark.parametrize("kw", ["if", "while", "action", "return", "count", "self", "match"])
def test_keyword_as_assignment(kw):
    assert "palavra reservada" in rejects(f"{kw} = 5")


@pytest.mark.parametrize("kw", ["if", "while", "return", "import"])
def test_keyword_as_param(kw):
    assert "palavra reservada" in rejects(f"action f({kw}) {{ return 1 }}")


def test_keyword_as_loop_var():
    assert "palavra reservada" in rejects("for each while in [1] {\n post(1)\n}")


def test_keyword_as_action_name():
    assert "palavra reservada" in rejects("action while() { return 1 }")


def test_keyword_as_catch_var():
    assert "palavra reservada" in rejects("try {\n post(1)\n} catch (return) {\n post(2)\n}")


def test_keyword_compound_assign():
    assert "palavra reservada" in rejects("while += 1")


def test_error_message_names_the_word():
    assert "'while'" in rejects("while = 1")


# ── o que PRECISA continuar funcionando ─────────────────────────────────────
def test_self_still_valid_as_param():
    parse("Entity T() {\n    action __init__(self) { self.x = 1 }\n}")


def test_base_still_callable_as_super():
    parse("Entity F(P) {\n    action __init__(self, v) {\n        base(v)\n    }\n}")


def test_base_as_member_access():
    parse("action f(o) { return o.base }")


# `base` é kwarg real de psodbc/db.query() — a checagem é de nome LIGADO,
# não pode vazar pra argumento nomeado de chamada.
def test_base_as_call_kwarg():
    parse('import db\nr = db.query(base="x.db", cmd="SELECT 1")')


def test_base_as_only_kwarg():
    parse('import db\nr = db.query(base="x.db")')


def test_reserved_word_as_call_kwarg():
    # kwarg com nome de keyword também é só um rótulo de argumento
    parse('import db\nr = db.query(base="x.db", type="sqlite")')


# `count` é keyword E nome de kwarg real em regex.sub()/.replace() — antes
# esses parâmetros eram impossíveis de passar por nome (só posicional).
def test_count_kwarg_in_regex_sub():
    parse('import regex\npost(regex.sub("a", "b", "aaa", count=2))')


def test_count_kwarg_in_str_replace():
    parse('str s = "aaa"\npost(s.replace("a", "b", count=2))')


def test_count_prefix_operator_still_works():
    parse("list n = [1, 7, 7]\npost(count int(7) in n)")


def test_count_infix_operator_still_works():
    parse("list n = [1, 7, 7]\npost(int(7) count in n)")


def test_count_each_block_still_works():
    parse("list n = [7, 7]\naction f() {\n    count each int(7) in n {\n        return;\n    }\n}")


def test_name_containing_reserved_word_is_fine():
    parse("base_dados = 5\npost(base_dados)")
    parse("action database() { return 1 }")


def test_comparison_not_affected():
    parse('x = 1\nif (x == 1) { post("ok") }')


def test_count_operator_not_affected():
    parse("list n = [1, 7, 7]\npost(count int(7) in n)")
