"""
PoolScript — Runner Unificado (PyPy + CPython)
===============================================
Ponto de entrada único para executar código .ps.
Garante:
  1. Nenhum traceback Python vaza para o usuário final.
  2. sys.exit limpo com código correto em todo caminho de erro.
  3. Detecção automática de PyPy e log de runtime no modo --verbose.
  4. Captura de TODOS os tipos de exceção, incluindo os inesperados.
"""
from __future__ import annotations

import sys
import os
import signal
from pathlib import Path
from typing import Optional

from .ps_errors import (
    PoolError, _BasePoolSyntaxError as PoolSyntaxError,
    _BasePoolParseError as PoolParseError, _BasePoolRuntimeError as PoolRuntimeError,
    PoolInternalError, shield, _FakeNode, IS_PYPY, RUNTIME,
    RED, BOLD, DIM, YELLOW,
)

# Sinais de controle interno — nunca devem vazar para o usuário
_INTERNAL_SIGNALS = frozenset({
    "ReturnSignal", "BreakSignal", "ContinueSignal", "YieldSignal",
})


def run_file(path: Path, verbose: bool = False) -> int:
    """
    Executa um arquivo .ps.
    Retorna 0 em sucesso, 1 em erro de usuário, 2 em erro crítico.
    Nunca levanta exceção — sempre encapsula e imprime.
    """
    if not path.exists():
        _print_err(f"arquivo não encontrado: {path.name}")
        return 2
    if not path.is_file():
        _print_err(f"não é um arquivo: {path.name}")
        return 2
    if path.suffix != ".ps":
        _print_warn(f"{path.name} não tem extensão .ps — tentando mesmo assim")

    try:
        source = path.read_text(encoding="utf-8")
    except UnicodeDecodeError:
        _print_err(f"erro de codificação ao ler {path.name} — certifique-se que o arquivo está em UTF-8")
        return 2
    except OSError as e:
        _print_err(f"não foi possível ler {path.name}: {e.strerror}")
        return 2

    if verbose:
        _print_info(f"runtime: {RUNTIME}")
        _print_info(f"arquivo: {path.name} ({len(source)} bytes)")

    return _execute(source, str(path), verbose=verbose)


def run_source(source: str, filename: str = "<script>", verbose: bool = False) -> int:
    """
    Executa código-fonte diretamente (usado no REPL e em testes).
    Retorna código de saída.
    """
    return _execute(source, filename, verbose=verbose)


def run_source_raises(source: str, filename: str = "<script>") -> list[str]:
    """
    Executa código-fonte e retorna output. Levanta PoolError em caso de erro.
    Usado internamente pelos testes.
    """
    from .lexer import Lexer
    from .parser import Parser
    from .interpreter import Interpreter

    tokens  = Lexer(source, filename).tokenize()
    program = Parser(tokens, source, filename).parse()
    interp  = Interpreter(source=source, filename=filename)
    return interp.run(program)


# ── Execução interna ──────────────────────────────────────────────────────────

def _execute(source: str, filename: str, verbose: bool = False) -> int:
    """Núcleo de execução com camadas de proteção."""
    try:
        return _run_pipeline(source, filename, verbose)

    # ── Erros PoolScript conhecidos ──────────────────────────────────
    except PoolError as e:
        sys.stderr.write(e.pool_message() + "\n")
        sys.stderr.flush()
        return 1

    # ── Sinais de controle que vazaram do interpretador ───────────────
    except Exception as exc:
        name = type(exc).__name__
        if name in _INTERNAL_SIGNALS:
            # ReturnSignal etc. vazando do topo-level — bug no interpretador
            err = PoolInternalError(context=f"sinal de controle não capturado: {name}")
            sys.stderr.write(err.pool_message() + "\n")
            sys.stderr.flush()
            return 2

        # ── Exceções Python puras ─────────────────────────────────────
        # Converte para PoolError sem expor o traceback Python
        pool_err = shield(exc, node=_FakeNode(0, 0), source=source, filename=filename)
        sys.stderr.write(pool_err.pool_message() + "\n")
        sys.stderr.flush()
        return 1

    # ── KeyboardInterrupt ─────────────────────────────────────────────
    except KeyboardInterrupt:
        sys.stderr.write("\n" + DIM("Execução interrompida pelo usuário.") + "\n")
        sys.stderr.flush()
        return 130   # convenção Unix: 128 + SIGINT

    # ── SystemExit do próprio script ──────────────────────────────────
    except SystemExit as e:
        return int(e.code) if e.code is not None else 0


def _run_pipeline(source: str, filename: str, verbose: bool) -> int:
    """
    Pipeline: Lexer → Parser → Interpreter.
    Erros aqui são sempre PoolError — nunca Python puro.
    """
    # Importa lazily para não carregar tudo em flags como --version
    from .lexer       import Lexer
    from .parser      import Parser
    from .interpreter import Interpreter

    # ── Fase 1: Lexer ────────────────────────────────────────────────
    try:
        tokens = Lexer(source, filename).tokenize()
    except PoolSyntaxError:
        raise
    except Exception as exc:
        raise shield(exc, _FakeNode(0, 0), source, filename) from None

    if verbose:
        _print_info(f"tokens: {len(tokens)}")

    # ── Fase 2: Parser ───────────────────────────────────────────────
    try:
        program = Parser(tokens, source, filename).parse()
    except (PoolSyntaxError, PoolParseError):
        raise
    except Exception as exc:
        raise shield(exc, _FakeNode(0, 0), source, filename) from None

    if verbose:
        _print_info(f"statements: {len(program.statements)}")

    # ── Fase 3: Interpreter ──────────────────────────────────────────
    try:
        interp = Interpreter(source=source, filename=filename)
        interp.run(program)
    except (PoolSyntaxError, PoolParseError, PoolRuntimeError):
        raise
    except SystemExit:
        raise
    except KeyboardInterrupt:
        raise
    except Exception as exc:
        raise shield(exc, _FakeNode(0, 0), source, filename) from None

    return 0


# ── REPL ──────────────────────────────────────────────────────────────────────

def run_repl(version: str = "?") -> int:
    """
    REPL interativo com blindagem total.
    Nenhum erro interrompe a sessão — sempre imprime e continua.
    """
    from .interpreter import Interpreter
    from .lexer       import Lexer, PoolSyntaxError as _SE
    from .parser      import Parser, PoolParseError as _PE

    print(f"PoolScript v{version} — REPL  [{RUNTIME}]")
    print("Digite 'sair', 'exit' ou Ctrl+C para sair.\n")

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

            # aguarda fechar todos os blocos abertos
            if buffer.count("{") > buffer.count("}"):
                continue

            # ── tenta executar o buffer ───────────────────────────────
            try:
                tokens  = Lexer(buffer, "<repl>").tokenize()
                program = Parser(tokens, buffer, "<repl>").parse()
                for stmt in program.statements:
                    result = interp.exec_statement(stmt, interp.globals)
                    if result is not None:
                        print(f"=> {interp.stringify(result)}")
                buffer = ""

            except (_SE, _PE, PoolRuntimeError) as e:
                # erros PoolScript — formata e continua
                err_msg = e.pool_message() if isinstance(e, PoolError) else str(e)
                sys.stderr.write(err_msg + "\n")
                sys.stderr.flush()
                buffer = ""

            except SystemExit as e:
                return int(e.code) if e.code is not None else 0

            except Exception as exc:
                # qualquer outro erro Python — escuda e continua
                pool_err = shield(exc, _FakeNode(0, 0), buffer, "<repl>")
                sys.stderr.write(pool_err.pool_message() + "\n")
                sys.stderr.flush()
                buffer = ""

        except KeyboardInterrupt:
            if buffer:
                buffer = ""
                print()
            else:
                print("\nUse 'sair' para sair.")

    return 0


# ── Helpers de output ─────────────────────────────────────────────────────────

def _print_err(msg: str) -> None:
    sys.stderr.write(BOLD(RED("Erro")) + f": {msg}\n")
    sys.stderr.flush()

def _print_warn(msg: str) -> None:
    sys.stderr.write(BOLD(YELLOW("Aviso")) + f": {msg}\n")
    sys.stderr.flush()

def _print_info(msg: str) -> None:
    sys.stderr.write(DIM(f"[pool] {msg}") + "\n")
    sys.stderr.flush()


# ── Instalação do handler de sinal SIGSEGV (PyPy/CPython) ────────────────────
# Em caso de crash nativo (ex: extensão C corrompida), tenta dar mensagem limpa

def _install_crash_handler() -> None:
    """Instala handler para sinais fatais — melhor esforço, não garantido."""
    def _crash(signum, frame):
        sys.stderr.write(
            "\n" + BOLD(RED("Erro Crítico")) +
            ": o interpretador encerrou inesperadamente.\n"
            "Por favor, reporte em: https://github.com/poolscript/poolscript/issues\n"
        )
        sys.stderr.flush()
        os._exit(3)

    for sig in (signal.SIGABRT,):
        try:
            signal.signal(sig, _crash)
        except (OSError, ValueError):
            pass   # plataforma não suporta — ignora silenciosamente


# ── Mensagem de primeiro uso ──────────────────────────────────────────────────

def _first_run_notice() -> None:
    """
    Imprime orientação sobre PyPy apenas na primeira execução.
    Cria um marcador em ~/.poolscript/.noticed para não repetir.
    """
    import os
    marker_dir  = Path.home() / ".poolscript"
    marker_file = marker_dir / ".noticed"

    if marker_file.exists():
        return

    try:
        marker_dir.mkdir(exist_ok=True)
        marker_file.touch()
    except OSError:
        return   # sem permissão — ignora silenciosamente

    if IS_PYPY:
        msg = (
            "\n✓ PoolScript rodando em PyPy — JIT ativo para alta performance.\n"
        )
    else:
        msg = (
            "\n💡 Dica: instale com PyPy para 3-8x mais performance.\n"
            "   Linux/macOS: bash install_pypy.sh  |  Windows: install_pypy.bat\n"
        )

    sys.stderr.write(DIM(msg) + "\n")
    sys.stderr.flush()
