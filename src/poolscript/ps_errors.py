"""
PoolScript — Sistema de Erros (v2)
===================================
Garante que NENHUM traceback Python vaze para o usuário.
Todo erro — seja de sintaxe, runtime ou interno — passa por aqui
e é formatado com o traceback próprio da PoolScript.

Hierarquia:
  PoolError (base)
    ├── PoolSyntaxError    — lexer/parser
    ├── PoolParseError     — parser semântico
    ├── PoolRuntimeError   — execução
    └── PoolInternalError  — bug no interpretador (nunca expõe stack Python)
"""
from __future__ import annotations
import sys
import os
import sqlite3 as _sqlite3

# ── Detecção de runtime ───────────────────────────────────────────────────────
IS_PYPY = hasattr(sys, "pypy_version_info")
RUNTIME  = f"PyPy {sys.pypy_version_info[:3]}" if IS_PYPY else f"CPython {sys.version_info[:3]}"

# ── Cores ANSI ────────────────────────────────────────────────────────────────
_USE_COLOR = sys.stderr.isatty() and os.environ.get("NO_COLOR") is None

def _c(code: str, text: str) -> str:
    return f"\033[{code}m{text}\033[0m" if _USE_COLOR else text

RED     = lambda t: _c("31", t)
YELLOW  = lambda t: _c("33", t)
CYAN    = lambda t: _c("36", t)
BOLD    = lambda t: _c("1",  t)
DIM     = lambda t: _c("2",  t)

# ── Códigos de erro do Spec ───────────────────────────────────────────────────
ATTRIBUTTED_VALUE_ERROR   = "AtributtedValueError"
OUTPUT_UNEXPECTED_VALUES  = "OutputUnexpectedValues"
SOME_VALUE_UNEXPECTED     = "SomeValueUnexpected"
INDEX_OUT_OF_BOUNDS       = "IndexOutOfBoundsWarning"


def warn(message: str, line: int, col: int, code: str = INDEX_OUT_OF_BOUNDS) -> None:
    """Warning amarelo no stderr — não interrompe a execução."""
    sys.stderr.write(f"{YELLOW(code)}: {message} (L{line}:C{col})\n")
    sys.stderr.flush()


# ── Base ──────────────────────────────────────────────────────────────────────
class PoolError(Exception):
    """Raiz de todos os erros PoolScript. Nunca expõe internos Python."""

    def pool_message(self) -> str:
        """Mensagem limpa para exibição. Override nas subclasses."""
        return str(self)

    def exit(self, code: int = 1) -> None:
        """Imprime o erro formatado e encerra o processo."""
        sys.stderr.write(self.pool_message() + "\n")
        sys.stderr.flush()
        sys.exit(code)


# ── SyntaxError ───────────────────────────────────────────────────────────────
class _BasePoolSyntaxError(PoolError):
    def __init__(self, msg: str, line: int, col: int, source_line: str = "",
                 filename: str = "<script>"):
        self.msg         = msg
        self.line        = line
        self.col         = col
        self.source_line = source_line
        self.filename    = filename
        super().__init__(self._build())

    def _build(self) -> str:
        pointer  = " " * max(0, self.col - 1) + "^^^"
        src      = self.source_line.rstrip("\n")
        fname    = _clean_filename(self.filename)
        lines = [
            BOLD(RED("SyntaxError")) + f": {self.msg}",
            DIM(f"  → {fname}, linha {self.line}, coluna {self.col}"),
            f"  | {src}",
            f"  | {YELLOW(pointer)}",
        ]
        return "\n".join(lines)

    def pool_message(self) -> str:
        return self._build()


# ── ParseError ────────────────────────────────────────────────────────────────
class _BasePoolParseError(PoolError):
    def __init__(self, msg: str, token, source: str = "", filename: str = "<script>"):
        self.msg      = msg
        self.token    = token
        self.source   = source
        self.filename = filename
        super().__init__(self._build())

    def _build(self) -> str:
        line = getattr(self.token, "line", 0)
        col  = getattr(self.token, "col",  0)
        src_lines = self.source.splitlines()
        src  = src_lines[line - 1] if 0 < line <= len(src_lines) else ""
        pointer = " " * max(0, col - 1) + "^^^"
        fname = _clean_filename(self.filename)
        lines = [
            BOLD(RED("SyntaxError")) + f": {self.msg}",
            DIM(f"  → {fname}, linha {line}, coluna {col}"),
            f"  | {src}",
            f"  | {YELLOW(pointer)}",
        ]
        return "\n".join(lines)

    def pool_message(self) -> str:
        return self._build()


# ── RuntimeError ──────────────────────────────────────────────────────────────
class _BasePoolRuntimeError(PoolError):
    def __init__(
        self,
        msg:        str,
        node,
        source:     str  = "",
        code:       str  = "RuntimeError",
        filename:   str  = "<script>",
        call_stack: list | None = None,
    ):
        self.msg        = msg
        self.node       = node
        self.source     = source
        self.code       = code
        self.filename   = filename
        self.call_stack = call_stack or []
        super().__init__(self._build())

    # compat com código legado que chama ._unwrap()
    @staticmethod
    def _unwrap(value):
        if hasattr(value, "value") and type(value).__name__ == "TransientValue":
            return value.value
        return value

    def _fmt_frame(self, filename: str, line: int, col: int, source: str) -> str:
        src_lines = source.splitlines() if source else []
        src  = src_lines[line - 1] if 0 < line <= len(src_lines) else ""
        pointer = " " * max(0, col - 1) + "^^^"
        fname = _clean_filename(filename)
        return (
            f"  {DIM('em')} {CYAN(fname)}, linha {line}\n"
            f"  | {src}\n"
            f"  | {YELLOW(pointer)}"
        )

    def _build(self) -> str:
        parts = [BOLD(RED(self.code)) + f": {self.msg}"]

        if self.call_stack:
            parts.append(BOLD("\nTraceback (chamada mais recente por último):"))
            for (fname, line, col, fsource) in self.call_stack:
                parts.append(self._fmt_frame(fname, line, col, fsource))

        node = self.node
        line = getattr(node, "line", 0)
        col  = getattr(node, "col",  0)
        parts.append(self._fmt_frame(
            self.filename or "<script>", line, col, self.source
        ))
        return "\n".join(parts)

    def pool_message(self) -> str:
        return self._build()


# ── InternalError — bugs no interpretador, nunca expõe Python ────────────────
class PoolInternalError(PoolError):
    """
    Usado quando algo inesperado acontece no próprio interpretador.
    Nunca mostra o stack Python — apenas uma mensagem amigável + código de suporte.
    """
    def __init__(self, context: str = ""):
        self._context = context
        super().__init__(self._build())

    def _build(self) -> str:
        lines = [
            BOLD(RED("InternalError")) + ": ocorreu um erro inesperado no interpretador PoolScript.",
            "  Por favor, reporte em: https://github.com/poolscript/poolscript/issues",
        ]
        if self._context:
            lines.append(DIM(f"  Contexto: {self._context}"))
        return "\n".join(lines)

    def pool_message(self) -> str:
        return self._build()


# ── Helpers ───────────────────────────────────────────────────────────────────
def _clean_filename(filename: str) -> str:
    """Remove caminhos absolutos do sistema — expõe só o nome do arquivo."""
    if not filename or filename.startswith("<"):
        return filename or "<script>"
    # mostra só o path relativo ao cwd, ou só o basename se estiver fora
    try:
        rel = os.path.relpath(filename, os.getcwd())
        # se ficou mais curto, usa relativo; senão só basename
        return rel if len(rel) < len(filename) else os.path.basename(filename)
    except ValueError:
        return os.path.basename(filename)


def shield(exc: Exception, node=None, source: str = "", filename: str = "<script>") -> PoolError:
    """
    Converte qualquer exceção Python em um PoolError sem expor internos.
    Usado nos blocos except genéricos do interpretador.
    """
    if isinstance(exc, PoolError):
        return exc

    # Mapeia tipos Python comuns para mensagens amigáveis
    _MAP = {
        ZeroDivisionError:  ("divisão por zero",                 SOME_VALUE_UNEXPECTED),
        TypeError:          ("operação inválida entre os tipos",  SOME_VALUE_UNEXPECTED),
        ValueError:         ("valor inválido",                    SOME_VALUE_UNEXPECTED),
        KeyError:           ("chave não encontrada",              "KeyError"),
        IndexError:         ("índice fora dos limites",           INDEX_OUT_OF_BOUNDS),
        RecursionError:     ("limite de recursão atingido — possível recursão infinita", "RecursionError"),
        MemoryError:        ("memória insuficiente",              "MemoryError"),
        UnicodeDecodeError: ("erro de codificação de texto",      "UnicodeError"),
        FileNotFoundError:  ("arquivo não encontrado",            "IOError"),
        PermissionError:    ("permissão negada",                  "IOError"),
        TimeoutError:       ("operação expirou",                  "TimeoutError"),
        ConnectionError:    ("falha de conexão",                  "NetworkError"),
        NotImplementedError:("recurso ainda não implementado",    "NotImplemented"),
        _sqlite3.Error:     ("erro de banco de dados",            "DatabaseError"),
        OSError:            ("erro de sistema/arquivo",           "OSError"),
        RuntimeError:       ("erro de execução",                  "RuntimeError"),
    }
    for exc_type, (friendly_msg, code) in _MAP.items():
        if isinstance(exc, exc_type):
            detail = str(exc)
            msg = f"{friendly_msg}: {detail}" if detail else friendly_msg
            # Import tardio (evita import circular ps_errors <-> interpreter):
            # constrói a subclasse pública `interpreter.PoolRuntimeError`, não a
            # base interna — quem importa `poolscript.PoolRuntimeError` e faz
            # `isinstance`/`except PoolRuntimeError` precisa que TODO erro de
            # runtime (nativo do Python ou levantado explicitamente) seja dessa
            # classe, senão o catch/isinstance externo silenciosamente não bate.
            try:
                from .interpreter import PoolRuntimeError as _PublicRuntimeError
                error_cls = _PublicRuntimeError
            except ImportError:
                error_cls = _BasePoolRuntimeError
            return error_cls(msg, node or _FakeNode(0, 0), source, code, filename)

    # Fallback — erro desconhecido → InternalError, mas ainda assim informa
    # tipo + mensagem originais (sem traceback) pra não virar caixa-preta.
    detail = str(exc)
    context = f"{type(exc).__name__}: {detail}" if detail else type(exc).__name__
    return PoolInternalError(context=context)


class _FakeNode:
    """Nó sintético para erros sem posição no AST."""
    __slots__ = ("line", "col")
    def __init__(self, line: int, col: int):
        self.line = line
        self.col  = col


# ── Aliases públicos (compatibilidade com imports existentes) ─────────────────
PoolSyntaxError  = _BasePoolSyntaxError
PoolParseError   = _BasePoolParseError
PoolRuntimeError = _BasePoolRuntimeError
