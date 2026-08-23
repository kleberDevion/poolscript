"""Doc viva: os exemplos da doc de builtins e métodos de string EXECUTAM.

Cada página em docs/builtins/ e docs/string/ traz pares de blocos

    ```ps
    post(len("abc"))
    ```
    ```saida
    3
    ```

Este teste extrai todos os pares e roda o código nos DOIS motores — a saída
tem que bater com a documentada nos dois. Documentação desatualizada quebra a
suíte em vez de apodrecer em silêncio.
"""
import io
import os
import re
import sys
from contextlib import redirect_stdout
from pathlib import Path

import pytest

from poolscript.interpreter import Interpreter
from poolscript.parser import parse_source

vm = pytest.importorskip(
    "poolscript.vm.poolscript_vm",
    reason="extensão C não compilada — rode: ./rebuild_vm.sh",
)

RAIZ = Path(__file__).resolve().parent.parent
PASTAS = [RAIZ / "docs" / "builtins", RAIZ / "docs" / "string",
          RAIZ / "docs" / "sockets"]

# o par pode ter o fechamento do <details> (código dobrável + print) entre
# os fences — tolera qualquer coisa que NÃO seja outro bloco no meio
BLOCO = re.compile(r"```ps\n(.*?)```\s*(?:</details>)?\s*```saida\n(.*?)```", re.S)


def casos():
    for pasta in PASTAS:
        for md in sorted(pasta.rglob("*.md")):
            texto = md.read_text(encoding="utf-8")
            for i, (codigo, saida) in enumerate(BLOCO.findall(texto)):
                rel = md.relative_to(RAIZ / "docs")
                yield pytest.param(codigo.rstrip("\n"), saida.rstrip("\n"),
                                   id=f"{rel}#{i}")


def via_interpretador(src):
    buf = io.StringIO()
    i = Interpreter(source=src, filename="<doc>")
    with redirect_stdout(buf):
        i.run(parse_source(src, "<doc>"))
    return buf.getvalue().rstrip("\n")


def via_c(src):
    sys.stdout.flush()
    r, w = os.pipe()
    original = os.dup(1)
    try:
        os.dup2(w, 1)
        os.close(w)
        try:
            vm.executa_fonte(src)
        finally:
            sys.stdout.flush()
            os.dup2(original, 1)
    finally:
        os.close(original)
    with os.fdopen(r, "rb") as f:
        return f.read().decode("utf-8", "replace").rstrip("\n")


CASOS = list(casos())


def test_coleta_nao_vazia():
    # parametrize com lista VAZIA skipa em silêncio = falso verde. Se a doc
    # mudar de formato e a extração parar de achar os pares, este teste FALHA.
    assert len(CASOS) >= 90, f"extracao da doc viva quebrou: só {len(CASOS)} exemplos achados"


@pytest.mark.parametrize("codigo,saida", CASOS)
def test_exemplo_da_doc(codigo, saida):
    assert via_interpretador(codigo) == saida, "doc diverge do INTERPRETADOR"
    assert via_c(codigo) == saida, "doc diverge da VM"
