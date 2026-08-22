"""Handler baseado em CLASSE num decorador — paridade INTERP/VM.

`@registrar.metodo(...)` decorando uma `class` faz o decorador ENXERGAR a action
DENTRO da classe (nome qualquer, fora de __init__), instanciar a classe e
registrar o método. O método precisa de `self` (mesma regra/erro nos dois
motores). `@dataentity` sobre uma Entity continua definindo a classe normalmente.
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


def _run(cmd, src, tmp_path):
    entry = tmp_path / "s.ps"
    entry.write_text(src + NL, encoding="utf-8")
    env = dict(os.environ)
    env["PYTHONPATH"] = str(RAIZ / "src")
    env["GUZER_HEADLESS"] = "1"
    return subprocess.run(cmd + [str(entry)], capture_output=True, text=True, env=env)


# registrar customizado (Entity) — observável sem subir servidor jinker
REG = NL.join([
    "Entity Reg():",
    "    action rota(self, path):",
    "        return self",
    "    action register(self, fn):",
    '        post("ok:", fn())',
    "        return fn",
    "app = Reg()",
])

# método com self, NOME QUALQUER (não precisa nome padrão)
COM_SELF = REG + NL + NL.join([
    '@app.rota("/x")',
    "class login():",
    "    action qualquerNome(self):",
    '        return "V"',
])

# @dataentity continua definindo a classe (regressão)
DATAENT = NL.join([
    "@dataentity",
    "Entity P():",
    "    x: int",
    "p = P(5)",
    "post(p.x)",
])


def test_interp_class_handler(tmp_path):
    r = _run([sys.executable, "-m", "poolscript"], COM_SELF, tmp_path)
    assert r.returncode == 0, r.stdout + r.stderr
    assert r.stdout.strip() == "ok: V", r.stdout


def test_paridade_class_handler(tmp_path):
    a = _run([sys.executable, "-m", "poolscript"], COM_SELF, tmp_path).stdout
    b = _run([str(POOL_BIN)], COM_SELF, tmp_path).stdout
    assert a == b == "ok: V" + NL, "INT:[" + a + "] VM:[" + b + "]"


def test_class_handler_sem_self_erra_nos_dois(tmp_path):
    src = REG + NL + '@app.rota("/x")' + NL + "class login():" + NL + \
        "    action semself():" + NL + '        return "V"'
    ri = _run([sys.executable, "-m", "poolscript"], src, tmp_path)
    rv = _run([str(POOL_BIN)], src, tmp_path)
    assert ri.returncode != 0 and rv.returncode != 0
    assert "self" in (ri.stdout + ri.stderr) and "self" in (rv.stdout + rv.stderr)


def test_interp_dataentity_regressao(tmp_path):
    r = _run([sys.executable, "-m", "poolscript"], DATAENT, tmp_path)
    assert r.returncode == 0, r.stdout + r.stderr
    assert r.stdout.strip() == "5"


def test_paridade_dataentity_regressao(tmp_path):
    a = _run([sys.executable, "-m", "poolscript"], DATAENT, tmp_path).stdout
    b = _run([str(POOL_BIN)], DATAENT, tmp_path).stdout
    assert a == b == "5" + NL
