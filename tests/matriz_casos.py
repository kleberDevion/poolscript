# -*- coding: utf-8 -*-
"""Gerador COMBINATÓRIO de programas `.ps` para o teste diferencial.

A ideia é não depender de alguém "lembrar" do caso: os programas nascem do
produto cartesiano dos eixos da linguagem (valores × operadores × métodos ×
posições de expressão × formas de erro). Cada programa gerado roda nos DOIS
motores e as saídas têm que ser IDÊNTICAS — inclusive a mensagem de erro.

Foi escrito depois de uma sequência de bugs que passaram porque o teste só
cobria o caminho feliz que alguém imaginou: `Classe.metodo()` sem `@static`
(a VM dropava o `self` calada), f-string com nome fora de escopo (imprimia
`{nome}` em vez de erro), tupla que a VM deixava mutar.

`casos()` devolve (id, fonte). Sem env var vem o conjunto REDUZIDO (rápido,
roda na suíte); com `MATRIZ_COMPLETA=1` vem tudo.
"""
from __future__ import annotations

import os
import re
from pathlib import Path

RAIZ = Path(__file__).resolve().parent.parent
VM_C = RAIZ / "vm" / "poolscript_vm.c"


# ── vocabulário da linguagem, lido do MOTOR (não de uma lista à mão) ────────

def metodos_da_tabela(nome: str) -> list[str]:
    src = VM_C.read_text(encoding="utf-8")
    m = re.search(r"static const MetodoNat " + nome + r"\[\] = \{(.*?)\n\};", src, re.S)
    if not m:
        return []
    return sorted(set(re.findall(r'\{\s*"([A-Za-z_]\w*)"\s*,', m.group(1))))


MET_STR = metodos_da_tabela("METODOS_STR")
MET_LIST = metodos_da_tabela("METODOS_LIST")
MET_DICT = metodos_da_tabela("METODOS_DICT")
MET_TUP = metodos_da_tabela("METODOS_TUPLA")

# valores de cada tipo — o mesmo literal usado em todos os eixos
VALORES = {
    "int": "7",
    "flo": "2.5",
    "str": '"ab"',
    "bool": "true",
    "null": "null",
    "list": "[1, 2]",
    "tup": "(1, 2)",
    "dict": '{ "a": 1 }',
    "bytes": 'bytes.new(2)',
}

OPERADORES = ["+", "-", "*", "/", "%", "==", "!=", "<", ">", "<=", ">=",
              "and", "or", "in", "is"]

# argumentos de teste por método — um válido e um inválido por aridade comum
ARGS = ['', '1', '"a"', '1, 2', '"a", 1', 'null', '[1]', '{ "a": 1 }']


def _id(*partes) -> str:
    txt = "-".join(str(p) for p in partes)
    return re.sub(r"[^A-Za-z0-9_.=<>+*/%-]+", "_", txt)[:110]


# ── eixos ───────────────────────────────────────────────────────────────────

def eixo_operadores():
    """Todo operador × todo par de tipos: resultado OU erro, igual nos dois."""
    for op in OPERADORES:
        for ta, va in VALORES.items():
            for tb, vb in VALORES.items():
                if va.startswith("bytes") or vb.startswith("bytes"):
                    src = f"import bytes\npost({va} {op} {vb})\n"
                else:
                    src = f"post({va} {op} {vb})\n"
                yield _id("op", op, ta, tb), src


def eixo_metodos(tipo: str, receptor: str, metodos: list[str]):
    """Todo método do tipo × cada lista de argumentos — inclusive as erradas."""
    for met in metodos:
        for args in ARGS:
            src = f"x = {receptor}\npost(x.{met}({args}))\n"
            yield _id("met", tipo, met, args or "vazio"), src


def eixo_indexacao():
    alvos = {"str": '"abcde"', "list": "[1, 2, 3]", "tup": "(1, 2, 3)",
             "dict": '{ "a": 1 }', "int": "7"}
    indices = ["0", "2", "-1", "-9", "9", '"a"', "null", "1.5"]
    for t, alvo in alvos.items():
        for i in indices:
            yield _id("idx", t, i), f"x = {alvo}\npost(x[{i}])\n"
    fatias = ["0:2", "1:", ":2", ":", "-2:", "0:9", "9:1", "::2", "::-1", "1:4:2"]
    for t, alvo in alvos.items():
        if t in ("dict", "int"):
            continue
        for f in fatias:
            yield _id("fatia", t, f), f"x = {alvo}\npost(x[{f}])\n"


def eixo_nome_indefinido():
    """O MESMO nome inexistente em cada posição de expressão — a mensagem tem
    que ser a mesma nos dois motores em TODAS elas."""
    posicoes = {
        "leitura": "post(zzz)",
        "chamada": "zzz()",
        "chamada_arg": "post(len(zzz))",
        "membro": "post(zzz.campo)",
        "metodo": "post(zzz.metodo())",
        "indice": "post(zzz[0])",
        "indice_de": 'post([1, 2][zzz])',
        "fstring": 'post(f"v: {zzz}")',
        "fstring_expr": 'post(f"v: {zzz + 1}")',
        "soma": "post(1 + zzz)",
        "condicao": "if zzz:\n    post(1)",
        "while": "while zzz:\n    break",
        "for": "for each i in zzz:\n    post(i)",
        "retorno": "action f():\n    return zzz\npost(f())",
        "atribuicao": "x = zzz",
        "arg_nomeado": "action f(a):\n    return a\npost(f(a=zzz))",
        "lista": "post([1, zzz])",
        "dict_valor": 'post({ "a": zzz })',
        "tupla": "post((1, zzz))",
        "not": "post(not zzz)",
        "in": "post(1 in zzz)",
        "index_set": "l = [1]\nl[0] = zzz",
        "raise": "raise ValueError(zzz)",
    }
    for nome, corpo in posicoes.items():
        yield _id("indef", nome), corpo + "\n"


def eixo_chamadas():
    base = "action f(a, b=2):\n    return a\n"
    formas = ["f()", "f(1)", "f(1, 2)", "f(1, 2, 3)", "f(a=1)", "f(b=1)",
              "f(a=1, b=2)", "f(1, b=2)", "f(1, a=2)", "f(c=1)", "f(1, c=2)"]
    for forma in formas:
        yield _id("call", forma), base + f"post({forma})\n"
    # chamar o que não é função
    for t, v in VALORES.items():
        if v.startswith("bytes"):
            continue
        yield _id("call_nao_fn", t), f"x = {v}\npost(x())\n"


def eixo_entity():
    corpo_metodos = {
        "normal": "    action m(self, a):\n        return a\n",
        "static_sem_self": "    @static\n    action m(a):\n        return a\n",
        "static_com_self": "    @static\n    action m(self, a):\n        return a\n",
    }
    usos = {
        "na_classe_sem_arg": "post(C.m())",
        "na_classe_com_arg": "post(C.m(5))",
        "na_instancia": "post(C().m(5))",
        "na_instancia_sem_arg": "post(C().m())",
        "membro_inexistente": "post(C.naoexiste())",
        "campo_inexistente": "post(C().campo)",
    }
    for nome_m, met in corpo_metodos.items():
        for nome_u, uso in usos.items():
            src = "class C():\n" + met + uso + "\n"
            yield _id("entity", nome_m, nome_u), src
    # self fora de lugar
    yield _id("entity", "self_topo"), "post(self)\n"
    yield _id("entity", "self_em_action"), "action f():\n    return self\npost(f())\n"
    yield _id("entity", "self_em_static"), (
        "class C():\n    @static\n    action m():\n        return self.x\npost(C.m())\n")


def eixo_fstring():
    trechos = ['{oi}', '{oi.upper()}', '{1 + 1}', '{zzz}', '{}', '{ }',
               '{{oi}}', '{oi} e {zzz}', '{[1,2][0]}', '{ "x" }',
               '{oi[0]}', '{oi[9]}', '{len(oi)}', '{naoexiste()}']
    for t in trechos:
        yield _id("fstr", t), 'oi = "ola"\npost(f"v: ' + t + '")\n'


def eixo_escopo():
    casos = {
        "bloco_if": "if true:\n    dentro = 5\npost(dentro)",
        "bloco_else": "if false:\n    x = 1\nelse:\n    x = 2\npost(x)",
        "for": "for each i in [1, 2]:\n    y = i\npost(y)",
        "for_var": "for each i in [1, 2]:\n    post(i)\npost(i)",
        "while": "n = 0\nwhile n < 1:\n    n = n + 1\n    z = 9\npost(z)",
        "action_le_global": "g = 1\naction f():\n    return g\npost(f())",
        "action_escreve_global": "g = 1\naction f():\n    g = 2\nf()\npost(g)",
        "aninhada": "action fora():\n    a = 1\n    action dentro():\n        return a\n    return dentro()\npost(fora())",
        "try_vaza": "try:\n    t = 1\ncatch (e):\n    t = 2\npost(t)",
        "catch_var_depois": "try:\n    raise ValueError(\"x\")\ncatch (e):\n    post(1)\npost(e)",
    }
    for nome, src in casos.items():
        yield _id("escopo", nome), src + "\n"


def eixo_erros():
    casos = {
        "div_zero_int": "post(1 / 0)",
        "div_zero_flo": "post(1.0 / 0)",
        "mod_zero": "post(1 % 0)",
        "raise_simples": 'raise ValueError("x")',
        "raise_capturado": 'try:\n    raise ValueError("x")\ncatch (e):\n    post(e)',
        "raise_tipo_certo": 'try:\n    raise ValueError("x")\ncatch (ValueError e):\n    post("pegou")',
        "raise_tipo_errado": 'try:\n    raise ValueError("x")\ncatch (KeyError e):\n    post("pegou")',
        "erro_dentro_action": 'action f():\n    return 1 / 0\npost(f())',
        "erro_2_niveis": 'action a():\n    return 1 / 0\naction b():\n    return a()\npost(b())',
        "reraise": 'try:\n    raise ValueError("x")\ncatch (e):\n    raise ValueError(f"Erro: {e}")',
        "chave_ausente": 'd = { "a": 1 }\npost(d["z"])',
        "campo_ausente": 'd = { "a": 1 }\npost(d.z)',
        "index_erro": "post([1, 2].index(9))",
        "int_de_texto": 'post(int("abc"))',
        "conversao_null": "post(int(null))",
    }
    for nome, src in casos.items():
        yield _id("erro", nome), src + "\n"


def eixo_builtins():
    nomes = ["len", "str", "int", "flo", "bool", "list", "type", "abs", "round",
             "min", "max", "sum", "sorted", "reversed", "range", "enumerate"]
    for b in nomes:
        for t, v in VALORES.items():
            if v.startswith("bytes"):
                continue
            yield _id("builtin", b, t), f"post({b}({v}))\n"


EIXOS = [
    ("operadores", eixo_operadores),
    ("met_str", lambda: eixo_metodos("str", '"ab"', MET_STR)),
    ("met_list", lambda: eixo_metodos("list", "[1, 2]", MET_LIST)),
    ("met_dict", lambda: eixo_metodos("dict", '{ "a": 1 }', MET_DICT)),
    ("met_tup", lambda: eixo_metodos("tup", "(1, 2)", MET_TUP)),
    ("indexacao", eixo_indexacao),
    ("indefinido", eixo_nome_indefinido),
    ("chamadas", eixo_chamadas),
    ("entity", eixo_entity),
    ("fstring", eixo_fstring),
    ("escopo", eixo_escopo),
    ("erros", eixo_erros),
    ("builtins", eixo_builtins),
]

# na suíte normal, 1 de cada N casos dos eixos gigantes (operadores/métodos);
# os eixos pequenos entram inteiros. MATRIZ_COMPLETA=1 roda tudo.
_GRANDES = {"operadores", "met_str", "met_list", "met_dict", "met_tup",
            "indexacao", "builtins"}
_PASSO = 7


def casos(completo: bool | None = None):
    """(id, fonte). O id é ÚNICO: dois trechos diferentes podem virar o mesmo
    nome depois de saneado (`{}` e `{ }`), e aí o pytest renomeia por conta
    própria e a lista de divergências conhecidas deixa de casar."""
    if completo is None:
        completo = os.environ.get("MATRIZ_COMPLETA") == "1"
    vistos: dict[str, int] = {}
    for nome_eixo, gen in EIXOS:
        for i, (cid, src) in enumerate(gen()):
            if not completo and nome_eixo in _GRANDES and i % _PASSO:
                continue
            pleno = f"{nome_eixo}:{cid}"
            vistos[pleno] = vistos.get(pleno, 0) + 1
            if vistos[pleno] > 1:
                pleno = f"{pleno}~{vistos[pleno]}"
            yield pleno, src
