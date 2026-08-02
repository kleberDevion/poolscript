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

    def save(self, destino: str = "."):
        """Salva o conteúdo do arquivo num caminho (como os outros `.save()`):
        se `destino` é pasta ('.', termina em '/', ou é diretório), o nome vem
        do próprio arquivo. Devolve o caminho final."""
        import os
        import shutil
        self._f.flush()
        if destino in (".", "..") or destino.endswith("/") or os.path.isdir(destino):
            alvo = os.path.join(destino, os.path.basename(self.path))
        else:
            alvo = destino
        if os.path.abspath(alvo) != os.path.abspath(self.path):
            pai = os.path.dirname(alvo)
            if pai:
                os.makedirs(pai, exist_ok=True)
            shutil.copyfile(self.path, alvo)
        return alvo

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
    # `PoolTypeRef` é subclasse de str no Python, então sem este teste
    # `type(int)` devolvia "str" — o vazamento acertava por acidente só em
    # `type(str)`. Referência de tipo é do tipo `type`, e é o que faz
    # `str is type` valer.
    from .interpreter import PoolTypeRef
    if isinstance(value, PoolTypeRef):
        return "type"
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
        return "dict"
    if isinstance(value, tuple):
        return "tup"
    # `TransientValue` do Parsing se reporta pelo tipo de ORIGEM, não pela
    # classe Python que o embrulha — testado: o origem sempre coincide com o
    # tipo real do valor, então isto só remove o vazamento do nome.
    origem = getattr(value, "origin_type", None)
    if origem is not None:
        return origem
    # Gerador é `generator` — `PoolGenerator` é o nome da classe Python que o
    # implementa, e não existe na linguagem.
    if type(value).__name__ == "PoolGenerator":
        return "generator"
    # Instância de Entity se reporta pelo nome da Entity, não pela classe
    # Python que a implementa — `PoolEntityInstance` não existe na linguagem.
    ent = getattr(value, "_ps_entity", None)
    if ent is not None:
        return getattr(ent, "name", "object")
    # Classe exposta como referência de tipo (`PoolFile`) é `type`, igual a
    # `str`/`int` — ela aparece do lado direito de `is`, não é chamável do
    # ponto de vista da linguagem.
    if isinstance(value, type):
        return "type"
    # qualquer chamável é `action` — sem isto vazava o nome da classe do
    # Python ("builtin_function_or_method", "UserFunction"…)
    if callable(value) and not hasattr(value, "name"):
        return "action"
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
    "list":     list,      # list() → []  |  list("ab") → ["a","b"]  |  list(range(3)) → [0,1,2]
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
