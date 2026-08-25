"""Matriz DIFERENCIAL gerada: milhares de programas, os dois motores, saída
byte a byte igual — INCLUSIVE a mensagem de erro.

Os programas não são escolhidos a dedo: saem do produto cartesiano dos eixos
da linguagem (tests/matriz_casos.py). É a rede para a classe de bug que
escapava — o caso que ninguém pensou em escrever à mão.

    pytest tests/test_matriz_diferencial.py                 # subconjunto
    MATRIZ_COMPLETA=1 pytest tests/test_matriz_diferencial.py   # tudo

Comparação: código de saída, stdout e a 1ª linha do stderr (tipo + mensagem).
Divergir em QUALQUER um é falha — o interpretador é a autoridade.
"""
import os
import subprocess
import sys
from pathlib import Path

import pytest

from matriz_casos import casos

RAIZ = Path(__file__).resolve().parent.parent
POOL = RAIZ / "pool"
SRC = RAIZ / "src"

# VM em C NÃO se pula: sem o binário o teste FALHA (skip = falso verde).
assert POOL.exists(), "binário 'pool' não compilado — rode ./rebuild_vm.sh."

INTERP = [sys.executable, "-m", "poolscript"]
VM = [str(POOL)]

_ANSI = __import__("re").compile(r"\x1b\[[0-9;]*m")


def _roda(cmd, arquivo):
    env = dict(os.environ, PYTHONPATH=str(SRC), NO_COLOR="1", GUZER_HEADLESS="1")
    try:
        p = subprocess.run(cmd + [str(arquivo)], capture_output=True, text=True,
                           env=env, timeout=25)
    except subprocess.TimeoutExpired:
        return ("TIMEOUT", "", "")
    err = _ANSI.sub("", p.stderr).strip().splitlines()
    # 1ª linha do stderr = "Tipo: mensagem" (o resto é traceback/cursor)
    return (p.returncode, p.stdout, err[0] if err else "")


def _pares():
    return [pytest.param(src, id=cid) for cid, src in casos()]


# ── catraca: divergências já conhecidas ─────────────────────────────────────
# Enquanto os dois motores existirem, um punhado de casos ainda diverge no
# TEXTO do erro. Em vez de deixar o arquivo vermelho (o que faz todo mundo
# parar de olhar) ou de silenciar (falso verde), a lista fica EXPLÍCITA e o
# teste vira catraca: divergência nova quebra, e divergência consertada TAMBÉM
# quebra — pedindo pra tirar da lista. Só encolhe.
BASE = Path(__file__).parent / "matriz_divergencias_conhecidas.txt"


def _conhecidas() -> dict:
    fora = {}
    if BASE.is_file():
        for ln in BASE.read_text(encoding="utf-8").splitlines():
            if not ln.strip() or ln.startswith("#"):
                continue
            classe, _, cid = ln.partition("\t")
            fora[cid.strip()] = classe.strip()
    return fora


CONHECIDAS = _conhecidas()


@pytest.mark.parametrize("fonte", _pares())
def test_motores_concordam(tmp_path, fonte, request):
    cid = request.node.callspec.id
    arq = tmp_path / "m.ps"
    arq.write_text(fonte, encoding="utf-8")
    rc_i, out_i, err_i = _roda(INTERP, arq)
    rc_v, out_v, err_v = _roda(VM, arq)

    assert rc_i != "TIMEOUT", f"interp travou\n--- fonte\n{fonte}"
    assert rc_v != "TIMEOUT", f"VM travou\n--- fonte\n{fonte}"

    ok_i, ok_v = rc_i == 0, rc_v == 0
    divergiu = None
    if ok_i != ok_v:
        divergiu = ("ACEITA", f"interp rc={rc_i} {err_i!r} | VM rc={rc_v} {err_v!r}")
    elif out_i != out_v:
        divergiu = ("SAIDA", f"interp {out_i!r} | VM {out_v!r}")
    elif not ok_i and err_i != err_v:
        classe = "TIPO" if err_i.split(":")[0] != err_v.split(":")[0] else "MSG"
        divergiu = (classe, f"interp {err_i!r} | VM {err_v!r}")

    conhecida = CONHECIDAS.get(cid)
    if divergiu and not conhecida:
        pytest.fail(f"DIVERGÊNCIA NOVA ({divergiu[0]})\n--- fonte\n{fonte}--- {divergiu[1]}")
    if conhecida and not divergiu:
        pytest.fail(
            f"este caso PAROU de divergir — tire a linha de "
            f"tests/matriz_divergencias_conhecidas.txt:\n  {conhecida}\t{cid}")


def test_catraca_nao_cresceu():
    """A lista de divergências conhecidas é um teto — nunca um depósito."""
    n = len(CONHECIDAS)
    assert n <= 198, (
        f"a catraca subiu para {n} divergências (teto 198): alguma mudança "
        f"aumentou a diferença entre os motores em vez de diminuir")


def test_matriz_tem_tamanho():
    """Guard: se o gerador quebrar e devolver pouco, a matriz vira teatro."""
    n = sum(1 for _ in casos())
    assert n >= 300, f"matriz reduzida demais: {n} casos"
