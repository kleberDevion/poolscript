"""
CLI da PoolScript — comandos `pool` e `psl`.

Comandos suportados:
    pool arquivo.ps                Roda um arquivo
    pool build                     Roda todos os .ps da pasta atual
    pool repl                      REPL interativo
    pool --version / -V            Mostra versão e runtime
    pool --help  / -h              Ajuda
    psl //doc                      Imprime URL da spec
    psl install <arquivo.ps>       Instala (lê #!lib / #!cmd na 1ª linha; padrão: comando)
    psl install <arquivo.ps> -asLib   Força lib importável (`import nome`)
    psl install <nome> -py         Instala lib Python via pip
    psl install <nome> [-asLib]    Busca <nome> no registro configurado
    psl uninstall <nome>           Remove (acha sozinho: comando, lib pool ou lib py)
    psl uninstall <nome> [-asLib|-py]   Força a categoria (desambigua)
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
    # OBS: "sem argumento nenhum" NÃO entra aqui — precisa cair no main(argv)
    # abaixo pra abrir o REPL (_cmd_repl), igual a `python`/`node` sem args.
    if argv and argv[0] in ("--version", "-V"):
        _cmd_version()
        sys.exit(0)

    if argv and argv[0] in ("--help", "-h", "help"):
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
    # runtime tag: o binário `pool` (VM em C) mostra "[PSVM]"; o interpretador
    # de referência roda sobre CPython/PyPy e diz onde está hospedado — assim o
    # `--version` sozinho já revela em qual motor você está.
    try:
        from .ps_errors import RUNTIME
        print(f"PoolScript v{version} [{RUNTIME}]")
    except Exception:
        print(f"PoolScript v{version}")


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

  psl install arquivo.ps          Instala (o arquivo decide via #!lib / #!cmd)
  psl install arquivo.ps -asLib   Força lib importável (import nome)
  psl install nome -py            Instala lib Python via pip
  psl install nome [-asLib]       Busca nome no registro configurado
  psl uninstall nome              Remove (acha sozinho: comando/lib pool/lib py)
  psl uninstall nome [-asLib|-py] Força a categoria, se estiver em mais de uma
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
            # sem flag: o arquivo decide via marcador #!lib / #!cmd (padrão: comando)
            msg = pkgmgr.install_auto(target)
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
            # sem flag: descobre sozinho (comando, lib pool ou lib py)
            msg = pkgmgr.uninstall_auto(name)
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
    if not args or args[0] != "release":
        print("uso: psl -up release", file=sys.stderr)
        return 1

    import re
    import subprocess

    from . import __version__ as current_version

    # src/poolscript/cli.py -> raiz do projeto (2 níveis acima). Só funciona
    # numa instalação via clone git (editable install) — não faz sentido pra
    # instalação via wheel/PyPI, que não tem histórico git pra comparar.
    repo_root = Path(__file__).resolve().parents[2]
    if not (repo_root / ".git").is_dir():
        print("Instalação não é um clone git — não dá pra verificar atualização automaticamente.")
        print("Baixe a versão mais nova em: https://github.com/kleberDevion/poolscript-lang")
        return 1

    def _git(*a: str) -> subprocess.CompletedProcess:
        return subprocess.run(["git", *a], cwd=repo_root, capture_output=True, text=True)

    fetch = _git("fetch", "--quiet", "origin")
    if fetch.returncode != 0:
        print(f"Erro ao buscar atualizações: {fetch.stderr.strip()}", file=sys.stderr)
        return 1

    branch = _git("rev-parse", "--abbrev-ref", "HEAD").stdout.strip()
    local_head = _git("rev-parse", "HEAD").stdout.strip()
    remote_head = _git("rev-parse", f"origin/{branch}").stdout.strip()

    if local_head == remote_head:
        print(f"PoolScript v{current_version} já é a versão mais recente ({branch} em dia).")
        return 0

    counts = _git("rev-list", "--left-right", "--count", f"{local_head}...{remote_head}").stdout.split()
    behind = int(counts[1]) if len(counts) == 2 else 0
    if behind == 0:
        print(f"PoolScript v{current_version}: sua cópia está à frente de origin/{branch} (nada pra atualizar).")
        return 0

    # --ff-only: nunca reescreve histórico nem descarta mudança local — se
    # houver commit/edição local que conflite, falha limpo sem tocar em nada.
    merge = _git("merge", "--ff-only", f"origin/{branch}")
    if merge.returncode != 0:
        print("Não deu pra atualizar automaticamente (histórico divergiu ou há mudanças locais).", file=sys.stderr)
        print(f"Resolva manualmente em {repo_root} (ex: 'git status', 'git pull').", file=sys.stderr)
        return 1

    new_version = "?"
    try:
        pyproject_text = (repo_root / "pyproject.toml").read_text(encoding="utf-8")
        m = re.search(r'^version\s*=\s*"([^"]+)"', pyproject_text, re.MULTILINE)
        if m:
            new_version = m.group(1)
    except OSError:
        pass

    print(f"Atualizado: v{current_version} -> v{new_version} ({behind} commit(s) novo(s) em {branch}).")
    print("Reinicie 'pool'/'psl' para usar a versão nova.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
