"""
Módulo `regex` da PoolScript.

Uso:
    from regex import match, findall, sub, split, escape

    match("\\d+", "abc123")          → False
    match("\\d+", "123")             → True
    findall("\\d+", "a1b2c3")        → ["1", "2", "3"]
    sub("\\s+", "_", "hello world")  → "hello_world"
    split("\\s+", "a b  c")          → ["a", "b", "c"]
    escape("a.b*c")                  → "a\\.b\\*c"
"""
from __future__ import annotations
import re as _re
from .strmethod_lib import PoolStr


class Pattern:
    """Padrão já COMPILADO (`regex.compile(...)`) — o mesmo nome do `re` do
    Python. Tem os métodos do módulo, sem repetir o padrão a cada chamada:
    compila uma vez e reusa, que é o ganho num laço."""

    def __init__(self, pattern: str, flags: int = 0):
        self.pattern = pattern
        self.flags = int(flags)
        self._rx = _re.compile(pattern, flags)

    def match(self, string: str) -> bool:
        """Casa a string INTEIRA (igual ao `regex.match`)."""
        return bool(self._rx.fullmatch(string))

    def fullmatch(self, string: str) -> bool:
        return bool(self._rx.fullmatch(string))

    def search(self, string: str) -> bool:
        return bool(self._rx.search(string))

    def findall(self, string: str) -> list:
        return self._rx.findall(string)

    def sub(self, repl: str, string: str, count: int = 0) -> PoolStr:
        return PoolStr(self._rx.sub(repl, string, count))

    def split(self, string: str, maxsplit: int = 0) -> list:
        return self._rx.split(string, maxsplit)

    def __repr__(self):
        return f"<Pattern {self.pattern!r}>"


def compile(pattern: str, flags: int = 0) -> "Pattern":
    """Compila o padrão UMA vez e devolve um `Pattern` reusável."""
    return Pattern(pattern, flags)


def match(pattern: str, string: str, flags: int = 0) -> bool:
    """Verifica se a string casa completamente com o padrão."""
    return bool(_re.fullmatch(pattern, string, flags))

def fullmatch(pattern: str, string: str, flags: int = 0) -> bool:
    """Casa a string INTEIRA — o nome do Python pro que `match` já faz aqui."""
    return bool(_re.fullmatch(pattern, string, flags))

def search(pattern: str, string: str, flags: int = 0) -> bool:
    """Verifica se o padrão existe em qualquer posição da string."""
    return bool(_re.search(pattern, string, flags))

def findall(pattern: str, string: str, flags: int = 0) -> list:
    """Retorna lista com todas as ocorrências do padrão."""
    return _re.findall(pattern, string, flags)

def sub(pattern: str, repl: str, string: str, count: int = 0, flags: int = 0) -> PoolStr:
    """Substitui ocorrências do padrão. Retorna PoolStr (encadeável)."""
    return PoolStr(_re.sub(pattern, repl, string, count, flags))

def split(pattern: str, string: str, maxsplit: int = 0, flags: int = 0) -> list:
    """Divide a string pelo padrão."""
    return _re.split(pattern, string, maxsplit, flags)

def escape(string: str) -> PoolStr:
    """Escapa caracteres especiais de regex."""
    return PoolStr(_re.escape(string))

EXPORTS = {
    "compile": compile,
    "match":   match,
    "fullmatch": fullmatch,
    "search":  search,
    "findall": findall,
    "sub":     sub,
    "split":   split,
    "escape":  escape,
}
