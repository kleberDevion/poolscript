"""GC sob estresse — paridade INTERP/VM.

Depois de refatorar o mark do GC para uma tabela por-tipo (GC_INFO, cada
ObjType declara seu tracer; boot aborta tipo não-declarado), este teste aloca
MUITO (dicts/listas/strings aninhados) pra forçar várias coletas e garantir que
nada vivo é liberado (use-after-free) e o resultado bate nos dois motores.
"""
import os
import subprocess
import sys
from pathlib import Path

import pytest

RAIZ = Path(__file__).resolve().parent.parent
POOL_BIN = RAIZ / "pool"
assert POOL_BIN.exists(), "binário pool não compilado — rode ./rebuild_vm.sh (VM em C não se pula: skip = falso verde)"
NL = chr(10)

# aninha dict/list/string/tupla e mantém referências vivas cruzadas — se o mark
# esquecer algum tipo, o valor some e a soma diverge (ou trava).
SCRIPT = NL.join([
    "total = 0",
    "guardados = []",
    "for each i in range(120000):",
    '    d = { "a": i, "b": [i, i + 1], "s": str(i), "t": (i, i) }',
    '    l = [d, d["b"], "x" + str(i)]',
    "    if i % 1000 == 0:",
    "        guardados.append(d)",       # mantém alguns vivos entre coletas
    '    total = total + d["a"] + d["b"][0] + d["t"][0]',
    "post(total, len(guardados))",
]) + NL


def _run(cmd, tmp_path):
    entry = tmp_path / "g.ps"
    entry.write_text(SCRIPT, encoding="utf-8")
    env = dict(os.environ)
    env["PYTHONPATH"] = str(RAIZ / "src")
    env["GUZER_HEADLESS"] = "1"
    return subprocess.run(cmd + [str(entry)], capture_output=True, text=True, env=env)


def test_interp_gc_stress(tmp_path):
    r = _run([sys.executable, "-m", "poolscript"], tmp_path)
    assert r.returncode == 0, r.stdout + r.stderr
    # cada i soma a + b[0] + t[0] = 3*i; guardados = os i múltiplos de 1000
    esperado = 3 * sum(range(120000))
    guardados = len(range(0, 120000, 1000))
    assert r.stdout.strip() == str(esperado) + " " + str(guardados), r.stdout


def test_paridade_gc_stress(tmp_path):
    a = _run([sys.executable, "-m", "poolscript"], tmp_path)
    b = _run([str(POOL_BIN)], tmp_path)
    assert a.returncode == 0 and b.returncode == 0, a.stderr + b.stderr
    assert a.stdout == b.stdout, "GC divergiu:" + NL + "INT:" + a.stdout + "VM:" + b.stdout
