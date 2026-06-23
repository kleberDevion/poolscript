"""
CLI da PoolScript — comandos `pool` e `psl`.

Comandos suportados (espelham o spec):
    pool [arquivo.ps]      Roda um arquivo
    pool build             Roda todos os .ps da pasta atual (não recursivo)
    pool --version         Mostra versão
    psl --help / -h        Lista diretório + email de contato
    psl //doc              Imprime URL da spec
    psl install <lib>      Stub: registra lib (placeholder)
    psl -up release        Stub: nenhuma atualização disponível
"""
from __future__ import annotations

import sys
from pathlib import Path

from . import __version__
from .lexer import PoolSyntaxError
from .parser import PoolParseError
from .interpreter import PoolRuntimeError, run_source
from .stdlib import list_libs


CONTACT_EMAIL = "poolscript@proton.me"
SPEC_URL = "https://github.com/poolscript/poolscript/blob/main/Spec.md"


def _run_file(path: Path) -> int:
    if not path.is_file():
        print(f"erro: arquivo não encontrado: {path}", file=sys.stderr)
        return 2
    if path.suffix != ".ps":
        print(f"aviso: {path.name} não tem extensão .ps", file=sys.stderr)
    source = path.read_text(encoding="utf-8")
    try:
        run_source(source, str(path))
    except (PoolSyntaxError, PoolParseError, PoolRuntimeError) as exc:
        print(exc, file=sys.stderr)
        return 2
    return 0


def _cmd_build() -> int:
    cwd = Path.cwd()
    files = sorted(cwd.glob("*.ps"))
    if not files:
        print(f"nenhum arquivo .ps encontrado em {cwd}", file=sys.stderr)
        return 1
    rc = 0
    for f in files:
        print(f"━━━ {f.name} ━━━")
        result = _run_file(f)
        if result != 0:
            rc = result
    return rc


def _cmd_help() -> int:
    print(f"""PoolScript v{__version__}

USO:
  pool <arquivo.ps>       Roda um arquivo PoolScript
  pool build              Roda todos os arquivos .ps da pasta atual
  pool --version          Mostra a versão
  psl --help, -h          Mostra esta ajuda
  psl //doc               Imprime URL da documentação oficial
  psl install <lib>       Registra biblioteca (placeholder nesta versão)
  psl -up release         Verifica atualizações (placeholder)

LIBS DISPONÍVEIS:
  {', '.join(list_libs())}

CONTATO:
  email: {CONTACT_EMAIL}
  docs:  {SPEC_URL}
""")
    return 0


def _cmd_doc() -> int:
    print(SPEC_URL)
    return 0



def _cmd_compile() -> int:
    """Compila a PoolScript com mypyc para código nativo C (3-5x mais rápido)."""
    import subprocess, sys, os
    # cli.py está em src/poolscript/cli.py → sobe 3 níveis para poolscript-entity/
    build_script = Path(__file__).parent.parent.parent / "build_native.py"
    if not build_script.exists():
        # Fallback: cwd atual (usuário rodando de dentro da pasta)
        build_script = Path.cwd() / "build_native.py"
    if not build_script.exists():
        print("❌ build_native.py não encontrado.", file=sys.stderr)
        print("   Baixe o ZIP fonte da PoolScript e execute: python build_native.py")
        return 1
    result = subprocess.run([sys.executable, str(build_script)])
    return result.returncode



def _cmd_uninstall(args: list[str]) -> int:
    """psl uninstall <nome> — remove comando do PATH."""
    import platform as _plat
    import os as _os

    if not args:
        print("uso: psl uninstall <nome>", file=sys.stderr)
        return 1

    cmd_name = args[0].replace(".ps", "")
    system   = _plat.system().lower()

    if system == "windows":
        import sysconfig
        scripts_dir = Path(sysconfig.get_path("scripts"))
        bat = scripts_dir / f"{cmd_name}.bat"
        if bat.exists():
            bat.unlink()
            print(f"[psl uninstall] '{cmd_name}' removido.")
        else:
            print(f"[psl uninstall] '{cmd_name}' não encontrado.")
        return 0

    bin_dirs = [
        Path.home() / ".local" / "bin",
        Path(_os.environ.get("PREFIX", "")) / "bin",
        Path("/usr/local/bin"),
        Path("/usr/bin"),
    ]
    for bin_dir in bin_dirs:
        script = bin_dir / cmd_name
        if script.exists():
            script.unlink()
            print(f"[psl uninstall] '{cmd_name}' removido de {bin_dir}")
            return 0

    print(f"[psl uninstall] '{cmd_name}' não encontrado no PATH.")
    return 1

def _cmd_version() -> int:
    print(f"PoolScript v{__version__}")
    return 0


def _cmd_install(args: list[str]) -> int:
    """psl install arquivo.ps — registra comandos do .ps no PATH do OS.

    Detecta action run_selfwith_ no arquivo e cria um script executável
    com o nome do arquivo (sem .ps) no PATH.
    """
    import os as _os
    import stat as _stat
    import platform as _plat

    if not args:
        print("uso: psl install <arquivo.ps>", file=sys.stderr)
        return 1

    target = args[0]
    ps_file = Path(target)

    # se for um .ps — registra como comando CLI
    if ps_file.suffix == ".ps":
        if not ps_file.is_file():
            print(f"erro: arquivo não encontrado: {ps_file}", file=sys.stderr)
            return 1

        cmd_name = ps_file.stem   # "tes.ps" → "tes"
        ps_abs   = str(ps_file.resolve())

        system = _plat.system().lower()

        if system == "windows":
            # cria .bat no diretório do Python (que está no PATH)
            import sysconfig
            scripts_dir = sysconfig.get_path("scripts")
            bat_path = Path(scripts_dir) / f"{cmd_name}.bat"
            bat_path.write_text(
                f'@echo off\npython -m poolscript "{ps_abs}" %*\n',
                encoding="utf-8"
            )
            print(f"[psl install] comando '{cmd_name}' registrado.")
            print(f"[psl install] use: {cmd_name} <acao> <args>")
        else:
            # Linux/Mac — cria script em /usr/local/bin
            bin_dirs = [
                Path.home() / ".local" / "bin",
                Path(_os.environ.get("PREFIX", "")) / "bin",   # Termux
                Path("/usr/local/bin"),
                Path("/usr/bin"),
            ]
            installed = False
            for bin_dir in bin_dirs:
                if bin_dir.exists() and _os.access(str(bin_dir), _os.W_OK):
                    script = bin_dir / cmd_name
                    script.write_text(
                        f'#!/bin/sh\npython3 -m poolscript "{ps_abs}" "$@"\n',
                        encoding="utf-8"
                    )
                    script.chmod(script.stat().st_mode | _stat.S_IEXEC | _stat.S_IXGRP | _stat.S_IXOTH)
                    print(f"[psl install] comando '{cmd_name}' registrado em {bin_dir}")
                    print(f"[psl install] use: {cmd_name} <acao> <args>")
                    installed = True
                    break
            if not installed:
                print("erro: sem permissão para instalar. Tente com sudo.", file=sys.stderr)
                return 1
        return 0

    # fallback — comportamento antigo
    for lib in args:
        print(f"[psl install] '{lib}' registrada")
    return 0


def _cmd_update(args: list[str]) -> int:
    if args and args[0] == "release":
        print("[psl -up release] nenhuma atualização disponível (placeholder).")
        return 0
    print("uso: psl -up release", file=sys.stderr)
    return 1


def _cmd_repl() -> int:
    from .interpreter import Interpreter
    from .parser import parse_source, PoolParseError
    from .lexer import PoolSyntaxError

    print(f"PoolScript v{__version__} — REPL")
    print("Digite 'sair' ou Ctrl+C para sair.\n")

    interp = Interpreter(source="", filename="<repl>")

    buffer = ""

    while True:
        try:
            prompt = "... " if buffer else ">>> "
            try:
                line = input(prompt)
            except EOFError:
                print()
                break

            if line.strip() in {"sair", "exit", "quit"}:
                break

            buffer += line + "\n"

            # Aguarda fechar todos os blocos abertos
            open_braces = buffer.count("{") - buffer.count("}")
            if open_braces > 0:
                continue

            try:
                program = parse_source(buffer, "<repl>")
                for stmt in program.statements:
                    result = interp.exec_statement(stmt, interp.globals)
                    if result is not None:
                        print(f"=> {interp.stringify(result)}")
                buffer = ""
            except PoolParseError as e:
                print(f"SyntaxError: {e.msg}")
                buffer = ""
            except PoolSyntaxError as e:
                print(f"SyntaxError: {e}")
                buffer = ""
            except PoolRuntimeError as e:
                print(f"RuntimeError: {e.msg}")
                buffer = ""
            except Exception as e:
                print(f"Erro: {e}")
                buffer = ""

        except KeyboardInterrupt:
            if buffer:
                buffer = ""
                print()
            else:
                print()
                break

    print("Até mais!")
    return 0


def fast_main():
    """Entry point com short-circuit — flags simples respondem sem carregar o interpretador."""
    import sys as _sys

    argv = _sys.argv[1:]

    if argv and argv[0] in ("--version", "-V"):
        from . import __version__
        print(f"PoolScript v{__version__}")
        _sys.exit(0)

    if argv and argv[0] in ("--help", "-h", "help"):
        print("""PoolScript — linguagem de programação híbrida

Uso:
  pool arquivo.ps       Roda um arquivo .ps
  pool repl             Abre o REPL interativo
  pool --version        Mostra a versão
  pool --help           Mostra esta ajuda
  pool build            Roda todos os .ps da pasta atual

Libs: os, dotenv, date, db, mail, request, hash, jwt, jinker, manpu

Contato: poolscript@proton.me""")
        _sys.exit(0)

    _sys.exit(main())


def main(argv=None):
    argv = list(sys.argv[1:] if argv is None else argv)

    if not argv:
        return _cmd_repl()

    cmd = argv[0]

    if cmd in {"--version", "-V"}:
        return _cmd_version()
    if cmd in {"--help", "-h", "help"}:
        return _cmd_help()
    if cmd == "//doc":
        return _cmd_doc()
    if cmd == "install":
        return _cmd_install(argv[1:])
    if cmd == "uninstall":
        return _cmd_uninstall(argv[1:])
    if cmd == "compile":
        return _cmd_compile()
    if cmd == "-up":
        return _cmd_update(argv[1:])
    if cmd == "build":
        return _cmd_build()
    if cmd == "repl":
        return _cmd_repl()

    return _run_file(Path(cmd))


if __name__ == "__main__":
    raise SystemExit(main())
