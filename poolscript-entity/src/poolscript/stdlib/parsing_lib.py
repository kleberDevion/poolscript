"""
Módulo `Parsing` da PoolScript — dados transientes + conversões de tipo.
"""
from __future__ import annotations
import re as _re
import json as _json


class TransientValue:
    """Wrapper transiente — carrega valor temporário na stack frame.
    Consumido e descartado após a instrução. String original intacta.
    """
    __slots__ = ("value", "origin_type")

    def __init__(self, value, origin_type: str):
        self.value = value
        self.origin_type = origin_type

    def type(self) -> str:
        return self.origin_type

    # Transparência aritmética
    def __int__(self):      return int(self.value)
    def __float__(self):    return float(self.value)
    def __bool__(self):     return bool(self.value)
    def __index__(self):    return int(self.value)
    def __len__(self):      return len(str(self.value))
    def __repr__(self):     return repr(self.value)
    def __str__(self):      return str(self.value)
    def __eq__(self, o):    return self.value == (o.value if isinstance(o, TransientValue) else o)
    def __ne__(self, o):    return self.value != (o.value if isinstance(o, TransientValue) else o)
    def __lt__(self, o):    return self.value < (o.value if isinstance(o, TransientValue) else o)
    def __le__(self, o):    return self.value <= (o.value if isinstance(o, TransientValue) else o)
    def __gt__(self, o):    return self.value > (o.value if isinstance(o, TransientValue) else o)
    def __ge__(self, o):    return self.value >= (o.value if isinstance(o, TransientValue) else o)
    def __add__(self, o):   return TransientValue(self.value + (o.value if isinstance(o, TransientValue) else o), self.origin_type)
    def __radd__(self, o):  return TransientValue((o.value if isinstance(o, TransientValue) else o) + self.value, self.origin_type)
    def __sub__(self, o):   return TransientValue(self.value - (o.value if isinstance(o, TransientValue) else o), self.origin_type)
    def __mul__(self, o):   return TransientValue(self.value * (o.value if isinstance(o, TransientValue) else o), self.origin_type)
    def __truediv__(self, o): return TransientValue(self.value / (o.value if isinstance(o, TransientValue) else o), "flo")
    def __mod__(self, o):   return TransientValue(self.value % (o.value if isinstance(o, TransientValue) else o), self.origin_type)

    def len(self) -> int:
        return len(str(self.value))


def _clean_numeric(value: str, to_type: str) -> str:
    """Extrai caracteres numéricos — Read-Only."""
    s = str(value)
    if to_type in ("int", "str"):
        return _re.sub(r"[^\d]", "", s)
    elif to_type in ("flo", "float"):
        # suporte a separador BR: 1.299,90 → 1299.9
        s = s.replace(" ", "")
        if "," in s and "." in s:
            s = s.replace(".", "").replace(",", ".")
        elif "," in s:
            s = s.replace(",", ".")
        cleaned = _re.sub(r"[^\d.]", "", s)
        parts = cleaned.split(".")
        if len(parts) > 2:
            cleaned = parts[0] + "." + "".join(parts[1:])
        return cleaned
    return s


class _ParsingModule:
    """Objeto builtin `Parsing` — disponível globalmente."""

    # ── string ───────────────────────────────────────────────────────
    def string(self, value, to_type: str = "str"):
        """Remove caracteres não textuais e retorna PoolStr."""
        from .strmethod_lib import PoolStr
        cleaned = _clean_numeric(str(value), "str")
        if to_type == "int":
            return TransientValue(int(cleaned) if cleaned else 0, "int")
        elif to_type in ("flo", "float"):
            return TransientValue(float(cleaned) if cleaned else 0.0, "flo")
        return PoolStr(" ".join(str(value).split()))

    # ── integer ──────────────────────────────────────────────────────
    def integer(self, value, to_type: str = "int") -> TransientValue:
        """Converte para inteiro — trunca float sem arredondar.
        123.7 → 123, não 1237.
        """
        if isinstance(value, float):
            return TransientValue(int(value), "int")
        if isinstance(value, int):
            return TransientValue(value, "int")
        cleaned = _clean_numeric(str(value), "int")
        # se tiver ponto, trunca antes dele
        if "." in cleaned:
            cleaned = cleaned.split(".")[0]
        return TransientValue(int(cleaned) if cleaned else 0, "int")

    # ── floating ─────────────────────────────────────────────────────
    def floating(self, value, to_type: str = "flo") -> TransientValue:
        """Converte para float — suporte a separador BR."""
        if isinstance(value, (int, float)):
            return TransientValue(float(value), "flo")
        cleaned = _clean_numeric(str(value), "flo")
        return TransientValue(float(cleaned) if cleaned else 0.0, "flo")

    # ── boolean ──────────────────────────────────────────────────────
    def boolean(self, value, to_type: str = "bool") -> bool:
        """Converte para bool."""
        if isinstance(value, str):
            return value.lower() not in ("", "0", "false", "null", "none")
        return bool(value)

    # ── TransientValue (genérico) ─────────────────────────────────────
    def TransientValue(self, value, to_type: str = "str"):
        """Converte preservando o valor original sem distorção.
        
        float → int: trunca (123.7 → 123, não 1237)
        str   → int: extrai dígitos antes do ponto
        str   → flo: suporte a separador BR
        """
        if to_type == "int":
            return self.integer(value, "int")
        elif to_type in ("flo", "float"):
            return self.floating(value, "flo")
        elif to_type == "str":
            from .strmethod_lib import PoolStr
            return PoolStr(str(value))
        return value

    # ── JSON ─────────────────────────────────────────────────────────
    def JSONformatt(self, value, to_type: str = "json"):
        """Converte para dict/list (JSON).
        str  → parse JSON
        dict → retorna como está
        list → retorna como está
        """
        if isinstance(value, (dict, list)):
            return value
        if isinstance(value, str):
            try:
                return _json.loads(value)
            except Exception:
                return {}
        return {}

    # ── Array ─────────────────────────────────────────────────────────
    def Arrayformatt(self, value, to_type: str = "list") -> list:
        """Converte para lista.
        tuple/set → list
        str       → lista de caracteres
        dict      → lista de chaves
        qualquer  → [value]
        """
        if isinstance(value, list):
            return value
        if isinstance(value, (tuple, set)):
            return list(value)
        if isinstance(value, dict):
            return list(value.keys())
        if isinstance(value, str):
            return list(value)
        return [value]

    # ── Tupla ─────────────────────────────────────────────────────────
    def Tuplasformatt(self, value, to_type: str = "tup") -> tuple:
        """Converte para tupla."""
        if isinstance(value, tuple):
            return value
        if isinstance(value, (list, set)):
            return tuple(value)
        if isinstance(value, str):
            return tuple(value)
        return (value,)

    def __repr__(self):
        return "<Parsing>"


Parsing = _ParsingModule()

EXPORTS = {
    "Parsing": Parsing,
    "TransientValue": TransientValue,
}
