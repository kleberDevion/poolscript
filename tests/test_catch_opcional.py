"""A variável do erro no `catch` é OPCIONAL. Quatro formas válidas, com saída
IDÊNTICA nos dois motores (paridade):

    catch (Tipo e)   -> casa o tipo e liga o erro em `e`
    catch (Tipo)     -> casa o tipo, SEM ligar variável
    catch (e)        -> captura tudo, ligando em `e`
    catch ()         -> captura tudo, sem variável
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

SCRIPT = (
    'try { raise "b1" } catch (RuntimeError) { post("A") }' + NL +
    'try { raise "b2" } catch (RuntimeError e) { post("B", e) }' + NL +
    'try { raise "b3" } catch (e) { post("C", e) }' + NL +
    'try { raise "b4" } catch () { post("D") }' + NL +
    'post("fim")' + NL
)


def _run(cmd, tmp_path):
    entry = tmp_path / "c.ps"
    entry.write_text(SCRIPT, encoding="utf-8")
    env = dict(os.environ)
    env["PYTHONPATH"] = str(RAIZ / "src")
    return subprocess.run(cmd + [str(entry)], capture_output=True, text=True, env=env)


def test_interp_catch_var_opcional(tmp_path):
    r = _run([sys.executable, "-m", "poolscript"], tmp_path)
    assert r.returncode == 0, r.stdout + r.stderr
    linhas = r.stdout.strip().splitlines()
    assert linhas[0] == "A"                       # catch (Tipo) sem var
    assert any(l.startswith("B") for l in linhas)  # catch (Tipo e)
    assert any(l.startswith("C") for l in linhas)  # catch (e)
    assert "D" in linhas                           # catch ()
    assert linhas[-1] == "fim"


def test_paridade_catch_var_opcional(tmp_path):
    a = _run([sys.executable, "-m", "poolscript"], tmp_path).stdout
    b = _run([str(POOL_BIN)], tmp_path).stdout
    assert a == b, "divergência:" + NL + "INTERP:" + NL + a + NL + "VM C:" + NL + b
    assert a.strip().splitlines()[0] == "A"
