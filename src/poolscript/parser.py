from __future__ import annotations

from dataclasses import dataclass, field
from typing import Any

from .lexer import Lexer, PoolSyntaxError, Token
from .ps_errors import _BasePoolParseError as _PSBaseParseError


@dataclass(slots=True)
class Node:
    line: int
    col: int


@dataclass(slots=True)
class Program(Node):
    statements: list[Node] = field(default_factory=list)


@dataclass(slots=True)
class Name(Node):
    value: str


@dataclass(slots=True)
class Literal(Node):
    value: Any
    kind: str


@dataclass(slots=True)
class InterpolatedString(Node):
    parts: list[Node]


@dataclass(slots=True)
class ListLiteral(Node):
    items: list[Node]


@dataclass(slots=True)
class DictEntry(Node):
    key: Node
    value: Node


@dataclass(slots=True)
class DictLiteral(Node):
    entries: list[DictEntry]


@dataclass(slots=True)
class UnaryOp(Node):
    operator: str
    operand: Node


@dataclass(slots=True)
class BinaryOp(Node):
    left: Node
    operator: str
    right: Node


@dataclass(slots=True)
class TypeName(Node):
    name: str


@dataclass(slots=True)
class CallArg(Node):
    value: Node
    name: str | None = None


@dataclass(slots=True)
class Call(Node):
    callee: Node
    args: list[CallArg]


@dataclass(slots=True)
class MemberAccess(Node):
    target: Node
    member: str


@dataclass(slots=True)
class SliceAccess(Node):
    """Slice: target[start:stop:step] — qualquer parte pode ser None."""
    target: Node
    start: "Node | None"
    stop: "Node | None"
    step: "Node | None"


@dataclass(slots=True)
class IndexAccess(Node):
    target: Node
    index: Node


@dataclass(slots=True)
class PostfixOp(Node):
    operand: Node
    operator: str


@dataclass(slots=True)
class VarDecl(Node):
    declared_type: str
    name: str
    value: Node


@dataclass(slots=True)
class Assignment(Node):
    target: str
    operator: str
    value: Node


@dataclass(slots=True)
class ExpressionStmt(Node):
    expression: Node


@dataclass(slots=True)
class Block(Node):
    style: str
    statements: list[Node]


@dataclass(slots=True)
class IfBranch(Node):
    condition: Node | None
    block: Block


@dataclass(slots=True)
class IfStmt(Node):
    branches: list[IfBranch]


@dataclass(slots=True)
class WhileStmt(Node):
    condition: Node
    block: Block


@dataclass(slots=True)
class ForEachStmt(Node):
    item_name: str
    iterable: Node
    block: Block


@dataclass(slots=True)
class LambdaExpr(Node):
    """action(params) { ... } — função anônima/lambda."""
    params: list[str]
    block: "Block"


@dataclass(slots=True)
class ActionDecl(Node):
    name: str
    params: list[str]
    block: Block
    defaults: dict = None       # {param_name: default_node}
    return_type: str = None     # None | "int" | "bool"
    is_async: bool = False       # `async action` / `async reaction`


@dataclass(slots=True)
class RaiseStmt(Node):
    value: Node


@dataclass(slots=True)
class YieldStmt(Node):
    value: Node | None


@dataclass(slots=True)
class ReturnStmt(Node):
    value: Node | None


@dataclass
@dataclass(slots=True)
class CatchClause(Node):
    """Um bloco catch com tipo opcional: catch (TypeError e) { ... }"""
    error_name: str
    error_type: str | None  # None = captura qualquer erro
    block: "Block"


@dataclass(slots=True)
class TryCatchStmt(Node):
    try_block: "Block"
    catches: "list[CatchClause]"  # um ou mais catches
    finally_block: "Block | None" = None


@dataclass(slots=True)
class ImportStmt(Node):
    mode: str
    module: list[str]
    names: list[str]
    module_alias: str | None = None
    name_aliases: dict[str, str] = field(default_factory=dict)


@dataclass(slots=True)
class DecoratorCall(Node):
    path: list[str]
    args: list[CallArg]


@dataclass(slots=True)
class DecoratorStmt(Node):
    decorator: DecoratorCall
    block: Block | None


@dataclass(slots=True)
class UsingStmt(Node):
    """`using <expr> as <name> { ... }` — context manager (chama .close() ao sair)."""
    resource: Node
    var_name: str
    block: Block


@dataclass(slots=True)
class CountExpr(Node):
    """Operador `count` (v0.5.2). Conta ocorrências; truthy se > 0.

    Formas suportadas:
      - count <type>(<value>) in <container>     → mode="prefix_value"
      - count <type> in <container>              → mode="prefix_type"
      - <type>(<value>) count in <container>     → mode="infix"
    Quando value_node é None, conta itens daquele tipo no container.
    """
    target_type: str
    value_node: Node | None
    container: Node
    mode: str = "prefix_value"


@dataclass(slots=True)
class CountEachStmt(Node):
    """`count each <type>(<value>) in <container> { ... }`.

    Para cada ocorrência, executa o bloco com `_match` e `_index` no escopo.
    `return;` (sem valor) devolve o total acumulado.
    `return <expr>` interrompe e devolve a expressão.
    Caso contrário, ao final, devolve o total.
    """
    target_type: str
    value_node: Node | None
    container: Node
    block: Block


@dataclass(slots=True)
class CountEachExpr(Node):
    """`count each <type>[(<value>)] in <container>` SEM bloco — expressão.

    Avalia para o número total de ocorrências (int). Usado em:
      - `nome = count each int in frase`
      - `post(count each char in frase)`
    """
    target_type: str
    value_node: Node | None
    container: Node


@dataclass(slots=True)
class RunSelfWithStmt(Node):
    """`run_selfwith_("label") { nome_funcao() }` — ponto de entrada do programa.

    Executa o bloco imediatamente, como uma IIFE. O label é só identificação.
    """
    label: str
    block: Block


@dataclass(slots=True)
class ModelField(Node):
    """Campo de um model: `nome: str(length=60)`"""
    name: str
    type_name: str        # "str", "int", "flo", "bool"
    length: int | None    # max caracteres ou dígitos


@dataclass(slots=True)
class ModelDecl(Node):
    """`model User() { nome: str(length=60) ... }`"""
    name: str
    fields: list["ModelField"]


@dataclass(slots=True)
class ContinueStmt(Node):
    """Ignora e segue o fluxo — equivalente ao pass do Python."""
    pass


@dataclass(slots=True)
class BreakStmt(Node):
    """Para o loop imediatamente."""
    pass



@dataclass(slots=True)
class MatchPattern(Node):
    """Padrão de um case — valor, wildcard, lista, dict ou captura."""
    kind: str          # "value", "wildcard", "capture", "list", "dict", "or"
    value: "Any" = None       # para kind="value" — o literal
    name: str = None          # para kind="capture" — nome da variável
    items: list = None        # para kind="list" — lista de sub-padrões
    keys: dict = None         # para kind="dict" — {chave: sub-padrão}
    patterns: list = None     # para kind="or" — lista de padrões alternativos
    guard: "Node | None" = None  # condição if opcional


@dataclass(slots=True)
class MatchCase(Node):
    """Um case dentro do match."""
    pattern: MatchPattern
    body: Block


@dataclass(slots=True)
class MatchStmt(Node):
    """`match valor: case ...: case _: ...`"""
    subject: Node
    cases: list[MatchCase]


@dataclass(slots=True)
class TupleLiteral(Node):
    """Tupla: (valor1, valor2, ...)"""
    items: list[Node]


@dataclass(slots=True)
class UnpackTarget(Node):
    """Um nível de alvo de desempacotamento: `a, b` ou `(a, b)` ou `a, *b, c`.
    Cada item de `elements` é `str` (nome simples) ou `UnpackTarget` (grupo aninhado).
    `star_index` é a posição em `elements` do alvo `*nome` (sempre um `str`), ou None."""
    elements: list
    star_index: "int | None" = None
    trailing_comma: bool = False


@dataclass(slots=True)
class UnpackAssignment(Node):
    """`a, b = 1, 2` / `a, (b, c) = 1, (2, 3)` / `a, *resto = [1, 2, 3]`."""
    targets: "UnpackTarget"
    value: Node


@dataclass(slots=True)
class EntityField(Node):
    """`nome: tipo` ou `nome: tipo = default` dentro de Entity (para @dataentity)."""
    field_name: str
    type_name:  str
    default:    "Any" = None

@dataclass(slots=True)
class EntityDecl(Node):
    """`Entity NomeClasse(Pai, Pai2) { ... }` — declaração de classe."""
    name: str
    parents: "list[str]"   # lista de pais (vazia = sem herança)
    body: list[Node]
    fields: "list[EntityField]" = None  # campos tipados (para @dataentity)


@dataclass(slots=True)
class BaseCall(Node):
    """`base(NomePai, args...)` ou `base(args...)` — chama __init__ do pai."""
    args: list["CallArg"]
    target: "str | None" = None   # nome do pai alvo (herança múltipla)



@dataclass(slots=True)
class MemberAssignment(Node):
    """`self.x = valor` ou `obj.x = valor` — atribuição de membro."""
    target: Node        # a expressão do objeto (ex: Name("self"))
    member: str         # nome do atributo
    value: Node         # valor a atribuir



@dataclass(slots=True)
class ColorStrExpr(Node):
    """`<hex>"texto"` — string colorida com ANSI. color = hex sem # ou nome."""
    color: str   # ex: "2196f3" ou "red"
    expr: Node   # expressão de string (STR, f-string, variável…)


@dataclass(slots=True)
class AwaitExpr(Node):
    """`await <expr>` — aguarda o resultado de uma action/reaction async.
    Se <expr> não for um PoolFuture, retorna o valor como está (pass-through)."""
    value: Node


class PoolParseError(_PSBaseParseError):
    def pool_message(self) -> str:
        line = getattr(self.token, "line", 0)
        col  = getattr(self.token, "col",  0)
        src_lines = (self.source or "").splitlines()
        src  = src_lines[line - 1] if 0 < line <= len(src_lines) else ""
        pointer = " " * max(0, col - 1) + "^^^"
        fname = self.filename or "<script>"
        return (
            f"\033[1m\033[31mSyntaxError\033[0m: {self.msg}\n"
            f"  \033[2m→ {fname}, linha {line}, coluna {col}\033[0m\n"
            f"  | {src}\n"
            f"  | \033[33m{pointer}\033[0m"
        )

    def __init__(self, msg: str, token: Token, source: str = "", filename: str = ""):
        self.msg = msg
        self.token = token
        self.source = source
        self.filename = filename
        # bypass base __init__ (incompatible signature)
        Exception.__init__(self, self.pool_message())

    def format(self) -> str:
        lines = self.source.splitlines()
        src = lines[self.token.line - 1] if 0 <= self.token.line - 1 < len(lines) else ""
        pointer = " " * (self.token.col - 1) + "^^^"
        file_info = f"  Arquivo: {self.filename}\n" if self.filename and self.filename != "<stdin>" else ""
        return (
            f"SyntaxError: {self.msg}\n"
            f"{file_info}"
            f"  L{self.token.line}:C{self.token.col}\n"
            f"  | {src}\n"
            f"  | {pointer}________"
        )


ASSIGN_OPS = {"=", "+=", "-=", "*=", "/=", "%="}
COMPARE_OPS = {"==", "!=", "===", "!==", "<", ">", "<=", ">="}
TYPE_KEYWORDS = {"str", "int", "flo", "bool", "list", "json", "type"}
EXPR_NAME_KEYWORDS = {"post", "input", "create", "clear", "space", "addEnd", "char", "list", "json", "JSON", "full", "mei", "self",
                      "upper", "lower", "replace", "split", "strip", "join", "startswith", "endswith",
                      "find", "index", "format", "encode", "decode", "lstrip", "rstrip", "title", "capitalize"}
# Tipos aceitos no operador `count` (inclui `char`, distinto de `str`).
COUNT_TYPE_KEYWORDS = {"str", "int", "flo", "bool", "list", "json", "char"}
STATEMENT_START_KEYWORDS = {
    "if", "elif", "else", "while", "for", "each", "action", "return", "try", "catch",
    "import", "from", "PUSH", "GET", "str", "int", "flo", "bool", "using", "model",
}


class Parser:
    def __init__(self, tokens: list[Token], source: str = "", filename: str = "<stdin>"):
        self.tokens = tokens
        self.source = source
        self.filename = filename
        self.pos = 0

    def current(self) -> Token:
        return self.tokens[self.pos]

    def peek(self, offset: int = 1) -> Token:
        idx = min(self.pos + offset, len(self.tokens) - 1)
        return self.tokens[idx]

    def match(self, type_: str, value: str | None = None) -> Token | None:
        tok = self.current()
        if tok.type != type_:
            return None
        if value is not None and tok.value != value:
            return None
        self.pos += 1
        return tok

    def expect(self, type_: str | tuple, value: str | None = None, msg: str | None = None, token=None) -> Token:
        tok = self.current()
        types = (type_,) if isinstance(type_, str) else type_
        if tok.type in types and (value is None or tok.value == value):
            self.pos += 1
            return tok
        expected = value if value is not None else type_
        blame = token or tok
        raise self.error(msg or f"esperado {expected}, encontrou {tok.type}:{tok.value!r}", blame)

    def error(self, msg: str, token: Token | None = None) -> PoolParseError:
        return PoolParseError(msg, token or self.current(), self.source, self.filename)

    def skip_separators(self) -> None:
        while self.current().type in {"NEWLINE", "SEMI"}:
            self.pos += 1

    def parse(self) -> Program:
        start = self.current()
        statements: list[Node] = []
        self.skip_separators()
        while self.current().type != "EOF":
            statements.append(self.parse_statement())
            self.skip_separators()
        return Program(line=start.line, col=start.col, statements=statements)

    def parse_statement(self) -> Node:
        self.skip_separators()
        tok = self.current()

        if tok.type == "KW":
            # 'async' — sempre antes de [tipo] action/reaction
            # async action foo() / async int reaction foo()
            if tok.value == "async" and self.pos + 1 < len(self.tokens):
                nxt = self.tokens[self.pos + 1]
                if nxt.type == "KW" and nxt.value in {"action", "reaction"}:
                    return self.parse_action_decl()
                if (nxt.type == "KW" and nxt.value in {"int", "bool", "str", "flo"}
                        and self.pos + 2 < len(self.tokens)
                        and self.tokens[self.pos + 2].type == "KW"
                        and self.tokens[self.pos + 2].value in {"action", "reaction"}):
                    return self.parse_action_decl()
            # tipo de retorno opcional ANTES de var_decl: int reaction / bool action
            if (tok.value in {"int", "bool", "str", "flo"}
                    and self.pos + 1 < len(self.tokens)
                    and self.tokens[self.pos + 1].type == "KW"
                    and self.tokens[self.pos + 1].value in {"action", "reaction"}):
                return self.parse_action_decl()
            if tok.value in {"str", "int", "flo", "bool"}:
                return self.parse_var_decl()
            if tok.value == "if":
                return self.parse_if_stmt()
            if tok.value == "while":
                return self.parse_while_stmt()
            if tok.value == "for":
                return self.parse_for_stmt()
            if tok.value in {"action", "reaction"}:
                return self.parse_action_decl()
            if tok.value == "model":
                return self.parse_model_decl()
            if tok.value == "Entity":
                return self.parse_entity_decl()
            if tok.value == "match":
                # match(...) = chamada de função (regex); match expr: = match/case
                if self.peek().type != "LPAREN":
                    return self.parse_match_stmt()
            if tok.value == "continue":
                self.pos += 1
                self.consume_optional_semi()
                return ContinueStmt(line=tok.line, col=tok.col)
            if tok.value == "break":
                self.pos += 1
                self.consume_optional_semi()
                return BreakStmt(line=tok.line, col=tok.col)
            if tok.value == "try":
                return self.parse_try_stmt()
            if tok.value == "using":
                return self.parse_using_stmt()
            if tok.value == "return":
                return self.parse_return_stmt()
            if tok.value == "raise":
                self.pos += 1
                value = self.parse_expression()
                self.consume_optional_semi()
                return RaiseStmt(line=tok.line, col=tok.col, value=value)
            if tok.value == "yield":
                self.pos += 1
                value = None
                if self.current().type not in {"RBRACE", "DEDENT", "NEWLINE", "EOF"}:
                    value = self.parse_expression()
                self.consume_optional_semi()
                return YieldStmt(line=tok.line, col=tok.col, value=value)
            if tok.value in {"import", "from", "PUSH"}:
                return self.parse_import_stmt()
            if tok.value == "count" and self.peek().type == "KW" and self.peek().value == "each":
                # `count each ...` pode ser STATEMENT (com bloco `{...}` ou `:`)
                # ou EXPRESSÃO (sem bloco — usado em `nome = count each ...`).
                # Olhamos à frente para decidir.
                if self._count_each_has_block():
                    return self.parse_count_each_stmt()
                # Sem bloco → trata como expressão num ExpressionStmt.
                expr = self.parse_count_each_expr()
                self.consume_optional_semi()
                return ExpressionStmt(line=tok.line, col=tok.col, expression=expr)

        if tok.type == "AT":
            return self.parse_decorator_stmt()

        if tok.type == "IDENT" and tok.value == "run_selfwith_":
            return self.parse_run_selfwith()

        if tok.type == "IDENT" and self.peek().type == "OP" and self.peek().value in ASSIGN_OPS:
            return self.parse_assignment()

        # Desempacotamento: `a, b = 1, 2` / `a, (b, c) = 1, (2, 3)` / `a, *resto = [...]`
        # Lookahead puro (sem consumir tokens) — só entra se de fato houver uma
        # lista de alvos seguida de '='. Qualquer coisa que não bata cai no
        # caminho normal (chamada, tupla-literal, comparação, atribuição de membro...).
        if (tok.type in {"IDENT", "IDENT_UPPER", "LPAREN"}
                or (tok.type == "OP" and tok.value == "*")):
            if self._looks_like_unpack_stmt():
                return self.parse_unpack_assignment()

        # self.x = valor  ou  obj.x = valor  (member assignment)
        # Lookahead puro sem consumir tokens — só avança se confirmar o padrão
        if (tok.type == "KW" and tok.value == "self") or tok.type in {"IDENT", "IDENT_UPPER"}:
            # Verifica padrão: TOK (DOT IDENT)+ OP(=) sem consumir nada
            peek = self.pos + 1  # após o objeto
            chain: list[str] = []
            while (peek < len(self.tokens)
                   and self.tokens[peek].type == "DOT"
                   and peek + 1 < len(self.tokens)
                   and self.tokens[peek + 1].type in {"IDENT", "KW"}):
                chain.append(str(self.tokens[peek + 1].value))
                peek += 2
            if (chain
                    and peek < len(self.tokens)
                    and self.tokens[peek].type == "OP"
                    and self.tokens[peek].value == "="):
                # padrão confirmado — agora consome
                obj_tok = self.current()
                self.pos = peek + 1  # pula objeto + cadeia de dots + "="
                val = self.parse_expression()
                self.consume_optional_semi()
                base_node = Name(line=obj_tok.line, col=obj_tok.col, value=str(obj_tok.value))
                for attr in chain[:-1]:
                    base_node = MemberAccess(line=obj_tok.line, col=obj_tok.col,
                                              target=base_node, member=attr)
                return MemberAssignment(line=obj_tok.line, col=obj_tok.col,
                                         target=base_node, member=chain[-1], value=val)

        expr = self.parse_expression()
        self.consume_optional_semi()
        return ExpressionStmt(line=tok.line, col=tok.col, expression=expr)

    def consume_optional_semi(self) -> None:
        self.match("SEMI")

    def parse_var_decl(self) -> VarDecl:
        type_tok = self.expect("KW")
        name_tok = self.expect(("IDENT", "IDENT_UPPER"), msg="nome de variável inválido")
        self.expect("OP", "=", "declaração de variável exige '='")
        value = self.parse_expression()
        self.consume_optional_semi()
        return VarDecl(line=type_tok.line, col=type_tok.col, declared_type=str(type_tok.value), name=str(name_tok.value), value=value)

    def parse_assignment(self) -> Assignment:
        name_tok = self.expect("IDENT")
        op_tok = self.expect("OP")
        if op_tok.value not in ASSIGN_OPS:
            raise self.error("operador de atribuição inválido", op_tok)
        if self.current().type in {"NEWLINE", "SEMI", "EOF", "RBRACE", "DEDENT"}:
            raise self.error("atribuição sem expressão à direita", op_tok)
        value = self.parse_expression()
        self.consume_optional_semi()
        return Assignment(line=name_tok.line, col=name_tok.col, target=str(name_tok.value), operator=str(op_tok.value), value=value)

    # ── Desempacotamento de tuplas (unpacking), estilo Python ────────────────

    def _looks_like_unpack_stmt(self) -> bool:
        """Lookahead puro (não consome tokens): decide se o statement atual é
        uma lista de alvos de desempacotamento seguida de '=', ex: `a, b = 1, 2`.
        Retorna False para qualquer coisa que deva seguir o caminho normal
        (chamada, tupla-literal, comparação, atribuição aumentada/de membro...).
        """
        star_seen = False
        nested_comma_seen = False

        def skip_target(i: int):
            nonlocal star_seen, nested_comma_seen
            if i >= len(self.tokens):
                return None
            tok = self.tokens[i]
            if tok.type == "OP" and tok.value == "*":
                nxt = self.tokens[i + 1] if i + 1 < len(self.tokens) else None
                if nxt is None or nxt.type not in {"IDENT", "IDENT_UPPER"}:
                    return None
                star_seen = True
                return i + 2
            if tok.type in {"IDENT", "IDENT_UPPER"}:
                nxt = self.tokens[i + 1] if i + 1 < len(self.tokens) else None
                if nxt is not None and nxt.type in {"LPAREN", "DOT", "LBRACK"}:
                    return None  # chamada/membro/índice — não é alvo simples
                return i + 1
            if tok.type == "LPAREN":
                j = i + 1
                while True:
                    j2 = skip_target(j)
                    if j2 is None:
                        return None
                    j = j2
                    if j < len(self.tokens) and self.tokens[j].type == "COMMA":
                        nested_comma_seen = True
                        j += 1
                        if j < len(self.tokens) and self.tokens[j].type == "RPAREN":
                            break
                        continue
                    break
                if j >= len(self.tokens) or self.tokens[j].type != "RPAREN":
                    return None
                return j + 1
            return None

        j = skip_target(self.pos)
        if j is None:
            return False
        count = 1
        trailing_comma_seen = False
        while j < len(self.tokens) and self.tokens[j].type == "COMMA":
            j += 1
            if j < len(self.tokens) and self.tokens[j].type == "OP" and self.tokens[j].value == "=":
                trailing_comma_seen = True
                break  # vírgula final logo antes do '='
            j2 = skip_target(j)
            if j2 is None:
                return False
            j = j2
            count += 1
        if j >= len(self.tokens) or not (self.tokens[j].type == "OP" and self.tokens[j].value == "="):
            return False
        # Evita falso-positivo em `(a) = 5` (nunca foi válido, comportamento inalterado)
        if count < 2 and not star_seen and not nested_comma_seen and not trailing_comma_seen:
            return False
        return True

    def parse_unpack_target_list(self) -> UnpackTarget:
        """Consome `alvo (',' alvo)* [',']` — usado no topo e dentro de `(...)`."""
        start = self.current()
        elements: list = []
        star_index: int | None = None
        trailing_comma = False

        def parse_one() -> None:
            nonlocal star_index
            tok = self.current()
            if tok.type == "OP" and tok.value == "*":
                if star_index is not None:
                    raise self.error(
                        "apenas um alvo com '*' é permitido por nível de desempacotamento", tok
                    )
                self.pos += 1
                name_tok = self.expect(("IDENT", "IDENT_UPPER"), msg="esperado nome após '*' em desempacotamento")
                star_index = len(elements)
                elements.append(str(name_tok.value))
                return
            if tok.type == "LPAREN":
                self.pos += 1
                inner = self.parse_unpack_target_list()
                self.expect("RPAREN", msg="faltou ')' em alvo de desempacotamento")
                # Colapsa grupo de 1 elemento simples (sem vírgula/estrela) — mesma
                # regra de colapso de `(expr)` já usada em TupleLiteral.
                if len(inner.elements) == 1 and inner.star_index is None and not inner.trailing_comma:
                    elements.append(inner.elements[0])
                else:
                    elements.append(inner)
                return
            name_tok = self.expect(("IDENT", "IDENT_UPPER"), msg="alvo de desempacotamento inválido")
            elements.append(str(name_tok.value))

        parse_one()
        while self.match("COMMA"):
            cur = self.current()
            if (cur.type == "OP" and cur.value == "=") or cur.type == "RPAREN":
                trailing_comma = True
                break
            parse_one()

        if len(elements) == 1 and star_index == 0 and not trailing_comma:
            raise self.error(
                "alvo com '*' sozinho precisa de vírgula: use '*nome, = valor'", start
            )

        return UnpackTarget(line=start.line, col=start.col, elements=elements,
                            star_index=star_index, trailing_comma=trailing_comma)

    def parse_unpack_assignment(self) -> UnpackAssignment:
        start = self.current()
        targets = self.parse_unpack_target_list()
        self.expect("OP", "=", "desempacotamento exige '='")
        expr = self.parse_expression()
        if self.match("COMMA"):
            items = [expr]
            while self.current().type not in {"NEWLINE", "SEMI", "EOF", "RBRACE", "DEDENT"}:
                items.append(self.parse_expression())
                if not self.match("COMMA"):
                    break
            value: Node = TupleLiteral(line=start.line, col=start.col, items=items)
        else:
            value = expr
        self.consume_optional_semi()
        return UnpackAssignment(line=start.line, col=start.col, targets=targets, value=value)

    def parse_if_stmt(self) -> IfStmt:
        start = self.expect("KW", "if")
        branches = [IfBranch(line=start.line, col=start.col, condition=self.parse_condition(), block=self.parse_block())]
        while self.current().type == "KW" and self.current().value == "elif":
            tok = self.expect("KW", "elif")
            branches.append(IfBranch(line=tok.line, col=tok.col, condition=self.parse_condition(), block=self.parse_block()))
        if self.current().type == "KW" and self.current().value == "else":
            tok = self.expect("KW", "else")
            branches.append(IfBranch(line=tok.line, col=tok.col, condition=None, block=self.parse_block()))
        return IfStmt(line=start.line, col=start.col, branches=branches)

    def parse_while_stmt(self) -> WhileStmt:
        start = self.expect("KW", "while")
        condition = self.parse_condition(allow_parenthesized_only=False)
        block = self.parse_block()
        return WhileStmt(line=start.line, col=start.col, condition=condition, block=block)

    def parse_for_stmt(self) -> ForEachStmt:
        start = self.expect("KW", "for")
        self.expect("KW", "each", "esperado 'each' em 'for each'")
        name_tok = self.expect(("IDENT", "IDENT_UPPER"), msg="nome de variável inválido")
        self.expect("KW", "in", "esperado 'in' no loop for each")
        iterable = self.parse_expression()
        block = self.parse_block()
        return ForEachStmt(line=start.line, col=start.col, item_name=str(name_tok.value), iterable=iterable, block=block)

    def parse_action_decl(self) -> ActionDecl:
        # 'async' — prefixo opcional, vem antes de tudo
        # async action foo() / async int reaction foo()
        is_async = False
        if self.current().type == "KW" and self.current().value == "async":
            is_async = True
            self.pos += 1

        # Tipo de retorno opcional antes da keyword: int, bool
        return_type: str | None = None
        if (self.current().type == "KW"
                and self.current().value in {"int", "bool", "str", "flo"}
                and self.pos + 1 < len(self.tokens)
                and self.tokens[self.pos + 1].type == "KW"
                and self.tokens[self.pos + 1].value in {"action", "reaction"}):
            return_type = str(self.current().value)
            self.pos += 1  # consume tipo

        # aceita 'action' ou 'reaction' como keyword de declaração
        tok = self.current()
        if tok.type == "KW" and tok.value in {"action", "reaction"}:
            start = tok
            self.pos += 1
        else:
            start = self.expect("KW", "action")

        name_tok = self.expect(("IDENT", "IDENT_UPPER"), msg="nome de variável inválido")
        self.expect("LPAREN")
        params: list[str] = []
        defaults: dict = {}
        if self.current().type != "RPAREN":
            while True:
                # 'self' é KW mas válido como parâmetro dentro de Entity
                if self.current().type == "KW" and self.current().value == "self":
                    param_name = "self"
                    self.pos += 1
                else:
                    param_name = str(self.expect("IDENT", msg="parâmetro inválido").value)
                params.append(param_name)
                # valor padrão: param=valor
                if self.current().type == "OP" and self.current().value == "=":
                    self.pos += 1  # consume =
                    defaults[param_name] = self.parse_expression()
                if not self.match("COMMA"):
                    break
        self.expect("RPAREN")
        # Permite { na próxima linha — pula NEWLINEs antes de parse_block
        while self.current().type in {"NEWLINE", "NL"}:
            self.pos += 1
        block = self.parse_block()
        self.consume_optional_semi()
        return ActionDecl(line=start.line, col=start.col, name=str(name_tok.value),
                          params=params, block=block, defaults=defaults,
                          return_type=return_type, is_async=is_async)

    def parse_return_stmt(self) -> ReturnStmt:
        start = self.expect("KW", "return")
        if self.current().type in {"NEWLINE", "SEMI", "RBRACE", "DEDENT", "EOF"}:
            value = None
        else:
            expr = self.parse_expression()
            # Suporte a `return jsonify({...}), 409` — empacota como tupla (expr, status)
            if self.match("COMMA"):
                status = self.parse_expression()
                value = TupleLiteral(line=start.line, col=start.col, items=[expr, status])
            else:
                value = expr
        self.consume_optional_semi()
        return ReturnStmt(line=start.line, col=start.col, value=value)

    def parse_try_stmt(self) -> TryCatchStmt:
        start = self.expect("KW", "try")
        try_block = self.parse_block()
        catches = []
        while self.current().type == "KW" and self.current().value == "catch":
            self.expect("KW", "catch")
            self.expect("LPAREN", msg="esperado '(' após 'catch'")
            tok = self.current()
            next_tok = self.peek()
            if tok.type == "IDENT_UPPER" and next_tok.type in {"IDENT", "IDENT_UPPER"}:
                # catch (TipoErro nome)
                error_type = str(tok.value)
                self.pos += 1
                error_name = str(self.expect(("IDENT", "IDENT_UPPER"), msg="nome do erro").value)
            else:
                # catch (e) — captura qualquer erro
                error_type = None
                error_name = str(self.expect(("IDENT", "IDENT_UPPER"), msg="nome do erro em catch").value)
            self.expect("RPAREN", msg="esperado ')' após nome do erro")
            catch_block = self.parse_block()
            catches.append(CatchClause(
                line=tok.line, col=tok.col,
                error_name=error_name,
                error_type=error_type,
                block=catch_block
            ))
        if not catches:
            raise self.error("esperado 'catch' após bloco do try", self.current())
        # finally opcional
        finally_block = None
        self.skip_separators()
        if self.current().type == "KW" and self.current().value == "finally":
            self.pos += 1
            finally_block = self.parse_block()
        return TryCatchStmt(line=start.line, col=start.col, try_block=try_block,
                            catches=catches, finally_block=finally_block)

    def parse_using_stmt(self) -> "UsingStmt":
        """`using <expr> as <name> { ... }` — fecha o recurso ao sair."""
        start = self.expect("KW", "using")
        resource = self.parse_expression()
        self.expect("KW", "as", "esperado 'as' após expressão de 'using'")
        var_name = self.parse_name_like("esperado nome de variável após 'as'")
        block = self.parse_block()
        return UsingStmt(line=start.line, col=start.col, resource=resource, var_name=var_name, block=block)

    def parse_model_decl(self) -> "ModelDecl":
        """`model NomeModel() { campo: tipo(length=N) ... }`"""
        start = self.expect("KW", "model")
        name_tok = self.expect(("IDENT", "IDENT_UPPER"), msg="esperado nome do model após 'model'")
        name = str(name_tok.value)
        self.expect("LPAREN", msg="esperado '(' após nome do model")
        self.expect("RPAREN", msg="esperado ')' após '('")
        self.skip_separators()
        self.expect("LBRACE", msg="esperado '{' para abrir o model")
        self.skip_separators()
        fields: list[ModelField] = []
        while self.current().type != "RBRACE":
            # campo: tipo(length=N)
            field_tok = self.expect(("IDENT", "IDENT_UPPER"), msg="esperado nome do campo")
            field_name = str(field_tok.value)
            self.expect("COLON", msg="esperado ':' após nome do campo")
            type_tok = self.current()
            if type_tok.type != "KW" or type_tok.value not in {"str", "int", "flo", "bool"}:
                raise self.error("tipo do campo deve ser str, int, flo ou bool", type_tok)
            type_name = str(type_tok.value)
            self.pos += 1
            length = None
            if self.match("LPAREN"):
                # length=N
                self.expect(("IDENT",), msg="esperado 'length'")
                self.expect("OP", "=", msg="esperado '=' após 'length'")
                len_tok = self.expect("INT", msg="esperado número após 'length='")
                length = int(len_tok.value)
                self.expect("RPAREN", msg="esperado ')' após o valor de length")
            self.consume_optional_semi()
            self.skip_separators()
            fields.append(ModelField(line=field_tok.line, col=field_tok.col,
                                     name=field_name, type_name=type_name, length=length))
        self.expect("RBRACE")
        return ModelDecl(line=start.line, col=start.col, name=name, fields=fields)


    def parse_entity_decl(self) -> "EntityDecl":
        """`Entity NomeClasse(Pai) { action __init__(self, ...) { } ... }`"""
        start = self.expect("KW", "Entity")
        name_tok = self.expect(("IDENT", "IDENT_UPPER"), msg="esperado nome da Entity após 'Entity'")
        name = str(name_tok.value)

        # herança opcional: Entity Filho(Pai) ou Entity Filho(Pai1, Pai2)
        parents: list[str] = []
        self.expect("LPAREN", msg="esperado '(' após nome da Entity")
        while self.current().type in {"IDENT", "IDENT_UPPER"}:
            parents.append(str(self.current().value))
            self.pos += 1
            if not self.match("COMMA"):
                break
        self.expect("RPAREN", msg="esperado ')' após herança da Entity")

        # corpo: aceita {} ou : + indentação
        self.skip_separators()
        body: list[Node] = []
        fields: list = []

        if self.match("LBRACE"):
            # estilo chaves
            self.skip_separators()
            while self.current().type not in {"RBRACE", "EOF"}:
                if self.current().type == "AT":
                    body.append(self.parse_decorator_stmt(capture_action=False))
                elif self._starts_action_decl():
                    body.append(self.parse_action_decl())
                elif self.current().type in {"IDENT", "IDENT_UPPER"}:
                    # campo tipado: nome: tipo [= default]
                    ef = self._parse_entity_field()
                    if ef: fields.append(ef)
                else:
                    raise self.error(
                        "dentro de Entity só são permitidas declarações 'action', decoradores ou campos 'nome: tipo'",
                        self.current()
                    )
                self.skip_separators()
            self.expect("RBRACE")

        elif self.match("COLON"):
            # estilo Python
            self.expect("NEWLINE", msg="faltou quebra de linha após ':'")
            self.expect("INDENT", msg="faltou indentação após ':'")
            self.skip_separators()
            while self.current().type not in {"DEDENT", "EOF"}:
                if self.current().type == "AT":
                    body.append(self.parse_decorator_stmt(capture_action=False))
                elif self._starts_action_decl():
                    body.append(self.parse_action_decl())
                elif self.current().type in {"IDENT", "IDENT_UPPER"}:
                    # campo tipado: nome: tipo [= default]
                    ef = self._parse_entity_field()
                    if ef: fields.append(ef)
                else:
                    raise self.error(
                        "dentro de Entity só são permitidas declarações 'action', decoradores ou campos 'nome: tipo'",
                        self.current()
                    )
                self.skip_separators()
            self.expect("DEDENT")
        else:
            raise self.error("esperado '{' ou ':' para abrir o corpo da Entity", self.current())

        return EntityDecl(line=start.line, col=start.col, name=name, parents=parents, body=body, fields=fields if fields else None)


    def _starts_action_decl(self) -> bool:
        """True se o token atual inicia uma declaração de action/reaction,
        considerando prefixos opcionais 'async' e tipo de retorno (int/bool/str/flo)."""
        tok = self.current()
        if tok.type != "KW":
            return False
        if tok.value in {"action", "reaction"}:
            return True
        if tok.value == "async" and self.pos + 1 < len(self.tokens):
            nxt = self.tokens[self.pos + 1]
            if nxt.type == "KW" and nxt.value in {"action", "reaction"}:
                return True
            if (nxt.type == "KW" and nxt.value in {"int", "bool", "str", "flo"}
                    and self.pos + 2 < len(self.tokens)
                    and self.tokens[self.pos + 2].type == "KW"
                    and self.tokens[self.pos + 2].value in {"action", "reaction"}):
                return True
        if (tok.value in {"int", "bool", "str", "flo"}
                and self.pos + 1 < len(self.tokens)
                and self.tokens[self.pos + 1].type == "KW"
                and self.tokens[self.pos + 1].value in {"action", "reaction"}):
            return True
        return False

    def _parse_entity_field(self):
        """Parseia `nome: tipo [= default]` dentro de Entity."""
        tok = self.current()
        if tok.type not in {"IDENT", "IDENT_UPPER"}:
            return None
        # lookahead: próximo token deve ser COLON
        if self.pos + 1 >= len(self.tokens) or self.tokens[self.pos + 1].type != "COLON":
            return None
        name_tok = self.tokens[self.pos]
        self.pos += 1  # consume nome
        self.pos += 1  # consume ':'
        # tipo
        type_tok = self.current()
        if type_tok.type not in {"IDENT", "IDENT_UPPER", "KW"}:
            raise self.error("esperado tipo após ':' no campo da Entity", type_tok)
        type_name = str(type_tok.value)
        self.pos += 1
        # default opcional
        default = None
        if self.current().type == "OP" and self.current().value == "=":
            self.pos += 1  # consume '='
            default = self.parse_expr()
        return EntityField(
            line=name_tok.line, col=name_tok.col,
            field_name=str(name_tok.value),
            type_name=type_name,
            default=default,
        )


    def parse_match_stmt(self) -> "MatchStmt":
        """`match valor: case ...:` ou `match valor { case ... { } }`"""
        start = self.expect("KW", "match")
        subject = self.parse_expression()
        self.skip_separators()
        cases: list = []

        if self.match("LBRACE"):
            # estilo {}
            self.skip_separators()
            while self.current().type != "RBRACE":
                if self.current().type == "KW" and self.current().value == "case":
                    cases.append(self._parse_match_case_brace())
                self.skip_separators()
            self.expect("RBRACE")
        else:
            # estilo :
            self.expect("COLON", msg="esperado ':' ou '{' após expressão do match")
            self.skip_separators()
            self.expect("INDENT", msg="esperado indentação após 'match:'")
            self.skip_separators()
            while self.current().type == "KW" and self.current().value == "case":
                cases.append(self._parse_match_case())
                self.skip_separators()
            self.expect("DEDENT")

        return MatchStmt(line=start.line, col=start.col, subject=subject, cases=cases)

    def _parse_match_case_brace(self) -> "MatchCase":
        """Case com estilo {}: case 200 { post(...) }"""
        start = self.expect("KW", "case")
        pattern = self._parse_match_pattern()
        guard = None
        if self.current().type == "KW" and self.current().value == "if":
            self.pos += 1
            guard = self.parse_expression()
        pattern.guard = guard
        self.skip_separators()
        body = self.parse_block()   # parse_block já lida com {}
        return MatchCase(line=start.line, col=start.col, pattern=pattern, body=body)

    def _parse_match_case(self) -> "MatchCase":
        start = self.expect("KW", "case")
        pattern = self._parse_match_pattern()
        # guard: if condition
        guard = None
        if self.current().type == "KW" and self.current().value == "if":
            self.pos += 1
            guard = self.parse_expression()
        pattern.guard = guard
        self.expect("COLON", msg="esperado ':' após padrão do case")
        self.skip_separators()
        # body: coleta statements até o próximo case/dedent/eof
        stmts = []
        if self.current().type == "INDENT":
            self.pos += 1  # consume INDENT
            self.skip_separators()
            while (self.current().type not in {"DEDENT", "EOF"}
                   and not (self.current().type == "KW" and self.current().value == "case")):
                stmts.append(self.parse_statement())
                self.skip_separators()
            if self.current().type == "DEDENT":
                self.pos += 1  # consume DEDENT
        else:
            stmts.append(self.parse_statement())
        body = Block(line=start.line, col=start.col, style="colon", statements=stmts)
        return MatchCase(line=start.line, col=start.col, pattern=pattern, body=body)

    def _parse_match_pattern(self) -> "MatchPattern":
        tok = self.current()

        # wildcard: _
        if tok.type == "IDENT" and tok.value == "_":
            self.pos += 1
            pat = MatchPattern(line=tok.line, col=tok.col, kind="wildcard")
            return self._maybe_or_pattern(pat)

        # lista: ["val1", val2]
        if tok.type == "LBRACK":
            self.pos += 1
            items = []
            while self.current().type != "RBRACK":
                items.append(self._parse_match_pattern())
                if not self.match("COMMA"):
                    break
            self.expect("RBRACK")
            pat = MatchPattern(line=tok.line, col=tok.col, kind="list", items=items)
            return self._maybe_or_pattern(pat)

        # dict: {chave: padrão}
        if tok.type == "LBRACE":
            self.pos += 1
            keys = {}
            while self.current().type != "RBRACE":
                key_tok = self.current()
                key = str(key_tok.value)
                self.pos += 1
                self.expect("COLON", msg="esperado ':' no padrão de dict")
                val_pat = self._parse_match_pattern()
                keys[key] = val_pat
                if not self.match("COMMA"):
                    break
            self.expect("RBRACE")
            pat = MatchPattern(line=tok.line, col=tok.col, kind="dict", keys=keys)
            return self._maybe_or_pattern(pat)

        # literal: string, int, float, bool, Null
        if tok.type in {"STR", "INT", "FLO", "BOOL", "NULL"}:
            self.pos += 1
            pat = MatchPattern(line=tok.line, col=tok.col, kind="value", value=tok.value)
            return self._maybe_or_pattern(pat)

        # captura com nome: e, resultado, nome etc.
        if tok.type == "IDENT":
            self.pos += 1
            pat = MatchPattern(line=tok.line, col=tok.col, kind="capture", name=str(tok.value))
            return self._maybe_or_pattern(pat)

        # valor negativo: -10
        if tok.type == "OP" and tok.value == "-":
            self.pos += 1
            num = self.current()
            self.pos += 1
            pat = MatchPattern(line=tok.line, col=tok.col, kind="value", value=-num.value)
            return self._maybe_or_pattern(pat)

        raise self.error("padrão de case inválido", tok)

    def _maybe_or_pattern(self, pat: "MatchPattern") -> "MatchPattern":
        """Verifica se há | após o padrão — combina em MatchPattern(kind='or')."""
        if self.current().type == "OP" and self.current().value == "|":
            patterns = [pat]
            while self.current().type == "OP" and self.current().value == "|":
                self.pos += 1
                patterns.append(self._parse_match_pattern())
            return MatchPattern(line=pat.line, col=pat.col, kind="or", patterns=patterns)
        return pat

    def parse_run_selfwith(self) -> "RunSelfWithStmt":
        """`run_selfwith_("label") { ... }` — ponto de entrada do programa."""
        start = self.expect("IDENT")  # consome run_selfwith_
        self.expect("LPAREN", msg="esperado '(' após run_selfwith_")
        label_tok = self.current()
        if label_tok.type != "STR":
            raise self.error("run_selfwith_ espera um nome entre aspas, ex: run_selfwith_(\"main\")", label_tok)
        label = str(label_tok.value)
        self.pos += 1
        self.expect("RPAREN", msg="esperado ')' após o nome em run_selfwith_")
        block = self.parse_block()
        return RunSelfWithStmt(line=start.line, col=start.col, label=label, block=block)

    # ── Operador count ──────────────────────────────────────────────────
    def _parse_count_type_and_value(self) -> tuple[str, Node | None]:
        """Lê `<type>` ou `<type>(<value>)` para o operador count."""
        type_tok = self.current()
        if type_tok.type != "KW" or type_tok.value not in COUNT_TYPE_KEYWORDS:
            raise self.error(
                "esperado tipo (str, int, flo, bool, list, json, char) após 'count'",
                type_tok,
            )
        self.pos += 1
        target_type = str(type_tok.value)
        value_node: Node | None = None
        if self.match("LPAREN"):
            if self.current().type == "RPAREN":
                value_node = None
            else:
                value_node = self.parse_expression()
            self.expect("RPAREN", msg="faltou ')' no valor de 'count'")
        return target_type, value_node

    def parse_count_prefix(self) -> "CountExpr":
        """`count <type>[(<value>)] in <container>`"""
        start = self.expect("KW", "count")
        target_type, value_node = self._parse_count_type_and_value()
        self.expect("KW", "in", "esperado 'in' após o tipo de 'count'")
        container = self.parse_add()
        mode = "prefix_value" if value_node is not None else "prefix_type"
        return CountExpr(
            line=start.line, col=start.col,
            target_type=target_type, value_node=value_node,
            container=container, mode=mode,
        )

    def parse_count_each_stmt(self) -> "CountEachStmt":
        """`count each <type>[(<value>)] in <container> { ... }`

        Se NÃO houver bloco `{`/`:` na sequência, isto NÃO é statement —
        o chamador deve tratar como expressão via `parse_count_each_expr`.
        """
        start = self.expect("KW", "count")
        self.expect("KW", "each", "esperado 'each' após 'count' no statement")
        target_type, value_node = self._parse_count_type_and_value()
        self.expect("KW", "in", "esperado 'in' após o tipo de 'count each'")
        container = self.parse_add()
        block = self.parse_block()
        return CountEachStmt(
            line=start.line, col=start.col,
            target_type=target_type, value_node=value_node,
            container=container, block=block,
        )

    def _count_each_has_block(self) -> bool:
        """Olha à frente para decidir se `count each ...` termina em `{`/`:`.

        Não consome tokens. Faz um scan mínimo a partir do `count` atual.
        """
        i = self.pos
        toks = self.tokens
        # Esperado: count each <tipo> [( ... )] in <expr> { ou :
        # Pulamos count, each, tipo
        if i + 2 >= len(toks):
            return False
        i += 2  # count, each
        if i >= len(toks) or toks[i].type != "KW":
            return False
        i += 1  # tipo
        # Possível (...) — pulamos balanceado
        if i < len(toks) and toks[i].type == "LPAREN":
            depth = 1
            i += 1
            while i < len(toks) and depth > 0:
                if toks[i].type == "LPAREN":
                    depth += 1
                elif toks[i].type == "RPAREN":
                    depth -= 1
                i += 1
        # Esperado 'in'
        if i >= len(toks) or not (toks[i].type == "KW" and toks[i].value == "in"):
            return False
        i += 1
        # Avançamos até encontrar NEWLINE/SEMI/EOF/LBRACE/COLON, ignorando
        # parênteses/colchetes balanceados internos do container.
        depth = 0
        while i < len(toks):
            t = toks[i]
            if depth == 0:
                if t.type == "LBRACE":
                    return True
                if t.type == "COLON":
                    return True
                if t.type in {"NEWLINE", "SEMI", "EOF", "RBRACE", "DEDENT"}:
                    return False
            if t.type in {"LPAREN", "LBRACK"}:
                depth += 1
            elif t.type in {"RPAREN", "RBRACK"}:
                if depth == 0:
                    return False
                depth -= 1
            i += 1
        return False

    def parse_count_each_expr(self) -> "CountEachExpr":
        """`count each <type>[(<value>)] in <container>` (sem bloco) → expressão."""
        start = self.expect("KW", "count")
        self.expect("KW", "each", "esperado 'each' após 'count'")
        target_type, value_node = self._parse_count_type_and_value()
        self.expect("KW", "in", "esperado 'in' após o tipo de 'count each'")
        container = self.parse_add()
        return CountEachExpr(
            line=start.line, col=start.col,
            target_type=target_type, value_node=value_node,
            container=container,
        )

    def _reject_count_equality(self, start: Token) -> None:
        """Garante que `count ... == X` (e variantes) NÃO seja aceito.

        O operador `count` da PoolScript não compara quantidade com número.
        Use o resultado direto (atribuição/post) ou `if (count ... in ...)`
        para testar existência.
        """
        tok = self.current()
        if tok.type == "OP" and tok.value in COMPARE_OPS:
            raise self.error(
                "'count' não usa comparação ('==', '!=', etc.). "
                "Use 'nome = count each <tipo> in <alvo>' ou "
                "'if (count <tipo> in <alvo>) { ... }'.",
                tok,
            )

    def parse_import_stmt(self) -> ImportStmt:
        start = self.current()
        if self.match("KW", "import"):
            module = self.parse_module_path()
            module_alias = None
            if self.match("KW", "as"):
                module_alias = self.parse_name_like("esperado nome após 'as'")
            self.consume_optional_semi()
            return ImportStmt(
                line=start.line, col=start.col, mode="import",
                module=module, names=[], module_alias=module_alias,
            )
        if self.match("KW", "from"):
            module = self.parse_module_path()
            self.expect("KW", "import")
            names, name_aliases = self.parse_name_list_with_aliases()
            self.consume_optional_semi()
            return ImportStmt(
                line=start.line, col=start.col, mode="from",
                module=module, names=names, name_aliases=name_aliases,
            )
        self.expect("KW", "PUSH")
        module = self.parse_module_path()
        module_alias = None
        if self.match("KW", "as"):
            module_alias = self.parse_name_like("esperado nome após 'as'")
        names: list[str] = []
        name_aliases: dict[str, str] = {}
        if self.match("KW", "GET"):
            names, name_aliases = self.parse_name_list_with_aliases()
        self.consume_optional_semi()
        return ImportStmt(
            line=start.line, col=start.col, mode="push",
            module=module, names=names, module_alias=module_alias,
            name_aliases=name_aliases,
        )

    def parse_name_list_with_aliases(self) -> tuple[list[str], dict[str, str]]:
        names: list[str] = []
        aliases: dict[str, str] = {}
        first = self.parse_name_like("esperado nome importado")
        names.append(first)
        if self.match("KW", "as"):
            aliases[first] = self.parse_name_like("esperado nome após 'as'")
        while self.match("COMMA"):
            n = self.parse_name_like("esperado nome após ','")
            names.append(n)
            if self.match("KW", "as"):
                aliases[n] = self.parse_name_like("esperado nome após 'as'")
        return names, aliases

    def parse_decorator_stmt(self, capture_action: bool = True) -> DecoratorStmt:
        start = self.expect("AT")
        decorator = self.parse_decorator_call(start)
        block = None
        self.skip_separators()
        if self.current().type in {"COLON", "LBRACE"}:
            block = self.parse_block()
        elif capture_action and self.current().type == "KW" and self.current().value in {"action", "reaction"}:
            # @NonNull action foo(...) { } — captura a action como bloco single-node
            action_node = self.parse_action_decl()
            from dataclasses import fields
            block = Block(line=action_node.line, col=action_node.col, style="brace", statements=[action_node])
        elif capture_action and self.current().type == "KW" and self.current().value == "async":
            # @NonNull async action foo(...) / @NonNull async int reaction foo(...)
            nxt = self.tokens[self.pos + 1] if self.pos + 1 < len(self.tokens) else None
            is_async_action = (
                nxt is not None and nxt.type == "KW" and nxt.value in {"action", "reaction"}
            ) or (
                nxt is not None and nxt.type == "KW" and nxt.value in {"int", "bool", "str", "flo"}
                and self.pos + 2 < len(self.tokens)
                and self.tokens[self.pos + 2].type == "KW"
                and self.tokens[self.pos + 2].value in {"action", "reaction"}
            )
            if is_async_action:
                action_node = self.parse_action_decl()
                block = Block(line=action_node.line, col=action_node.col, style="brace", statements=[action_node])
        return DecoratorStmt(line=start.line, col=start.col, decorator=decorator, block=block)

    def parse_decorator_call(self, at_tok: Token) -> DecoratorCall:
        path = [self.parse_name_like("esperado nome após '@'")]
        while self.match("DOT"):
            # Após '.', qualquer token word-like é válido como nome de método
            tok = self.current()
            if tok.type in {"IDENT", "IDENT_UPPER", "KW"}:
                self.pos += 1
                path.append(str(tok.value))
            else:
                raise self.error("esperado nome após '.'", tok)
        args: list[CallArg] = []
        if self.match("LPAREN"):
            if self.current().type != "RPAREN":
                args = self.parse_call_args_until("RPAREN")
            self.expect("RPAREN")
        return DecoratorCall(line=at_tok.line, col=at_tok.col, path=path, args=args)

    def parse_module_path(self) -> list[str]:
        parts = [self.parse_name_like("esperado caminho de módulo")]
        while self.match("DOT"):
            parts.append(self.parse_name_like("esperado identificador após '.' no módulo"))
        return parts

    def parse_name_list(self) -> list[str]:
        names = [self.parse_name_like("esperado nome importado")]
        while self.match("COMMA"):
            names.append(self.parse_name_like("esperado nome após ','"))
        return names

    def parse_name_like(self, msg: str) -> str:
        """Aceita IDENT, IDENT_UPPER ou qualquer KW como nome de membro.
        Keywords como 'count', 'type', 'input' são válidas como atributos de objeto.
        """
        tok = self.current()
        if tok.type in {"IDENT", "IDENT_UPPER", "KW"}:
            self.pos += 1
            return str(tok.value)
        raise self.error(msg, tok)

    def parse_condition(self, allow_parenthesized_only: bool = True) -> Node:
        if self.match("LPAREN"):
            expr = self.parse_expression()
            self.expect("RPAREN", msg="faltou ')' na condição")
            return expr
        return self.parse_expression()

    def parse_block(self) -> Block:
        tok = self.current()
        if self.match("LBRACE"):
            statements: list[Node] = []
            self.skip_separators()
            while self.current().type not in {"RBRACE", "EOF"}:
                statements.append(self.parse_statement())
                self.skip_separators()
            if self.current().type == "EOF":
                raise self.error("bloco com '{' não foi fechado com '}'", tok)
            self.expect("RBRACE")
            return Block(line=tok.line, col=tok.col, style="brace", statements=statements)
        if self.match("COLON"):
            self.expect("NEWLINE", msg="faltou quebra de linha após ':'")
            self.expect("INDENT", msg="faltou indentação após ':'")
            statements: list[Node] = []
            self.skip_separators()
            while self.current().type not in {"DEDENT", "EOF"}:
                statements.append(self.parse_statement())
                self.skip_separators()
            if self.current().type == "EOF":
                raise self.error("bloco indentado não foi fechado corretamente", self.current())
            self.expect("DEDENT")
            return Block(line=tok.line, col=tok.col, style="colon", statements=statements)
        raise self.error("esperado início de bloco com '{' ou ':'", tok)

    def parse_expression(self) -> Node:
        return self.parse_or()

    def parse_or(self) -> Node:
        node = self.parse_and()
        while (self.current().type == "KW" and self.current().value == "or") or (self.current().type == "OP" and self.current().value == "||"):
            op = self.current()
            self.pos += 1
            node = BinaryOp(line=op.line, col=op.col, left=node, operator=str(op.value), right=self.parse_and())
        return node

    def parse_and(self) -> Node:
        node = self.parse_not()
        while (self.current().type == "KW" and self.current().value == "and") or (self.current().type == "OP" and self.current().value == "&&"):
            op = self.current()
            self.pos += 1
            node = BinaryOp(line=op.line, col=op.col, left=node, operator=str(op.value), right=self.parse_not())
        return node

    def parse_not(self) -> Node:
        tok = self.current()
        if (tok.type == "KW" and tok.value in {"not", "Not"}) or (tok.type == "OP" and tok.value == "!"):
            self.pos += 1
            return UnaryOp(line=tok.line, col=tok.col, operator=str(tok.value), operand=self.parse_not())
        return self.parse_compare()

    def parse_compare(self) -> Node:
        node = self.parse_add()
        while True:
            tok = self.current()
            # Forma infixa do count: `<type>(<value>) count in <container>`
            if tok.type == "KW" and tok.value == "count" and self.peek().type == "KW" and self.peek().value == "in":
                target_type, value_node = self._extract_count_left(node, tok)
                self.pos += 1  # consome 'count'
                self.pos += 1  # consome 'in'
                container = self.parse_add()
                node = CountExpr(
                    line=tok.line, col=tok.col,
                    target_type=target_type, value_node=value_node,
                    container=container, mode="infix",
                )
                continue
            if tok.type == "OP" and tok.value in COMPARE_OPS:
                self.pos += 1
                node = BinaryOp(line=tok.line, col=tok.col, left=node, operator=str(tok.value), right=self.parse_add())
                continue
            if tok.type == "KW" and tok.value in {"is", "in"}:
                self.pos += 1
                if tok.value == "is" and self.current().type == "KW" and self.current().value in {"not", "Not"}:
                    self.pos += 1
                    node = BinaryOp(line=tok.line, col=tok.col, left=node, operator="is not", right=self.parse_add())
                else:
                    node = BinaryOp(line=tok.line, col=tok.col, left=node, operator=str(tok.value), right=self.parse_add())
                # Forma sufixa do count: `<tipo> in <container> count`
                # (ex.: `if (int in lista_x count) { ... }`)
                if (tok.value == "in" and self.current().type == "KW"
                        and self.current().value == "count"):
                    count_tok = self.current()
                    target_type, value_node = self._extract_count_left(
                        node.left, count_tok,
                    )
                    container = node.right
                    self.pos += 1  # consome 'count'
                    node = CountExpr(
                        line=count_tok.line, col=count_tok.col,
                        target_type=target_type, value_node=value_node,
                        container=container, mode="suffix",
                    )
                continue
            if tok.type == "KW" and tok.value in {"not", "Not"} and self.peek().type == "KW" and self.peek().value in {"is", "in"}:
                self.pos += 1
                next_tok = self.current()
                self.pos += 1
                op = "is not" if next_tok.value == "is" else "not in"
                node = BinaryOp(line=tok.line, col=tok.col, left=node, operator=op, right=self.parse_add())
                continue
            break
        return node

    def parse_add(self) -> Node:
        node = self.parse_mul()
        while self.current().type == "OP" and self.current().value in {"+", "-"}:
            tok = self.current()
            self.pos += 1
            node = BinaryOp(line=tok.line, col=tok.col, left=node, operator=str(tok.value), right=self.parse_mul())
        return node

    def parse_mul(self) -> Node:
        node = self.parse_unary()
        while self.current().type == "OP" and self.current().value in {"*", "/", "%"}:
            tok = self.current()
            self.pos += 1
            node = BinaryOp(line=tok.line, col=tok.col, left=node, operator=str(tok.value), right=self.parse_unary())
        return node

    def parse_unary(self) -> Node:
        tok = self.current()
        if tok.type == "OP" and tok.value in {"+", "-"}:
            self.pos += 1
            return UnaryOp(line=tok.line, col=tok.col, operator=str(tok.value), operand=self.parse_unary())
        if tok.type == "KW" and tok.value == "await":
            self.pos += 1
            return AwaitExpr(line=tok.line, col=tok.col, value=self.parse_unary())
        return self.parse_postfix()

    def parse_postfix(self) -> Node:
        node = self.parse_primary()
        while True:
            if self.match("LPAREN"):
                args = []
                if self.current().type != "RPAREN":
                    args = self.parse_call_args_until("RPAREN")
                end = self.expect("RPAREN", msg="faltou ')' na chamada")
                node = Call(line=end.line, col=end.col, callee=node, args=args)
                continue
            if self.match("DOT"):
                member = self.parse_name_like("esperado membro após '.'")
                node = MemberAccess(line=node.line, col=node.col, target=node, member=member)
                continue
            if self.match("LBRACK"):
                # Detecta slice: [start:stop:step]
                # Se o primeiro token for COLON, start é None
                start = None
                stop = None
                step = None
                is_slice = False

                if self.current().type == "COLON":
                    # [:stop] ou [:stop:step] ou [::step]
                    is_slice = True
                    self.pos += 1
                    if self.current().type not in {"RBRACK", "COLON"}:
                        stop = self.parse_expression()
                    if self.match("COLON"):
                        if self.current().type != "RBRACK":
                            step = self.parse_expression()
                else:
                    first = self.parse_expression()
                    if self.current().type == "COLON":
                        # [start:stop] ou [start:stop:step] ou [start:]
                        is_slice = True
                        start = first
                        self.pos += 1
                        if self.current().type not in {"RBRACK", "COLON"}:
                            stop = self.parse_expression()
                        if self.match("COLON"):
                            if self.current().type != "RBRACK":
                                step = self.parse_expression()
                    else:
                        first_val = first

                self.expect("RBRACK", msg="faltou ']'")

                if is_slice:
                    node = SliceAccess(line=node.line, col=node.col,
                                       target=node, start=start, stop=stop, step=step)
                else:
                    node = IndexAccess(line=node.line, col=node.col, target=node, index=first_val)
                continue
            if self.current().type == "OP" and self.current().value in {"++", "--"}:
                tok = self.current()
                self.pos += 1
                node = PostfixOp(line=tok.line, col=tok.col, operand=node, operator=str(tok.value))
                continue
            break
        return node

    # Tokens que podem iniciar uma expressão (usado para detectar args justapostos sem vírgula)
    _EXPR_START_TYPES = {
        "INT", "FLO", "STR", "FSTRING", "BOOL", "NULL",
        "IDENT", "IDENT_UPPER", "LPAREN", "LBRACK", "LBRACE",
    }

    def _can_start_expression(self, tok: Token) -> bool:
        if tok.type in self._EXPR_START_TYPES:
            return True
        if tok.type == "KW" and tok.value in EXPR_NAME_KEYWORDS:
            return True
        if tok.type == "OP" and tok.value in {"+", "-", "!"}:
            return True
        if tok.type == "KW" and tok.value in {"not", "Not"}:
            return True
        return False

    def parse_call_args_until(self, closing_type: str) -> list[CallArg]:
        args: list[CallArg] = []
        self.skip_separators()
        while self.current().type != closing_type:
            if self.current().type == "IDENT" and self.peek().type == "OP" and self.peek().value == "=":
                name = str(self.expect("IDENT").value)
                eq_tok = self.expect("OP", "=")
                value = self.parse_expression()
                args.append(CallArg(line=eq_tok.line, col=eq_tok.col, name=name, value=value))
            else:
                expr = self.parse_expression()
                args.append(CallArg(line=expr.line, col=expr.col, value=expr))
            self.skip_separators()
            if self.match("COMMA"):
                self.skip_separators()
                continue
            if self.current().type == closing_type:
                break
            if self._can_start_expression(self.current()):
                continue
            break
        return args

    def parse_primary(self) -> Node:
        tok = self.current()
        if tok.type in {"INT", "FLO", "BOOL", "NULL"}:
            self.pos += 1
            return Literal(line=tok.line, col=tok.col, value=tok.value, kind=tok.type)
        if tok.type == "FSTRING":
            self.pos += 1
            return Literal(line=tok.line, col=tok.col, value=tok.value, kind="FSTRING")
        if tok.type == "COLOR":
            self.pos += 1
            str_expr = self.parse_primary()
            return ColorStrExpr(color=tok.value, expr=str_expr, line=tok.line, col=tok.col)
        if tok.type == "STR":
            return self.parse_string_like()
        # Forma prefixa do count em expressão: `count <type>[(<value>)] in <container>`
        if tok.type == "KW" and tok.value == "count":
            # `count each <tipo> in <alvo>` (sem bloco) também é expressão.
            if self.peek().type == "KW" and self.peek().value == "each":
                return self.parse_count_each_expr()
            return self.parse_count_prefix()
        # Lambda: action(params) { ... } como expressão
        if tok.type == "KW" and tok.value in {"action", "reaction"} and self.peek().type == "LPAREN":
            self.pos += 1
            self.expect("LPAREN")
            params = []
            while self.current().type != "RPAREN":
                p = self.expect(("IDENT", "IDENT_UPPER"), msg="esperado nome de parâmetro")
                params.append(str(p.value))
                if not self.match("COMMA"):
                    break
            self.expect("RPAREN")
            block = self.parse_block()
            return LambdaExpr(line=tok.line, col=tok.col, params=params, block=block)
        if tok.type == "KW" and tok.value in TYPE_KEYWORDS and self.peek().type != "LPAREN":
            self.pos += 1
            return TypeName(line=tok.line, col=tok.col, name=str(tok.value))
        if tok.type == "KW" and tok.value in TYPE_KEYWORDS and self.peek().type == "LPAREN":
            self.pos += 1
            return Name(line=tok.line, col=tok.col, value=str(tok.value))
        # base(NomePai, args...) ou base(args...) — chama __init__ do pai
        if tok.type == "IDENT" and tok.value == "base":
            self.pos += 1
            self.expect("LPAREN", msg="esperado '(' após 'base'")
            # detecta se primeiro arg é um IDENT_UPPER (nome de Entity pai)
            target_parent = None
            if (self.current().type == "IDENT_UPPER"
                    and self.peek().type == "COMMA"):
                target_parent = str(self.current().value)
                self.pos += 1  # consume nome do pai
                self.pos += 1  # consume COMMA
            args: list[CallArg] = []
            if self.current().type != "RPAREN":
                while True:
                    arg_val = self.parse_expression()
                    if (isinstance(arg_val, Name)
                            and self.current().type == "OP"
                            and self.current().value == "="
                            and self.peek().type not in {"COMMA", "RPAREN"}):
                        self.pos += 1
                        kv = self.parse_expression()
                        args.append(CallArg(line=tok.line, col=tok.col, value=kv, name=arg_val.value))
                    else:
                        args.append(CallArg(line=tok.line, col=tok.col, value=arg_val))
                    if not self.match("COMMA"):
                        break
            self.expect("RPAREN", msg="faltou ')' em base()")
            return BaseCall(line=tok.line, col=tok.col, args=args, target=target_parent)

        # to int / to float — argumento de tipo para Parsing.string(...)
        if tok.type == "KW" and tok.value == "to":
            self.pos += 1
            type_tok = self.current()
            if type_tok.type not in {"IDENT", "KW"} or type_tok.value not in ("int", "float", "str", "flo", "bool", "json", "list", "tup", "dict"):
                raise self.error("esperado 'int', 'float' ou 'str' após 'to'", type_tok)
            self.pos += 1
            return Literal(line=tok.line, col=tok.col, value=str(type_tok.value), kind="str")

        # match(...) como função (ex: regex match) — KW seguido de LPAREN
        if tok.type == "KW" and tok.value == "match" and self.peek().type == "LPAREN":
            self.pos += 1
            return Name(line=tok.line, col=tok.col, value="match")

        # base(...) — chama __init__ do pai (IDENT pois não é keyword global)
        if tok.type == "IDENT" and tok.value == "base":
            self.pos += 1
            self.expect("LPAREN", msg="esperado '(' após 'base'")
            args: list[CallArg] = []
            if self.current().type != "RPAREN":
                while True:
                    arg_val = self.parse_expression()
                    # kwargs: nome=valor
                    if (isinstance(arg_val, Name)
                            and self.current().type == "OP"
                            and self.current().value == "="
                            and self.peek().type not in {"COMMA", "RPAREN"}):
                        self.pos += 1
                        kv = self.parse_expression()
                        args.append(CallArg(line=tok.line, col=tok.col, value=kv, name=arg_val.value))
                    else:
                        args.append(CallArg(line=tok.line, col=tok.col, value=arg_val))
                    if not self.match("COMMA"):
                        break
            self.expect("RPAREN", msg="faltou ')' em base()")
            return BaseCall(line=tok.line, col=tok.col, args=args)
        if tok.type in {"IDENT", "IDENT_UPPER"} or (tok.type == "KW" and tok.value in EXPR_NAME_KEYWORDS):
            self.pos += 1
            return Name(line=tok.line, col=tok.col, value=str(tok.value))
        if self.match("LPAREN"):
            lparen_tok = self.tokens[self.pos - 1]   # salva posição do '('
            # Tupla vazia ()
            if self.current().type == "RPAREN":
                self.pos += 1
                return TupleLiteral(line=tok.line, col=tok.col, items=[])
            try:
                expr = self.parse_expression()
            except PoolParseError:
                raise self.error(
                    f"expressão incompleta — faltou ')' para fechar '(' da linha {lparen_tok.line}, coluna {lparen_tok.col}",
                    lparen_tok
                )
            # Se vier vírgula depois da primeira expr → é tupla
            if self.match("COMMA"):
                items = [expr]
                while self.current().type != "RPAREN":
                    items.append(self.parse_expression())
                    if not self.match("COMMA"):
                        break
                self.expect("RPAREN", msg=f"faltou ')' para fechar '(' da linha {lparen_tok.line}, coluna {lparen_tok.col}", token=lparen_tok)
                return TupleLiteral(line=tok.line, col=tok.col, items=items)
            self.expect("RPAREN", msg=f"faltou ')' para fechar '(' da linha {lparen_tok.line}, coluna {lparen_tok.col}", token=lparen_tok)
            return expr
        if self.match("LBRACK"):
            return self.parse_list_literal(tok)
        if self.match("LBRACE"):
            return self.parse_dict_literal(tok)
        raise self.error("expressão inválida", tok)

    def parse_string_like(self) -> Node:
        start = self.expect("STR")
        parts: list[Node] = [Literal(line=start.line, col=start.col, value=start.value, kind="STR")]
        if self.current().type != "LBRACE":
            return parts[0]
        while self.match("LBRACE"):
            expr = self.parse_expression()
            self.expect("RBRACE", msg="faltou '}' na interpolação")
            parts.append(expr)
            if self.current().type == "STR":
                str_tok = self.expect("STR")
                parts.append(Literal(line=str_tok.line, col=str_tok.col, value=str_tok.value, kind="STR"))
        return InterpolatedString(line=start.line, col=start.col, parts=parts)

    def _extract_count_left(self, node: Node, tok: Token) -> tuple[str, Node | None]:
        """Extrai (tipo, valor) do lado esquerdo do `count` infixo.

        Aceita:
          - `tipo(valor)` → Call(Name(tipo), [valor])
          - `tipo(...)`  → Call(Name(tipo), [valor])
          - `tipo`       → Name(tipo) ou TypeName(tipo)
        """
        # tipo(valor)
        if isinstance(node, Call) and isinstance(node.callee, Name) and node.callee.value in TYPE_KEYWORDS:
            if any(arg.name is not None for arg in node.args):
                raise self.error("'count' não aceita argumentos nomeados no valor", tok)
            if len(node.args) == 0:
                return node.callee.value, None
            if len(node.args) > 1:
                raise self.error("'count' aceita no máximo um valor entre parênteses", tok)
            return node.callee.value, node.args[0].value
        # tipo (sem parênteses)
        if isinstance(node, TypeName):
            return node.name, None
        if isinstance(node, Name) and node.value in TYPE_KEYWORDS:
            return node.value, None
        raise self.error(
            "lado esquerdo de 'count in' deve ser um tipo (ex: int(7) count in lista)",
            tok,
        )

    def parse_list_literal(self, start_tok: Token) -> ListLiteral:
        items: list[Node] = []
        if self.current().type != "RBRACK":
            while True:
                items.append(self.parse_expression())
                if not self.match("COMMA"):
                    break
        self.expect("RBRACK", msg="faltou ']' na lista")
        return ListLiteral(line=start_tok.line, col=start_tok.col, items=items)

    def parse_dict_literal(self, start_tok: Token) -> DictLiteral:
        entries: list[DictEntry] = []
        self.skip_separators()
        if self.current().type != "RBRACE":
            while True:
                self.skip_separators()
                key_tok = self.current()
                if key_tok.type == "STR":
                    key: Node = Literal(line=key_tok.line, col=key_tok.col, value=key_tok.value, kind="STR")
                    self.pos += 1
                elif key_tok.type in {"IDENT", "IDENT_UPPER"}:
                    key = Name(line=key_tok.line, col=key_tok.col, value=str(key_tok.value))
                    self.pos += 1
                else:
                    raise self.error("chave de dicionário deve ser string ou identificador", key_tok)
                colon = self.expect("COLON", msg="faltou ':' entre chave e valor do dicionário")
                value = self.parse_expression()
                entries.append(DictEntry(line=colon.line, col=colon.col, key=key, value=value))
                self.skip_separators()
                if not self.match("COMMA"):
                    break
                self.skip_separators()
        self.expect("RBRACE", msg="faltou '}' no dicionário")
        return DictLiteral(line=start_tok.line, col=start_tok.col, entries=entries)



def parse_source(source: str, filename: str = "<stdin>") -> Program:
    import sys as _sys
    lexer = Lexer(source, filename)
    tokens = lexer.tokenize()
    for w in lexer.warnings:
        print(w, file=_sys.stderr)
    return Parser(tokens, source=source, filename=filename).parse()


if __name__ == "__main__":
    import sys
    from collections import Counter

    if len(sys.argv) < 2:
        print("uso: python parser.py <arquivo.ps>")
        raise SystemExit(1)

    path = sys.argv[1]
    with open(path, "r", encoding="utf-8") as f:
        source = f.read()

    try:
        program = parse_source(source, path)
    except (PoolSyntaxError, PoolParseError) as exc:
        print(exc)
        raise SystemExit(2)

    counts = Counter(type(stmt).__name__ for stmt in program.statements)
    print(f"OK: {len(program.statements)} statements")
    for name, total in sorted(counts.items()):
        print(f"- {name}: {total}")
