"""
Módulo `json` da PoolScript.
"""
from __future__ import annotations
import json as _json


def parse(text):
    if isinstance(text, (dict, list)):
        return text
    return _json.loads(text)


def stringify(value) -> str:
    return _json.dumps(value, ensure_ascii=False)


EXPORTS = {
    "parse": parse,
    "stringify": stringify,
}
