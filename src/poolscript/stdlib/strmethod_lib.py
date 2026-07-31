"""
String methods estendidos da PoolScript.

Suporta:
- Encadeamento: .replace("a","b").replace("c","d")
- Replace com lista: .replace(["a","b"], "")
- .isalpha(), .isdigit(), .isalnum(), .strip(), .upper(), .lower(), etc.
- Integração com regex: .match(pattern), .findall(pattern), .sub(pattern, repl)
"""
from __future__ import annotations
import re as _re


def _para_texto_pool(v):
    """Renderização PoolScript de um valor — `Null` em vez de `None`.

    Mora aqui e é usada também pelo `Interpreter.stringify`: duas cópias
    divergiriam na primeira correção, e foi exatamente assim que
    `post([Null])` e `"{}".format([Null])` acabaram discordando.
    """
    if v is None:
        return "null"
    if v is True:
        return "True"
    if v is False:
        return "False"
    # `PoolTypeRef` é subclasse de str, então precisa vir ANTES — senão
    # `post([str])` imprime o repr do Enum do Python. Testado pelo nome da
    # classe porque importar o interpretador daqui seria ciclo: é ele que
    # importa este módulo.
    if type(v).__name__ == "PoolTypeRef":
        return str(v.value)
    if isinstance(v, PoolStr) or isinstance(v, str):
        return repr(str(v))
    if isinstance(v, list):
        return "[" + ", ".join(_para_texto_pool(x) for x in v) + "]"
    if isinstance(v, tuple):
        if len(v) == 1:
            return "(" + _para_texto_pool(v[0]) + ",)"
        return "(" + ", ".join(_para_texto_pool(x) for x in v) + ")"
    if isinstance(v, dict):
        return "{" + ", ".join(f"{_para_texto_pool(k)}: {_para_texto_pool(x)}"
                               for k, x in v.items()) + "}"
    # Classe usada como referência de tipo (`PoolFile`) sai pelo nome. Sem
    # isto vazava `<class 'poolscript.stdlib.os_lib.PoolFile'>`, que expõe o
    # caminho do módulo Python — nada disso existe na linguagem.
    if isinstance(v, type):
        return v.__name__
    return str(v)


def _troca(v):
    """Só container e Null passam por texto — número tem que chegar número no
    `format`, senão `{:.2f}` recebe string e estoura."""
    if v is None or isinstance(v, (list, tuple, dict)):
        return _para_texto_pool(v)
    return v


class PoolStr(str):
    """Subclasse de str com métodos estendidos e suporte a encadeamento."""

    def __new__(cls, value=""):
        return str.__new__(cls, value)

    # ── replace estendido ────────────────────────────────────────────
    def replace(self, old, new="", count=-1):
        """
        .replace("alvo", "novo")           → substitui string
        .replace(["a1","a2"], "novo")      → substitui todos da lista
        .replace(["a1","a2"], ["n1","n2"]) → substitui par a par
        Retorna PoolStr para encadeamento.
        """
        result = str(self)
        if isinstance(old, list):
            if isinstance(new, list):
                for o, n in zip(old, new):
                    result = result.replace(o, n)
            else:
                for o in old:
                    result = result.replace(o, new)
        else:
            if count == -1:
                result = str.replace(result, old, new)
            else:
                result = str.replace(result, old, new, count)
        return PoolStr(result)

    # ── métodos padrão — retornam PoolStr para encadeamento ──────────
    def strip(self, chars=None):
        return PoolStr(str.strip(self, chars) if chars else str.strip(self))

    def lstrip(self, chars=None):
        return PoolStr(str.lstrip(self, chars) if chars else str.lstrip(self))

    def rstrip(self, chars=None):
        return PoolStr(str.rstrip(self, chars) if chars else str.rstrip(self))

    def upper(self):
        return PoolStr(str.upper(self))

    def lower(self):
        return PoolStr(str.lower(self))

    def title(self):
        return PoolStr(str.title(self))

    def capitalize(self):
        return PoolStr(str.capitalize(self))

    def split(self, sep=None, maxsplit=-1):
        return str.split(self, sep, maxsplit)

    def join(self, iterable):
        return PoolStr(str.join(self, iterable))

    # ── verificações — retornam bool ─────────────────────────────────
    def format(self, *args, **kwargs):
        """`"{}".format(Null)` é "null", não "None".

        O `str.format` do Python chama `str(None)` e devolve "None" — o mesmo
        vazamento que já foi tirado do `post` e do `str()`. Trocar antes de
        delegar mantém o resto da mini-linguagem intacto (`{:.2f}` continua
        recebendo o número, não o texto dele).
        """
        return PoolStr(str.format(self,
                                  *[_troca(a) for a in args],
                                  **{k: _troca(v) for k, v in kwargs.items()}))

    def format_map(self, mapping):
        return PoolStr(str.format_map(self, {k: _troca(v) for k, v in dict(mapping).items()}))

    def isalpha(self) -> bool:
        return str.isalpha(self)

    def isdigit(self) -> bool:
        return str.isdigit(self)

    def isnumeric(self) -> bool:
        return str.isnumeric(self)

    def isalnum(self) -> bool:
        return str.isalnum(self)

    def isspace(self) -> bool:
        return str.isspace(self)

    def isupper(self) -> bool:
        return str.isupper(self)

    def islower(self) -> bool:
        return str.islower(self)

    def startswith(self, prefix) -> bool:
        return str.startswith(self, prefix)

    def endswith(self, suffix) -> bool:
        return str.endswith(self, suffix)

    def contains(self, sub: str) -> bool:
        """Alias legível: 'abc'.contains('a') → True"""
        return sub in self

    def has(self, sub: str) -> bool:
        """Alias de contains — 'abc'.has('a') → True (consistente com dict/list.has)."""
        return sub in self

    # ── len ─────────────────────────────────────────────────────────
    def len(self) -> int:
        return len(self)

    # ── regex integrado ──────────────────────────────────────────────
    def match(self, pattern: str) -> bool:
        """Verifica se a string casa com o padrão (fullmatch)."""
        return bool(_re.fullmatch(pattern, self))

    def findall(self, pattern: str) -> list:
        """Retorna todas as ocorrências do padrão."""
        return _re.findall(pattern, self)

    def sub(self, pattern: str, repl: str) -> "PoolStr":
        """Substitui por regex. Encadeável."""
        return PoolStr(_re.sub(pattern, repl, self))

    def __repr__(self):
        return str.__repr__(self)
