"""
Compilador AST → bytecode da PoolScript.

Diferença central pro tree-walker: **nomes viram índices em tempo de
compilação**. No interpretador atual, ler `n` dentro de `fib` faz
`Scope.get("n")` — hash da string + busca no dict + possível subida na cadeia
de escopos, a cada acesso. Aqui isso vira `locals[0]`, um índice de array
resolvido uma vez só, durante a compilação.

Só isso já mata as 200 mil chamadas a `Scope.get` que o profile mostrou no
fib.

Escopo desta etapa: o subconjunto necessário pra rodar os benchmarks reais
(fib recursivo e loop) — literais, nomes, aritmética, comparação, bitwise,
if/elif/else, while, action, call, return, atribuição. Qualquer nó fora
disso levanta `NaoSuportado`, pra nunca compilar errado calado.
"""
from __future__ import annotations

from typing import Any

from ..parser import (
    ActionDecl, Assignment, BinaryOp, Block, Call, DictLiteral, ExpressionStmt,
    IfStmt, IndexAccess, ListLiteral, Literal, Name, Node, Program, ReturnStmt,
    UnaryOp, VarDecl, WhileStmt,
)
from . import opcodes as op


class NaoSuportado(Exception):
    """Nó do AST que esta etapa da VM ainda não compila."""


class CodeObj:
    """Unidade compilada — um módulo ou uma action."""

    __slots__ = ("name", "code", "consts", "nlocals", "nparams", "nomes_locais")

    def __init__(self, name: str) -> None:
        self.name = name
        self.code: list[int] = []
        self.consts: list[Any] = []
        self.nlocals = 0
        self.nparams = 0
        self.nomes_locais: list[str] = []

    def __repr__(self) -> str:
        return f"<CodeObj {self.name} {len(self.code)//2} instr, {self.nlocals} locais>"

    def desmontar(self) -> str:
        linhas = []
        i = 0
        while i < len(self.code):
            o, a = self.code[i], self.code[i + 1]
            nome = op.NOMES.get(o, f"?{o}")
            extra = ""
            if o == op.LOAD_CONST:
                extra = f"   ({self.consts[a]!r})"
            elif o in (op.LOAD_LOCAL, op.STORE_LOCAL) and a < len(self.nomes_locais):
                extra = f"   ({self.nomes_locais[a]})"
            linhas.append(f"  {i:4} {nome:<15} {a:4}{extra}")
            i += 2
        return "\n".join(linhas)


class Compilador:
    def __init__(self, tabela_globais: dict[str, int]) -> None:
        # nome global → índice. Compartilhada entre módulo e actions, porque
        # uma action precisa enxergar as globais (inclusive ela mesma, pra
        # recursão).
        self.globais = tabela_globais

    # ── helpers de emissão ─────────────────────────────────────────────────
    def _emite(self, co: CodeObj, opcode: int, arg: int = 0) -> int:
        pos = len(co.code)
        co.code.append(opcode)
        co.code.append(arg)
        return pos

    def _const(self, co: CodeObj, valor: Any) -> int:
        # dedup só por identidade de valor+tipo: `1` e `True` não podem
        # colidir (em Python `1 == True`), senão o pool devolveria o índice
        # errado e a VM empurraria o valor de outro tipo.
        for i, v in enumerate(co.consts):
            if type(v) is type(valor) and v == valor:
                return i
        co.consts.append(valor)
        return len(co.consts) - 1

    def _idx_global(self, nome: str) -> int:
        idx = self.globais.get(nome, -1)
        if idx < 0:
            idx = len(self.globais)
            self.globais[nome] = idx
        return idx

    def _idx_local(self, co: CodeObj, nome: str) -> int:
        for i, n in enumerate(co.nomes_locais):
            if n == nome:
                return i
        co.nomes_locais.append(nome)
        co.nlocals = len(co.nomes_locais)
        return co.nlocals - 1

    # ── módulo ─────────────────────────────────────────────────────────────
    def compila_modulo(self, prog: Program) -> CodeObj:
        co = CodeObj("<module>")
        for stmt in prog.statements:
            self._stmt(co, stmt, no_topo=True)
        self._emite(co, op.HALT)
        return co

    # ── statements ─────────────────────────────────────────────────────────
    def _stmt(self, co: CodeObj, no: Node, no_topo: bool = False) -> None:
        cls = no.__class__

        if cls is ActionDecl:
            filho = self._compila_action(no)  # type: ignore[arg-type]
            k = self._const(co, filho)
            self._emite(co, op.MAKE_FUNCTION, k)
            # `_guarda_nome`, não STORE_GLOBAL fixo: action declarada DENTRO
            # de outra é local da que a contém — é o que o interpretador faz
            # (`scope.define` no escopo corrente). Emitir sempre STORE_GLOBAL
            # vazava a action interna pro escopo global, divergindo da
            # semântica da linguagem.
            self._guarda_nome(co, no.name, no_topo)  # type: ignore[attr-defined]
            return

        if cls is VarDecl or cls is Assignment:
            nome: str = no.name if cls is VarDecl else no.target  # type: ignore[attr-defined]
            operador = "=" if cls is VarDecl else no.operator     # type: ignore[attr-defined]

            if operador != "=":
                # `x += v`  →  carrega x, calcula, guarda.
                # operador[:-1] tira o '=' final: "+=" vira "+".
                self._carrega_nome(co, nome, no)
                self._expr(co, no.value)  # type: ignore[attr-defined]
                self._emite(co, op.BINARIO[operador[:-1]])
            else:
                self._expr(co, no.value)  # type: ignore[attr-defined]
            # Tipo declarado converte e cobra: `flo x = 5` guarda 5.0.
            # Só os quatro escalares — `list x = (1,2)` passa sem checagem,
            # como no interpretador.
            if cls is VarDecl:
                escalares = ("str", "int", "flo", "bool")
                declarado = getattr(no, "declared_type", None)
                if declarado in escalares:
                    # Empacota nome+tipo num operando só: tipo nos 2 bits baixos,
                    # índice do nome (const string) no resto — espelha ps_compiler.c,
                    # pra VM dizer "variável X esperava T".
                    ni = self._const(co, nome)
                    self._emite(co, op.COERCE_DECL, (ni << 2) | escalares.index(declarado))
            self._guarda_nome(co, nome, no_topo)
            return

        if cls is ExpressionStmt:
            self._expr(co, no.expression)  # type: ignore[attr-defined]
            self._emite(co, op.POP_TOP)
            return

        if cls is ReturnStmt:
            if no.value is None:  # type: ignore[attr-defined]
                self._emite(co, op.LOAD_CONST, self._const(co, None))
            else:
                self._expr(co, no.value)  # type: ignore[attr-defined]
            self._emite(co, op.RETURN)
            return

        if cls is IfStmt:
            self._if(co, no, no_topo)  # type: ignore[arg-type]
            return

        if cls is WhileStmt:
            self._while(co, no, no_topo)  # type: ignore[arg-type]
            return

        if cls is Block:
            for s in no.statements:  # type: ignore[attr-defined]
                self._stmt(co, s, no_topo)
            return

        raise NaoSuportado(f"statement {cls.__name__} ainda não compila na VM")

    def _bloco(self, co: CodeObj, bloco: Block, no_topo: bool) -> None:
        for s in bloco.statements:
            self._stmt(co, s, no_topo)

    def _if(self, co: CodeObj, no: IfStmt, no_topo: bool) -> None:
        saltos_fim: list[int] = []
        for ramo in no.branches:
            if ramo.condition is None:          # else
                self._bloco(co, ramo.block, no_topo)
                break
            self._expr(co, ramo.condition)
            salto_falso = self._emite(co, op.JUMP_IF_FALSE, 0)
            self._bloco(co, ramo.block, no_topo)
            saltos_fim.append(self._emite(co, op.JUMP, 0))
            co.code[salto_falso + 1] = len(co.code)   # patch: pula pro próximo ramo
        fim = len(co.code)
        for s in saltos_fim:
            co.code[s + 1] = fim

    def _while(self, co: CodeObj, no: WhileStmt, no_topo: bool) -> None:
        topo = len(co.code)
        self._expr(co, no.condition)
        sai = self._emite(co, op.JUMP_IF_FALSE, 0)
        self._bloco(co, no.block, no_topo)
        self._emite(co, op.JUMP, topo)
        co.code[sai + 1] = len(co.code)

    # ── action ─────────────────────────────────────────────────────────────
    def _compila_action(self, no: ActionDecl) -> CodeObj:
        co = CodeObj(no.name)
        for p in no.params:
            self._idx_local(co, p)
        co.nparams = len(no.params)
        self._bloco(co, no.block, no_topo=False)
        # action sem return explícito devolve Null
        self._emite(co, op.LOAD_CONST, self._const(co, None))
        self._emite(co, op.RETURN)
        return co

    # ── nomes ──────────────────────────────────────────────────────────────
    def _carrega_nome(self, co: CodeObj, nome: str, no: Node) -> None:
        if co.name != "<module>":
            for i, n in enumerate(co.nomes_locais):
                if n == nome:
                    self._emite(co, op.LOAD_LOCAL, i)
                    return
        self._emite(co, op.LOAD_GLOBAL, self._idx_global(nome))

    def _guarda_nome(self, co: CodeObj, nome: str, no_topo: bool) -> None:
        if co.name == "<module>":
            self._emite(co, op.STORE_GLOBAL, self._idx_global(nome))
        else:
            self._emite(co, op.STORE_LOCAL, self._idx_local(co, nome))

    # ── expressões ─────────────────────────────────────────────────────────
    def _expr(self, co: CodeObj, no: Node) -> None:
        cls = no.__class__

        if cls is Literal:
            self._emite(co, op.LOAD_CONST, self._const(co, no.value))  # type: ignore[attr-defined]
            return

        if cls is Name:
            self._carrega_nome(co, no.value, no)  # type: ignore[attr-defined]
            return

        if cls is BinaryOp:
            operador: str = no.operator  # type: ignore[attr-defined]
            codigo = op.BINARIO.get(operador, -1)
            if codigo < 0:
                raise NaoSuportado(f"operador binário {operador!r} ainda não compila na VM")
            self._expr(co, no.left)   # type: ignore[attr-defined]
            self._expr(co, no.right)  # type: ignore[attr-defined]
            self._emite(co, codigo)
            return

        if cls is UnaryOp:
            operador = no.operator  # type: ignore[attr-defined]
            self._expr(co, no.operand)  # type: ignore[attr-defined]
            if operador == "-":
                self._emite(co, op.NEG)
            elif operador == "~":
                self._emite(co, op.BIT_NOT)
            elif operador == "+":
                pass
            else:
                raise NaoSuportado(f"operador unário {operador!r} ainda não compila na VM")
            return

        if cls is Call:
            args = no.args  # type: ignore[attr-defined]
            for a in args:
                if a.name is not None:
                    raise NaoSuportado("argumento nomeado ainda não compila na VM")
            self._expr(co, no.callee)  # type: ignore[attr-defined]
            for a in args:
                self._expr(co, a.value)
            self._emite(co, op.CALL, len(args))
            return

        if cls is ListLiteral:
            itens = no.items  # type: ignore[attr-defined]
            for item in itens:
                self._expr(co, item)
            self._emite(co, op.BUILD_LIST, len(itens))
            return

        if cls is DictLiteral:
            entradas = no.entries  # type: ignore[attr-defined]
            for e in entradas:
                # chave sem aspas (`{nome: 1}`) é tratada como string literal,
                # igual o interpretador faz
                if e.key.__class__ is Name:
                    self._emite(co, op.LOAD_CONST, self._const(co, e.key.value))
                else:
                    self._expr(co, e.key)
                self._expr(co, e.value)
            self._emite(co, op.BUILD_DICT, len(entradas))
            return

        if cls is IndexAccess:
            self._expr(co, no.target)  # type: ignore[attr-defined]
            self._expr(co, no.index)   # type: ignore[attr-defined]
            self._emite(co, op.INDEX_GET)
            return

        raise NaoSuportado(f"expressão {cls.__name__} ainda não compila na VM")


def compila(prog: Program) -> tuple[CodeObj, dict[str, int]]:
    tabela: dict[str, int] = {}
    co = Compilador(tabela).compila_modulo(prog)
    return co, tabela
