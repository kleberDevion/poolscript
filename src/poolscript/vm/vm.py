"""
Loop de execução da VM da PoolScript.

Escrito para o mypyc — e, depois, para tradução direta em C:

  * bytecode é `list[int]` plano → vira `int32_t[]` em C sem mudança;
  * o despacho é if/elif ordenado por frequência medida (não dict de
    handlers): o mypyc gera comparações de inteiro, e em C isso vira
    `switch` com tabela de salto;
  * locais são array indexado, não dict — nenhum hash no caminho quente;
  * nada de exceção pra controle de fluxo (o tree-walker usa `ReturnSignal`,
    que custa caro): `RETURN` simplesmente sai do laço.

Semântica preservada do interpretador: `Null == 0` é `True`, `Null` é falsy,
e divisão é a do Python (`/` verdadeira).
"""
from __future__ import annotations

from typing import Any

from . import opcodes as op
from .compiler import CodeObj


class ErroVM(Exception):
    pass


class Funcao:
    """Função já compilada, pronta pra chamada."""

    __slots__ = ("co",)

    def __init__(self, co: CodeObj) -> None:
        self.co = co

    def __repr__(self) -> str:
        return f"<action {self.co.name}>"


def _truthy(v: Any) -> bool:
    if v is None:
        return False
    return bool(v)


def _iguais(a: Any, b: Any) -> bool:
    # spec da linguagem: Null == 0 → True
    if a is None:
        return b is None or b == 0
    if b is None:
        return a == 0
    return bool(a == b)


def executa(co: CodeObj, gvars: list[Any], args: list[Any]) -> Any:
    """Executa um CodeObj. `args` preenche os primeiros locais (parâmetros)."""
    code = co.code
    consts = co.consts

    locais: list[Any] = [None] * co.nlocals
    for i in range(len(args)):
        locais[i] = args[i]

    # Pilha PRÉ-ALOCADA com ponteiro manual (`sp`), em vez de append/pop.
    # Motivo medido: com append/pop, o loop de 300k iterações gastava 2,7
    # milhões de chamadas de método só em tráfego de pilha — mais caro que
    # todo o resto junto. Indexação direta não chama método nenhum, e é
    # exatamente a forma que o porte pra C vai ter (`Value stack[N]; int sp`).
    # Teto garantido: cada instrução empilha no máximo 1 valor, então o
    # número de instruções é um limite superior seguro pra profundidade.
    # (No porte pra C isso vira o tamanho fixo do frame, calculado pelo
    # compilador — aqui a cota grosseira já basta e nunca estoura.)
    pilha: list[Any] = [None] * (len(code) // 2 + 8)
    sp = 0

    ip = 0
    while True:
        o = code[ip]
        arg = code[ip + 1]
        ip += 2

        # ── quentes primeiro (ordem vinda do profile do fib) ──────────────
        if o == op.LOAD_LOCAL:
            pilha[sp] = locais[arg]; sp += 1
        elif o == op.LOAD_CONST:
            pilha[sp] = consts[arg]; sp += 1
        elif o == op.ADD:
            sp -= 1; pilha[sp - 1] = pilha[sp - 1] + pilha[sp]
        elif o == op.SUB:
            sp -= 1; pilha[sp - 1] = pilha[sp - 1] - pilha[sp]
        elif o == op.LT:
            sp -= 1; pilha[sp - 1] = pilha[sp - 1] < pilha[sp]
        elif o == op.CALL:
            n = arg
            sp -= n
            novos = pilha[sp:sp + n]
            sp -= 1
            chamavel = pilha[sp]
            if chamavel.__class__ is Funcao:
                alvo = chamavel.co
                if n != alvo.nparams:
                    raise ErroVM(
                        f"{alvo.name}() esperava {alvo.nparams} argumento(s), "
                        f"recebeu {n}"
                    )
                pilha[sp] = executa(alvo, gvars, novos)
            else:
                pilha[sp] = chamavel(*novos)
            sp += 1
        elif o == op.RETURN:
            return pilha[sp - 1]
        elif o == op.STORE_LOCAL:
            sp -= 1; locais[arg] = pilha[sp]
        elif o == op.JUMP_IF_FALSE:
            sp -= 1
            v = pilha[sp]
            # fast-path: comparação devolve bool, que é o caso dominante —
            # evita a chamada a _truthy() em todo teste de laço/if.
            if v is False or v is None:
                ip = arg
            elif v is not True and not v:
                ip = arg
        elif o == op.JUMP:
            ip = arg
        elif o == op.LOAD_GLOBAL:
            v = gvars[arg]
            if v is _NAO_DEF:
                raise ErroVM("variável global não definida")
            pilha[sp] = v; sp += 1
        elif o == op.STORE_GLOBAL:
            sp -= 1; gvars[arg] = pilha[sp]

        # ── resto da aritmética ──────────────────────────────────────────
        elif o == op.MUL:
            sp -= 1; pilha[sp - 1] = pilha[sp - 1] * pilha[sp]
        elif o == op.DIV:
            sp -= 1; pilha[sp - 1] = pilha[sp - 1] / pilha[sp]
        elif o == op.MOD:
            sp -= 1; pilha[sp - 1] = pilha[sp - 1] % pilha[sp]
        elif o == op.NEG:
            pilha[sp - 1] = -pilha[sp - 1]

        # Tipo declarado: converte o que a linguagem manda converter
        # (`flo x = 5`, `int x = "7"`) e recusa o resto.
        elif o == op.COERCE_DECL:
            # operando empacotado: tipo nos 2 bits baixos, índice do nome
            # (const string) no resto — ver compiler.py / ps_compiler.c.
            tipo = arg & 3
            nome_idx = arg >> 2
            v = pilha[sp - 1]
            if tipo == 1 and isinstance(v, str):
                pilha[sp - 1] = int(v.strip())
            elif tipo == 2 and isinstance(v, str):
                pilha[sp - 1] = float(v.strip())
            elif tipo == 2 and isinstance(v, int) and not isinstance(v, bool):
                pilha[sp - 1] = float(v)
            else:
                esperado = ("str", "int", "flo", "bool")[tipo]
                tipos = (str, int, (int, float), bool)[tipo]
                ok = isinstance(v, tipos) and (tipo == 0 or tipo == 3
                                               or not isinstance(v, bool))
                if tipo == 3:
                    ok = isinstance(v, bool)
                if not ok:
                    vn = consts[nome_idx] if 0 <= nome_idx < len(consts) else ""
                    raise ErroVM(f"variável {vn} esperava {esperado}")

        # ── resto das comparações ────────────────────────────────────────
        elif o == op.GT:
            sp -= 1; pilha[sp - 1] = pilha[sp - 1] > pilha[sp]
        elif o == op.LE:
            sp -= 1; pilha[sp - 1] = pilha[sp - 1] <= pilha[sp]
        elif o == op.GE:
            sp -= 1; pilha[sp - 1] = pilha[sp - 1] >= pilha[sp]
        elif o == op.EQ:
            sp -= 1; pilha[sp - 1] = _iguais(pilha[sp - 1], pilha[sp])
        elif o == op.NE:
            sp -= 1; pilha[sp - 1] = not _iguais(pilha[sp - 1], pilha[sp])

        # ── bitwise ──────────────────────────────────────────────────────
        elif o == op.BIT_OR:
            sp -= 1; pilha[sp - 1] = pilha[sp - 1] | pilha[sp]
        elif o == op.BIT_XOR:
            sp -= 1; pilha[sp - 1] = pilha[sp - 1] ^ pilha[sp]
        elif o == op.BIT_AND:
            sp -= 1; pilha[sp - 1] = pilha[sp - 1] & pilha[sp]
        elif o == op.LSHIFT:
            sp -= 1; pilha[sp - 1] = pilha[sp - 1] << pilha[sp]
        elif o == op.RSHIFT:
            sp -= 1; pilha[sp - 1] = pilha[sp - 1] >> pilha[sp]
        elif o == op.BIT_NOT:
            pilha[sp - 1] = ~pilha[sp - 1]

        elif o == op.POP_TOP:
            sp -= 1
        elif o == op.MAKE_FUNCTION:
            pilha[sp] = Funcao(consts[arg]); sp += 1
        elif o == op.HALT:
            return None
        else:
            raise ErroVM(f"opcode desconhecido: {o}")


class _NaoDefinido:
    __slots__ = ()
    def __repr__(self) -> str:
        return "<não definido>"


_NAO_DEF = _NaoDefinido()


def roda(co: CodeObj, tabela_globais: dict[str, int],
         builtins: dict[str, Any] | None = None) -> list[Any]:
    """Prepara a tabela de globais e executa o módulo."""
    gvars: list[Any] = [_NAO_DEF] * max(len(tabela_globais), 1)
    if builtins:
        for nome, valor in builtins.items():
            idx = tabela_globais.get(nome, -1)
            if idx >= 0:
                gvars[idx] = valor
    executa(co, gvars, [])
    return gvars
