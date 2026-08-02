"""
PoolScript — Entry point com short-circuit e blindagem total.
"""
import sys

_argv = sys.argv[1:]

# ── Short-circuit: flags que não precisam carregar o interpretador ────────────
if _argv and _argv[0] in ("--version", "-V"):
    from . import __version__
    # runtime tag: o binário `pool` (PSVM, VM em C) mostra "[PSVM]"; aqui roda o
    # interpretador de referência sobre CPython/PyPy — o `--version` já revela o motor.
    try:
        from .ps_errors import RUNTIME
        print(f"PoolScript v{__version__} [{RUNTIME}]")
    except Exception:
        print(f"PoolScript v{__version__}")
    sys.exit(0)

if _argv and _argv[0] in ("--help", "-h", "help"):
    print("""PoolScript — linguagem de programação híbrida

Uso:
  pool arquivo.ps       Roda um arquivo .ps
  pool repl             Abre o REPL interativo
  pool build            Roda todos os .ps da pasta atual
  pool --version        Mostra a versão e runtime
  pool --help           Mostra esta ajuda

Libs: os, dotenv, date, db, mail, request, hash, jwt, jinker, manpu

Contato: poolscript@proton.me
Docs:    https://github.com/poolscript/poolscript/blob/main/Spec.md""")
    sys.exit(0)

# ── Boot completo ─────────────────────────────────────────────────────────────
from .ps_runner import _install_crash_handler
_install_crash_handler()

from .cli import main

if __name__ == "__main__":
    raise SystemExit(main())
