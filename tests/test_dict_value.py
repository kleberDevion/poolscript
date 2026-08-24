"""`dict.value()` — DIFERENCIAL nos 2 motores.

`x in d` testa a CHAVE (como no Python). Pra testar o VALOR faltava um método
curto: `x in d.value()`. É o mesmo conjunto de `values()`, com o nome que o
usuário pediu.

Cobre também a regra de resolução: método vem ANTES de chave, então num dict
com a chave "value" o `d.value` devolve o método e `d["value"]` devolve o
valor — igual já acontecia com `keys`/`items`/`get`/...
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


def _run(cmd, tmp_path, prog):
    ps = tmp_path / "t.ps"
    ps.write_text(prog, encoding="utf-8")
    env = dict(os.environ, PYTHONPATH=str(SRC))
    return subprocess.run(cmd + [str(ps)], capture_output=True, text=True,
                          env=env, timeout=30)


def _ambos(tmp_path, prog):
    a = _run(INTERP, tmp_path, prog)
    b = _run(VM, tmp_path, prog)
    assert a.returncode == 0, a.stderr
    assert b.returncode == 0, b.stderr
    assert a.stdout == b.stdout, (
        f"DIVERGÊNCIA interp x VM\n--- interp\n{a.stdout}\n--- VM\n{b.stdout}")
    return a.stdout


def test_in_no_dict_testa_chave_in_no_value_testa_valor(tmp_path):
    prog = ('d = { "nome": "ana", "idade": 30, "peso": 1.5 }\n'
            'post("nome" in d, "ana" in d, 30 in d)\n'
            'post("ana" in d.value(), 30 in d.value(), 1.5 in d.value())\n')
    assert _ambos(tmp_path, prog) == "True False False\nTrue True True\n"


def test_value_com_lista_tupla_e_aninhado(tmp_path):
    """O caso que motivou: valor que é list/tup/str/int/flo."""
    prog = ('d = { "a": [1, 2], "b": (3, 4), "c": "txt", "d": 9, "e": 2.5 }\n'
            'post([1, 2] in d.value(), (3, 4) in d.value())\n'
            'post("txt" in d.value(), 9 in d.value(), 2.5 in d.value())\n'
            'post([9] in d.value(), "naotem" in d.value())\n')
    assert _ambos(tmp_path, prog) == "True True\nTrue True True\nFalse False\n"


def test_value_e_values_dao_o_mesmo(tmp_path):
    prog = ('d = { "a": 1, "b": [2], "c": "x" }\n'
            'post(d.value() == d.values())\n'
            'post(d.value())\n'
            'post(len(d.value()), len(d))\n')
    assert _ambos(tmp_path, prog) == "True\n[1, [2], 'x']\n3 3\n"


def test_dict_vazio(tmp_path):
    prog = ('d = {}\n'
            'post(d.value(), len(d.value()))\n'
            'post(1 in d.value())\n')
    assert _ambos(tmp_path, prog) == "[] 0\nFalse\n"


def test_value_nao_aceita_argumento(tmp_path):
    prog = 'd = { "a": 1 }\npost(d.value("a"))\n'
    a = _run(INTERP, tmp_path, prog)
    b = _run(VM, tmp_path, prog)
    assert a.returncode != 0 and b.returncode != 0, (a.stdout, b.stdout)


def test_metodo_vem_antes_da_chave(tmp_path):
    """Dict com a chave "value": o MÉTODO ganha em `d.value` (igual a `d.keys`);
    o valor continua acessível por `d["value"]`."""
    prog = ('d = { "value": 42, "outro": 1 }\n'
            'post(d["value"])\n'
            'post(d.value())\n'
            'post(42 in d.value())\n')
    assert _ambos(tmp_path, prog) == "42\n[42, 1]\nTrue\n"


def test_iterar_o_resultado(tmp_path):
    prog = ('d = { "a": 1, "b": 2, "c": 3 }\n'
            'soma = 0\n'
            'for each v in d.value():\n'
            '    soma = soma + v\n'
            'post(soma)\n')
    assert _ambos(tmp_path, prog) == "6\n"
