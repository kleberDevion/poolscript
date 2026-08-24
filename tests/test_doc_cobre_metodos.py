"""Todo método de tipo do MOTOR tem página de doc — e vice-versa.

`str`, `list`, `dict` e `tup` expõem métodos direto no valor; as páginas saem
das specs em `scripts/doc_specs_*.py`. Sem este cruzamento, um método novo na
tabela do VM ficava sem doc em silêncio (foi o que aconteceu: `list` e `dict`
só tinham a tabela agrupada da seção 12, nenhuma página por método).

A fonte da verdade é a tabela `METODOS_*` do `vm/poolscript_vm.c` — a mesma
que o `test_paridade_membros.py` usa pra cruzar VM x interpretador.
"""
import re
from pathlib import Path

import pytest

RAIZ = Path(__file__).resolve().parent.parent
VM_C = RAIZ / "vm" / "poolscript_vm.c"

import sys
sys.path.insert(0, str(RAIZ))
from scripts.doc_specs_string import STRMET    # noqa: E402
from scripts.doc_specs_list import LISTMET     # noqa: E402
from scripts.doc_specs_dict import DICTMET     # noqa: E402


def _tabela_do_vm(nome: str) -> set:
    src = VM_C.read_text(encoding="utf-8")
    m = re.search(r"static const MetodoNat " + nome + r"\[\] = \{(.*?)\n\};", src, re.S)
    assert m, f"tabela {nome} não achada no vm/poolscript_vm.c"
    nomes = set(re.findall(r'\{\s*"([A-Za-z_]\w*)"\s*,', m.group(1)))
    assert nomes, f"tabela {nome} vazia?"
    return nomes


# (rótulo, tabela do VM, spec, pasta da doc)
GRUPOS = [
    ("str", "METODOS_STR", STRMET, "string"),
    ("list", "METODOS_LIST", LISTMET, "list"),
    ("dict", "METODOS_DICT", DICTMET, "dict"),
]


@pytest.mark.parametrize("rotulo,tabela,spec,pasta", GRUPOS)
def test_todo_metodo_do_motor_tem_spec(rotulo, tabela, spec, pasta):
    do_vm = _tabela_do_vm(tabela)
    faltando = sorted(do_vm - set(spec))
    assert not faltando, (
        f"{rotulo}: método no motor SEM doc: {faltando} — "
        f"adicione em scripts/doc_specs_{pasta}.py e rode scripts/gera_doc.py")


@pytest.mark.parametrize("rotulo,tabela,spec,pasta", GRUPOS)
def test_nenhuma_doc_de_metodo_inexistente(rotulo, tabela, spec, pasta):
    do_vm = _tabela_do_vm(tabela)
    sobrando = sorted(set(spec) - do_vm)
    assert not sobrando, (
        f"{rotulo}: doc de método que NÃO existe no motor: {sobrando}")


@pytest.mark.parametrize("rotulo,tabela,spec,pasta", GRUPOS)
def test_toda_spec_virou_pagina(rotulo, tabela, spec, pasta):
    """A doc materializada tem que estar em dia com a spec (rodou o gerador)."""
    base = RAIZ / "docs" / pasta
    faltando = [n for n in spec if not (base / n / f"{n}.md").is_file()]
    assert not faltando, (
        f"{rotulo}: spec sem página gerada: {faltando} — rode "
        f"PYTHONPATH=.:src python3 scripts/gera_doc.py")
    indice = base / f"{pasta}.md"
    assert indice.is_file(), f"índice {indice} não existe"
    texto = indice.read_text(encoding="utf-8")
    fora = [n for n in spec if f"]({n}/{n}.md)" not in texto]
    assert not fora, f"{rotulo}: fora do índice: {fora}"


def test_tupla_so_tem_metodos_de_leitura():
    """A tabela da tupla é um SUBCONJUNTO da lista, sem nada que mute —
    tupla é imutável (o VM já deixou mutar; ver notas/LIMITACOES.md)."""
    tup = _tabela_do_vm("METODOS_TUPLA")
    lista = _tabela_do_vm("METODOS_LIST")
    assert tup <= lista, f"tupla tem método que a lista não tem: {sorted(tup - lista)}"
    mutadores = {"append", "extend", "insert", "pop", "remove",
                 "reverse", "sort", "clear", "copy"}
    assert not (tup & mutadores), (
        f"tupla é imutável, mas a tabela expõe: {sorted(tup & mutadores)}")
    # os de leitura têm que estar TODOS lá
    assert tup == {"index", "count", "contains", "has", "len"}, sorted(tup)
