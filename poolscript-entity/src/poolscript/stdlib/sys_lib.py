"""
Módulo `sys` da PoolScript.

Uso:
    import sys

    comando = sys.argv[0]   # primeiro argumento
    acao    = sys.argv[1]   # segundo argumento
"""
from __future__ import annotations
import sys as _sys


class _SysArgv:
    """Acesso aos argumentos da linha de comando."""

    def __getitem__(self, index: int):
        # sys.argv[0] = primeiro arg após o nome do arquivo
        args = _sys.argv[2:]   # [0]=pool [1]=arquivo.ps [2+]=args do dev
        if index < len(args):
            return args[index]
        return None

    def __len__(self):
        return len(_sys.argv[2:])

    def __iter__(self):
        return iter(_sys.argv[2:])

    def __repr__(self):
        return repr(_sys.argv[2:])


argv = _SysArgv()


def exit(code: int = 0):
    """Encerra o programa."""
    _sys.exit(code)


def platform() -> str:
    """Retorna o sistema operacional: 'windows', 'linux', 'darwin'."""
    p = _sys.platform
    if p.startswith("win"):
        return "windows"
    if p.startswith("darwin"):
        return "darwin"
    return "linux"


class _Stdout:
    """sys.stdout — saída padrão.

    write(text)         — escreve sem newline automático
    write(text, end=True) — adiciona \n automaticamente (padrão útil)
    writeln(text)       — sempre adiciona \n (alias conveniente)
    """
    def write(self, text: str, end: bool = False):
        import sys as _s
        _s.stdout.write(str(text) + ("\n" if end else ""))
        _s.stdout.flush()

    def writeln(self, text: str):
        """Escreve text + newline e faz flush automatico."""
        import sys as _s
        _s.stdout.write(str(text) + "\n")
        _s.stdout.flush()

    def flush(self):
        import sys as _s
        _s.stdout.flush()

    def __repr__(self):
        return "<sys.stdout>"


class _Stderr:
    """sys.stderr — saida de erro."""
    def write(self, text: str, end: bool = False):
        import sys as _s
        _s.stderr.write(str(text) + ("\n" if end else ""))
        _s.stderr.flush()

    def writeln(self, text: str):
        """Escreve text + newline e faz flush automatico."""
        import sys as _s
        _s.stderr.write(str(text) + "\n")
        _s.stderr.flush()

    def flush(self):
        import sys as _s
        _s.stderr.flush()

    def __repr__(self):
        return "<sys.stderr>"


stdout = _Stdout()
stderr = _Stderr()


import os as _os

def RelativePath(name: str) -> str:
    """Busca recursivamente a partir do diretório de trabalho atual
    por um arquivo ou pasta com o nome informado.
    Retorna o caminho absoluto (str) se encontrado, ou null se não existir.

    Uso:
        caminho = sys.RelativePath("config.json")
        caminho = sys.RelativePath("assets")
    """
    base = _os.getcwd()
    for root, dirs, files in _os.walk(base):
        if name in files or name in dirs:
            return _os.path.join(root, name)
    return None


EXPORTS = {
    "stdout": stdout,
    "stderr": stderr,
    "argv":     argv,
    "exit":     exit,
    "platform": platform,
    "RelativePath": RelativePath,
}
