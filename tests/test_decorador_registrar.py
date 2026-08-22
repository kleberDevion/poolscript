"""Decorador geral `@obj.metodo(...)` com registrar que é Entity do usuário.

Antes divergia: funcionava na VM em C (protocolo genérico
`registrar.register(action)`) mas o interp errava ("BoundMethod not callable"),
porque o caminho do interp era especializado em jinker (getattr(...)() do Python
+ embrulho req/res). Agora o interp trata registrar-Entity pelo motor, com a
action pura — paridade com a VM.
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

SCRIPT = NL.join([
    "Entity Reg():",
    "    action rota(self, path):",
    '        post("preparando", path)',
    "        return self",
    "    action register(self, fn):",
    '        post("registrou:", fn())',
    "        return fn",
    "app = Reg()",
    '@app.rota("/x")',
    "action handler():",
    '    return "oi"',
]) + NL

ESPERADO = ["preparando /x", "registrou: oi"]


def _run(cmd, tmp_path):
    entry = tmp_path / "d.ps"
    entry.write_text(SCRIPT, encoding="utf-8")
    env = dict(os.environ)
    env["PYTHONPATH"] = str(RAIZ / "src")
    env["GUZER_HEADLESS"] = "1"
    return subprocess.run(cmd + [str(entry)], capture_output=True, text=True, env=env)


def test_interp_decorador_registrar_entity(tmp_path):
    r = _run([sys.executable, "-m", "poolscript"], tmp_path)
    assert r.returncode == 0, r.stdout + r.stderr
    assert r.stdout.strip().splitlines() == ESPERADO, r.stdout


def test_paridade_decorador_registrar_entity(tmp_path):
    a = _run([sys.executable, "-m", "poolscript"], tmp_path).stdout
    b = _run([str(POOL_BIN)], tmp_path).stdout
    assert a == b, "INT:" + NL + a + NL + "VM:" + NL + b
    assert a.strip().splitlines() == ESPERADO
