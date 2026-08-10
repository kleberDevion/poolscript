"""Testes profundos ("não rasos") cobrindo toda a linguagem PoolScript:
tipos, controle de fluxo, try/catch/finally, match/case, model, Entity
(herança, @static, @dataentity, @NonNull), count/count each, geradores,
operadores (is/in/not), strings (f/r/cor), indentação híbrida e os erros
nomeados do spec. Cada seção testa casos de borda, não só o caminho feliz.
"""
from __future__ import annotations

import pytest

from poolscript.interpreter import run_source, PoolRuntimeError
from poolscript.parser import parse_source, PoolParseError
from poolscript.lexer import Lexer, PoolSyntaxError
from poolscript.errors import (
    ATTRIBUTTED_VALUE_ERROR,
    OUTPUT_UNEXPECTED_VALUES,
    SOME_VALUE_UNEXPECTED,
)


NL = chr(10)


def run(src: str):
    return run_source(src, "<test>")


# ═══════════════════════════ Tipos e coerção ════════════════════════════════

def test_typed_int_var_ok():
    assert run("int x = 5\npost(x)") == ["5"]


def test_typed_int_var_wrong_type_raises():
    # "abc" não é conversível para int (diferente de "123", que seria aceito
    # via coerção estilo input()) — deve levantar erro de runtime.
    with pytest.raises(PoolRuntimeError):
        run('int x = "abc"')


def test_typed_flo_accepts_int_literal_or_not():
    # flo declarado com valor inteiro — comportamento de coerção do spec
    out = run("flo x = 5\npost(x)")
    assert out == ["5"] or out == ["5.0"]


def test_typed_bool_wrong_type_raises():
    with pytest.raises(PoolRuntimeError):
        run("bool x = 1")


def test_null_variants_all_equivalent():
    out = run(
        'a = Null\nb = null\nc = None\nd = none\n'
        'post(a == b)\npost(b == c)\npost(c == d)\n'
    )
    assert out == ["True", "True", "True"]


def test_null_equals_zero_nullish():
    assert run("post(Null == 0)") == ["True"]


def test_str_plus_int_raises_attributted():
    with pytest.raises(PoolRuntimeError) as e:
        run('post("a" + 1)')
    assert e.value.code == ATTRIBUTTED_VALUE_ERROR


def test_redeclaration_same_scope_raises():
    with pytest.raises(PoolRuntimeError) as e:
        run("int x = 1\nint x = 2")
    assert e.value.code == OUTPUT_UNEXPECTED_VALUES


def test_math_type_mismatch_raises_some_value_unexpected():
    with pytest.raises(PoolRuntimeError) as e:
        run('post("a" * "b")')
    assert e.value.code == SOME_VALUE_UNEXPECTED


# ═══════════════════════════ Controle de fluxo ══════════════════════════════

def test_if_elif_elif_else_chain_picks_first_true():
    # No estilo com chaves, `elif`/`else` precisam ficar "colados" ao `}`
    # anterior na mesma linha (ver docs/PoolScript.md) — sem isso, o parser
    # não reconhece a continuação da cadeia if/elif/else.
    src = (
        "int nota = 6\n"
        "if (nota >= 9) {\n    post(\"A\")\n"
        "} elif (nota >= 7) {\n    post(\"B\")\n"
        "} elif (nota >= 5) {\n    post(\"C\")\n"
        "} else {\n    post(\"D\")\n}\n"
    )
    assert run(src) == ["C"]


def test_while_with_break_and_continue():
    src = (
        "i = 0\n"
        "resultado = []\n"
        "while (i < 10) {\n"
        "    i = i + 1\n"
        "    if (i % 2 == 0) { continue }\n"
        "    if (i > 7) { break }\n"
        "    addEnd(resultado, i)\n"
        "}\n"
        "post(resultado)\n"
    )
    assert run(src) == ["[1, 3, 5, 7]"]


def test_nested_loops_inner_break_does_not_affect_outer():
    src = (
        "out = []\n"
        "for each i in [1, 2, 3] {\n"
        "    for each j in [1, 2, 3] {\n"
        "        if (j == 2) { break }\n"
        "        addEnd(out, i * 10 + j)\n"
        "    }\n"
        "}\n"
        "post(out)\n"
    )
    assert run(src) == ["[11, 21, 31]"]


def test_for_each_over_string_iterates_chars():
    # Nota: usa estilo `:` de propósito — uma string-literal bruta seguida
    # imediatamente de `{` é ambígua com a sintaxe de interpolação
    # `"texto" {expr}` (ver docs/PoolScript.md), então `for each c in "abc" {`
    # tenta interpretar o `{` como interpolação em vez de abrir o bloco do
    # loop. Limitação conhecida do parser, não relacionada a este teste.
    out = run('resultado = []\nfor each c in "abc":\n    addEnd(resultado, c)\npost(resultado)')
    assert out == ["['a', 'b', 'c']"]


# ═══════════════════════════ try/catch/finally ══════════════════════════════

def test_try_catch_typed_matches_correct_clause():
    src = (
        "try {\n"
        "    x = 1 / 0\n"
        "} catch (SomeValueUnexpected e) {\n"
        "    post(\"tipo_certo\")\n"
        "} catch (e) {\n"
        "    post(\"generico\")\n"
        "}\n"
    )
    assert run(src) == ["tipo_certo"]


def test_try_catch_typed_wrong_type_falls_to_generic():
    src = (
        "try {\n"
        "    raise \"algo\"\n"
        "} catch (SomeValueUnexpected e) {\n"
        "    post(\"tipo_certo\")\n"
        "} catch (e) {\n"
        "    post(\"generico:\" e)\n"
        "}\n"
    )
    out = run(src)
    # o valor do catch carrega o ponto do erro por default (linha do raise)
    assert out == ["generico: algo (linha 2)"]


def test_try_finally_runs_on_success():
    # A gramática exige pelo menos um `catch` — `try/finally` sem catch não
    # é uma forma suportada (diferente do Python).
    out = run("try {\n    post(\"a\")\n} catch (e) {\n    post(\"nunca\")\n} finally {\n    post(\"fim\")\n}\n")
    assert out == ["a", "fim"]


def test_try_finally_runs_on_error_then_propagates():
    with pytest.raises(PoolRuntimeError):
        run("try {\n    x = 1/0\n} catch (KeyError e) {\n    post(\"nunca\")\n} finally {\n    post(\"fim\")\n}\n")


def test_try_catch_unmatched_error_propagates_after_finally():
    src = (
        "try {\n"
        "    x = 1/0\n"
        "} catch (KeyError e) {\n"
        "    post(\"nao_deveria\")\n"
        "} finally {\n"
        "    post(\"fim\")\n"
        "}\n"
    )
    with pytest.raises(PoolRuntimeError):
        run(src)


def test_nested_try_catch():
    src = (
        "try {\n"
        "    try {\n"
        "        x = 1/0\n"
        "    } catch (KeyError e) {\n"
        "        post(\"errado\")\n"
        "    }\n"
        "} catch (SomeValueUnexpected e) {\n"
        "    post(\"certo\")\n"
        "}\n"
    )
    assert run(src) == ["certo"]


def test_raise_custom_string_message():
    with pytest.raises(PoolRuntimeError):
        run('raise "mensagem customizada"')


# ═══════════════════════════ match / case ═══════════════════════════════════

def test_match_wildcard_default():
    src = "match 999:\n    case 1:\n        post(\"um\")\n    case _:\n        post(\"outro\")\n"
    assert run(src) == ["outro"]


def test_match_or_pattern_multiple_values():
    src = (
        'dia = "Terca"\n'
        'match dia:\n'
        '    case "Sabado" | "Domingo":\n'
        '        post("fim")\n'
        '    case "Segunda" | "Terca" | "Quarta":\n'
        '        post("inicio")\n'
        '    case _:\n'
        '        post("outro")\n'
    )
    assert run(src) == ["inicio"]


def test_match_guard_with_binding():
    src = (
        "x = 15\n"
        "match x:\n"
        "    case v if v < 10:\n"
        "        post(\"pequeno\")\n"
        "    case v if v < 20:\n"
        "        post(\"medio\")\n"
        "    case _:\n"
        "        post(\"grande\")\n"
    )
    assert run(src) == ["medio"]


def test_match_no_case_matches_is_noop():
    src = "match 5:\n    case 1:\n        post(\"a\")\n    case 2:\n        post(\"b\")\n"
    assert run(src) == []


# ═══════════════════════════ model ══════════════════════════════════════════

def test_model_valid_data_matches():
    src = (
        "model Usuario() {\n"
        "    nome: str(length=60)\n"
        "    idade: int(length=3)\n"
        "    ativo: bool\n"
        "}\n"
        'data = {"nome": "ana", "idade": 20, "ativo": true}\n'
        "if (data == Usuario) {\n    post(\"valido\")\n} else {\n    post(\"invalido\")\n}\n"
    )
    assert run(src) == ["valido"]


def test_model_missing_field_invalid():
    src = (
        "model Usuario() {\n"
        "    nome: str(length=60)\n"
        "    idade: int(length=3)\n"
        "}\n"
        'data = {"nome": "ana"}\n'
        "if (data == Usuario) {\n    post(\"valido\")\n} else {\n    post(\"invalido\")\n}\n"
    )
    assert run(src) == ["invalido"]


def test_model_wrong_type_field_invalid():
    src = (
        "model Usuario() {\n"
        "    idade: int(length=3)\n"
        "}\n"
        'data = {"idade": "vinte"}\n'
        "if (data == Usuario) {\n    post(\"valido\")\n} else {\n    post(\"invalido\")\n}\n"
    )
    assert run(src) == ["invalido"]


def test_model_exceeds_length_invalid():
    src = (
        "model Usuario() {\n"
        "    nome: str(length=3)\n"
        "}\n"
        'data = {"nome": "abcdef"}\n'
        "if (data == Usuario) {\n    post(\"valido\")\n} else {\n    post(\"invalido\")\n}\n"
    )
    assert run(src) == ["invalido"]


# ═══════════════════════════ Entity: herança / static / dataentity / NonNull ═

def test_entity_three_level_inheritance():
    # Herança de 3 níveis com override e base() explícito em cada nível.
    src = (
        "Entity Animal():\n"
        "    action __init__(self, nome):\n"
        "        self.nome = nome\n"
        "    action falar(self):\n"
        "        return \"...\"\n"
        "Entity Mamifero(Animal):\n"
        "    action __init__(self, nome):\n"
        "        base(nome)\n"
        "Entity Cachorro(Mamifero):\n"
        "    action __init__(self, nome):\n"
        "        base(nome)\n"
        "    action falar(self):\n"
        "        return \"Au!\"\n"
        "c = Cachorro(\"Rex\")\n"
        "post(c.nome)\n"
        "post(c.falar())\n"
    )
    assert run(src) == ["Rex", "Au!"]


def test_entity_static_method_no_instance_needed():
    src = (
        "Entity Util():\n"
        "    @static\n"
        "    action triplo(n):\n"
        "        return n * 3\n"
        "post(Util.triplo(4))\n"
    )
    assert run(src) == ["12"]


def test_dataentity_roundtrip_all_helpers():
    src = (
        "from datasentity import dataentity, asdict, astuple, aslist\n"
        "@dataentity\n"
        "Entity P():\n"
        "    nome: str\n"
        "    idade: int\n"
        "p = P(nome=\"Ana\", idade=30)\n"
        "post(asdict(p)[\"nome\"])\n"
        "post(astuple(p)[1])\n"
        "post(aslist(p)[0])\n"
    )
    assert run(src) == ["Ana", "30", "Ana"]


def test_nonnull_decorator_rejects_null_argument():
    src = (
        "@NonNull\n"
        "action precisa(v) {\n"
        "    return v\n"
        "}\n"
        "precisa(Null)\n"
    )
    with pytest.raises(PoolRuntimeError):
        run(src)


def test_nonnull_decorator_allows_non_null_argument():
    src = (
        "@NonNull\n"
        "action precisa(v) {\n"
        "    return v\n"
        "}\n"
        "post(precisa(5))\n"
    )
    assert run(src) == ["5"]


# @NonNull precisa funcionar em toda combinação de forma de declaração:
# action/reaction, com/sem tipo de retorno (int/bool/str/flo), com/sem async.
# Cada uma é um caminho de parsing diferente em parse_decorator_stmt — testar
# só `action` (como acima) deixa passar bugs como o de `@NonNull int reaction`
# sem `async`, que quebrava com "@NonNull deve ser seguido de uma action".
@pytest.mark.parametrize("decl, call, expected", [
    ("reaction precisa(v) { return v }", "precisa(5)", "5"),
    ("int reaction precisa(v) { return v }", "precisa(5)", "5"),
    ("bool reaction precisa(v) { return v }", "precisa(true)", "True"),
    ("str reaction precisa(v) { return v }", "precisa(\"ok\")", "ok"),
    ("flo reaction precisa(v) { return v }", "precisa(1.5)", "1.5"),
    ("action precisa(v) { return v }", "precisa(5)", "5"),
    ("async action precisa(v) { return v }", "await precisa(5)", "5"),
    ("async reaction precisa(v) { return v }", "await precisa(5)", "5"),
    ("async int reaction precisa(v) { return v }", "await precisa(5)", "5"),
    ("async bool reaction precisa(v) { return v }", "await precisa(true)", "True"),
])
def test_nonnull_decorator_parses_every_declaration_form(decl, call, expected):
    src = f"@NonNull\n{decl}\npost({call})\n"
    assert run(src) == [expected]


@pytest.mark.parametrize("decl", [
    "reaction precisa(v) { return v }",
    "int reaction precisa(v) { return v }",
    "action precisa(v) { return v }",
    "async int reaction precisa(v) { return v }",
])
def test_nonnull_decorator_rejects_null_in_every_declaration_form(decl):
    is_async = decl.startswith("async")
    call = "await precisa(Null)" if is_async else "precisa(Null)"
    src = f"@NonNull\n{decl}\n{call}\n"
    with pytest.raises(PoolRuntimeError):
        run(src)


# ═══════════════════════════ count / count each ═════════════════════════════

def test_count_prefix_form():
    assert run("nums = [1, 7, 2, 7, 3, 7]\npost(count int(7) in nums)") == ["3"]


def test_count_infix_form():
    assert run("nums = [1, 7, 2, 7]\npost(int(7) count in nums)") == ["2"]


def test_count_without_value_counts_all_of_type():
    assert run('post(count int in [1, "a", 2, "b", 3])') == ["3"]


def test_count_each_block_defines_match_index_count():
    src = (
        "nums = [7, 1, 7, 2, 7]\n"
        "count each int(7) in nums {\n"
        "    post(_index)\n"
        "}\n"
    )
    assert run(src) == ["0", "2", "4"]


def test_count_each_bare_return_accumulates_total():
    src = (
        "action total() {\n"
        "    nums = [7, 1, 7, 2, 7]\n"
        "    count each int(7) in nums {\n"
        "        return;\n"
        "    }\n"
        "}\n"
        "post(total())\n"
    )
    assert run(src) == ["3"]


def test_count_each_explicit_return_short_circuits():
    src = (
        "action primeiro() {\n"
        "    nums = [1, 2, 7, 9, 7]\n"
        "    count each int(7) in nums {\n"
        "        return _index\n"
        "    }\n"
        "}\n"
        "post(primeiro())\n"
    )
    assert run(src) == ["2"]


def test_count_in_conditional_truthy():
    assert run("nums = [1,2,3]\nif (count int(2) in nums) { post(\"achou\") }") == ["achou"]


def test_count_zero_is_falsy():
    out = run("nums = [1,2,3]\nif (count int(9) in nums) { post(\"achou\") } else { post(\"nao\") }")
    assert out == ["nao"]


# ═══════════════════════════ geradores (yield) ══════════════════════════════

def test_generator_multiple_yields_collected():
    src = (
        "action contar(n) {\n"
        "    i = 0\n"
        "    while (i < n) {\n"
        "        yield i\n"
        "        i = i + 1\n"
        "    }\n"
        "}\n"
        "resultado = []\n"
        "for each v in contar(4) {\n"
        "    addEnd(resultado, v)\n"
        "}\n"
        "post(resultado)\n"
    )
    assert run(src) == ["[0, 1, 2, 3]"]


def test_generator_early_break_stops_consumption():
    src = (
        "action contar(n) {\n"
        "    i = 0\n"
        "    while (i < n) {\n"
        "        yield i\n"
        "        i = i + 1\n"
        "    }\n"
        "}\n"
        "resultado = []\n"
        "for each v in contar(10) {\n"
        "    if (v > 2) { break }\n"
        "    addEnd(resultado, v)\n"
        "}\n"
        "post(resultado)\n"
    )
    assert run(src) == ["[0, 1, 2]"]


def test_generator_empty_range_yields_nothing():
    src = (
        "action contar(n) {\n"
        "    i = 0\n"
        "    while (i < n) {\n"
        "        yield i\n"
        "        i = i + 1\n"
        "    }\n"
        "}\n"
        "resultado = []\n"
        "for each v in contar(0) {\n"
        "    addEnd(resultado, v)\n"
        "}\n"
        "post(resultado)\n"
    )
    assert run(src) == ["[]"]


# ═══════════════════════════ operadores is/in/not ═══════════════════════════

def test_is_type_checks():
    src = (
        'a = "x"\nb = 1\nc = 1.5\nd = true\n'
        'if (a is str and b is int and c is flo and d is bool) {\n    post("todos_ok")\n}\n'
    )
    assert run(src) == ["todos_ok"]


def test_not_is_and_is_not_equivalent():
    out = run(
        "x = \"texto\"\n"
        "if (x not is int) { post(\"a\") }\n"
        "if (x is not None) { post(\"b\") }\n"
    )
    assert out == ["a", "b"]


def test_in_and_not_in_membership():
    out = run(
        "items = [1, 2, 3]\n"
        "if (2 in items) { post(\"tem2\") }\n"
        "if (9 not in items) { post(\"sem9\") }\n"
    )
    assert out == ["tem2", "sem9"]


def test_and_or_not_boolean_logic_combinations():
    out = run(
        "a = true\nb = false\n"
        "if (a and not b) { post(\"1\") }\n"
        "if (a or b) { post(\"2\") }\n"
        "if (not (a and b)) { post(\"3\") }\n"
        "if (not a and b) { post(\"nao\") } else { post(\"4\") }\n"
    )
    assert out == ["1", "2", "3", "4"]


def test_compound_condition_with_count_and_and_or():
    src = (
        "a = [1, 3, 1]\nb = [3, 3]\n"
        "if (count int(1) in a and count int(3) in b == 1) {\n"
        "    post(\"complexo_ok\")\n"
        "}\n"
    )
    # count int(3) in b == 1 é falso (tem 2 treses), então o `and` inteiro é falso
    assert run(src) == []


# ═══════════════════════════ strings: f/r/cor ═══════════════════════════════

def test_fstring_interpolation_brace_form():
    out = run('grau = 25\npost(f"Clima: {grau} graus")')
    assert out == ["Clima: 25 graus"]


def test_string_concat_brace_interpolation_form():
    out = run('grau = 25\npost("Clima: " {grau} " graus")')
    assert out == ["Clima: 25 graus"]


def test_raw_string_preserves_backslashes():
    out = run(r'post(r"C:\Users\test")')
    assert out == ["C:\\Users\\test"]


def test_normal_string_processes_escapes():
    out = run(r'post("linha1\nlinha2")')
    assert out == ["linha1\nlinha2"]


def test_color_named_wraps_ansi():
    out = run('post(<red>"alerta")')
    assert "alerta" in out[0] and "\x1b[" in out[0]


def test_color_hex_produces_expected_ansi_rgb():
    out = run('post(<2196f3>"azul")')
    assert "\x1b[38;2;33;150;243m" in out[0]


# ═══════════════════════════ indentação híbrida ═════════════════════════════

def test_indent_style_colon_basic_block():
    out = run("if (1 == 1):\n    post(\"ok\")\n")
    assert out == ["ok"]


def test_indent_style_brace_and_colon_can_coexist_across_statements():
    out = run(
        "if (1 == 1) { post(\"a\") }\n"
        "if (2 == 2):\n"
        "    post(\"b\")\n"
    )
    assert out == ["a", "b"]


def test_indent_tab_rejected():
    with pytest.raises((PoolSyntaxError,)):
        Lexer("if (1 == 1):\n\tpost(\"x\")\n").tokenize()


def test_indent_non_multiple_of_4_rejected():
    with pytest.raises((PoolSyntaxError,)):
        Lexer("if (1 == 1):\n  post(\"x\")\n").tokenize()


def test_indent_jump_more_than_one_level_rejected():
    src = "if (1 == 1):\n    if (2 == 2):\n            post(\"x\")\n"
    with pytest.raises((PoolSyntaxError,)):
        Lexer(src).tokenize()


def test_indent_inconsistent_dedent_rejected():
    src = "if (1 == 1):\n    if (2 == 2):\n        post(\"x\")\n      post(\"y\")\n"
    with pytest.raises((PoolSyntaxError,)):
        Lexer(src).tokenize()


def test_mixed_style_no_warning(capsys):
    # aviso de "mistura" removido (falso-positivo com ':' de dict/CSS)
    src = (
        "if 1 == 1 { post(\"a\") }\n"
        "if 1 == 1:\n"
        "    post(\"b\")\n"
    )
    parse_source(src, "<mix>")
    err = capsys.readouterr().err
    assert "mistura" not in err


def test_pure_brace_style_no_warning(capsys):
    parse_source('if 1 == 1 { post("a") }\n', "<t>")
    assert "mistura" not in capsys.readouterr().err


def test_pure_colon_style_no_warning(capsys):
    parse_source('if 1 == 1:\n    post("a")\n', "<t>")
    assert "mistura" not in capsys.readouterr().err


# ══════════════ regressão de otimizações (cache de generator, Scope) ════════

def test_generator_detection_cache_correct_across_repeated_calls():
    # `_has_yield` agora é cacheado por UserFunction — precisa continuar
    # correto ao chamar a MESMA action várias vezes com yield dentro de
    # if/while aninhados.
    src = (
        "action gen(n) {\n"
        "    i = 0\n"
        "    while (i < n) {\n"
        "        if (i == 1) {\n"
        "            yield 999\n"
        "        }\n"
        "        yield i\n"
        "        i = i + 1\n"
        "    }\n"
        "}\n"
        "r1 = []\n"
        "for each v in gen(2) { addEnd(r1, v) }\n"
        "r2 = []\n"
        "for each v in gen(3) { addEnd(r2, v) }\n"
        "post(r1)\n"
        "post(r2)\n"
    )
    assert run(src) == ["[0, 999, 1]", "[0, 999, 1, 2]"]


def test_non_generator_function_not_misdetected_after_cache():
    # Chama uma action comum (sem yield) várias vezes — o cache não pode
    # fazer com que ela vire generator incorretamente.
    src = (
        "action soma(a, b) {\n"
        "    return a + b\n"
        "}\n"
        "post(soma(1, 2))\n"
        "post(soma(3, 4))\n"
        "post(soma(5, 6))\n"
    )
    assert run(src) == ["3", "7", "11"]


def test_deep_scope_chain_variable_lookup_still_works():
    # Scope.get/set viraram loop iterativo em vez de recursão — precisa
    # continuar resolvendo variáveis em closures/blocos bem aninhados.
    src = (
        "x = 1\n"
        "if (true) {\n"
        "    if (true) {\n"
        "        if (true) {\n"
        "            if (true) {\n"
        "                y = x + 1\n"
        "                post(y)\n"
        "            }\n"
        "        }\n"
        "    }\n"
        "}\n"
    )
    assert run(src) == ["2"]


def test_enumerate_returns_list_of_tuples_not_raw_iterator():
    # enumerate() precisa devolver uma lista materializada (igual ao zip()),
    # não um iterador Python cru — senão nem post() nem for each conseguem
    # usá-lo (for each só aceita list/tuple/str).
    assert run("post(enumerate([10, 20]))") == ["[(0, 10), (1, 20)]"]
    out = run("resultado = []\nfor each par in enumerate([10, 20]) { addEnd(resultado, par) }\npost(resultado)")
    assert out == ["[(0, 10), (1, 20)]"]


def test_map_and_filter_argument_order_is_list_then_function():
    assert run("post(map([1, 2, 3], action(x) { return x * 2 }))") == ["[2, 4, 6]"]
    assert run("post(filter([1, 2, 3], action(x) { return x > 1 }))") == ["[2, 3]"]


def test_deep_scope_chain_assignment_to_outer_variable():
    src = (
        "total = 0\n"
        "i = 0\n"
        "while (i < 5) {\n"
        "    if (true) {\n"
        "        if (true) {\n"
        "            total = total + i\n"
        "        }\n"
        "    }\n"
        "    i = i + 1\n"
        "}\n"
        "post(total)\n"
    )
    assert run(src) == ["10"]


def test_global_stmt_writes_persist_across_calls():
    src = (
        "contador = 0\n"
        "action incrementar() {\n"
        "    global contador\n"
        "    contador = contador + 1\n"
        "}\n"
        "incrementar()\n"
        "incrementar()\n"
        "incrementar()\n"
        "post(contador)\n"
    )
    assert run(src) == ["3"]


def test_global_stmt_creates_variable_visible_outside_action():
    src = (
        "action registrar() {\n"
        "    global visitas\n"
        "    visitas = 1\n"
        "}\n"
        "registrar()\n"
        "post(visitas)\n"
    )
    assert run(src) == ["1"]


def test_global_stmt_reaches_through_nested_block():
    src = (
        "contador = 0\n"
        "action bump() {\n"
        "    global contador\n"
        "    if (true) {\n"
        "        contador = contador + 100\n"
        "    }\n"
        "}\n"
        "bump()\n"
        "post(contador)\n"
    )
    assert run(src) == ["100"]


def test_without_global_stmt_local_assignment_does_not_leak():
    src = (
        "action f() {\n"
        "    x = 99\n"
        "}\n"
        "f()\n"
        "try {\n"
        "    post(x)\n"
        "} catch (e) {\n"
        "    post(\"nao-vazou\")\n"
        "}\n"
    )
    assert run(src) == ["nao-vazou"]


# ═══════════════════ `dict` e `tup` como palavra-chave de tipo ══════════════
# `type(x)` devolve "dict" e "tup", então declarar e testar com esses nomes
# tem que valer. `dict` é apelido de `json` (mesmo tipo); `tup` nomeia a
# tupla, que não tinha palavra-chave nenhuma.
#
# Interpretador só: na VM isso depende do nó `TypeName` e do `count`, que
# ainda estão na camada 3 da migração.

@pytest.mark.parametrize("src,esperado", [
    ('dict d = {"a": 1}' + NL + "post(d)",        ["{'a': 1}"]),
    ("tup t = (1, 2)" + NL + "post(t)",           ["(1, 2)"]),
    ('json j = {"a": 1}' + NL + "post(j)",        ["{'a': 1}"]),
])
def test_declaracao_com_dict_e_tup(src, esperado):
    assert run(src) == esperado


@pytest.mark.parametrize("src,esperado", [
    ('d = {"a": 1}' + NL + "post(d is dict)",     ["True"]),
    ('d = {"a": 1}' + NL + "post(d is json)",     ["True"]),
    ("t = (1, 2)" + NL + "post(t is tup)",        ["True"]),
    ("t = (1, 2)" + NL + "post(t is dict)",       ["False"]),
    ('d = {"a": 1}' + NL + "post(d is tup)",      ["False"]),
    ("l = [1]" + NL + "post(l is dict)",          ["False"]),
    ('d = {"a": 1}' + NL + "post(d not is tup)",  ["True"]),
])
def test_is_com_dict_e_tup(src, esperado):
    assert run(src) == esperado


@pytest.mark.parametrize("src,esperado", [
    ("post(count dict in [{}, {}, 1])",           ["2"]),
    ("post(count json in [{}, 1])",               ["1"]),
    ("post(count tup in [(1,), 1])",              ["1"]),
])
def test_count_com_dict_e_tup(src, esperado):
    assert run(src) == esperado


@pytest.mark.parametrize("src", [
    "dict = 5",
    "tup = 5",
    "action dict() {" + NL + " return 1" + NL + "}",
    "Entity tup() {" + NL + "}",
])
def test_dict_e_tup_viraram_reservadas(src):
    """Palavra-chave nova não pode mais servir de nome — igual a `json`."""
    with pytest.raises(Exception):
        run(src)
