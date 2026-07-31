"""VM de bytecode — precisa dar exatamente o mesmo resultado que o tree-walker.

O valor destes testes é a comparação: nenhum resultado é escrito à mão, todos
são conferidos contra o interpretador atual. Assim a VM não pode divergir da
semântica da linguagem sem que a suíte acuse.
"""
import io
import sys
from contextlib import redirect_stdout

import pytest

from poolscript.interpreter import Interpreter
from poolscript.parser import parse_source
from poolscript.vm import NaoSuportado, compila, roda


def _fmt(x):
    if x is None:
        return "null"
    if x is True:
        return "True"
    if x is False:
        return "False"
    return str(x)


def via_vm(src):
    co, tab = compila(parse_source(src, "<test>"))
    saida = []
    roda(co, tab, builtins={"post": lambda *a: saida.append(" ".join(_fmt(x) for x in a))})
    return saida


def via_interp(src):
    i = Interpreter(source=src, filename="<test>")
    with redirect_stdout(io.StringIO()):
        i.run(parse_source(src, "<test>"))
    return i.output


def mesmo(src):
    """VM e interpretador produzem a mesma saída."""
    assert via_vm(src) == via_interp(src)


# ── aritmética e precedência ────────────────────────────────────────────────
@pytest.mark.parametrize("src", [
    "post(2 + 3 * 4)",
    "post((2 + 3) * 4)",
    "post(10 - 3 - 2)",
    "post(7 / 2)",
    "post(7 % 3)",
    "post(0 - 5)",
    "post(((1 + 2) * (3 + 4)) - (5 * (6 - 4)))",
])
def test_aritmetica(src):
    mesmo(src)


# ── comparação e igualdade ──────────────────────────────────────────────────
@pytest.mark.parametrize("src", [
    "post(3 < 5)", "post(5 <= 5)", "post(3 > 5)", "post(5 >= 5)",
    "post(2 == 2)", "post(2 != 3)",
])
def test_comparacao(src):
    mesmo(src)


def test_null_igual_zero():
    # regra do spec: Null == 0 é True — a VM tem a própria _iguais()
    mesmo("post(Null == 0)")


# ── bitwise ─────────────────────────────────────────────────────────────────
@pytest.mark.parametrize("src", [
    "post(5 ^ 3)", "post(5 | 2)", "post(6 & 3)",
    "post(1 << 4)", "post(256 >> 4)", "post(~5)",
    "post(2 + 3 << 1)",
])
def test_bitwise(src):
    mesmo(src)


# ── variáveis ───────────────────────────────────────────────────────────────
def test_declaracao_e_uso():
    mesmo("int x = 10\npost(x)")


@pytest.mark.parametrize("op", ["+=", "-=", "*="])
def test_atribuicao_composta(op):
    mesmo(f"int x = 10\nx {op} 3\npost(x)")


# ── controle de fluxo ───────────────────────────────────────────────────────
def test_while():
    mesmo("int i = 0\nint s = 0\nwhile (i < 5) {\n s += i\n i += 1\n}\npost(s)")


@pytest.mark.parametrize("n", [15, 8, 3])
def test_if_elif_else(n):
    mesmo(f"int n = {n}\nif (n > 10) {{\n post(1)\n}} elif (n > 5) {{\n post(2)\n}} else {{\n post(3)\n}}")


# ── funções ─────────────────────────────────────────────────────────────────
def test_action_simples():
    mesmo("action dobro(n) {\n return n * 2\n}\npost(dobro(21))")


def test_action_sem_return_devolve_null():
    mesmo("action nada() {\n int x = 1\n}\npost(nada())")


def test_chamadas_aninhadas():
    mesmo("action d(n) {\n return n * 2\n}\npost(d(d(d(3))))")


def test_recursao():
    mesmo("action fib(n) {\n if (n < 2) {\n  return n\n }\n return fib(n-1) + fib(n-2)\n}\npost(fib(15))")


def test_locais_nao_vazam_entre_chamadas():
    # locais são slots por frame; uma chamada não pode ver a anterior
    mesmo("action f(a) {\n int b = a * 2\n return b\n}\npost(f(5))\npost(f(7))")


def test_parametros_multiplos():
    mesmo("action soma(a, b, c) {\n return a + b + c\n}\npost(soma(1, 2, 3))")


# ── erros claros em vez de compilação errada silenciosa ─────────────────────
def test_no_nao_suportado_levanta():
    # `using ... as` ainda não compila nesta etapa — precisa falhar explícito,
    # nunca gerar bytecode errado em silêncio.
    # (list/dict literais JÁ compilam desde que PSList/PSDict entraram.)
    with pytest.raises(NaoSuportado):
        compila(parse_source('using open("x") as f {\n post(1)\n}', "<test>"))


def test_kwarg_nao_suportado_levanta():
    with pytest.raises(NaoSuportado):
        compila(parse_source('f(base="x")', "<test>"))


def test_aridade_errada_e_erro():
    from poolscript.vm import ErroVM
    src = "action f(a, b) {\n return a\n}\npost(f(1))"
    co, tab = compila(parse_source(src, "<test>"))
    with pytest.raises(ErroVM):
        roda(co, tab, builtins={"post": lambda *a: None})
