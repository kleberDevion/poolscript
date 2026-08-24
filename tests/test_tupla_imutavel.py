"""Tupla é IMUTÁVEL — nos DOIS motores.

Achado auditando o completion de tipos: no VM a tupla caía na tabela de
métodos da LISTA inteira (`EH_SEQ`), então `(1,2,3).append(9)` **mutava a
tupla** e `.copy()` devolvia uma lista — enquanto o interpretador (autoridade)
recusava os dois com "membro inexistente". Nenhum teste exercitava método de
lista em tupla, então a divergência passou.

Agora o VM tem tabela própria de tupla: só os 5 métodos de LEITURA.
"""
import os
import subprocess
import sys
from pathlib import Path

import pytest

RAIZ = Path(__file__).resolve().parent.parent
POOL = RAIZ / "pool"
SRC = RAIZ / "src"

# VM em C NÃO se pula: sem o binário o teste FALHA (skip = falso verde).
assert POOL.exists(), "binário 'pool' não compilado — rode ./rebuild_vm.sh."

INTERP = [sys.executable, "-m", "poolscript"]
VM = [str(POOL)]

LEITURA = ["len()", "count(1)", "index(2)", "contains(2)", "has(2)"]
PROIBIDOS = ["append(9)", "extend([9])", "insert(0, 9)", "pop()", "remove(1)",
             "reverse()", "sort()", "clear()", "copy()"]


def _run(cmd, tmp_path, prog):
    ps = tmp_path / "t.ps"
    ps.write_text(prog, encoding="utf-8")
    env = dict(os.environ, PYTHONPATH=str(SRC))
    return subprocess.run(cmd + [str(ps)], capture_output=True, text=True,
                          env=env, timeout=30)


@pytest.mark.parametrize("met", LEITURA)
def test_leitura_funciona_igual(tmp_path, met):
    prog = f't = (1, 2, 3)\npost(t.{met})\n'
    a = _run(INTERP, tmp_path, prog)
    b = _run(VM, tmp_path, prog)
    assert a.returncode == 0, a.stderr
    assert b.returncode == 0, b.stderr
    assert a.stdout == b.stdout, (a.stdout, b.stdout)


@pytest.mark.parametrize("met", PROIBIDOS)
def test_metodo_de_lista_nao_existe_em_tupla(tmp_path, met):
    """Erra nos DOIS — e a tupla NÃO muda."""
    prog = f't = (1, 2, 3)\nt.{met}\n'
    a = _run(INTERP, tmp_path, prog)
    b = _run(VM, tmp_path, prog)
    assert a.returncode != 0, f"interp aceitou tupla.{met}: {a.stdout}"
    assert b.returncode != 0, f"VM aceitou tupla.{met}: {a.stdout}{b.stdout}"
    assert "membro inexistente" in a.stderr, a.stderr
    assert "membro inexistente" in b.stderr, b.stderr


def test_tupla_nao_muda_depois_da_tentativa(tmp_path):
    prog = ('t = (1, 2, 3)\n'
            'try { t.append(9) } catch (e) { post("recusou") }\n'
            'post(t, len(t))\n')
    a = _run(INTERP, tmp_path, prog)
    b = _run(VM, tmp_path, prog)
    assert a.returncode == 0 and b.returncode == 0, (a.stderr, b.stderr)
    assert a.stdout == b.stdout == "recusou\n(1, 2, 3) 3\n", (a.stdout, b.stdout)


def test_lista_continua_com_todos_os_metodos(tmp_path):
    """A tabela nova não pode ter tirado nada da LISTA."""
    prog = ('l = [3, 1, 2]\n'
            'l.append(4)\nl.sort()\n'
            'post(l, l.pop(), l.index(2), l.count(1), l.copy(), l.len())\n'
            'l.reverse()\npost(l)\nl.clear()\npost(l)\n')
    a = _run(INTERP, tmp_path, prog)
    b = _run(VM, tmp_path, prog)
    assert a.returncode == 0, a.stderr
    assert b.returncode == 0, b.stderr
    assert a.stdout == b.stdout, (a.stdout, b.stdout)
