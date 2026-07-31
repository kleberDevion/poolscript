"""Parser em C — precisa produzir a mesma AST que o parser Python.

A comparação é feita sobre uma S-expression gerada pelos dois lados
(`ast_sexp.py` no Python, `serializa()` no C). Qualquer diferença de
estrutura, ordem de filhos ou associatividade aparece como diff textual.

Nós ainda fora do subconjunto do parser em C (ImportStmt, TryCatchStmt,
UsingStmt, TypeName, InterpolatedString) fazem o lado Python levantar
`NaoCoberto`, e o caso é pulado — a suíte nunca "passa" comparando duas
coisas que não foram implementadas.
"""
import pathlib

import pytest

from poolscript.parser import parse_source

from ast_sexp import NaoCoberto, sexp

parser_c = pytest.importorskip(
    "poolscript.vm.ps_parser_c",
    reason="extensão C não compilada — rode: python setup_vm.py build_ext --inplace",
)


def mesmo(src):
    esperado = sexp(parse_source(src, "<test>"))
    assert parser_c.parse_sexp(src) == esperado


# ── expressões e precedência ────────────────────────────────────────────────
@pytest.mark.parametrize("src", [
    "x = 1",
    "x = 2 + 3 * 4",
    "x = (2 + 3) * 4",
    "x = 10 - 3 - 2",          # associatividade à esquerda
    "x = 2 * 3 / 4 % 5",
    "x = 0 - 5",
    "x = ~5",
])
def test_aritmetica(src):
    mesmo(src)


@pytest.mark.parametrize("src", [
    "x = 5 ^ 3 | 2 & 1",       # | mais fraco que ^ que é mais fraco que &
    "x = 2 + 3 << 1",          # shift mais fraco que soma
    "x = 1 << 2 & 6",
    "x = 8 & 4 ^ 2 | 1",
])
def test_precedencia_bitwise(src):
    mesmo(src)


@pytest.mark.parametrize("src", [
    "x = a == 1", "x = a != 1", "x = a === 1", "x = a !== 1",
    "x = a < 1", "x = a > 1", "x = a <= 1", "x = a >= 1",
])
def test_comparacao(src):
    mesmo(src)


@pytest.mark.parametrize("src", [
    "x = a and b or c",
    "x = not a",
    "x = a and not b",
])
def test_logicos(src):
    mesmo(src)


# ── literais ────────────────────────────────────────────────────────────────
@pytest.mark.parametrize("src", [
    'x = "oi"', "x = 3.14", "x = True", "x = False", "x = Null",
    'x = [1, 2, "tres"]', "x = []",
    'x = {"a": 1}', "x = {}",
    "x = {nome: 1}",           # chave sem aspas continua Name, não string
])
def test_literais(src):
    mesmo(src)


def test_fstring_continua_literal():
    """f-string é Literal com kind FSTRING — interpolação é em runtime."""
    mesmo('x = f"Ola, {nome}!"')


# ── acesso ──────────────────────────────────────────────────────────────────
@pytest.mark.parametrize("src", [
    "post(a.b.c)",
    "post(l[0])",
    "post(l[0][1])",
    "post(a.b[0].c)",
])
def test_posfixo(src):
    mesmo(src)


@pytest.mark.parametrize("src", [
    "post(1, 2)",
    "post()",
    'f(base="x")',             # kwarg
    'f(a, base="x")',
    "f(g(h(1)))",
])
def test_chamadas(src):
    mesmo(src)


# ── statements ──────────────────────────────────────────────────────────────
@pytest.mark.parametrize("src", [
    "int x = 10", "str s = \"a\"", "flo f = 1.5", "bool b = True",
])
def test_declaracao_tipada(src):
    mesmo(src)


@pytest.mark.parametrize("op", ["=", "+=", "-=", "*=", "/=", "%="])
def test_atribuicao(op):
    mesmo(f"x {op} 1")


def test_if_chaves():
    mesmo("if (x) {\n post(1)\n}")


def test_if_elif_else():
    mesmo("if (x) {\n post(1)\n} elif (y) {\n post(2)\n} else {\n post(3)\n}")


def test_if_colon():
    mesmo("if (x):\n    post(1)")


def test_while():
    mesmo("while (x) {\n post(1)\n}")


def test_for_each():
    mesmo("for each i in l {\n post(i)\n}")


def test_action():
    mesmo("action f(a, b) {\n return a + b\n}")


def test_action_self():
    mesmo("action m(self) {\n return 1\n}")


def test_action_com_default():
    mesmo("action f(a, b=1, c=2) {\n return a\n}")


def test_return_vazio():
    mesmo("action f() {\n return\n}")


def test_recursao():
    mesmo("action f(n) {\n if (n < 2) {\n  return n\n }\n return f(n-1) + f(n-2)\n}")


def test_blocos_aninhados():
    mesmo("action f() {\n if (a) {\n  while (b) {\n   post(1)\n  }\n }\n}")


# ── palavras reservadas: mesma regra do parser Python ───────────────────────
@pytest.mark.parametrize("src", [
    "base = 5",
    "int base = 5",
    "action f(base) {\n return 1\n}",
    "for each base in l {\n post(1)\n}",
    "action base() {\n return 1\n}",
    "if = 5",
    "while = 5",
    "action f(if) {\n return 1\n}",
])
def test_reservada_recusada_nos_dois(src):
    with pytest.raises(Exception):
        parse_source(src, "<test>")
    with pytest.raises(SyntaxError):
        parser_c.parse_sexp(src)


def test_base_como_kwarg_e_membro_continua_valido():
    mesmo('f(base="x")')
    mesmo("post(o.base)")


# ── AST autocontida (regressão de use-after-free) ───────────────────────────
def test_ast_sobrevive_aos_tokens():
    """Os nós precisam ter cópia própria dos textos, não ponteiro pro token.

    Os tokens são liberados assim que o parse termina. Na primeira versão os
    nós apontavam pra dentro deles, e a serialização saía com lixo binário
    (`(Assignment .\\x98\\x87\\x13 ...`) — use-after-free. Este teste falha
    com qualquer regressão nesse ponto, porque nomes longos e repetidos
    tornam a corrupção visível.
    """
    src = "variavel_com_nome_bem_longo_para_detectar_lixo = 1\n" \
          "outra_variavel_igualmente_longa = variavel_com_nome_bem_longo_para_detectar_lixo"
    s = parser_c.parse_sexp(src)
    assert "variavel_com_nome_bem_longo_para_detectar_lixo" in s
    assert s == sexp(parse_source(src, "<test>"))


# ── lote 1: statements e expressões simples ────────────────────────────────
@pytest.mark.parametrize("src", [
    "while (x) {\n break\n}",
    "while (x) {\n continue\n}",
    "raise 1",
    "action g() {\n yield 1\n}",
    "action g() {\n yield\n}",
    "action f() {\n global x\n}",
    "action f() {\n global x, y\n}",
    'run_selfwith_("main") {\n post(1)\n}',
])
def test_statements_simples(src):
    mesmo(src)


@pytest.mark.parametrize("src", [
    "x = l[1:3]", "x = l[:2]", "x = l[1:]",
    "x = l[::-1]", "x = l[::2]", "x = l[1:5:2]",
    "x = l[0]",            # índice continua IndexAccess, não SliceAccess
])
def test_slice_e_indice(src):
    mesmo(src)


@pytest.mark.parametrize("src", ["x = 1\nx++", "x = 1\nx--"])
def test_postfix(src):
    mesmo(src)


@pytest.mark.parametrize("src", [
    "x = a is int", "x = a is str", "x = a is flo", "x = a is bool",
])
def test_typename(src):
    mesmo(src)


@pytest.mark.parametrize("src", ["x = int(y)", "x = json.parse(y)"])
def test_tipo_com_paren_ou_ponto_nao_e_typename(src):
    """`int(x)` é chamada e `json.parse` é módulo — os dois viram Name."""
    mesmo(src)


@pytest.mark.parametrize("src", ['post(<FF0000>"vermelho")', 'post(<red>"txt")'])
def test_color(src):
    mesmo(src)


def test_lambda():
    mesmo("f = action(a, b) { return a + b }")


def test_await():
    mesmo("action f() {\n x = await g()\n}")


# ── lote 2: try/catch/finally e using ──────────────────────────────────────
@pytest.mark.parametrize("src", [
    "try {\n post(1)\n} catch (e) {\n post(2)\n}",
    "try {\n post(1)\n} catch (KeyError e) {\n post(2)\n}",
    "try {\n post(1)\n} catch (KeyError e) {\n post(2)\n} catch (e) {\n post(3)\n}",
    "try {\n post(1)\n} catch (e) {\n post(2)\n} finally {\n post(3)\n}",
    "try:\n    post(1)\ncatch (e):\n    post(2)",
])
def test_try_catch(src):
    mesmo(src)


@pytest.mark.parametrize("src", [
    'using open("x") as f {\n post(1)\n}',
    'using open("x") as f:\n    post(1)',
    'using mp.open(target="a.xlsx") as arq {\n post(1)\n}',
])
def test_using(src):
    mesmo(src)


# ── lote 3: match/case ─────────────────────────────────────────────────────
@pytest.mark.parametrize("src", [
    'match x {\n case 1 { post("um") }\n case _ { post("outro") }\n}',
    'match x:\n    case 1:\n        post("um")\n    case _:\n        post("outro")',
    'match x {\n case 1 | 2 | 3 { post("a") }\n}',
    'match s {\n case "abc" { post(1) }\n}',
    'match x {\n case v { post(v) }\n}',
    'match x {\n case -10 { post(1) }\n}',
    'match x {\n case [1, 2] { post(1) }\n}',
    'match x {\n case [1, [2, 3]] { post(1) }\n}',
    'match x {\n case {a: 1} { post(1) }\n}',
    'match x {\n case v if v > 5 { post(1) }\n}',
    'match x {\n case True { post(1) }\n case Null { post(2) }\n}',
])
def test_match(src):
    mesmo(src)


# ── actions tipadas e assíncronas ──────────────────────────────────────────
@pytest.mark.parametrize("src", [
    "action f() { return 1 }", "reaction f() { return 1 }",
    "int action f() { return 1 }", "bool action f() { return 1 }",
    "str action f() { return 1 }", "flo action f() { return 1 }",
    "int reaction f() { return 1 }", "bool reaction f() { return 1 }",
    "async action f() { return 1 }", "async reaction f() { return 1 }",
    # async + tipo de retorno juntos: sem tratamento próprio, `async` era
    # lido como expressão solta e a action perdia a marca de assíncrona
    "async int action f() { return 1 }", "async bool reaction f() { return 1 }",
    "async str action f() { return 1 }", "async flo reaction f() { return 1 }",
])
def test_action_tipada_e_async(src):
    mesmo(src)


# ── lote 4: model e count ──────────────────────────────────────────────────
@pytest.mark.parametrize("src", [
    'model Usuario() {\n nome: str(length=60)\n idade: int(length=3)\n ativo: bool\n}',
    'model M() {\n x: flo\n}',
])
def test_model(src):
    mesmo(src)


@pytest.mark.parametrize("src", [
    "post(count int(7) in nums)",      # prefixo com valor
    "post(count int in nums)",         # prefixo só com tipo
    "post(count int() in nums)",       # parênteses vazios = só tipo
    'post(count str("oi") in frase)',
    "post(count char in s)",           # `char` só existe no count
    "count each int(7) in nums {\n post(1)\n}",
    "count each int(7) in nums:\n    post(1)",
    "x = count each int(7) in nums",   # sem bloco => expressão
    "if (count int(1) in a) {\n post(1)\n}",
])
def test_count(src):
    mesmo(src)


# ── lote 5: imports (3 sintaxes) e decorators ──────────────────────────────
@pytest.mark.parametrize("src", [
    "import os", "import a.b.c", "import os as sistema",
    "from json import parse", "from os import getenv, cwd",
    "from json import parse as p", "from a.b import c",
    "from .mod import x", "from ..mod import x", "from . import x",
    "PUSH os", "PUSH os GET getenv", "PUSH os as sistema GET getenv, cwd",
])
def test_import(src):
    mesmo(src)


@pytest.mark.parametrize("src", [
    '@app\naction h() { return 1 }',
    '@app.route("/x")\naction h() { return 1 }',
    '@app.route("/x", methods=["GET"])\naction h() { return 1 }',
    '@server.route("/api") {\n action handler() { return 1 }\n}',
    '@NonNull\naction f(x) { return x }',
    '@static\naction f() { return 1 }',
    '@NonNull\nint action f() { return 1 }',
    '@NonNull\nasync action f() { return 1 }',
])
def test_decorator(src):
    mesmo(src)


def test_apenas_str_int_flo_bool_declaram_variavel():
    """`list`/`json` NÃO declaram: viram TypeName solto + Assignment.

    Regressão: eu aceitava os seis tipos em declaração, o que produzia
    `(VarDecl list nums ...)` onde a referência produz
    `(TypeName list)` + `(Assignment nums ...)`.
    """
    for t in ("str", "int", "flo", "bool"):
        mesmo(f"{t} x = 1")
    for t in ("list", "json"):
        mesmo(f"{t} x = 1")


# ── lote 6: Entity, base(), atribuição de membro, unpacking ────────────────
@pytest.mark.parametrize("src", [
    "Entity Pessoa() {\n action __init__(self, nome) {\n  self.nome = nome\n }\n}",
    "Entity Filho(Pai) {\n action m(self) { return 1 }\n}",
    "Entity F(A, B) {\n action m(self) { return 1 }\n}",
    "Entity P() {\n nome: str\n idade: int = 0\n}",
    "Entity P():\n    action m(self):\n        return 1",
    "class P() {\n action m(self) { return 1 }\n}",
])
def test_entity(src):
    mesmo(src)


def test_decorator_dentro_de_entity_nao_captura_action():
    """Dentro de Entity o decorador e a action são entradas SEPARADAS.

    Fora de Entity, `@NonNull action f()` captura a action como bloco. É o
    `capture_action=False` do parser Python — sem isso a AST do corpo da
    Entity fica com um nó a menos.
    """
    mesmo("Entity P() {\n @static\n action m() { return 1 }\n}")
    mesmo("Entity C() {\n @NonNull\n action s(self, v) { return v }\n}")
    mesmo('@NonNull\naction f(x) { return x }')      # fora: captura


@pytest.mark.parametrize("src", [
    "Entity F(P) {\n action __init__(self, v) {\n  base(v)\n }\n}",
    "Entity F(P) {\n action __init__(self) {\n  base()\n }\n}",
])
def test_base_call(src):
    mesmo(src)


@pytest.mark.parametrize("src", [
    "action f(self) {\n self.x = 1\n}",
    "action f(o) {\n o.a.b = 1\n}",
])
def test_member_assignment(src):
    mesmo(src)


@pytest.mark.parametrize("src", [
    "a, b = 1, 2", "a, b = b, a", "a, b = l",
    "a, *resto = l", "a, *m, z = l", "*i, z = l",
    "(a, b), c = x", "a, (b, c) = x",
    "a, b, = l", "a, b, c = 1, 2, 3",
])
def test_unpacking(src):
    mesmo(src)


@pytest.mark.parametrize("src", [
    "f(a, b)",        # chamada
    "x = (a, b)",     # tupla-literal
    "o.a = 1",        # atribuição de membro
    "a = 1",          # atribuição simples
    "x = a == b",     # comparação
])
def test_lookahead_nao_confunde_com_unpacking(src):
    mesmo(src)


# ── `{` depois do iterável abre BLOCO, não interpola ───────────────────────
@pytest.mark.parametrize("src", [
    'for each c in "abc" {\n post(c)\n}',      # literal direto, estilo chaves
    'for each c in "abc":\n    post(c)',        # estilo colon já funcionava
    'for each i in [1,2,3] {\n post(i)\n}',
])
def test_for_each_com_literal_direto(src):
    """Antes exigia passar a string por variável.

    `"abc" {` era sempre lido como interpolação, então o bloco do laço
    sumia e o parse falhava. Agora o parser sabe que ali o `{` abre bloco.
    """
    mesmo(src)


@pytest.mark.parametrize("src", [
    'post("Clima: " {g} "C")',                  # interpolação normal segue igual
    'b = 1\nfor each x in ["a" {b}] {\n post(x)\n}',   # dentro de [] interpola
    'for each x in f("a" {b}) {\n post(x)\n}',          # dentro de () interpola
])
def test_interpolacao_continua_valendo_dentro_de_grupo(src):
    """A marca vale só no nível 0 — dentro de (), [] ou {} não há bloco."""
    mesmo(src)


# ── arquivos reais do repositório ───────────────────────────────────────────
RAIZ = pathlib.Path(__file__).resolve().parent.parent
ARQUIVOS = sorted((RAIZ / "examples").rglob("*.ps")) + sorted((RAIZ / "tests").rglob("*.ps"))


@pytest.mark.parametrize("arquivo", ARQUIVOS, ids=lambda p: p.name)
def test_arquivos_reais_do_repo(arquivo):
    src = arquivo.read_text()
    try:
        esperado = sexp(parse_source(src, str(arquivo)))
    except NaoCoberto as e:
        pytest.skip(f"nó {e} ainda fora do subconjunto do parser em C")
    assert parser_c.parse_sexp(src) == esperado
