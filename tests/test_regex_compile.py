"""`regex.compile()` (objeto `Pattern`) e `regex.fullmatch()` — DIFERENCIAL.

Faltavam os dois: o módulo tinha match/search/findall/sub/split/escape e parou
aí. `fullmatch` é o nome do Python pro que o `match` da PoolScript já faz
(casa a string INTEIRA — `_re.fullmatch` no interp, `ps_regex_casa_tudo` no
VM). `compile` compila UMA vez e devolve um objeto reusável — num laço, é a
diferença entre compilar N vezes e compilar uma.

Guarda também o bug de posse que isso destapou no VM: `rx_sub`/`rx_findall`
LIBERAVAM o PSRegex recebido, então a segunda chamada num Pattern compilado
usava ponteiro solto e dava segfault. Agora quem compila é quem libera.
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
    assert b.returncode == 0, b.stderr          # segfault cai aqui
    assert a.stdout == b.stdout, (
        f"DIVERGÊNCIA interp x VM\n--- interp\n{a.stdout}\n--- VM\n{b.stdout}")
    return a.stdout


def test_compile_tipo_e_pattern(tmp_path):
    prog = ('import regex\n'
            'p = regex.compile("\\\\d+")\n'
            'post(type(p))\n'
            'post(p.pattern)\n')
    assert _ambos(tmp_path, prog) == "Pattern\n\\d+\n"


def test_metodos_do_pattern(tmp_path):
    prog = ('import regex\n'
            'p = regex.compile("\\\\d+")\n'
            'post(p.match("123"), p.match("a123"), p.fullmatch("123"))\n'
            'post(p.search("abc123"), p.search("abc"))\n'
            'post(p.findall("a1b22c333"))\n'
            'post(p.sub("#", "a1b22"))\n'
            'post(p.split("a1b22c"))\n')
    assert _ambos(tmp_path, prog) == (
        "True False True\n"
        "True False\n"
        "['1', '22', '333']\n"
        "a#b#\n"
        "['a', 'b', 'c']\n"
    )


def test_reuso_repetido_nao_quebra(tmp_path):
    """O bug: `sub` e `findall` liberavam o regex do objeto — a 2ª chamada
    era ponteiro solto (segfault). Aqui cada método roda VÁRIAS vezes."""
    prog = ('import regex\n'
            'p = regex.compile("\\\\d+")\n'
            'saida = []\n'
            'for each i in range(5):\n'
            '    saida.append(p.sub("#", "a1b22"))\n'
            '    saida.append(str(p.findall("x9y8")))\n'
            '    saida.append(str(p.split("a1b2")))\n'
            '    saida.append(str(p.match("77")))\n'
            'post(len(saida))\n'
            'post(saida[0], saida[1], saida[2], saida[3])\n'
            'post(saida[16], saida[17], saida[18], saida[19])\n')
    saida = _ambos(tmp_path, prog)
    linhas = saida.splitlines()
    assert linhas[0] == "20"
    # a última volta tem que dar exatamente o mesmo da primeira
    assert linhas[1] == linhas[2], saida


def test_sub_e_split_no_mesmo_post(tmp_path):
    """Repro exata do segfault: dois métodos do MESMO Pattern na mesma linha."""
    prog = ('import regex\n'
            'p = regex.compile("\\\\d+")\n'
            'post(p.sub("#", "a1b22"), p.split("a1b22c"))\n')
    assert _ambos(tmp_path, prog) == "a#b# ['a', 'b', 'c']\n"


def test_modulo_continua_igual_depois_da_mudanca_de_posse(tmp_path):
    """As funções soltas (que compilam e jogam fora) não podem ter regredido."""
    prog = ('import regex\n'
            'post(regex.sub("\\\\d+", "#", "a1b22"))\n'
            'post(regex.findall("\\\\d+", "a1b22c333"))\n'
            'post(regex.split("\\\\d+", "a1b22c"))\n'
            'post(regex.match("\\\\d+", "123"), regex.search("\\\\d+", "abc1"))\n'
            'post(regex.escape("a.b*c"))\n'
            'post("a1b22".sub("\\\\d+", "#"), "a1b22".findall("\\\\d+"))\n')
    assert _ambos(tmp_path, prog) == (
        "a#b#\n"
        "['1', '22', '333']\n"
        "['a', 'b', 'c']\n"
        "True True\n"
        "a\\.b\\*c\n"
        "a#b# ['1', '22']\n"
    )


def test_fullmatch_do_modulo(tmp_path):
    prog = ('import regex\n'
            'post(regex.fullmatch("\\\\d+", "123"), regex.fullmatch("\\\\d+", "a123"))\n'
            'post(regex.fullmatch("\\\\d+", "123") == regex.match("\\\\d+", "123"))\n')
    assert _ambos(tmp_path, prog) == "True False\nTrue\n"


def test_padrao_invalido_erra_nos_dois(tmp_path):
    prog = 'import regex\nregex.compile("[a-")\n'
    a = _run(INTERP, tmp_path, prog)
    b = _run(VM, tmp_path, prog)
    assert a.returncode != 0 and b.returncode != 0, (a.stdout, b.stdout)


def test_muitos_patterns_sem_vazar(tmp_path):
    """Compila 2000 patterns e deixa o GC recolher — o finalizer tem que
    liberar o PSRegex e o texto do padrão (senão vaza a cada compile)."""
    prog = ('import regex\n'
            'n = 0\n'
            'for each i in range(2000):\n'
            '    p = regex.compile("a" + str(i) + "\\\\d+")\n'
            '    if p.search("a" + str(i) + "77"):\n'
            '        n = n + 1\n'
            'post(n)\n')
    assert _ambos(tmp_path, prog) == "2000\n"
