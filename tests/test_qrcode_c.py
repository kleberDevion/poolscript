"""Módulo `qrcode` em C — encoder próprio (ps_qr.c), sem libqrencode.

Dois níveis de teste, porque QR não dá pra comparar byte a byte com a lib
Python (encoders de PNG diferentes, e a lib otimiza o modo enquanto a VM usa
byte puro):

1. **Correção da matriz** — a matriz de módulos que a VM gera tem que ser
   IDÊNTICA à da lib `qrcode` quando as duas usam byte mode. Isso prova o
   Reed-Solomon, a seleção de máscara e o format/version info. É o teste que
   garante que o QR realmente escaneia (sem depender de um leitor instalado).

2. **Comportamento do módulo** — `gen`/`make`/constantes batendo com o
   `qrcode_lib.py` no que é observável sem os bytes do PNG (tipo, nome, ext,
   tamanho > 0, magic do PNG, `save=` devolvendo PoolFile).

O nível 1 usa o binário `pool` com um `-e` que não existe pra extrair a
matriz; em vez disso, compara via o próprio módulo, rodando o mesmo dado nos
dois e conferindo que o PNG da VM, relido, reproduz a matriz da lib.
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
qrlib = pytest.importorskip("qrcode", reason="lib qrcode não instalada")
import qrcode.util as _qu
from qrcode.constants import (
    ERROR_CORRECT_L, ERROR_CORRECT_M, ERROR_CORRECT_Q, ERROR_CORRECT_H,
)

NL = chr(10)
_EC = {"L": ERROR_CORRECT_L, "M": ERROR_CORRECT_M, "Q": ERROR_CORRECT_Q, "H": ERROR_CORRECT_H}


# ── helpers ──────────────────────────────────────────────────────────────────

def _matriz_da_lib(data, nivel):
    qr = qrlib.QRCode(error_correction=_EC[nivel], box_size=1, border=0)
    qr.add_data(_qu.QRData(data, mode=_qu.MODE_8BIT_BYTE))
    qr.make(fit=True)
    return [[1 if x else 0 for x in row] for row in qr.get_matrix()]


def _grade_do_png(raw, box=10, border=4):
    """Lê o PNG (via Pillow) e reconstrói a grade de módulos, amostrando o
    centro de cada célula."""
    from PIL import Image
    im = Image.open(io.BytesIO(raw)).convert("L")
    w, h = im.size
    px = im.load()
    dim = w // box - 2 * border
    grade = []
    for r in range(dim):
        row = []
        for c in range(dim):
            x = (border + c) * box + box // 2
            y = (border + r) * box + box // 2
            row.append(1 if px[x, y] < 128 else 0)
        grade.append(row)
    return grade


def _png_via_c(data, nivel, tmp_path):
    """Gera o PNG pela VM salvando em disco e lendo de volta."""
    alvo = tmp_path / "q.png"
    src = ('import qrcode' + NL
           + 'qrcode.gen(%r, save=%r, error_correction=%r)' % (data, str(alvo), nivel))
    velho = os.getcwd()
    os.chdir(tmp_path)
    try:
        with redirect_stdout(io.StringIO()):
            vm.executa_fonte(src, str(tmp_path / "t.ps"))
    finally:
        os.chdir(velho)
    return alvo.read_bytes()


# ── nível 1: a matriz escaneia (idêntica à lib em byte mode) ─────────────────

@pytest.mark.parametrize("data,nivel", [
    ("HELLO", "L"), ("HELLO", "M"), ("HELLO", "Q"), ("HELLO", "H"),
    ("https://poolscript.dev", "M"),
    ("x", "L"), ("teste 123 !@#", "Q"),
    ("A" * 50, "H"), ("z" * 200, "L"),
])
def test_matriz_identica_a_lib_byte_mode(data, nivel, tmp_path):
    """A grade que a VM desenha (relida do PNG) é a MESMA que a lib `qrcode`
    produz em byte mode. Prova RS + máscara + format/version — é o que garante
    que o código escaneia."""
    esperado = _matriz_da_lib(data, nivel)
    obtido = _grade_do_png(_png_via_c(data, nivel, tmp_path))
    assert obtido == esperado


# ── nível 2: comportamento do módulo vs interpretador ────────────────────────

def _via_interp(src, cwd):
    velho = os.getcwd()
    os.chdir(cwd)
    try:
        buf = io.StringIO()
        i = Interpreter(source=src, filename=str(cwd / "t.ps"))
        with redirect_stdout(buf):
            i.run(parse_source(src, "<t>"))
        return buf.getvalue().splitlines()
    finally:
        os.chdir(velho)


def _via_c(src, cwd):
    velho = os.getcwd()
    os.chdir(cwd)
    sys.stdout.flush()
    r, w = os.pipe()
    orig = os.dup(1)
    try:
        os.dup2(w, 1)
        os.close(w)
        try:
            vm.executa_fonte(src, str(cwd / "t.ps"))
        finally:
            sys.stdout.flush()
            os.dup2(orig, 1)
    finally:
        os.close(orig)
        os.chdir(velho)
    with os.fdopen(r, "rb") as f:
        return f.read().decode("utf-8", "replace").splitlines()


@pytest.fixture
def mesmo(tmp_path):
    def _m(src):
        a = tmp_path / "ia"; b = tmp_path / "vb"
        a.mkdir(); b.mkdir()
        assert _via_c(src, b) == _via_interp(src, a)
    return _m


IMP = "import qrcode" + NL


@pytest.mark.parametrize("src", [
    'f = qrcode.gen("HELLO")' + NL + 'post(type(f), f.name, f.ext, f.size > 0)',
    'f = qrcode.gen("X", name="meu.png")' + NL + 'post(f.name, f.ext)',
    'f = qrcode.gen("d", save="q.png")' + NL + 'post(type(f), f.name, f.size > 0)',
    'f = qrcode.gen("d", save="q.png")' + NL + 'post(f is PoolFile)',
    'f = qrcode.gen({"a": 1})' + NL + 'post(f.size > 0)',
    'f = qrcode.gen("d", error_correction="H")' + NL + 'post(f.size > 0)',
    'post(qrcode.ERROR_CORRECT_L, qrcode.ERROR_CORRECT_M, '
    'qrcode.ERROR_CORRECT_Q, qrcode.ERROR_CORRECT_H)',
])
def test_comportamento_do_gen(src, mesmo):
    mesmo(IMP + src)


def test_bytes_e_png(mesmo):
    """`bytes()` devolve o PNG cru — o magic (137 80 78 71) bate nos dois."""
    mesmo(IMP + 'f = qrcode.gen("HELLO")' + NL
          + 'b = f.bytes()' + NL + 'post(b[0], b[1], b[2], b[3])')


def test_save_sem_pasta_pai_erra_nos_dois(mesmo):
    """`.save()` não cria diretório — erra se o caminho não existe, como o
    QRPoolFile do interpretador."""
    mesmo(IMP + 'f = qrcode.gen("oi")' + NL
          + 'try { f.save("naoexiste/x.png") } catch (e) { post("erro") }')


def test_alias_qr(mesmo):
    """`import qr` é o mesmo módulo."""
    mesmo('import qr' + NL + 'f = qr.gen("HELLO")' + NL + 'post(f.size > 0)')
