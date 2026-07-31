"""
Erros nomeados da PoolScript.
Re-exporta de ps_errors para compatibilidade com código existente.
"""
from __future__ import annotations

# Códigos de erro do Spec
ATTRIBUTTED_VALUE_ERROR   = "AtributtedValueError"
OUTPUT_UNEXPECTED_VALUES  = "OutputUnexpectedValues"
SOME_VALUE_UNEXPECTED     = "SomeValueUnexpected"
INDEX_OUT_OF_BOUNDS_WARNING = "IndexOutOfBoundsWarning"

# Cores ANSI (legado — use ps_errors diretamente em código novo)
YELLOW = "\033[33m"
RED    = "\033[31m"
RESET  = "\033[0m"


def warn(message: str, line: int, col: int, code: str = INDEX_OUT_OF_BOUNDS_WARNING) -> None:
    """Warning amarelo no stderr — não interrompe a execução."""
    from .ps_errors import warn as _warn
    _warn(message, line, col, code)
