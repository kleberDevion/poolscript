"""
VM de bytecode da PoolScript (etapa 1 — ainda em Python, alvo do mypyc).

Caminho planejado:
  1. VM de bytecode em Python           ← esta etapa
  2. mypyc sobre a VM                   → C via C-API, sem tocar na stdlib
  3. porte do loop de execução pra C    → `setup_vm.py` já espera
                                          src/poolscript/vm/poolscript_vm.c

O compilador e a VM cobrem, por ora, o subconjunto medido nos benchmarks
(aritmética, comparação, bitwise, if/while, action/call/return). Nó fora
disso levanta `NaoSuportado` — nunca compila errado em silêncio.
"""
from .compiler import CodeObj, NaoSuportado, compila
from .vm import ErroVM, Funcao, executa, roda

__all__ = [
    "CodeObj", "NaoSuportado", "compila",
    "ErroVM", "Funcao", "executa", "roda",
]
