"""PoolScript - linguagem de programação híbrida (dinâmica/estática)."""
from .interpreter import run_source, Interpreter, PoolRuntimeError
from .parser import parse_source, PoolParseError
from .lexer import Lexer, PoolSyntaxError

__version__ = "4.5.10"

__all__ = [
    "run_source",
    "parse_source",
    "Interpreter",
    "Lexer",
    "PoolSyntaxError",
    "PoolParseError",
    "PoolRuntimeError",
    "__version__",
]
