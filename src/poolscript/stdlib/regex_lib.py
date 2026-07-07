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


def match(pattern: str, string: str, flags: int = 0) -> bool:
    """Verifica se a string casa completamente com o padrão."""
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
    "match":   match,
    "search":  search,
    "findall": findall,
    "sub":     sub,
    "split":   split,
    "escape":  escape,
}
