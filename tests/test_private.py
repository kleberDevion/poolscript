"""Encapsulamento — `private`/`public`.

Membro `private` só é acessível de DENTRO da classe (via `self`); de fora
(`obj.priv`) é erro. Default é público. O interpretador é a autoridade; os
testes de VM/paridade entram quando a VM também enforça.
"""
import io
from contextlib import redirect_stdout

import pytest

from poolscript.interpreter import Interpreter
from poolscript.parser import parse_source


def roda(src):
    buf = io.StringIO()
    interp = Interpreter(source=src, filename="<t>")
    with redirect_stdout(buf):
        interp.run(parse_source(src, "<t>"))
    return buf.getvalue()


CLASSE = (
    "Entity Conta() {\n"
    "    private saldo: int = 0\n"
    "    public dono: str = \"kleber\"\n"
    "    public reaction deposita(self, v) { self.saldo = self.saldo + v  return self.saldo }\n"
    "    private reaction _log(self) { return \"secreto\" }\n"
    "}\n"
)


def test_uso_interno_via_self_funciona():
    assert roda(CLASSE + "c = Conta()\npost(c.deposita(100))\n") == "100\n"


def test_publico_acessivel_de_fora():
    assert roda(CLASSE + "c = Conta()\npost(c.dono)\n") == "kleber\n"


def test_campo_private_de_fora_erra():
    with pytest.raises(Exception) as e:
        roda(CLASSE + "c = Conta()\npost(c.saldo)\n")
    assert "private" in str(e.value)


def test_metodo_private_de_fora_erra():
    with pytest.raises(Exception) as e:
        roda(CLASSE + "c = Conta()\npost(c._log())\n")
    assert "private" in str(e.value)


def test_escrita_em_private_de_fora_erra():
    with pytest.raises(Exception) as e:
        roda(CLASSE + "c = Conta()\nc.saldo = 999\n")
    assert "private" in str(e.value)


def test_sem_modificador_e_publico():
    # default é público — acesso de fora funciona (retrocompatível)
    src = ("Entity P() { valor: int = 7 }\n"
           "p = P()\npost(p.valor)\n")
    assert roda(src) == "7\n"


# ── paridade com a VM (o binário/extensão enforça igual ao interpretador) ────

import os as _os

vm = pytest.importorskip(
    "poolscript.vm.poolscript_vm",
    reason="extensão C não compilada — rode: ./rebuild_vm.sh",
)


def via_c(src):
    """Roda na VM (extensão C, mesmo pipeline do binário `pool`)."""
    import sys as _sys
    _sys.stdout.flush()
    r, w = _os.pipe()
    salvo = _os.dup(1)
    try:
        _os.dup2(w, 1); _os.close(w)
        try:
            vm.executa_fonte(src)
        finally:
            _sys.stdout.flush(); _os.dup2(salvo, 1)
    finally:
        _os.close(salvo)
    with _os.fdopen(r, "rb") as f:
        return f.read().decode("utf-8", "replace")


def test_vm_uso_interno_bate_com_interp():
    src = CLASSE + "c = Conta()\npost(c.deposita(100))\npost(c.dono)\n"
    assert via_c(src) == roda(src)   # paridade exata


def test_vm_campo_private_de_fora_erra():
    with pytest.raises(Exception):
        via_c(CLASSE + "c = Conta()\npost(c.saldo)\n")


def test_vm_metodo_private_de_fora_erra():
    with pytest.raises(Exception):
        via_c(CLASSE + "c = Conta()\npost(c._log())\n")


def test_vm_escrita_private_de_fora_erra():
    with pytest.raises(Exception):
        via_c(CLASSE + "c = Conta()\nc.saldo = 9\n")


def test_vm_publico_ok():
    assert via_c("Entity P() { v: int = 7 }\np = P()\npost(p.v)\n") == "7\n"
