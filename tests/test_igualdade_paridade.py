"""Igualdade (`==`/`!=`) e chaves de dict — paridade INTERP/VM.

Regressão de dois bugs de paridade que só a VM em C tinha:
  - `1 == true` dava False (bool não era tratado como subtipo de int);
  - `{"a":1} == {"a":1}` dava False (faltava igualdade estrutural de dict).
E o efeito nas chaves: `d[1]` e `d[true]` (e `d[0]`/`d[false]`) são a MESMA
chave, colapsando como no interpretador (mantém a 1ª chave, atualiza o valor).
"""
import os
import subprocess
import sys
from pathlib import Path

import pytest

RAIZ = Path(__file__).resolve().parent.parent
POOL_BIN = RAIZ / "pool"
NL = chr(10)

SCRIPT = NL.join([
    'post(1 == true)',                      # True  (bool é int)
    'post(0 == false)',                     # True
    'post(2 == true)',                      # False
    'post(true == 1.0)',                    # True
    'post({"a":1} == {"a":1})',             # True  (estrutural)
    'post({"a":1} == {"a":2})',             # False
    'post({"a":1,"b":2} == {"b":2,"a":1})', # True  (independe da ordem)
    'post({1:"a"} == {true:"a"})',          # True
    'd = {}',
    'd[1] = "a"',
    'd[true] = "b"',                        # colapsa em 1
    'post(len(d), d[1])',                   # 1 b
    'd2 = {}',
    'd2[0] = "x"',
    'd2[false] = "y"',                      # colapsa em 0
    'post(len(d2))',                        # 1
]) + NL

ESPERADO = ["True", "True", "False", "True",
            "True", "False", "True", "True",
            "1 b", "1"]


def _run(cmd, tmp_path):
    entry = tmp_path / "eq.ps"
    entry.write_text(SCRIPT, encoding="utf-8")
    env = dict(os.environ)
    env["PYTHONPATH"] = str(RAIZ / "src")
    env["GUZER_HEADLESS"] = "1"
    return subprocess.run(cmd + [str(entry)], capture_output=True, text=True, env=env)


def test_interp_igualdade(tmp_path):
    r = _run([sys.executable, "-m", "poolscript"], tmp_path)
    assert r.returncode == 0, r.stdout + r.stderr
    assert r.stdout.strip().splitlines() == ESPERADO, r.stdout


@pytest.mark.skipif(not POOL_BIN.exists(), reason="binário ./pool não compilado")
def test_paridade_igualdade(tmp_path):
    a = _run([sys.executable, "-m", "poolscript"], tmp_path).stdout
    b = _run([str(POOL_BIN)], tmp_path).stdout
    assert a == b, "divergência:" + NL + "INTERP:" + NL + a + NL + "VM C:" + NL + b
    assert a.strip().splitlines() == ESPERADO
