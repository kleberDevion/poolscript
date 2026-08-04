"""Testes profundos do lexer (tokenização): cobre todas as keywords,
operadores (com ambiguidades entre curtos/longos), strings (normal/f/raw),
escapes, cores, números, comentários, e todas as regras de indentação
estilo Python (incluindo os casos de erro). Complementa tests/test_lexer.py,
que só cobria o caminho feliz superficialmente.
"""
from __future__ import annotations

import pytest

from poolscript.lexer import Lexer, PoolSyntaxError, KEYWORDS, MULTI_OPS, SINGLE_OPS, NAMED_COLORS


def toks(src):
    return Lexer(src).tokenize()


def types(src):
    return [t.type for t in toks(src) if t.type != "NEWLINE"]


def values(src):
    return [t.value for t in toks(src) if t.type != "NEWLINE"]


# ═══════════════════════════ keywords ═══════════════════════════════════════

def test_every_keyword_tokenizes_as_kw():
    for kw in sorted(KEYWORDS):
        t = toks(kw)[0]
        assert t.type == "KW", f"keyword {kw!r} não virou KW (virou {t.type})"
        assert t.value == kw


def test_keyword_prefix_of_identifier_is_not_a_keyword():
    # "importante" começa com "import" mas deve virar UM identificador, não
    # a keyword "import" seguida de lixo.
    t = toks("importante")[0]
    assert t.type == "IDENT"
    assert t.value == "importante"


def test_identifier_uppercase_first_letter_is_ident_upper():
    t = toks("MinhaClasse")[0]
    assert t.type == "IDENT_UPPER"


def test_identifier_lowercase_or_underscore_is_ident():
    assert toks("nome")[0].type == "IDENT"
    assert toks("_privado")[0].type == "IDENT"


# ═══════════════════════════ operadores ═════════════════════════════════════

def test_all_multi_char_ops_tokenize_whole_not_split():
    for op in MULTI_OPS:
        t = toks(f"a {op} b")[1]
        assert t.type == "OP"
        assert t.value == op, f"operador {op!r} não bateu (virou {t.value!r})"


def test_all_single_char_ops_tokenize():
    # ',', '.' e '@' também estão em SINGLE_OPS mas são interceptados antes
    # pela tabela PUNCT (viram COMMA/DOT/AT, não OP) — são pontuação, não
    # operadores de expressão, então são tratados à parte abaixo.
    punct_overlap = {",": "COMMA", ".": "DOT", "@": "AT"}
    for op in sorted(SINGLE_OPS):
        t = toks(f"a {op} b")[1]
        if op in punct_overlap:
            assert t.type == punct_overlap[op]
        else:
            assert t.type == "OP"
            assert t.value == op


def test_not_equal_is_one_token_not_two_bangs():
    tk = types("a != b")
    assert tk == ["IDENT", "OP", "IDENT", "EOF"]
    assert values("a != b")[1] == "!="


def test_triple_equals_takes_priority_over_double():
    v = values("a === b")
    assert v[1] == "==="


def test_double_equals_does_not_swallow_extra_equals_incorrectly():
    # "a == b" não deve virar "a" "===" ... (limite de token correto)
    v = values("a == b")
    assert v[1] == "=="


def test_increment_decrement_tokenize_as_single_ops():
    assert values("a++")[1] == "++"
    assert values("a--")[1] == "--"


def test_augmented_assignment_ops():
    for op in ("+=", "-=", "*=", "/=", "%="):
        v = values(f"a {op} 1")
        assert v[1] == op


def test_logical_and_or_double_char():
    v = values("a && b || c")
    assert v[1] == "&&"
    assert v[3] == "||"


def test_bare_bang_is_single_op_not_multi():
    # "!" sozinho (sem "=" na sequência) deve virar OP "!" isolado.
    t = toks("!")[0]
    assert t.type == "OP"
    assert t.value == "!"


# ═══════════════════════════ números ════════════════════════════════════════

def test_integer_literal():
    t = toks("42")[0]
    assert t.type == "INT" and t.value == 42


def test_float_literal():
    t = toks("3.14")[0]
    assert t.type == "FLO" and t.value == 3.14


def test_integer_then_dot_member_access_not_confused_with_float():
    # "5.isdigit()" — a regex de número casa só dígitos+"."+dígitos, então
    # um "." sem dígito depois não deveria virar parte do número.
    tk = types("x = 5\nx.len")
    assert "INT" in tk


# ═══════════════════════════ strings: normal / escapes ══════════════════════

def test_string_basic():
    t = toks('"ola"')[0]
    assert t.type == "STR" and t.value == "ola"


def test_string_recognized_escapes():
    assert toks(r'"a\nb"')[0].value == "a\nb"
    assert toks(r'"a\tb"')[0].value == "a\tb"
    assert toks(r'"a\rb"')[0].value == "a\rb"
    assert toks(r'"a\\b"')[0].value == "a\\b"
    assert toks(r'"a\"b"')[0].value == 'a"b'


def test_string_ansi_e_escapes_c():
    # ANSI + escapes de C/Python: octal \033, hex \x1b, \e — todos = ESC (0x1b).
    assert toks(r'"\033[1m"')[0].value == "\x1b[1m"
    assert toks(r'"\x1b[0m"')[0].value == "\x1b[0m"
    assert toks(r'"\e[3m"')[0].value == "\x1b[3m"
    assert toks(r'"\101"')[0].value == "A"          # octal → 'A'
    assert toks(r'"\x41"')[0].value == "A"          # hex → 'A'
    assert toks(r'"\xe9"')[0].value == "é"      # hex não-ASCII → codepoint
    assert toks(r'"\a\b\f\v"')[0].value == "\a\b\f\v"


def test_string_unrecognized_escape_drops_backslash():
    # Comportamento documentado (igual a várias linguagens C-like): uma
    # sequência de escape não reconhecida perde o backslash e mantém só a
    # letra. É a causa raiz de um bug real encontrado nesta sessão (paths
    # do Windows embutidos sem usar string raw r"...").
    assert toks(r'"C:\Users"')[0].value == "C:Users"


def test_unterminated_string_raises():
    with pytest.raises(PoolSyntaxError):
        toks('"abc\n')


def test_unterminated_string_eof_raises():
    with pytest.raises(PoolSyntaxError):
        toks('"abc')


# ═══════════════════════════ f-strings / raw strings ════════════════════════

def test_fstring_tokenizes_as_fstring():
    t = toks('f"oi {x}"')[0]
    assert t.type == "FSTRING"
    assert t.value == "oi {x}"


def test_raw_string_double_quote_preserves_backslashes():
    t = toks(r'r"C:\Users\test"')[0]
    assert t.type == "STR"
    assert t.value == r"C:\Users\test"


def test_raw_string_single_quote_preserves_backslashes():
    t = toks(r"r'C:\a\b'")[0]
    assert t.value == r"C:\a\b"


def test_raw_string_unterminated_at_newline_raises():
    with pytest.raises(PoolSyntaxError):
        toks('r"abc\n')


def test_identifier_named_f_or_r_without_adjacent_quote_is_ident():
    # "f" e "r" isolados (sem aspas coladas) continuam identificadores normais.
    assert toks("f = 1")[0].type == "IDENT"
    assert toks("r = 2")[0].type == "IDENT"


# ═══════════════════════════ strings multi-linha '''...''' ══════════════════
# Aspas duplas triplas (""") já são comentário de bloco — string multi-linha
# usa aspas simples triplas pra não colidir com isso.

def test_triple_single_quote_string_spans_lines():
    t = toks("'''linha 1\nlinha 2'''")[0]
    assert t.type == "STR"
    assert t.value == "linha 1\nlinha 2"


def test_triple_single_quote_fstring_tokenizes_as_fstring():
    t = toks("f'''oi {x}\nsegunda'''")[0]
    assert t.type == "FSTRING"
    assert t.value == "oi {x}\nsegunda"


def test_triple_single_quote_raw_string_preserves_backslashes():
    t = toks("r'''C:\\Users\\test\nsem escape'''")[0]
    assert t.type == "STR"
    assert t.value == "C:\\Users\\test\nsem escape"


def test_triple_single_quote_string_processes_escapes_when_not_raw():
    t = toks("'''a\\nb'''")[0]
    assert t.value == "a\nb"


def test_triple_double_quote_remains_block_comment_not_string():
    # não regride: aspas duplas triplas continuam sendo comentário, não string
    tk = types('"""comentario"""\nx')
    assert "STR" not in tk
    assert tk.count("IDENT") == 1


def test_unterminated_triple_string_raises():
    with pytest.raises(PoolSyntaxError):
        toks("'''abc sem fechar")


def test_line_tracking_correct_after_multiline_string():
    # depois de consumir uma string de N linhas, o token seguinte precisa
    # reportar o número de linha certo (regressão: contagem manual de \n
    # dentro de _read_triple_string tem que bater com o resto do lexer)
    tokens = toks("x = '''a\nb\nc'''\ny = 1")
    y_tok = next(t for t in tokens if t.type == "IDENT" and t.value == "y")
    assert y_tok.line == 4


# ═══════════════════════════ cores ══════════════════════════════════════════

def test_hex_color_6_digits():
    t = toks('<2196f3>"x"')[0]
    assert t.type == "COLOR" and t.value == "2196f3"


def test_hex_color_3_digits():
    t = toks('<fff>"x"')[0]
    assert t.type == "COLOR" and t.value == "fff"


def test_named_color_all_names_recognized():
    for name in NAMED_COLORS:
        t = toks(f'<{name}>"x"')[0]
        assert t.type == "COLOR", f"cor {name!r} não reconhecida"


def test_invalid_color_length_falls_back_to_operator():
    # <12345> não é hex válido (nem 3 nem 6 dígitos) nem nome conhecido —
    # deve tratar "<" como operador normal, não como início de cor.
    t = toks('<12345>"x"')[0]
    assert t.type == "OP" and t.value == "<"


def test_unknown_color_name_falls_back_to_operator():
    t = toks('<naoexiste>"x"')[0]
    assert t.type == "OP" and t.value == "<"


def test_less_than_operator_still_works_normally():
    v = values("a < b")
    assert v[1] == "<"


# ═══════════════════════════ comentários ════════════════════════════════════

def test_line_comment_double_slash_skipped():
    tk = types("x = 1 // comentario\ny = 2")
    assert "STR" not in tk
    assert tk.count("IDENT") == 2


def test_line_comment_hash_skipped():
    tk = types("x = 1 # comentario\ny = 2")
    assert tk.count("IDENT") == 2


def test_block_comment_skipped_multiline():
    tk = types('"""\nvarias\nlinhas\n"""\nx = 1')
    assert tk[:3] == ["IDENT", "OP", "INT"]


def test_block_comment_unterminated_raises():
    with pytest.raises(PoolSyntaxError):
        toks('"""\nsem fechar')


# ═══════════════════════════ indentação ═════════════════════════════════════

def test_indent_and_dedent_emitted_for_colon_block():
    all_types = [t.type for t in toks("if x:\n    post(1)\npost(2)")]
    assert "INDENT" in all_types
    assert "DEDENT" in all_types


def test_indent_exact_4_spaces_ok():
    Lexer("if x:\n    post(1)\n").tokenize()  # não deve levantar


def test_indent_tab_rejected():
    with pytest.raises(PoolSyntaxError):
        Lexer("if x:\n\tpost(1)\n").tokenize()


def test_indent_2_spaces_rejected():
    with pytest.raises(PoolSyntaxError):
        Lexer("if x:\n  post(1)\n").tokenize()


def test_indent_3_spaces_rejected():
    with pytest.raises(PoolSyntaxError):
        Lexer("if x:\n   post(1)\n").tokenize()


def test_indent_8_spaces_in_one_jump_rejected():
    with pytest.raises(PoolSyntaxError):
        Lexer("if x:\n        post(1)\n").tokenize()


def test_indent_nested_two_levels_ok():
    src = "if a:\n    if b:\n        post(1)\n"
    all_types = [t.type for t in Lexer(src).tokenize()]
    assert all_types.count("INDENT") == 2
    assert all_types.count("DEDENT") == 2


def test_indent_inconsistent_dedent_rejected():
    # volta pra uma coluna que não bate com nenhum nível da pilha
    src = "if a:\n    if b:\n        post(1)\n      post(2)\n"
    with pytest.raises(PoolSyntaxError):
        Lexer(src).tokenize()


def test_indent_closes_all_pending_at_eof():
    src = "if a:\n    if b:\n        post(1)\n"
    all_types = [t.type for t in Lexer(src).tokenize()]
    # 2 INDENT abertos precisam fechar com 2 DEDENT antes do EOF
    assert all_types[-3:] == ["DEDENT", "DEDENT", "EOF"]


def test_blank_line_does_not_affect_indent_stack():
    src = "if a:\n    post(1)\n\n    post(2)\n"
    Lexer(src).tokenize()  # não deve levantar por causa da linha em branco


def test_comment_only_line_does_not_affect_indent_stack():
    src = "if a:\n    post(1)\n    // comentario\n    post(2)\n"
    Lexer(src).tokenize()  # não deve levantar


def test_parens_suppress_indent_tracking_for_multiline_calls():
    # Dentro de (), quebras de linha não geram NEWLINE nem INDENT/DEDENT —
    # permite chamadas de função multi-linha.
    src = "foo(\n    1,\n    2,\n)\n"
    all_types = [t.type for t in Lexer(src).tokenize()]
    assert "INDENT" not in all_types
    assert "DEDENT" not in all_types


def test_brace_block_disables_indent_tracking():
    # Dentro de {} não há checagem de indentação — qualquer recuo é aceito.
    src = "if a { post(1)\n      post(2)\n  post(3) }\n"
    Lexer(src).tokenize()  # não deve levantar mesmo com recuos irregulares


def test_mixed_style_sets_both_flags():
    lx = Lexer("if a { post(1) }\nif b:\n    post(2)\n")
    lx.tokenize()
    assert lx.saw_brace_block is True
    assert lx.saw_colon_block is True


def test_pure_brace_only_sets_brace_flag():
    lx = Lexer('if a { post(1) }\n')
    lx.tokenize()
    assert lx.saw_brace_block is True
    assert lx.saw_colon_block is False


# ═══════════════════════════ caracteres inválidos ═══════════════════════════

def test_unexpected_character_raises():
    with pytest.raises(PoolSyntaxError):
        toks("x = $")


def test_error_message_includes_line_and_col():
    try:
        toks("x = $")
    except PoolSyntaxError as e:
        assert e.line == 1
        assert e.col >= 5


# ── continuação com ponto inicial (method chaining multi-linha em colon) ──────
# `obj()` seguido de `.metodo()` em linha nova (modo colon) deve continuar a
# expressão, não virar erro. Antes só funcionava dentro de chaves {} (onde a
# indentação é ignorada); no modo `:` o NEWLINE/INDENT quebrava a cadeia.

def test_leading_dot_continuation_suppresses_newline_colon_mode():
    # usa toks() (inclui NEWLINE) — types() filtra NEWLINE
    src = "x = obj()\n    .a()\n    .b()\n"
    tt = [t.type for t in toks(src)]
    joined = " ".join(tt)
    # entre o RPAREN de obj() e o DOT de .a() NÃO pode haver NEWLINE/INDENT
    assert "RPAREN DOT" in joined, f"esperava continuacao (RPAREN DOT), veio: {joined}"


def test_leading_dot_float_not_treated_as_continuation():
    # `.5` no inicio de linha NAO e continuacao (ponto seguido de digito) —
    # deve haver NEWLINE antes (fluxo normal)
    src = "x = 1\n.5\n"
    tt = [t.type for t in toks(src)]
    assert "NEWLINE" in tt
