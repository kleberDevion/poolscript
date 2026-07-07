"""
CLI da PoolScript — comandos `pool` e `psl`.

Comandos suportados:
    pool arquivo.ps                Roda um arquivo
    pool build                     Roda todos os .ps da pasta atual
    pool repl                      REPL interativo
    pool --version / -V            Mostra versão e runtime
    pool --help  / -h              Ajuda
    psl //doc                      Imprime URL da spec
    psl install <arquivo.ps>       Instala como comando global
    psl install <arquivo.ps> -asLib   Instala como lib importável (`import nome`)
    psl install <nome> -py         Instala lib Python via pip
    psl install <nome> [-asLib]    Busca <nome> no registro configurado
    psl uninstall <nome> [-asLib|-py]   Remove o que foi instalado
    psl list                       Lista comandos/libs/libs Python instalados
    psl registry set-url <url>     Configura o índice de pacotes
    psl registry show              Mostra o registro configurado
    psl -up release                Stub: sem atualizações
"""
from __future__ import annotations

import sys
from pathlib import Path

# Importações pesadas SEMPRE lazy (dentro das funções)
# Isso garante que `pool --version` responda em <50ms
# e que um erro de import em qualquer módulo não quebre o CLI


CONTACT_EMAIL = "poolscript@proton.me"
SPEC_URL      = "https://github.com/poolscript/poolscript/blob/main/Spec.md"


# ── Entry points instalados pelo pip ─────────────────────────────────────────

def fast_main() -> None:
    """
    Entry point principal: 'pool' e 'psl'.
    Short-circuit para flags simples; execução completa para arquivos.
    """
    argv = sys.argv[1:]

    # ── Short-circuit sem carregar o interpretador ────────────────────
    if not argv or argv[0] in ("--version", "-V"):
        _cmd_version()
        sys.exit(0)

    if argv[0] in ("--help", "-h", "help"):
        _cmd_help()
        sys.exit(0)

    # ── Boot completo — instala proteções e despacha ──────────────────
    from .ps_runner import _install_crash_handler, _first_run_notice
    _install_crash_handler()
    _first_run_notice()

    sys.exit(main(argv))


def main(argv: list[str] | None = None) -> int:
    if argv is None:
        argv = sys.argv[1:]

    if not argv:
        return _cmd_repl()

    cmd = argv[0]

    if cmd in {"--version", "-V"}:
        _cmd_version(); return 0
    if cmd in {"--help", "-h", "help"}:
        _cmd_help(); return 0
    if cmd == "//doc":
        return _cmd_doc()
    if cmd == "install":
        return _cmd_install(argv[1:])
    if cmd == "uninstall":
        return _cmd_uninstall(argv[1:])
    if cmd == "list":
        return _cmd_list()
    if cmd == "registry":
        return _cmd_registry(argv[1:])
    if cmd == "compile":
        return _cmd_compile()
    if cmd == "-up":
        return _cmd_update(argv[1:])
    if cmd == "build":
        return _cmd_build()
    if cmd == "repl":
        return _cmd_repl()

    return _run_file(Path(cmd))


# ── Comandos ─────────────────────────────────────────────────────────────────

def _run_file(path: Path) -> int:
    from .ps_runner import run_file
    return run_file(path)


def _cmd_version() -> None:
    # Importa lazy — nunca falha mesmo se o interpretador tiver problemas
    try:
        from . import __version__
        version = __version__
    except Exception:
        version = "?"
    try:
        from .ps_errors import RUNTIME
        runtime = RUNTIME
    except Exception:
        runtime = f"Python {sys.version_info.major}.{sys.version_info.minor}"
    print(f"PoolScript v{version}  [{runtime}]")


def _cmd_help() -> None:
    try:
        from .ps_errors import RUNTIME
        runtime = RUNTIME
    except Exception:
        runtime = f"Python {sys.version_info.major}.{sys.version_info.minor}"
    print(f"""\
PoolScript — linguagem de programação híbrida  [{runtime}]

Uso:
  pool arquivo.ps       Roda um arquivo .ps
  pool repl             Abre o REPL interativo
  pool build            Roda todos os .ps da pasta atual
  pool --version        Mostra a versão e runtime
  pool --help           Mostra esta ajuda

  psl install arquivo.ps          Instala como comando global
  psl install arquivo.ps -asLib   Instala como lib importável (import nome)
  psl install nome -py            Instala lib Python via pip
  psl install nome [-asLib]       Busca nome no registro configurado
  psl uninstall nome [-asLib|-py] Remove o que foi instalado
  psl list                        Lista comandos/libs/libs Python instalados
  psl registry set-url <url>      Configura o índice de pacotes
  psl registry show               Mostra o registro configurado

Libs: os, dotenv, date, db, mail, request, hash, jwt, jinker, manpu

Contato: {CONTACT_EMAIL}
Docs:    {SPEC_URL}

Performance:
  Para 3-8x mais velocidade, instale com PyPy:
    Linux/macOS:  bash install_pypy.sh
    Windows:      install_pypy.bat""")


def _cmd_repl() -> int:
    from . import __version__
    from .ps_runner import run_repl
    return run_repl(version=__version__)


def _cmd_build() -> int:
    from .ps_runner import run_file
    cwd   = Path.cwd()
    files = sorted(cwd.glob("*.ps"))
    if not files:
        print(f"nenhum arquivo .ps encontrado em {cwd}", file=sys.stderr)
        return 1
    rc     = 0
    passed = 0
    failed = 0
    for f in files:
        print(f"━━━ {f.name} ━━━")
        result = run_file(f)
        if result != 0:
            rc = result
            failed += 1
        else:
            passed += 1
    print(f"\n{passed} arquivo(s) OK, {failed} com erro(s).")
    return rc


def _cmd_doc() -> int:
    print(f"Especificação da PoolScript:\n  {SPEC_URL}")
    return 0


def _cmd_install(args: list[str]) -> int:
    if not args:
        print("uso: psl install <arquivo.ps | nome> [-asLib | -py]", file=sys.stderr)
        return 1
    target = args[0]
    flags = set(args[1:])

    from . import pkgmgr
    try:
        if "-py" in flags:
            msg = pkgmgr.install_py(target)
        elif "-asLib" in flags:
            msg = pkgmgr.install_lib(target)
        else:
            msg = pkgmgr.install_command(target)
        print(msg)
        return 0
    except pkgmgr.PkgmgrError as e:
        print(f"Erro: {e}", file=sys.stderr)
        return 1


def _cmd_uninstall(args: list[str]) -> int:
    if not args:
        print("uso: psl uninstall <nome> [-asLib | -py]", file=sys.stderr)
        return 1
    name = args[0]
    flags = set(args[1:])

    from . import pkgmgr
    try:
        if "-py" in flags:
            msg = pkgmgr.uninstall_py(name)
        elif "-asLib" in flags:
            msg = pkgmgr.uninstall_lib(name)
        else:
            msg = pkgmgr.uninstall_command(name)
        print(msg)
        return 0
    except pkgmgr.PkgmgrError as e:
        print(f"Erro: {e}", file=sys.stderr)
        return 1


def _cmd_list() -> int:
    from . import pkgmgr
    print(pkgmgr.list_installed())
    return 0


def _cmd_registry(args: list[str]) -> int:
    from . import pkgmgr
    if not args or args[0] == "show":
        print(pkgmgr.registry_show())
        return 0
    if args[0] == "set-url":
        if len(args) < 2:
            print("uso: psl registry set-url <url>", file=sys.stderr)
            return 1
        pkgmgr.registry_set_url(args[1])
        print(f"registro configurado: {args[1]}")
        return 0
    print("uso: psl registry [show | set-url <url>]", file=sys.stderr)
    return 1


def _cmd_compile() -> int:
    # Stub — futuro: compilar com mypyc para extensão nativa
    print("compile: não disponível nesta versão.")
    return 0


def _cmd_update(args: list[str]) -> int:
    print("Nenhuma atualização disponível.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
