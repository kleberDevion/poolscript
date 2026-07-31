#!/usr/bin/env python3
"""Sincroniza bridge/poolscript_pkg/{lexer,parser,ps_errors}.py com o
poolscript-lang REAL instalado no Python atual.

O bridge de análise estática da extensão (bridge/analyze.py) roda com uma
cópia local de lexer/parser/ps_errors — nunca importa o pacote `poolscript`
inteiro, só isso, pra nunca correr o risco de executar código do usuário.
Antes essa cópia era atualizada manualmente e ficou parada numa versão bem
mais antiga da linguagem (o diff contra o real chegava a ~100% das linhas),
o que fazia a extensão dar diagnóstico/autocomplete errado pra qualquer
sintaxe adicionada depois daquele snapshot.

Rodar de novo sempre que lexer.py/parser.py/ps_errors.py mudarem no
poolscript-lang real:
    python bridge/sync_parser.py
"""
from __future__ import annotations
import shutil
import sys
from pathlib import Path

import poolscript  # precisa do poolscript-lang instalado (pip install -e .)

FILES = ["lexer.py", "parser.py", "ps_errors.py"]


def main():
    real_dir = Path(poolscript.__file__).parent
    dest_dir = Path(__file__).parent / "poolscript_pkg"
    for fname in FILES:
        src = real_dir / fname
        if not src.is_file():
            print(f"[erro] {src} não encontrado", file=sys.stderr)
            sys.exit(1)
        shutil.copy2(src, dest_dir / fname)
        print(f"sincronizado: {fname}")
    print(f"\nde: {real_dir}\npara: {dest_dir}")


if __name__ == "__main__":
    main()
