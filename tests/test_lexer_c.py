"""Lexer em C — precisa produzir exatamente os mesmos tokens que o de Python.

Nenhuma expectativa é escrita à mão: cada caso compara token a token
(tipo, valor, linha, coluna) contra `poolscript.lexer.Lexer`. Enquanto os
dois coexistirem, esta suíte é o contrato entre eles.

Pula se a extensão não estiver compilada
(`python setup_vm.py build_ext --inplace`).
"""
import pathlib

import pytest

from poolscript.lexer import Lexer

lexer_c = pytest.importorskip(
    "poolscript.vm.ps_lexer_c",
    reason="extensão C não compilada — rode: python setup_vm.py build_ext --inplace",
)


def por_python(src):
    return [(t.type, t.value, t.line, t.col) for t in Lexer(src, filename="<test>").tokenize()]


def por_c(src):
    return [(t[0], t[1], t[2], t[3]) for t in lexer_c.tokenize(src)]


def mesmo(src):
    assert por_c(src) == por_python(src)


# ── literais ────────────────────────────────────────────────────────────────
@pytest.mark.parametrize("src", [
    "x = 42",
    "y = 3.14",
    "z = 0",
    "n = 1234567890",
])
def test_numeros(src):
    mesmo(src)


@pytest.mark.parametrize("src", [
    'a = "oi"',
    "b = 'tchau'",
    'c = ""',
    'd = "linha\\nnova\\ttab"',
    'e = "aspas \\" dentro"',
])
def test_strings(src):
    mesmo(src)


def test_fstring():
    mesmo('nome = "p"\ns = f"ola {nome}"')


def test_raw_string():
    mesmo('r = r"sem\\escape"')


def test_string_tripla():
    mesmo("s = '''multi\nlinha'''")


@pytest.mark.parametrize("src", [
    "a = True", "a = true", "a = False", "a = false",
    "a = Null", "a = null", "a = None", "a = none",
])
def test_bool_e_null(src):
    mesmo(src)


# ── identificadores e keywords ──────────────────────────────────────────────
def test_keywords():
    mesmo("if (x == 1) {\n post(1)\n}")


def test_ident_upper_vs_lower():
    mesmo("Entity Foo() {\n action m(self) { return 1 }\n}")


# ── operadores ──────────────────────────────────────────────────────────────
def test_aritmeticos():
    mesmo("a = 1 + 2 * 3 - 4 / 5 % 6")


def test_comparacao():
    mesmo("b = a == 1\nc = a === 1\nd = a != 2\ne = a !== 2\nf = a <= 1\ng = a >= 1")


def test_bitwise():
    mesmo("a = 5 ^ 3 | 2 & 1\nb = 1 << 4 >> 2\nc = ~5")


def test_compostos_e_incremento():
    mesmo("x = 1\nx += 1\nx -= 1\nx *= 2\nx /= 2\nx %= 3\nx++\nx--")


# ── indentação (a parte mais delicada do porte) ─────────────────────────────
def test_indent_simples():
    mesmo("if (x):\n    post(1)\npost(2)")


def test_indent_aninhado():
    mesmo("if (x):\n    post(1)\n    if (y):\n        post(2)\npost(3)")


def test_indent_dentro_de_chaves_e_ignorado():
    mesmo("if (x) {\n post(1)\n}")


def test_indent_ignorado_dentro_de_parenteses():
    mesmo("f(\n  1,\n  2\n)")


def test_tab_e_erro_nos_dois():
    src = "if (x):\n\tpost(1)"
    with pytest.raises(Exception):
        por_python(src)
    with pytest.raises(Exception):
        por_c(src)


def test_indent_nao_multiplo_de_4_e_erro_nos_dois():
    src = "if (x):\n  post(1)"
    with pytest.raises(Exception):
        por_python(src)
    with pytest.raises(Exception):
        por_c(src)


def test_continuacao_com_ponto():
    """`.membro` na linha seguinte é continuação — não emite NEWLINE."""
    mesmo("obj = f()\nres = obj\n    .json()\n    .status()")


# ── comentários ─────────────────────────────────────────────────────────────
def test_comentarios():
    mesmo('// linha\n# outra\nx = 1\n"""\nbloco\n"""\ny = 2')


def test_comentario_bloco_nao_e_string():
    mesmo('"""isto e comentario"""\nx = 1')


# ── cores ───────────────────────────────────────────────────────────────────
def test_cor_hex_6():
    mesmo('post(<FF0000>"vermelho")')


def test_cor_hex_3():
    mesmo('post(<F00>"vermelho")')


def test_cor_nome():
    mesmo('post(<red>"vermelho")')


def test_hex_invalido_nao_vira_cor():
    """4 dígitos não é formato de cor — `<` volta a ser operador."""
    mesmo("a = 1\nb = a <FFFF> 2")


# ── UTF-8: coluna conta CARACTERE, não byte ─────────────────────────────────
def test_coluna_com_acento():
    """Regressão: `°` ocupa 2 bytes e 1 caractere.

    Contar bytes fazia a coluna derivar em toda linha com acento — pego pelo
    teste diferencial nos exemplos do repo, não por inspeção.
    """
    mesmo('post("Clima: " {grau} "°")')


def test_coluna_com_varios_acentos():
    mesmo('post("ação e coração")\nx = 1')


def test_string_utf8_preservada():
    assert por_c('a = "ação"')[2][1] == "ação"


# ── casos de borda ──────────────────────────────────────────────────────────
@pytest.mark.parametrize("src", ["", "// nada", "\n\n\n", "   "])
def test_vazio_ou_so_comentario(src):
    mesmo(src)


def test_listas_e_dicts():
    mesmo('l = [1, 2, "tres"]\nd = {"a": 1}')


def test_decorator():
    mesmo('@app.route("/x")\naction h() { return 1 }')


# ── todos os .ps reais do repositório ───────────────────────────────────────
RAIZ = pathlib.Path(__file__).resolve().parent.parent
ARQUIVOS = sorted((RAIZ / "examples").rglob("*.ps")) + sorted((RAIZ / "tests").rglob("*.ps"))


@pytest.mark.parametrize("arquivo", ARQUIVOS, ids=lambda p: p.name)
def test_arquivos_reais_do_repo(arquivo):
    mesmo(arquivo.read_text())
