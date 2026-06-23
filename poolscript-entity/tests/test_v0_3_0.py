"""Testes da v0.3.0: lib mail, lib date, built-in open + using."""
from __future__ import annotations
import os
import tempfile
import pytest

from poolscript import run_source
from poolscript.stdlib import REGISTRY, resolve_module
from poolscript.stdlib.date_lib import time as ps_time, today as ps_today, datahora
from poolscript.stdlib.mail_lib import MailServer, MailMessage, HOSTS_CONFIG
from poolscript.builtins import FileHandle, ps_open, ps_len, ps_range, ps_type


# ── stdlib registry ──────────────────────────────────────────────────────

def test_mail_lib_registrado():
    assert "mail" in REGISTRY
    assert "MailServer" in REGISTRY["mail"]
    assert "MailMessage" in REGISTRY["mail"]


def test_date_lib_registrada():
    assert "date" in REGISTRY
    for fn in ("time", "today", "datahora", "now", "timestamp"):
        assert fn in REGISTRY["date"]


# ── lib date ─────────────────────────────────────────────────────────────

def test_date_funcs_retornam_string():
    assert isinstance(ps_time(), str)
    assert isinstance(ps_today(), str)
    assert isinstance(datahora(), str)
    assert "/" in ps_today()  # formato dd/mm/yyyy
    assert ":" in ps_time()


def test_import_date_no_ps():
    out = run_source(
        'import date\n'
        'post("hoje:" date.today())\n'
    )
    assert out and out[0].startswith("hoje:")


def test_from_date_import():
    out = run_source(
        'from date import today\n'
        'str d = today()\n'
        'post(d)\n'
    )
    assert "/" in out[0]


# ── lib mail (sem rede) ──────────────────────────────────────────────────

def test_mail_message_construcao():
    msg = MailMessage()
    msg.from_address("a@x.com")
    msg.to("b@y.com")
    msg.subject("oi")
    msg.body("texto puro")
    raw = msg.get_as_string()
    assert "From: a@x.com" in raw
    assert "To: b@y.com" in raw
    assert "Subject: oi" in raw


def test_mail_message_html():
    msg = MailMessage()
    msg.body("<b>oi</b>", True)
    assert "text/html" in msg.get_as_string()


def test_mail_message_attach_file_not_found():
    msg = MailMessage()
    r = msg.attach("/tmp/nao_existe_xyz_123.pdf")
    assert isinstance(r, dict) and r.get("error") == "file_not_found"


def test_mail_message_attach_ok():
    with tempfile.NamedTemporaryFile(delete=False, suffix=".txt") as f:
        f.write(b"conteudo de teste")
        path = f.name
    try:
        msg = MailMessage()
        assert msg.attach(path) is True
        raw = msg.get_as_string()
        assert os.path.basename(path) in raw
    finally:
        os.unlink(path)


def test_mail_server_hosts_config():
    assert HOSTS_CONFIG["gmail.com"] == ("smtp.gmail.com", 587)
    assert "outlook.com" in HOSTS_CONFIG


def test_mail_server_send_sem_conn():
    s = MailServer()
    r = s.send("x@x.com", "s", "b")
    assert isinstance(r, dict) and r.get("error") == "not_connected"


# ── built-in open + FileHandle ───────────────────────────────────────────

def test_ps_open_retorna_filehandle():
    with tempfile.NamedTemporaryFile(delete=False) as f:
        f.write(b"abc")
        path = f.name
    try:
        fh = ps_open(path, "r")
        assert isinstance(fh, FileHandle)
        assert fh.read() == "abc"
        fh.close()
    finally:
        os.unlink(path)


def test_filehandle_context_manager():
    with tempfile.NamedTemporaryFile(delete=False) as f:
        path = f.name
    try:
        fh = ps_open(path, "w")
        with fh as bound:
            bound.write("hello")
        assert fh._closed
        assert open(path).read() == "hello"
    finally:
        os.unlink(path)


# ── outros built-ins globais ─────────────────────────────────────────────

def test_ps_len():
    assert ps_len("abc") == 3
    assert ps_len([1, 2, 3]) == 3
    assert ps_len(None) == 0


def test_ps_range():
    assert ps_range(3) == [0, 1, 2]
    assert ps_range(1, 4) == [1, 2, 3]


def test_ps_type():
    assert ps_type(None) == "Null"
    assert ps_type(True) == "bool"
    assert ps_type(1) == "int"
    assert ps_type(1.5) == "flo"
    assert ps_type("a") == "str"
    assert ps_type([1]) == "list"
    assert ps_type({"a": 1}) == "json"


# ── using ... as ─────────────────────────────────────────────────────────

def test_using_abre_e_fecha_arquivo(tmp_path):
    target = tmp_path / "log.txt"
    src = (
        f'using open("{target}", "w") as f {{\n'
        f'  f.write("linha um\\n")\n'
        f'  f.write("linha dois\\n")\n'
        f'}}\n'
        f'using open("{target}", "r") as f {{\n'
        f'  str c = f.read()\n'
        f'  post(c)\n'
        f'}}\n'
    )
    out = run_source(src)
    assert "linha um" in out[0] and "linha dois" in out[0]


def test_using_modo_append(tmp_path):
    target = tmp_path / "ap.txt"
    target.write_text("antes\n")
    run_source(
        f'using open("{target}", "a") as f {{\n'
        f'  f.write("novo\\n")\n'
        f'}}\n'
    )
    assert target.read_text() == "antes\nnovo\n"


def test_using_fecha_mesmo_com_excecao(tmp_path):
    """Garante que o handler chama close mesmo quando o bloco lança erro."""
    target = tmp_path / "x.txt"
    target.write_text("ok")
    src = (
        f'try {{\n'
        f'  using open("{target}", "r") as f {{\n'
        f'    str x = f.read()\n'
        f'    post(x)\n'
        f'    int y = 1 + "boom"\n'   # AtributtedValueError
        f'  }}\n'
        f'}} catch (e) {{\n'
        f'  post("pegou:" e)\n'
        f'}}\n'
    )
    out = run_source(src)
    assert any("pegou:" in line for line in out)


# ── request: novos métodos e error objects ───────────────────────────────

def test_request_lib_tem_patch():
    assert "patch" in REGISTRY["request"]


def test_request_connection_error_retorna_dict():
    from poolscript.stdlib.request_lib import get
    r = get("http://localhost:1/nao-existe", timeout=1)
    # connection error retorna dict, não Response
    assert isinstance(r, dict)
    assert r.get("error") == "connection_error"
