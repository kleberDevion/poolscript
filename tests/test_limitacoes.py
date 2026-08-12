"""Regressões de limitações da linguagem — código `.ps` que deveria funcionar
e não funcionava em lugar nenhum.

Separado dos outros arquivos de propósito. `test_pipeline_c.py` e companhia
cobrem a migração para C: comparam duas implementações da MESMA linguagem.
Aqui o assunto é outro — buraco na linguagem, que não aparece num diferencial
porque os dois lados estão igualmente errados.

Cada bloco corresponde a uma entrada de [`LIMITACOES.md`](../notas/LIMITACOES.md) e
checa as duas coisas: que o interpretador (a autoridade semântica) faz o certo,
e que a VM em C concorda com ele.
"""
import io
import os
import sys
from contextlib import redirect_stdout

import pytest

from poolscript.interpreter import Interpreter
from poolscript.parser import parse_source

vm = pytest.importorskip(
    "poolscript.vm.poolscript_vm",
    reason="extensão C não compilada — rode: ./rebuild_vm.sh",
)


def via_interpretador(src):
    i = Interpreter(source=src, filename="<test>")
    with redirect_stdout(io.StringIO()):
        i.run(parse_source(src, "<test>"))
    return i.output


def via_c(src):
    """`post` é nativo em C e escreve por printf — a captura é no fd."""
    sys.stdout.flush()
    r, w = os.pipe()
    original = os.dup(1)
    try:
        os.dup2(w, 1)
        os.close(w)
        try:
            vm.executa_fonte(src)
        finally:
            sys.stdout.flush()
            os.dup2(original, 1)
    finally:
        os.close(original)
    with os.fdopen(r, "rb") as f:
        return f.read().decode("utf-8").splitlines()


def roda(src, esperado):
    """Os dois motores têm que produzir exatamente `esperado`."""
    assert via_interpretador(src) == esperado
    assert via_c(src) == esperado


# ═══════════════ atribuição por índice: `l[0] = 9` ══════════════════════════
# Nunca existiu na linguagem. O opcode INDEX_SET estava implementado na VM mas
# nenhum caminho do compilador o emitia, porque o parser não produzia o nó.

@pytest.mark.parametrize("src,esperado", [
    ("l = [1,2,3]\nl[0] = 9\npost(l[0])",                    ["9"]),
    ("l = [1,2,3]\nl[0] = 9\npost(l[1])",                    ["2"]),
    ("l = [1,2,3]\nl[-1] = 8\npost(l[2])",                   ["8"]),
    ('d = {}\nd["a"] = 1\npost(d["a"])',                     ["1"]),
    ("d = {}\ni = 0\nd[i] = 7\npost(d[0])",                  ["7"]),
    ('d = {}\nk = "a"\nd[k] = 1\npost(d["a"])',              ["1"]),
    ('d = {"a": 1}\nd["a"] = 2\npost(d["a"])',               ["2"]),
])
def test_atribuicao_indexada(src, esperado):
    roda(src, esperado)


@pytest.mark.parametrize("src,esperado", [
    ("l = [1]\nl[0] += 5\npost(l[0])",                       ["6"]),
    ("l = [10]\nl[0] -= 3\npost(l[0])",                      ["7"]),
    ("l = [3]\nl[0] *= 4\npost(l[0])",                       ["12"]),
    ("l = [10]\nl[0] %= 3\npost(l[0])",                      ["1"]),
    ('d = {"a": 1}\nd["a"] += 2\npost(d["a"])',              ["3"]),
    ('l = ["a"]\nl[0] += "b"\npost(l[0])',                   ["ab"]),
])
def test_atribuicao_indexada_aumentada(src, esperado):
    """`l[i] += 1` lê e reescreve sem re-avaliar container e índice."""
    roda(src, esperado)


def test_aumentada_nao_reavalia_o_indice():
    """`l[f()] += 1` só pode chamar `f` uma vez — daí o opcode DUP2."""
    src = (
        "chamadas = 0\n"
        "action f() {\n"
        " chamadas = chamadas + 1\n"
        " return 0\n"
        "}\n"
        "l = [10]\n"
        "l[f()] += 5\n"
        "post(l[0])\n"
        "post(chamadas)\n"
    )
    roda(src, ["15", "1"])


@pytest.mark.parametrize("src,esperado", [
    # container é qualquer expressão, não só um nome
    ("l = [[1,2]]\nl[0][1] = 9\npost(l[0][1])",              ["9"]),
    ('d = {"x": [1,2]}\nd["x"][0] = 7\npost(d["x"][0])',     ["7"]),
])
def test_atribuicao_indexada_encadeada(src, esperado):
    roda(src, esperado)


def test_escrita_dentro_de_laco():
    src = (
        "l = [1,2,3]\n"
        "i = 0\n"
        "while i < 3 {\n"
        " l[i] = l[i] * 2\n"
        " i++\n"
        "}\n"
        "post(l[0])\npost(l[1])\npost(l[2])\n"
    )
    roda(src, ["2", "4", "6"])


def test_escrita_em_campo_de_entity():
    src = (
        "Entity P():\n"
        "    action __init__(self):\n"
        "        self.l = [1,2]\n"
        "p = P()\n"
        "p.l[0] = 9\n"
        "post(p.l[0])\n"
    )
    roda(src, ["9"])


@pytest.mark.parametrize("src", [
    "t = (1,2)\nt[0] = 9",          # tupla é imutável
    's = "ab"\ns[0] = "z"',         # string é imutável
    "l = [1]\nl[5] = 2",            # fora de faixa
])
def test_alvo_invalido_para_com_erro(src):
    """Os dois motores recusam — não escrevem em silêncio nem divergem."""
    with pytest.raises(Exception):
        via_interpretador(src)
    with pytest.raises(Exception):
        via_c(src)


# ═══════════════ `lista + lista` e `lista * int` na VM ══════════════════════
# Aqui o interpretador sempre esteve certo; era a VM que levantava
# "'+' entre tipos incompativeis".

@pytest.mark.parametrize("src,esperado", [
    ("l = [1,2] + [3]\npost(l[2])",                          ["3"]),
    ("l = [1,2] + [3]\npost(len(l))",                        ["3"]),
    ("l = [] + []\npost(len(l))",                            ["0"]),
    ("t = (1,2) + (3,)\npost(len(t))",                       ["3"]),
    ("t = (1,2) + (3,)\npost(t[2])",                         ["3"]),
])
def test_concatenacao_de_sequencia(src, esperado):
    roda(src, esperado)


@pytest.mark.parametrize("src,esperado", [
    ("l = [1,2] * 3\npost(len(l))",                          ["6"]),
    ("l = [1,2] * 3\npost(l[3])",                            ["2"]),
    ("l = 3 * [1,2]\npost(len(l))",                          ["6"]),
    ("l = [1] * 0\npost(len(l))",                            ["0"]),
    ("l = [1] * -2\npost(len(l))",                           ["0"]),
])
def test_repeticao_de_sequencia(src, esperado):
    roda(src, esperado)


def test_concatenacao_nao_muda_o_original():
    roda("l = [1]\nm = l + [2]\nl = l + [9]\npost(len(l))\npost(len(m))", ["2", "2"])


@pytest.mark.parametrize("src", [
    "l = [1] + (2,)",               # tipos diferentes não concatenam
    'd = {"a":1} + {"b":2}',        # dict não tem '+'
    "l = [1,2] - [1]",              # nem '-'
    'post("ab" * 2)',               # str * int é erro na PoolScript
])
def test_operacao_invalida_entre_sequencias(src):
    with pytest.raises(Exception):
        via_interpretador(src)
    with pytest.raises(Exception):
        via_c(src)
