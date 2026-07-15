"""
PoolScript - Lexer (Tokenizer)
Etapa 2 do interpretador. Lê código-fonte .ps e produz uma lista de tokens.

Estratégia:
  - Varredura caractere-a-caractere com regex auxiliares (lib `re`).
  - Mantém line/column para mensagens de erro estilo Python (^^^).
  - Emite INDENT/DEDENT estilo Python para blocos com `:`.
  - Blocos com `{}` ignoram indentação (modo "brace").
  - Comentários // e \"\"\" ... \"\"\" são descartados (não viram tokens).
"""
from __future__ import annotations
from dataclasses import dataclass
from typing import Iterator
import re
import sys
from .ps_errors import _BasePoolSyntaxError as _PSBaseSyntaxError

# ─────────────────────────────────────────────────────────────────────
# 1. Definição de Token
# ─────────────────────────────────────────────────────────────────────

@dataclass
class Token:
    type: str          # ex: "IDENT", "INT", "STR", "KW", "OP", "LBRACE"...
    value: object      # valor já convertido (int, float, str, bool, None) ou lexema
    line: int          # 1-based
    col: int           # 1-based
    lexeme: str = ""   # texto original (útil para mensagens de erro)

    def __repr__(self) -> str:
        v = repr(self.value) if self.value is not None else "None"
        return f"Token({self.type:<10} {v:<20} L{self.line}:C{self.col})"


# ─────────────────────────────────────────────────────────────────────
# 2. Palavras reservadas (espelha a EBNF)
# ─────────────────────────────────────────────────────────────────────

KEYWORDS = {
    # fluxo
    "if", "else", "elif", "while", "for", "each", "in", "is",
    "and", "or", "not", "Not",
    "action", "reaction", "return", "continue", "break", "model", "async", "await",
    "try", "catch", "as", "with", "of", "using",
    "import", "from", "PUSH", "GET",
    # tipos primitivos
    "str", "int", "flo", "bool",
    # built-ins / palavras da spec
    "post", "input", "listen", "route", "create",
    "clear", "space", "addEnd", "char", "list",
    "Class", "class", "type",
    # Entity (classes)
    "Entity", "self",
    # Match/case
    "match", "case",
    # Generator
    "yield",
    # Exceptions
    "raise", "finally", "reaction",
    # Parsing type target
    "to",
    # "base" NÃO é keyword global — tratado contextualmente no parser
    # operador `count` (v0.5.2)
    "count",
    # `global` — declara nome como referência à variável global (estilo Python)
    "global",
    # HTTP/JSON (também reservadas)
    "POST", "PUT", "DELETE", "JSON", "json",
    # manpu palavras reservadas
    "full", "mei",
}
BOOL_LITERALS = {"True", "true", "False", "false"}
NULL_LITERALS = {"Null", "null", "None", "none"}

# Operadores de múltiplos caracteres — ordem importa (mais longo primeiro)
MULTI_OPS = [
    "===", "!==",
    "==", "!=", "<=", ">=",
    "&&", "||",
    "+=", "-=", "*=", "/=", "%=",
    "++", "--",
]
SINGLE_OPS = set("+-*/%=<>!.,@|")

# ─── Cores nomeadas para sintaxe <color>"texto" ───────────────────────────────
NAMED_COLORS: dict[str, str] = {
    "red":     "FF3B30", "green":   "34C759", "blue":    "2196F3",
    "yellow":  "FFD60A", "cyan":    "5AC8FA", "magenta": "FF2D55",
    "white":   "FFFFFF", "black":   "000000", "purple":  "AF52DE",
    "orange":  "FF9500", "pink":    "FF2D55", "gray":    "8E8E93",
    "grey":    "8E8E93", "lime":    "30D158", "teal":    "5AC8FA",
}
_COLOR_RE = re.compile(r'<([A-Za-z0-9]{1,7})>')


# Caracteres de pontuação/estrutura (cada um vira um tipo de token próprio)
PUNCT = {
    "(": "LPAREN", ")": "RPAREN",
    "{": "LBRACE", "}": "RBRACE",
    "[": "LBRACK", "]": "RBRACK",
    ":": "COLON", ";": "SEMI",
    ",": "COMMA", ".": "DOT", "@": "AT",
}


# ─────────────────────────────────────────────────────────────────────
# 3. Erros do Lexer (já no formato da tabela de erros da PoolScript)
# ─────────────────────────────────────────────────────────────────────

class PoolSyntaxError(_PSBaseSyntaxError):
    """Erro de sintaxe estilo Python com indicador ^^^."""
    def __init__(self, msg: str, line: int, col: int, source_line: str = "", filename: str = "<script>"):
        self.msg = msg
        self.line = line
        self.col = col
        self.source_line = source_line
        self.filename = filename
        # bypass base __init__ since our signature differs
        Exception.__init__(self, self.format())

    def format(self) -> str:
        pointer = " " * max(0, self.col - 1) + "^^^"
        src = self.source_line.rstrip("\n") if self.source_line else ""
        fname = self.filename if hasattr(self, "filename") and self.filename else "<script>"
        return (
            f"\033[1m\033[31mSyntaxError\033[0m: {self.msg}\n"
            f"  \033[2m→ {fname}, linha {self.line}, coluna {self.col}\033[0m\n"
            f"  | {src}\n"
            f"  | \033[33m{pointer}\033[0m"
        )

    def pool_message(self) -> str:
        return self.format()


# ─────────────────────────────────────────────────────────────────────
# 4. Lexer
# ─────────────────────────────────────────────────────────────────────

class Lexer:
    """
    Uso:
        toks = Lexer(source_code).tokenize()
    """

    # IDENT começa com letra MINÚSCULA ou '_' (regra da spec).
    # Identificadores começando com maiúscula são reservados a libs/classes
    # — emitimos como IDENT_UPPER para o parser decidir o que fazer.
    _re_ident_lower = re.compile(r"[a-z_][A-Za-z0-9_]*")
    _re_ident_upper = re.compile(r"[A-Z][A-Za-z0-9_]*")
    _re_number      = re.compile(r"\d+(\.\d+)?")
    _re_newline     = re.compile(r"\r?\n")

    def __init__(self, source: str, filename: str = "<stdin>"):
        self.src = source
        self.filename = filename
        self.pos = 0
        self.line = 1
        self.col = 1
        self.tokens: list[Token] = []

        # Indentação estilo Python (apenas dentro de blocos `:`).
        # brace_depth>0 desativa o tracking de indent (modo `{}`).
        self.indent_stack: list[int] = [0]
        self.brace_depth = 0          # ( [ { abertos
        self.paren_only_depth = 0     # apenas ( [
        self.pending_colon_block = False  # último token foi ':'?

        # Detecção de mistura de estilos para warning
        self.saw_brace_block = False
        self.saw_colon_block = False
        self.warnings: list[str] = []
        self.INDENT_UNIT = 4          # estritamente 4 espaços por nível

        # Linhas-fonte cacheadas para mensagens de erro
        self._lines = source.splitlines()

    # ── helpers de baixo nível ──────────────────────────────────────
    def _peek(self, offset: int = 0) -> str:
        p = self.pos + offset
        return self.src[p] if p < len(self.src) else ""

    def _advance(self, n: int = 1) -> str:
        chunk = self.src[self.pos : self.pos + n]
        self.pos += n
        self.col += n
        return chunk

    def _error(self, msg: str) -> "PoolSyntaxError":
        src_line = self._lines[self.line - 1] if 0 <= self.line - 1 < len(self._lines) else ""
        return PoolSyntaxError(msg, self.line, self.col, src_line)

    def _emit(self, type_: str, value: object, lexeme: str, line: int, col: int) -> None:
        # Intern strings pra economizar memória — identifiers e keywords são muito repetidos
        if isinstance(value, str):
            value = sys.intern(value)
        if isinstance(lexeme, str):
            lexeme = sys.intern(lexeme)
        self.tokens.append(Token(type_, value, line, col, lexeme))

    def _colon_opens_indent_block(self) -> bool:
        if self.brace_depth > 0:
            return False
        p = self.pos + 1
        while p < len(self.src) and self.src[p] in (" ", "\t"):
            p += 1
        return p >= len(self.src) or self.src[p] in ("\n", "\r") or self.src.startswith("//", p)

    # ── tratamento de quebra de linha + indentação ──────────────────
    def _handle_newline(self) -> None:
        # consome o \n
        self.pos += 1
        self.line += 1
        self.col = 1

        # Dentro de () [] {}? Indentação não conta.
        if self.brace_depth > 0:
            return

        # NEWLINE token só é emitido fora de parênteses
        self._emit("NEWLINE", None, "\\n", self.line - 1, self.col)

        # Mede indentação da próxima linha. Política ESTRITA:
        #   - apenas espaços; TAB é proibido.
        #   - quantidade deve ser múltiplo de 4.
        indent = 0
        saw_tab = False
        while self.pos < len(self.src) and self.src[self.pos] in (" ", "\t"):
            if self.src[self.pos] == "\t":
                saw_tab = True
            else:
                indent += 1
            self.pos += 1
            self.col += 1

        # Linha vazia ou só comentário? Não mexe na pilha.
        if self.pos >= len(self.src) or self.src[self.pos] in ("\n", "\r"):
            return
        if self.src[self.pos] == "/" and self._peek(1) == "/":
            return

        if saw_tab:
            raise self._error("indentação com TAB não é permitida; use 4 espaços")
        if indent % self.INDENT_UNIT != 0:
            raise self._error(
                f"indentação deve ser múltiplo de {self.INDENT_UNIT} espaços (achou {indent})"
            )

        top = self.indent_stack[-1]
        if indent > top:
            # só pode crescer 1 nível por vez (= +4 espaços)
            if indent != top + self.INDENT_UNIT:
                raise self._error(
                    f"indentação avançou {indent - top} espaços; esperado exatamente {self.INDENT_UNIT}"
                )
            self.indent_stack.append(indent)
            self._emit("INDENT", indent, "", self.line, 1)
        else:
            while indent < self.indent_stack[-1]:
                self.indent_stack.pop()
                self._emit("DEDENT", indent, "", self.line, 1)
            if indent != self.indent_stack[-1]:
                raise self._error(
                    f"indentação inconsistente (esperado {self.indent_stack[-1]}, achou {indent})"
                )

    # ── comentários ────────────────────────────────────────────────
    def _skip_line_comment(self) -> None:
        # já estamos no primeiro '/'
        while self.pos < len(self.src) and self.src[self.pos] != "\n":
            self.pos += 1
            self.col += 1

    def _skip_block_comment(self) -> None:
        # consome o """ inicial
        start_line, start_col = self.line, self.col
        self._advance(3)
        while self.pos < len(self.src):
            if self.src[self.pos:self.pos + 3] == '"""':
                self._advance(3)
                return
            if self.src[self.pos] == "\n":
                self.pos += 1
                self.line += 1
                self.col = 1
            else:
                self.pos += 1
                self.col += 1
        raise PoolSyntaxError(
            'bloco de comentário """ não foi fechado',
            start_line, start_col,
            self._lines[start_line - 1] if start_line - 1 < len(self._lines) else "",
        )

    # ── strings ────────────────────────────────────────────────────
    def _read_string(self, quote: str, is_fstring: bool = False, is_raw: bool = False) -> None:
        start_line, start_col = self.line, self.col
        self._advance(1)  # consome a aspa inicial
        out = []
        while self.pos < len(self.src):
            c = self.src[self.pos]
            if is_raw:
                # raw string — não processa escapes, copia literalmente
                if c == quote:
                    self._advance(1)
                    self._emit("STR", "".join(out), quote + "".join(out) + quote, start_line, start_col)
                    return
                if c == "\n":
                    raise self._error("string não fechada antes da quebra de linha")
                out.append(c)
                self._advance(1)
                continue
            if c == "\\" and self.pos + 1 < len(self.src):
                nxt = self.src[self.pos + 1]
                escapes = {"n": "\n", "t": "\t", "r": "\r", "\\": "\\", '"': '"', "'": "'"}
                out.append(escapes.get(nxt, nxt))
                self._advance(2)
                continue
            if c == quote:
                self._advance(1)
                tok_type = "FSTRING" if is_fstring else "STR"
                self._emit(tok_type, "".join(out), quote + "".join(out) + quote, start_line, start_col)
                return
            if c == "\n":
                raise self._error("string não fechada antes da quebra de linha")
            out.append(c)
            self._advance(1)
        raise PoolSyntaxError(
            "string não fechada até o fim do arquivo",
            start_line, start_col,
            self._lines[start_line - 1] if start_line - 1 < len(self._lines) else "",
        )

    # ── números ────────────────────────────────────────────────────
    def _read_number(self) -> None:
        start_col = self.col
        m = self._re_number.match(self.src, self.pos)
        assert m is not None
        text = m.group(0)
        if "." in text:
            self._emit("FLO", float(text), text, self.line, start_col)
        else:
            self._emit("INT", int(text), text, self.line, start_col)
        self._advance(len(text))

    # ── identificadores / palavras-chave ───────────────────────────
    def _read_ident(self) -> None:
        start_col = self.col
        # tenta lower primeiro, senão upper (libs/classes/PUSH/GET)
        m = self._re_ident_lower.match(self.src, self.pos) or self._re_ident_upper.match(self.src, self.pos)
        assert m is not None
        text = m.group(0)
        # f-string: f"..." → identificador 'f' colado em aspa
        if text == "f" and self._peek(1) == '"':
            self._advance(1)  # consome o 'f'
            self._read_string('"', is_fstring=True)
            return
        # r-string: r"..." ou r'...' → raw string sem escapes
        if text == "r" and self._peek(1) in ('"', "'"):
            quote = self._peek(1)
            self._advance(1)  # consome o 'r'
            self._read_string(quote, is_raw=True)
            return
        # classifica
        if text in BOOL_LITERALS:
            self._emit("BOOL", text in ("True", "true"), text, self.line, start_col)
        elif text in NULL_LITERALS:
            self._emit("NULL", None, text, self.line, start_col)
        elif text in KEYWORDS:
            self._emit("KW", text, text, self.line, start_col)
        else:
            # IDENT_UPPER se começa com maiúscula (libs/classes), senão IDENT
            tok_type = "IDENT_UPPER" if text[0].isupper() else "IDENT"
            self._emit(tok_type, text, text, self.line, start_col)
        self._advance(len(text))


    # ── cor: <hex>"texto" ou <nome>"texto" ──────────────────────────
    def _read_color(self) -> bool:
        """Tenta consumir <hex> ou <nome> imediatamente seguido de string.
        Emite token COLOR e retorna True. Se não casar, retorna False."""
        m = _COLOR_RE.match(self.src, self.pos)
        if not m:
            return False
        val = m.group(1)
        # valida: hex de exatamente 3 ou 6 dígitos (igual a CSS), ou nome
        # conhecido. 4/5 dígitos não são um formato de cor válido — sem essa
        # checagem estrita, `_ansi_color()` (interpreter.py) recebe um hex
        # que não fecha em 6 caracteres após dobrar e silenciosamente não
        # aplica cor nenhuma, então é melhor não tratar como COLOR aqui.
        if not (re.fullmatch(r"[0-9A-Fa-f]{3}|[0-9A-Fa-f]{6}", val) or val in NAMED_COLORS):
            return False
        start_col = self.col
        self._advance(len(m.group(0)))   # consome <hex>
        self._emit("COLOR", val, m.group(0), self.line, start_col)
        return True

    # ── operadores ─────────────────────────────────────────────────
    def _read_operator(self) -> bool:
        # tenta os de múltiplos caracteres primeiro (ordem do array)
        for op in MULTI_OPS:
            if self.src.startswith(op, self.pos):
                start_col = self.col
                self._emit("OP", op, op, self.line, start_col)
                self._advance(len(op))
                return True
        c = self._peek()
        if c in SINGLE_OPS:
            start_col = self.col
            self._emit("OP", c, c, self.line, start_col)
            self._advance(1)
            return True
        return False

    # ── pontuação ──────────────────────────────────────────────────
    def _read_punct(self) -> bool:
        c = self._peek()
        if c not in PUNCT:
            return False
        start_col = self.col
        type_ = PUNCT[c]
        self._emit(type_, c, c, self.line, start_col)
        if c == "{":
            self.saw_brace_block = True
            self.brace_depth += 1
        elif c == ":":
            # ':' só conta como abertura de bloco se estiver no fim lógico da linha.
            # Dois-pontos de dicionário/objeto não devem gerar warning de estilo.
            if self._colon_opens_indent_block():
                self.saw_colon_block = True
        elif c in "([":
            self.brace_depth += 1
        elif c in ")]}":
            self.brace_depth = max(0, self.brace_depth - 1)
        self._advance(1)
        return True

    # ── loop principal ─────────────────────────────────────────────
    def tokenize(self) -> list[Token]:
        # indent inicial da primeira linha (caso o arquivo comece indentado)
        # consumimos espaços iniciais sem emitir INDENT (linha 1 começa em 0)
        while self.pos < len(self.src) and self.src[self.pos] in (" ", "\t"):
            self.pos += 1
            self.col += 1

        while self.pos < len(self.src):
            c = self._peek()

            # quebra de linha
            if c == "\n":
                self._handle_newline()
                continue
            if c == "\r":
                self.pos += 1
                continue

            # espaço em branco
            if c in (" ", "\t"):
                self.pos += 1
                self.col += 1
                continue

            # comentário de linha // ou #
            if c == "/" and self._peek(1) == "/":
                self._skip_line_comment()
                continue

            if c == "#":
                self._skip_line_comment()
                continue

            # comentário de bloco """ ... """
            if c == '"' and self._peek(1) == '"' and self._peek(2) == '"':
                self._skip_block_comment()
                continue

            # string
            if c in ('"', "'"):
                self._read_string(c)
                continue

            # número
            if c.isdigit():
                self._read_number()
                continue

            # identificador / keyword
            if c.isalpha() or c == "_":
                self._read_ident()
                continue

            # pontuação primeiro (), [], {}, :, ;, ., @, ,
            if c in PUNCT:
                self._read_punct()
                continue

            # cor: <hex>"texto" ou <nome>"texto"
            if c == "<" and self._read_color():
                continue

            # operadores
            if self._read_operator():
                continue

            raise self._error(f"caractere inesperado: {c!r}")

        # fecha indentações pendentes
        while len(self.indent_stack) > 1:
            self.indent_stack.pop()
            self._emit("DEDENT", 0, "", self.line, self.col)

        if self.saw_brace_block and self.saw_colon_block:
            self.warnings.append(
                "[poolscript:warn] mistura blocos com chaves {} e dois-pontos (:) "
                "no mesmo arquivo — padronize um único estilo"
            )

        self._emit("EOF", None, "", self.line, self.col)
        return self.tokens


# ─────────────────────────────────────────────────────────────────────
# 5. CLI rápida do lexer (debug):  python lexer.py arquivo.ps
# ─────────────────────────────────────────────────────────────────────
if __name__ == "__main__":
    import sys
    if len(sys.argv) < 2:
        print("uso: python lexer.py <arquivo.ps>")
        sys.exit(1)
    with open(sys.argv[1], "r", encoding="utf-8") as f:
        src = f.read()
    try:
        toks = Lexer(src, sys.argv[1]).tokenize()
    except PoolSyntaxError as e:
        print(e); sys.exit(2)
    for t in toks:
        print(t)
