"""Desmonta o bytecode do compilador Python no mesmo formato do C.

Espelha `ps_compiler_bind.c`. É o que permite o teste diferencial da camada 3:
os dois compiladores produzem texto e o teste exige igualdade — opcode,
argumento, ordem das constantes e índice de local, tudo comparado.

Não é utilitário de produção; existe enquanto os dois compiladores coexistem.
"""
from __future__ import annotations

from poolscript.parser import parse_source
from poolscript.vm import opcodes as op
from poolscript.vm.compiler import compila
from poolscript.vm.flatten import achata, para_c


class NaoCompila(Exception):
    """O compilador Python não suporta o nó — caso fica fora da comparação."""


# opcodes cujo argumento indexa alguma tabela
_CONST = op.LOAD_CONST
_GLOBAIS = (op.LOAD_GLOBAL, op.STORE_GLOBAL)
_PROTO = op.MAKE_FUNCTION


def _const_txt(v) -> str:
    if v is None:
        return "null"
    if v is True:
        return "bool True"
    if v is False:
        return "bool False"
    if isinstance(v, int):
        return f"int {v}"
    if isinstance(v, float):
        return f"flo {v:.17g}"
    if isinstance(v, str):
        return f'str "{v}"'
    raise NaoCompila(f"constante {type(v).__name__}")


def desmonta(src: str) -> str:
    """Compila com o pipeline Python e devolve o texto canônico."""
    co, tabela = compila(parse_source(src, "<test>"))
    protos = para_c(achata(co))

    # tabela de globais: nome por índice (o dict vem nome -> índice)
    globais = [""] * (max(tabela.values()) + 1 if tabela else 0)
    for nome, i in tabela.items():
        globais[i] = nome

    linhas: list[str] = []
    for pi, (code, consts, nlocals, nparams, nome) in enumerate(protos):
        linhas.append(f"proto {pi} {nome} nlocals={nlocals} nparams={nparams}")
        for i in range(0, len(code), 2):
            o, arg = code[i], code[i + 1]
            linha = f"  {i} {op.NOMES.get(o, '?')} {arg}"
            if o == _CONST and 0 <= arg < len(consts):
                linha += " ; " + _const_txt(consts[arg])
            elif o in _GLOBAIS and 0 <= arg < len(globais):
                linha += f" ; {globais[arg]}"
            elif o == _PROTO and 0 <= arg < len(protos):
                linha += f" ; -> proto {arg} {protos[arg][4]}"
            linhas.append(linha)
    return "\n".join(linhas) + "\n"
