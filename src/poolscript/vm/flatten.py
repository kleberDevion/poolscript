"""
Achata a árvore de CodeObj num formato consumível pela VM em C.

O compilador Python produz CodeObj aninhados: uma action vira uma constante
dentro do CodeObj pai, e `MAKE_FUNCTION` aponta pro índice dessa constante.
Em C não existe "constante que é um CodeObj" — o que existe é uma tabela
plana de protótipos, e `MAKE_FUNCTION` vira um índice nela.

Esta camada faz essa tradução: percorre a árvore, dá um id pra cada CodeObj,
e reescreve o argumento de cada `MAKE_FUNCTION` de "índice de constante" pra
"índice de protótipo".
"""
from __future__ import annotations

from typing import Any

from . import opcodes as op
from .compiler import CodeObj


class Proto:
    """Protótipo plano — espelha 1:1 a struct Proto do C."""

    __slots__ = ("name", "code", "consts", "nlocals", "nparams")

    def __init__(self, name: str, code: list[int], consts: list[Any],
                 nlocals: int, nparams: int) -> None:
        self.name = name
        self.code = code
        self.consts = consts
        self.nlocals = nlocals
        self.nparams = nparams

    def __repr__(self) -> str:
        return f"<Proto {self.name} {len(self.code)//2} instr>"


def achata(raiz: CodeObj) -> list[Proto]:
    """Devolve a tabela de protótipos; o índice 0 é sempre o módulo."""
    protos: list[Proto] = []
    ids: dict[int, int] = {}          # id(CodeObj) → índice do proto

    def registra(co: CodeObj) -> int:
        chave = id(co)
        if chave in ids:
            return ids[chave]
        idx = len(protos)
        ids[chave] = idx
        protos.append(Proto(co.name, [], [], co.nlocals, co.nparams))
        return idx

    def visita(co: CodeObj) -> None:
        idx = registra(co)
        code = list(co.code)
        consts: list[Any] = []

        for i in range(0, len(code), 2):
            if code[i] == op.MAKE_FUNCTION:
                filho = co.consts[code[i + 1]]
                if not isinstance(filho, CodeObj):
                    raise TypeError("MAKE_FUNCTION não aponta pra um CodeObj")
                ja_visto = id(filho) in ids
                code[i + 1] = registra(filho)
                if not ja_visto:
                    visita(filho)

        # Constante que era CodeObj SAI do pool (o argumento do MAKE_FUNCTION
        # já virou índice de protótipo, ninguém mais lê aquele slot). Deixá-la
        # como None deixaria um buraco morto e deslocaria todos os índices
        # seguintes — o que faz o bytecode divergir do compilador em C sem
        # nenhuma diferença real de comportamento.
        remap: dict[int, int] = {}
        for antigo, c in enumerate(co.consts):
            if isinstance(c, CodeObj):
                continue
            remap[antigo] = len(consts)
            consts.append(c)

        for i in range(0, len(code), 2):
            if code[i] == op.LOAD_CONST:
                code[i + 1] = remap[code[i + 1]]

        protos[idx].code = code
        protos[idx].consts = consts

    visita(raiz)
    return protos


def para_c(protos: list[Proto]) -> list[tuple]:
    """Forma de tupla que a extensão C recebe, sem precisar de atributos."""
    return [(p.code, p.consts, p.nlocals, p.nparams, p.name) for p in protos]
