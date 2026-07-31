"""Módulo `manpu`/`mp` em C — mesma resposta que o `manpu_lib.py`.

O xlsx é o `ps_xlsx.c` (ZIP+OOXML à mão, sem openpyxl/libzip). Os arquivos de
teste vêm do openpyxl (a autoridade) pra o C ler, e o C escreve pro openpyxl
reler — validação semântica (valores), já que byte a byte é impossível.
"""
import io
import os
import sys
from contextlib import redirect_stdout

import pytest

from poolscript.interpreter import Interpreter
from poolscript.parser import parse_source

vm = pytest.importorskip(
    "poolscript.vm.poolscript_vm",
    reason="extensão C não compilada — rode: ./rebuild_vm.sh",
)
openpyxl = pytest.importorskip("openpyxl", reason="openpyxl não instalado")

NL = chr(10)


@pytest.fixture
def caixa(tmp_path):
    def monta(base):
        d = base
        wb = openpyxl.Workbook(); ws = wb.active
        ws.append(["nome", "idade", "nota"])
        ws.append(["ana", 30, 9.5])
        ws.append(["bo", 7, None])
        wb.save(str(d / "d.xlsx"))
        (d / "c.csv").write_text("a,b" + NL + "1,2" + NL + "x,y" + NL, encoding="utf-8")
        (d / "t.txt").write_text("linha um" + NL + "linha dois", encoding="utf-8")
        (d / "cfg.json").write_text('{"k":[1,2],"n":3}', encoding="utf-8")
        (d / "p.xml").write_text('<raiz a="1"><item>oi</item><item>ok</item></raiz>', encoding="utf-8")
        (d / "pg.html").write_text("<html><body><h1>Ola</h1>" + NL * 3 + "<p>mundo</p></body></html>",
                                   encoding="utf-8")
    return monta


def _interp(src, cwd):
    velho = os.getcwd(); os.chdir(cwd)
    try:
        buf = io.StringIO()
        i = Interpreter(source=src, filename=str(cwd / "t.ps"))
        with redirect_stdout(buf):
            i.run(parse_source(src, "<t>"))
        return buf.getvalue().splitlines()
    finally:
        os.chdir(velho)


def _c(src, cwd):
    velho = os.getcwd(); os.chdir(cwd)
    sys.stdout.flush()
    r, w = os.pipe(); orig = os.dup(1)
    try:
        os.dup2(w, 1); os.close(w)
        try:
            vm.executa_fonte(src, str(cwd / "t.ps"))
        finally:
            sys.stdout.flush(); os.dup2(orig, 1)
    finally:
        os.close(orig); os.chdir(velho)
    with os.fdopen(r, "rb") as f:
        return f.read().decode("utf-8", "replace").splitlines()


@pytest.fixture
def mesmo(tmp_path, caixa):
    def _m(src):
        a = tmp_path / "ia"; b = tmp_path / "vb"
        a.mkdir(); b.mkdir()
        caixa(a); caixa(b)
        assert _c(src, b) == _interp(src, a)
    return _m


IMP = "import manpu" + NL


# ── read ─────────────────────────────────────────────────────────────────────

@pytest.mark.parametrize("src", [
    'post(manpu.read("d.xlsx"))',
    'd = manpu.read("d.xlsx")' + NL + 'post(d[0]["idade"], d[0]["nota"], d[1]["nota"])',
    'post(manpu.read("c.csv"))',
    'post(manpu.read("t.txt"))',
    'post(manpu.read("cfg.json"))',
    'post(manpu.read("p.xml"))',
    'd = manpu.read("p.xml")' + NL + 'post(d["tag"], d["attrs"]["a"], d["children"].len())',
    'post(manpu.read("pg.html"))',
    'post(manpu.read("naoexiste.txt"))',       # devolve string "Error: ...", não erro
])
def test_read(src, mesmo):
    """xlsx volta tipado (int/float/null), csv tudo string, xml aninhado,
    html limpo, json decodificado."""
    mesmo(IMP + src)


# ── write / ManpuResult ──────────────────────────────────────────────────────

@pytest.mark.parametrize("src", [
    'r = manpu.write(content="oi", target="z.txt")' + NL + 'post(r, r == true)',
    'post(type(manpu.write(content="x", target="z.txt")))',
    'manpu.write(content="linha", target="z.txt")' + NL + 'post(manpu.read("z.txt"))',
    'manpu.write(content="cab", column=0, celula=0, target="n.csv")' + NL
    + 'post(manpu.read("n.csv"))',
    'r = manpu.write(content="ana", column=0, celula=0, target="novo.xlsx")' + NL
    + 'post(r == true)',
])
def test_write(src, mesmo):
    mesmo(IMP + src)


def test_write_xlsx_roundtrip_openpyxl(tmp_path, caixa):
    """O xlsx que a VM escreve abre no openpyxl com os valores certos."""
    d = tmp_path / "w"; d.mkdir()
    src = (IMP + 'manpu.write(content="ana", column=0, celula=0, target="o.xlsx")' + NL
           + 'manpu.write(content=42, column=1, celula=0, target="o.xlsx")')
    _c(src, d)
    wb = openpyxl.load_workbook(str(d / "o.xlsx"))
    ws = wb.active
    assert ws.cell(row=1, column=1).value == "ana"
    assert ws.cell(row=1, column=2).value == 42        # número, não "42"


# ── remove ───────────────────────────────────────────────────────────────────

def test_remove(mesmo):
    mesmo(IMP + 'manpu.write(content="abcabc", target="rm.txt")' + NL
          + 'manpu.remove(value="a", amount="full", target="rm.txt")' + NL
          + 'post(manpu.read("rm.txt"))')


# ── src / load ───────────────────────────────────────────────────────────────

def test_src(mesmo):
    mesmo(IMP + 'post(manpu.src("d.xlsx").endswith("d.xlsx"))')


def test_load(mesmo):
    mesmo(IMP + 'post(manpu.load("t.txt"))')


def test_alias_mp(mesmo):
    mesmo('import mp' + NL + 'post(mp.read("c.csv"))')
