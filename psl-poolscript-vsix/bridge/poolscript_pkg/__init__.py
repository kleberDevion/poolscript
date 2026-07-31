"""Cópia mínima de poolscript (lexer + parser) usada só pelo bridge de análise
estática da extensão VS Code. NUNCA importa/roda o interpretador — só lexa e
parseia, igual a uma checagem estática (mesma garantia do Pylance: não
executa o código do usuário).

Mantida em sync manualmente com src/poolscript/{lexer,parser,ps_errors}.py do
projeto poolscript-lang. Se o parser real ganhar uma feature nova, atualize
esta cópia também.
"""
from .lexer import Lexer, PoolSyntaxError, Token
from .parser import parse_source, Parser, PoolParseError

__all__ = ["Lexer", "PoolSyntaxError", "Token", "parse_source", "Parser", "PoolParseError"]
