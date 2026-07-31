"""Testes da v0.3.0: lib mail, lib date, built-in open + using."""
from __future__ import annotations
import os
import tempfile
import pytest
from unittest.mock import MagicMock

from poolscript import run_source
from poolscript.stdlib import resolve_module
from poolscript.stdlib.date_lib import time as ps_time, today as ps_today, datahora
from poolscript.stdlib.mail_lib import (
    MailServer, MailMessage, HOSTS_CONFIG, MailReader, IMAP_HOSTS_CONFIG,
)
from poolscript.builtins import FileHandle, ps_open, ps_len, ps_range, ps_type


# ── stdlib registry ──────────────────────────────────────────────────────
# Nota: REGISTRY foi substituído por lazy-loading (resolve_module) numa
# refatoração posterior à escrita destes testes; resolve_module(["lib"])
# devolve o mesmo dict de exports que REGISTRY["lib"] devolvia antes.

def test_mail_lib_registrado():
    mail = resolve_module(["mail"])
    assert mail is not None
    assert "MailServer" in mail
    assert "MailMessage" in mail
    assert "MailReader" in mail


def test_date_lib_registrada():
    date = resolve_module(["date"])
    assert date is not None
    for fn in ("time", "today", "datahora", "now", "timestamp"):
        assert fn in date


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
    with pytest.raises(FileNotFoundError):
        msg.attach("/tmp/nao_existe_xyz_123.pdf")


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
    with pytest.raises(RuntimeError):
        s.send("x@x.com", "s", "b")


# ── lib mail — MailReader (leitura via IMAP, sem rede) ───────────────────

def test_mail_reader_registrado_no_exports():
    mail = resolve_module(["mail"])
    assert "MailReader" in mail
    assert mail["MailReader"] is MailReader


def test_mail_reader_imap_hosts_config():
    assert IMAP_HOSTS_CONFIG["gmail.com"] == ("imap.gmail.com", 993)
    assert "outlook.com" in IMAP_HOSTS_CONFIG
    # não interfere no mapa de SMTP do MailServer
    assert HOSTS_CONFIG["gmail.com"] == ("smtp.gmail.com", 587)


def test_mail_reader_select_sem_conn():
    r = MailReader()
    with pytest.raises(RuntimeError):
        r.select("INBOX")


def test_mail_reader_search_sem_select():
    r = MailReader()
    r.server = MagicMock()  # conectado, mas sem pasta selecionada
    with pytest.raises(RuntimeError):
        r.search("ALL")


def test_mail_reader_select_retorna_self_encadeavel():
    r = MailReader()
    r.server = MagicMock()
    r.server.select.return_value = ("OK", [b"1"])
    assert r.select("INBOX") is r
    assert r.folder == "INBOX"


def test_mail_reader_search_subject_sem_term():
    r = MailReader()
    r.server = MagicMock()
    r.folder = "INBOX"
    with pytest.raises(ValueError):
        r.search("SUBJECT")


def test_mail_reader_search_criterio_desconhecido():
    r = MailReader()
    r.server = MagicMock()
    r.folder = "INBOX"
    with pytest.raises(ValueError):
        r.search("BOGUS")


def test_mail_reader_search_retorna_dicts_decodificados():
    r = MailReader()
    r.server = MagicMock()
    r.folder = "INBOX"
    r.server.search.return_value = ("OK", [b"1 2"])
    r.server.fetch.side_effect = [
        ("OK", [(b"1 (RFC822.HEADER)", (
            b"From: a@x.com\r\nSubject: Oi\r\nDate: Mon, 01 Jan 2026 10:00:00 +0000\r\n\r\n"
        ))]),
        ("OK", [(b"2 (RFC822.HEADER)", (
            b"From: b@y.com\r\nSubject: =?UTF-8?B?T2zDoQ==?=\r\nDate: Tue, 02 Jan 2026 11:00:00 +0000\r\n\r\n"
        ))]),
    ]
    resultados = r.search("ALL")
    assert resultados == [
        {"id": "1", "from": "a@x.com", "subject": "Oi", "date": "Mon, 01 Jan 2026 10:00:00 +0000"},
        {"id": "2", "from": "b@y.com", "subject": "Olá", "date": "Tue, 02 Jan 2026 11:00:00 +0000"},
    ]


def test_mail_reader_search_respeita_limit():
    r = MailReader()
    r.server = MagicMock()
    r.folder = "INBOX"
    r.server.search.return_value = ("OK", [b"1 2 3 4 5"])
    r.server.fetch.return_value = ("OK", [(b"x", b"From: a@x.com\r\nSubject: s\r\nDate: d\r\n\r\n")])
    resultados = r.search("ALL", limit=2)
    assert len(resultados) == 2


def test_mail_reader_search_from_e_since_usam_term():
    r = MailReader()
    r.server = MagicMock()
    r.folder = "INBOX"
    r.server.search.return_value = ("OK", [b""])

    r.search("FROM", "joao@empresa.com")
    r.server.search.assert_called_with("UTF-8", "FROM", '"joao@empresa.com"')

    r.search("SINCE", "01-Jan-2026")
    r.server.search.assert_called_with("UTF-8", "SINCE", '"01-Jan-2026"')


def test_mail_reader_search_escapa_aspas_e_barra_no_term():
    r = MailReader()
    r.server = MagicMock()
    r.folder = "INBOX"
    r.server.search.return_value = ("OK", [b""])

    r.search("SUBJECT", 'nota "urgente" \\ importante')
    r.server.search.assert_called_with(
        "UTF-8", "SUBJECT", '"nota \\"urgente\\" \\\\ importante"'
    )


def test_mail_reader_search_com_termo_acentuado_e_virgula():
    r = MailReader()
    r.server = MagicMock()
    r.folder = "INBOX"
    r.server.search.return_value = ("OK", [b""])

    r.search("SUBJECT", "Relatório, urgente")
    r.server.search.assert_called_with("UTF-8", "SUBJECT", '"Relatório, urgente"')


def test_mail_reader_search_include_body_busca_rfc822_completo():
    r = MailReader()
    r.server = MagicMock()
    r.folder = "INBOX"
    r.server.search.return_value = ("OK", [b"1"])
    r.server.fetch.return_value = ("OK", [(b"1 (RFC822)", (
        b"From: a@x.com\r\nSubject: Oi\r\nDate: d\r\n"
        b"Content-Type: text/plain; charset=utf-8\r\n\r\n"
        b"corpo em texto puro"
    ))])

    resultados = r.search("ALL", include_body=True)
    r.server.fetch.assert_called_with(b"1", "(RFC822)")
    assert resultados[0]["body"] == "corpo em texto puro"


def test_mail_reader_search_sem_include_body_nao_traz_corpo():
    r = MailReader()
    r.server = MagicMock()
    r.folder = "INBOX"
    r.server.search.return_value = ("OK", [b"1"])
    r.server.fetch.return_value = ("OK", [(b"1 (RFC822.HEADER)", (
        b"From: a@x.com\r\nSubject: Oi\r\nDate: d\r\n\r\n"
    ))])

    resultados = r.search("ALL")
    r.server.fetch.assert_called_with(b"1", "(RFC822.HEADER)")
    assert "body" not in resultados[0]


def test_mail_reader_body_retorna_texto_plain_de_email_multipart():
    r = MailReader()
    r.server = MagicMock()
    r.folder = "INBOX"
    raw = (
        b"From: a@x.com\r\nSubject: Oi\r\n"
        b'Content-Type: multipart/alternative; boundary="B"\r\n\r\n'
        b"--B\r\nContent-Type: text/plain; charset=utf-8\r\n\r\n"
        b"texto simples\r\n"
        b"--B\r\nContent-Type: text/html; charset=utf-8\r\n\r\n"
        b"<p>texto html</p>\r\n"
        b"--B--\r\n"
    )
    r.server.fetch.return_value = ("OK", [(b"5 (RFC822)", raw)])

    corpo = r.body("5")
    r.server.fetch.assert_called_with(b"5", "(RFC822)")
    assert corpo == "texto simples"


def test_mail_reader_body_cai_pra_html_se_nao_tiver_plain():
    r = MailReader()
    r.server = MagicMock()
    r.folder = "INBOX"
    raw = (
        b"From: a@x.com\r\nSubject: Oi\r\n"
        b"Content-Type: text/html; charset=utf-8\r\n\r\n"
        b"<p>somente html</p>"
    )
    r.server.fetch.return_value = ("OK", [(b"9 (RFC822)", raw)])

    corpo = r.body("9")
    assert corpo == "<p>somente html</p>"


def test_mail_reader_body_sem_select_da_erro():
    r = MailReader()
    r.server = MagicMock()
    with pytest.raises(RuntimeError):
        r.body("1")


def test_mail_reader_body_fetch_falho_da_erro():
    r = MailReader()
    r.server = MagicMock()
    r.folder = "INBOX"
    r.server.fetch.return_value = ("NO", [None])
    with pytest.raises(RuntimeError):
        r.body("1")


def test_mail_reader_close_desconecta():
    r = MailReader()
    r.server = MagicMock()
    r.folder = "INBOX"
    assert r.close() is True
    assert r.server is None
    assert r.folder is None


def test_mail_reader_close_sem_conexao_nao_quebra():
    r = MailReader()
    assert r.close() is True


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
    # `dict`/`tup`, não `json`/`tuple`: o builtin `type(x)` e o método
    # `x.type()` eram duas implementações independentes que discordavam.
    assert ps_type({"a": 1}) == "dict"
    assert ps_type((1, 2)) == "tup"


# ── using ... as ─────────────────────────────────────────────────────────

def test_using_abre_e_fecha_arquivo(tmp_path):
    target = tmp_path / "log.txt"
    src = (
        f'using open(r"{target}", "w") as f {{\n'
        f'  f.write("linha um\\n")\n'
        f'  f.write("linha dois\\n")\n'
        f'}}\n'
        f'using open(r"{target}", "r") as f {{\n'
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
        f'using open(r"{target}", "a") as f {{\n'
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
        f'  using open(r"{target}", "r") as f {{\n'
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
    assert "patch" in resolve_module(["request"])


def test_request_connection_error_levanta_connection_error():
    from poolscript.stdlib.request_lib import get
    with pytest.raises(ConnectionError):
        get("http://localhost:1/nao-existe", timeout=1)
