"""Permite `python -m poolscript arquivo.ps` ou `python -m poolscript --help`.

Short-circuit: flags simples como --version e --help são respondidas
ANTES de importar qualquer módulo da PoolScript.
"""
import sys

# ── Short-circuit ──────────────────────────────────────────────────────
# Checa flags antes de carregar o interpretador inteiro
_argv = sys.argv[1:]

if not _argv or _argv[0] in ("--version", "-V"):
    # Lê a versão direto do pyproject.toml ou do __init__ sem carregar tudo
    import os as _os
    _here = _os.path.dirname(_os.path.abspath(__file__))
    _version = "1.0.8"  # fallback
    try:
        _toml = _os.path.join(_here, "..", "..", "..", "pyproject.toml")
        if _os.path.isfile(_toml):
            with open(_toml) as _f:
                for _line in _f:
                    if _line.startswith("version"):
                        _version = _line.split("=")[1].strip().strip('"')
                        break
    except Exception:
        pass
    if not _argv:
        # sem args → abre REPL (não é short-circuit)
        pass
    else:
        print(f"PoolScript v{_version}")
        sys.exit(0)

if _argv[0] in ("--help", "-h", "help"):
    print("""PoolScript — linguagem de programação híbrida

Uso:
  pool arquivo.ps       Roda um arquivo .ps
  pool repl             Abre o REPL interativo
  pool --version        Mostra a versão
  pool --help           Mostra esta ajuda
  pool build            Roda todos os .ps da pasta atual

Libs disponíveis: os, dotenv, date, db, mail, request, hash, jwt, jinker, manpu

Contato: poolscript@proton.me
Docs:    https://github.com/poolscript/poolscript/blob/main/Spec.md""")
    sys.exit(0)

# ── Boot completo — só aqui carrega o interpretador ───────────────────
from .cli import main

if __name__ == "__main__":
    raise SystemExit(main())
