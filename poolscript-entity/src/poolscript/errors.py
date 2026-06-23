"""
Erros nomeados da PoolScript (espelham a Tabela de Erros do Compilador no Spec).

Todos herdam de PoolRuntimeError (definido em interpreter.py) através do .code,
mas para evitar import circular, definimos apenas as constantes e helpers aqui.
"""
from __future__ import annotations


# Códigos de erro do Spec (§ Tabela de Erros do Compilador)
ATTRIBUTTED_VALUE_ERROR = "AtributtedValueError"          # str + int, etc.
OUTPUT_UNEXPECTED_VALUES = "OutputUnexpectedValues"        # redeclaração no mesmo escopo
SOME_VALUE_UNEXPECTED = "SomeValueUnexpected"              # operação matemática inválida
INDEX_OUT_OF_BOUNDS_WARNING = "IndexOutOfBoundsWarning"    # acesso fora de range — não é erro fatal


# ANSI cores para o terminal
YELLOW = "\033[33m"
RED = "\033[31m"
RESET = "\033[0m"


def warn(message: str, line: int, col: int, code: str = INDEX_OUT_OF_BOUNDS_WARNING) -> None:
    """Imprime warning amarelo no stderr sem interromper a execução."""
    import sys
    sys.stderr.write(f"{YELLOW}{code}: {message} (L{line}:C{col}){RESET}\n")
    sys.stderr.flush()
