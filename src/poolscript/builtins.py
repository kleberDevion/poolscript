"""
Built-ins globais da PoolScript que NÃO precisam de import.

Disponíveis sempre: open(), len(), str(), int(), flo(), bool(), type(), range().

Esses são instalados pelo Interpreter via _install_builtins(). O `post` e `input`
ficam no próprio interpreter porque dependem de `self.output`.
"""
from __future__ import annotations
from .stdlib.parsing_lib import Parsing
from .stdlib.os_lib import PoolFile
from typing import Any


class FileHandle:
    """
    Wrapper sobre file objects do Python para usar com `using ... as f { f.read() }`.

    Métodos: read(), readlines(), write(text), close(), __enter__/__exit__.
    """

    def __init__(self, path: str, mode: str = "r", encoding: str = "utf-8"):
        self.path = path
        self.mode = mode
        # bytes mode: não passa encoding
        if "b" in mode:
            self._f = open(path, mode)
        else:
            self._f = open(path, mode, encoding=encoding)
        self._closed = False

    def read(self, size: int = -1):
        return self._f.read(size)

    def readlines(self):
        return self._f.readlines()

    def readline(self):
        return self._f.readline()

    def write(self, text):
        if "b" in self.mode:
            # modo binário — aceita bytes ou PoolFile/PoolFileUpload
            if hasattr(text, "_data"):
                text = text._data
            elif hasattr(text, "_bytes"):
                text = text._bytes
            elif not isinstance(text, bytes):
                text = str(text).encode("utf-8")
        else:
            if not isinstance(text, str):
                text = str(text)
        return self._f.write(text)

    def writelines(self, lines):
        return self._f.writelines(str(l) for l in lines)

    def close(self):
        if not self._closed:
            self._f.close()
            self._closed = True
        return None

    # context manager — habilita `using ... as f`
    def __enter__(self):
        return self

    def __exit__(self, exc_type, exc_val, exc_tb):
        self.close()
        return False

    def __iter__(self):
        return iter(self._f)

    def __repr__(self):
        return f"<FileHandle {self.path!r} mode={self.mode!r}>"


def ps_open(path, mode: str = "r", encoding: str = "utf-8") -> FileHandle:
    """
    Built-in `open(path, mode)` — abre um arquivo. Use com `using` pra fechar auto:

        using open("log.txt", "a") as f {
            f.write("nova linha\n")
        }
    """
    return FileHandle(str(path), str(mode), str(encoding))


def ps_len(value: Any) -> int:
    if value is None:
        return 0
    try:
        return len(value)
    except TypeError:
        raise TypeError(f"len() não aplicável a {type(value).__name__}")


def ps_range(*args):
    return list(range(*[int(a) for a in args]))


def ps_type(value: Any) -> str:
    if value is None:
        return "Null"
    if isinstance(value, bool):
        return "bool"
    if isinstance(value, int):
        return "int"
    if isinstance(value, float):
        return "flo"
    if isinstance(value, str):
        return "str"
    if isinstance(value, list):
        return "list"
    if isinstance(value, dict):
        return "json"
    return type(value).__name__


GLOBAL_BUILTINS = {
    "open":     ps_open,
    "len":      ps_len,
    "range":    ps_range,
    "type":     ps_type,
    "str":      str,
    "int":      int,
    "flo":      float,
    "bool":     bool,
    "Parsing":  Parsing,
    "PoolFile": PoolFile,   # comparação de tipo (`x is PoolFile`) sem precisar de `from os import PoolFile`
    # ── Utilitários de memória e conversão ──────────────────────────
    "id":       id,        # endereço de memória do objeto
    "hex":      hex,       # int → "0xff"
    "bin":      bin,       # int → "0b1010"
    "oct":      oct,       # int → "0o17"
    "ord":      ord,       # char → int  (ex: ord("A") → 65)
    "chr":      chr,       # int → char  (ex: chr(65) → "A")
    "abs":      abs,       # valor absoluto
    "round":    round,     # arredondamento
    "sum":      sum,       # soma de lista
    "min":      min,       # mínimo
    "max":      max,       # máximo
    "sorted":   sorted,    # lista ordenada
    "reversed": lambda x: list(reversed(x)),  # lista invertida
    "enumerate": lambda x: list(enumerate(x)),  # enumerate(lista) → [(0,x),(1,y)...]
    "zip":      lambda *a: list(zip(*a)),  # zip(a,b) → [(a0,b0)...]
}
