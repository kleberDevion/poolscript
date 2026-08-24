"""Três bugs achados usando a PoolScript em scripts de apoio (dogfood), todos
com teste DIFERENCIAL (interp = autoridade, VM tem que bater):

1. Fatiamento/indexação de string na VM era em BYTES, com `len`/`find` em
   CARACTERES — `"padrão"[0:5]` cortava o "ã" no meio e `s[find(...):len(s)]`
   perdia o fim da string quando havia acento antes. Agora é tudo em
   codepoints, igual ao interp (Python).
2. `find/rfind/index/rindex/count(sub, inicio, fim)`: a doc prometia `inicio`,
   o interp aceitava (Python) e a VM recusava ("espera 1 argumento").
3. Comentário `//` ou linha em branco como PRIMEIRA linha de um bloco `:`
   dava `faltou indentação após ':'` nos dois motores.
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
                          env=env, timeout=20)


def _ambos(tmp_path, prog):
    a = _run(INTERP, tmp_path, prog)
    b = _run(VM, tmp_path, prog)
    assert a.returncode == 0, a.stderr
    assert b.returncode == 0, b.stderr
    assert a.stdout == b.stdout, f"DIVERGÊNCIA interp x VM\n--- interp\n{a.stdout}\n--- VM\n{b.stdout}"
    return a.stdout


# ── 1. fatia/índice em caracteres ────────────────────────────────────────────

FATIA = '''
s = "padrão: str"
post(s[5], s[6], s[-1])
post(s[-3:len(s)])
post(s[2:-2])
post(s[0:6] == "padrão")
post(s[::-1])
post(s[1:10:2])
post(len(s[3:len(s)]))
t = "regex.findall(padrão: str, texto: str, flags=0) -> list"
post(t[t.find("texto"):len(t)])
post(t[0:5] + "|" + t[14:20])
post("ação"[1], "ação"[-1], "ação"[1:3])
'''

FATIA_ESPERADO = (
    "o : r\n"
    "str\n"
    "drão: s\n"
    "True\n"
    "rts :oãrdap\n"
    "aro t\n"
    "8\n"
    "texto: str, flags=0) -> list\n"
    "regex|padrão\n"
    "ç o çã\n"
)


def test_fatia_e_indice_em_caracteres(tmp_path):
    assert _ambos(tmp_path, FATIA) == FATIA_ESPERADO


# ── 2. inicio/fim em find/rfind/index/rindex/count ──────────────────────────

BUSCA = '''
s = "banana"
post(s.find("na"), s.find("na", 3), s.find("na", 0, 3), s.find("na", 99))
post(s.rfind("na"), s.rfind("na", 0, 4), s.rfind("na", 5))
post(s.index("na", 3), s.rindex("na", 0, 4))
post(s.count("na"), s.count("na", 3), s.count("a", 1, 4), s.count("", 1, 3))
post(s.find("a", -2), s.find("a", -100), s.count("a", -3))
p = "pão de mel"
post(p.find("o"), p.find("e", -3), p.rfind("e", 0, 6), p.count("e", 4))
'''

BUSCA_ESPERADO = (
    "2 4 -1 -1\n"
    "4 2 -1\n"
    "4 2\n"
    "2 1 2 3\n"
    "5 1 2\n"
    "2 8 5 2\n"
)


def test_busca_com_inicio_e_fim(tmp_path):
    assert _ambos(tmp_path, BUSCA) == BUSCA_ESPERADO


@pytest.mark.parametrize("metodo", ["find", "rfind", "index", "rindex", "count"])
def test_busca_argumentos_demais_erra_igual(tmp_path, metodo):
    prog = f'post("abc".{metodo}("a", 0, 3, 9))\n'
    a = _run(INTERP, tmp_path, prog)
    b = _run(VM, tmp_path, prog)
    assert a.returncode != 0 and b.returncode != 0


def test_index_fora_da_faixa_erra_nos_dois(tmp_path):
    prog = 'post("banana".index("na", 0, 3))\n'
    a = _run(INTERP, tmp_path, prog)
    b = _run(VM, tmp_path, prog)
    assert a.returncode != 0 and b.returncode != 0
    assert "SomeValueUnexpected" in a.stderr and "SomeValueUnexpected" in b.stderr


# ── 3. comentário / linha vazia como 1ª linha de bloco ':' ───────────────────

BLOCO = '''
action f(x):
    // comentário na primeira linha do bloco
    return x + 1

action g(x):

    return x * 2

Entity E():
    // comentário
    action __init__(self, v):
        // outro
        self.v = v
    action dobro(self):
        return self.v * 2

n = 3
if n > 2:
    // só comentário aqui
    post("maior")
for each i in range(2):
    # comentário com cerquilha
    post(i)
post(f(1), g(2), E(5).dobro())
'''

BLOCO_ESPERADO = "maior\n0\n1\n2 4 10\n"


def test_comentario_como_primeira_linha_do_bloco(tmp_path):
    assert _ambos(tmp_path, BLOCO) == BLOCO_ESPERADO
