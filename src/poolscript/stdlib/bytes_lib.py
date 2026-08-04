"""
Módulo `bytes` da PoolScript — criar e manipular sequências de bytes.

Dá autonomia pra construir tools binárias sem depender de libs externas:
converter hex/base64, empacotar/desempacotar inteiros, fatiar, concatenar,
XOR de chave repetida, etc. O tipo `bytes` já existe (`"oi".encode()`, leitura
de arquivo em modo binário); este módulo é o que permite CRIAR do zero e
CONVERTER entre formatos.

Uso:
    import bytes

    b = bytes.new([72, 105])         # de lista de inteiros → b'Hi'
    b = bytes.new("Oi")              # de texto (utf-8)
    b = bytes.fromhex("48656c6c6f")  # de hex

    post(bytes.hex(b))               # "48656c6c6f"
    post(bytes.base64(b))            # "SGVsbG8="
    post(bytes.toint(b))             # inteiro (big-endian)
    post(bytes.tolist(b))            # [72, 101, 108, 108, 111]
"""
from __future__ import annotations

import base64 as _b64

# guarda os builtins que os métodos abaixo sombreiam (hex, slice) —
# assim `def hex`/`def slice` do módulo não quebram o uso interno.
_TIPO_LISTA = (list, tuple)


def _as_bytes(v, quem):
    if isinstance(v, (bytes, bytearray)):
        return bytes(v)
    raise TypeError(f"bytes.{quem}: esperava bytes, recebeu {_nome(v)}")


def _nome(v) -> str:
    if v is None:
        return "Null"
    return {bool: "bool", int: "int", float: "flo", str: "str",
            list: "list", dict: "json"}.get(type(v), type(v).__name__)


def new(x=0):
    """Cria bytes de: lista de inteiros (0-255), texto (utf-8), um tamanho
    (N bytes zerados) ou uma cópia de outros bytes."""
    if isinstance(x, (bytes, bytearray)):
        return bytes(x)
    if isinstance(x, str):
        return x.encode("utf-8")
    if isinstance(x, bool):
        raise TypeError("bytes.new: bool não é um tamanho válido")
    if isinstance(x, int):
        if x < 0:
            raise ValueError("bytes.new: tamanho negativo")
        return bytes(x)
    if isinstance(x, _TIPO_LISTA):
        try:
            return bytes(x)
        except (ValueError, TypeError):
            raise ValueError("bytes.new: a lista precisa conter inteiros de 0 a 255")
    raise TypeError(f"bytes.new: não sei criar bytes de {_nome(x)}")


def fromhex(s):
    """Bytes a partir de uma string hex ('48656c6c6f'). Espaços são ignorados."""
    if not isinstance(s, str):
        raise TypeError("bytes.fromhex: esperava str")
    limpo = "".join(s.split())
    try:
        return bytes.fromhex(limpo)
    except ValueError:
        raise ValueError(f"bytes.fromhex: hex inválido: {s!r}")


def hex(b):
    """String hex minúscula, sem separador ('48656c6c6f')."""
    return _as_bytes(b, "hex").hex()


def base64(b):
    """String base64 (padrão, com padding)."""
    return _b64.b64encode(_as_bytes(b, "base64")).decode("ascii")


def frombase64(s):
    """Bytes a partir de uma string base64."""
    if not isinstance(s, str):
        raise TypeError("bytes.frombase64: esperava str")
    try:
        return _b64.b64decode(s, validate=False)
    except Exception:
        raise ValueError("bytes.frombase64: base64 inválido")


def fromint(n, length=0, byteorder="big"):
    """Inteiro (>= 0) empacotado em bytes. `length` fixa a largura em bytes
    (zero-pad à esquerda em big-endian); omitido, usa o mínimo necessário.
    `byteorder` é 'big' (padrão) ou 'little'."""
    if isinstance(n, bool) or not isinstance(n, int):
        raise TypeError("bytes.fromint: esperava um inteiro")
    if n < 0:
        raise ValueError("bytes.fromint: negativo não suportado")
    if byteorder not in ("big", "little"):
        raise ValueError("bytes.fromint: byteorder deve ser 'big' ou 'little'")
    minimo = (n.bit_length() + 7) // 8 or 1
    if not isinstance(length, int) or isinstance(length, bool):
        raise TypeError("bytes.fromint: length deve ser inteiro")
    largura = length if length > 0 else minimo
    if largura < minimo:
        raise ValueError(f"bytes.fromint: {n} não cabe em {largura} byte(s)")
    return n.to_bytes(largura, byteorder)


def toint(b, byteorder="big"):
    """Desempacota bytes num inteiro (>= 0). `byteorder` 'big' (padrão) ou 'little'."""
    if byteorder not in ("big", "little"):
        raise ValueError("bytes.toint: byteorder deve ser 'big' ou 'little'")
    return int.from_bytes(_as_bytes(b, "toint"), byteorder)


def tolist(b):
    """Lista com o valor inteiro (0-255) de cada byte."""
    return [x for x in _as_bytes(b, "tolist")]


def concat(lista):
    """Junta uma lista de bytes num só."""
    if not isinstance(lista, _TIPO_LISTA):
        raise TypeError("bytes.concat: esperava uma lista de bytes")
    saida = bytearray()
    for i, parte in enumerate(lista):
        if not isinstance(parte, (bytes, bytearray)):
            raise TypeError(f"bytes.concat: item {i} não é bytes ({_nome(parte)})")
        saida += parte
    return bytes(saida)


def slice(b, ini=0, fim=None):
    """Uma fatia dos bytes [ini:fim]. Índices negativos contam do fim."""
    dados = _as_bytes(b, "slice")
    if isinstance(ini, bool) or not isinstance(ini, int):
        raise TypeError("bytes.slice: ini deve ser inteiro")
    if fim is not None and (isinstance(fim, bool) or not isinstance(fim, int)):
        raise TypeError("bytes.slice: fim deve ser inteiro")
    return dados[ini:fim]


def get(b, i):
    """O valor inteiro (0-255) do byte na posição `i` (negativo conta do fim)."""
    dados = _as_bytes(b, "get")
    if isinstance(i, bool) or not isinstance(i, int):
        raise TypeError("bytes.get: índice deve ser inteiro")
    if i < -len(dados) or i >= len(dados):
        raise ValueError(f"bytes.get: índice {i} fora do range (0..{len(dados) - 1})")
    return dados[i]


def xor(dados, chave):
    """XOR byte a byte de `dados` com `chave`. Se a chave for menor, ela repete
    (cifra XOR de chave repetida). Chave vazia é erro."""
    d = _as_bytes(dados, "xor")
    k = _as_bytes(chave, "xor")
    if len(k) == 0:
        raise ValueError("bytes.xor: chave vazia")
    return bytes(d[i] ^ k[i % len(k)] for i in range(len(d)))


EXPORTS = {
    "new": new,
    "fromhex": fromhex,
    "hex": hex,
    "base64": base64,
    "frombase64": frombase64,
    "fromint": fromint,
    "toint": toint,
    "tolist": tolist,
    "concat": concat,
    "slice": slice,
    "get": get,
    "xor": xor,
}
