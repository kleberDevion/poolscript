"""Módulo `mail` em C — mesma resposta que o `mail_lib.py`.

Não há servidor SMTP/IMAP no teste, então o que se compara é o que é
determinístico sem rede: a construção MIME (`MailMessage`), os tipos e reprs
dos objetos, e os erros de estado (chamar fora de ordem). Enviar e ler de
verdade fica pro dia do servidor de teste.

O boundary do MIME é aleatório no email lib do Python, então a comparação
normaliza `===============N==` dos dois lados — o que sobra é a estrutura, o
encoding do corpo (base64 quebrado em 76) e do assunto (RFC 2047 só quando há
acento), e a ordem dos headers.
"""
import io
import os
import re
import sys
from contextlib import redirect_stdout

import pytest

from poolscript.interpreter import Interpreter
from poolscript.parser import parse_source

vm = pytest.importorskip(
    "poolscript.vm.poolscript_vm",
    reason="extensão C não compilada — rode: ./rebuild_vm.sh",
)

NL = chr(10)
_BND = re.compile(r"===============\d+==")


def _norm(s):
    return _BND.sub("===B===", s)


def via_interpretador(src, dirbase):
    velho = os.getcwd()
    os.chdir(dirbase)
    try:
        buf = io.StringIO()
        i = Interpreter(source=src, filename=str(dirbase / "t.ps"))
        with redirect_stdout(buf):
            i.run(parse_source(src, "<t>"))
        return _norm(buf.getvalue())
    finally:
        os.chdir(velho)


def via_c(src, dirbase):
    velho = os.getcwd()
    os.chdir(dirbase)
    sys.stdout.flush()
    r, w = os.pipe()
    original = os.dup(1)
    try:
        os.dup2(w, 1)
        os.close(w)
        try:
            vm.executa_fonte(src, str(dirbase / "t.ps"))
        finally:
            sys.stdout.flush()
            os.dup2(original, 1)
    finally:
        os.close(original)
        os.chdir(velho)
    with os.fdopen(r, "rb") as f:
        return _norm(f.read().decode("utf-8", "replace"))


@pytest.fixture
def mesmo(tmp_path):
    def _mesmo(src):
        a = tmp_path / "interp"
        b = tmp_path / "vm"
        for d in (a, b):
            if not d.exists():
                d.mkdir()
                (d / "dado.bin").write_bytes(bytes(range(64)) * 3)
        assert via_c(src, b) == via_interpretador(src, a)
    return _mesmo


@pytest.fixture
def ambos_falham(tmp_path):
    def _ambos(src):
        a = tmp_path / "ia"
        b = tmp_path / "vb"
        a.mkdir()
        b.mkdir()
        with pytest.raises(Exception):
            via_interpretador(src, a)
        with pytest.raises(Exception):
            via_c(src, b)
    return _ambos


IMP = "import mail" + NL


# ── objetos ─────────────────────────────────────────────────────────────────

def test_tipos_e_repr(mesmo):
    mesmo(IMP + 'post(type(mail.MailServer()), type(mail.MailMessage()), '
              'type(mail.MailReader()))')
    mesmo(IMP + 'post(mail.MailServer())' + NL + 'post(mail.MailMessage())' + NL
              + 'post(mail.MailReader())')


def test_encadeamento_devolve_self(mesmo):
    """from_address/to/subject/body devolvem a própria mensagem — dá pra
    encadear, como no wrapper Python."""
    mesmo(IMP + 'm = mail.MailMessage()' + NL
              + 'post(m.from_address("a@b.c").to("d@e.f").subject("s") == m)')


# ── construção MIME ─────────────────────────────────────────────────────────

@pytest.mark.parametrize("src", [
    # corpo texto simples
    'm = mail.MailMessage()' + NL + 'm.body("oi mundo")' + NL + 'post(m.get_as_string())',
    # HTML + assunto com acento (vira =?utf-8?b?..?=)
    'm = mail.MailMessage()' + NL + 'm.from_address("eu@x.com")' + NL
    + 'm.to("voce@y.com")' + NL + 'm.subject("Olá çãé")' + NL
    + 'm.body("linha <b>dois</b>", true)' + NL + 'post(m.get_as_string())',
    # assunto ASCII não é codificado; corpo longo quebra em 76
    'm = mail.MailMessage()' + NL + 'm.subject("Puro ASCII")' + NL
    + 'm.body("' + "x" * 100 + '")' + NL + 'post(m.get_as_string())',
    # anexo de arquivo
    'm = mail.MailMessage()' + NL + 'm.body("corpo")' + NL
    + 'm.attach("dado.bin")' + NL + 'post(m.get_as_string())',
    # ordem dos headers preservada (subject antes de from)
    'm = mail.MailMessage()' + NL + 'm.subject("S")' + NL
    + 'm.from_address("a@b.c")' + NL + 'm.to("d@e.f")' + NL
    + 'm.body("z")' + NL + 'post(m.get_as_string())',
    # dois anexos + corpo
    'm = mail.MailMessage()' + NL + 'm.body("txt")' + NL
    + 'm.attach("dado.bin")' + NL + 'm.attach("dado.bin")' + NL
    + 'post(m.get_as_string())',
])
def test_mime(src, mesmo):
    mesmo(IMP + src)


# ── erros de estado ─────────────────────────────────────────────────────────

@pytest.mark.parametrize("src", [
    's = mail.MailServer()' + NL + 's.login("a", "b")',
    's = mail.MailServer()' + NL + 's.send("a@b.c", "x", "y")',
    'r = mail.MailReader()' + NL + 'r.login("a", "b")',
    'r = mail.MailReader()' + NL + 'r.select()',
    'r = mail.MailReader()' + NL + 'r.search()',
])
def test_ordem_errada_para_com_erro(src, ambos_falham):
    ambos_falham(IMP + src)


@pytest.mark.parametrize("src", [
    's = mail.MailServer()' + NL
    + 'try { s.login("a", "b") } catch (e) { post("pego: " + e) }',
    'r = mail.MailReader()' + NL
    + 'try { r.select() } catch (e) { post("pego: " + e) }',
])
def test_erro_de_estado_e_capturavel(src, mesmo):
    """A mensagem tem que bater byte a byte — é o que `catch (e)` expõe."""
    mesmo(IMP + src)


def test_quit_e_close_sem_conexao(mesmo):
    """Fechar o que nunca abriu é no-op que devolve True, não erro."""
    mesmo(IMP + 's = mail.MailServer()' + NL + 'post(s.quit())')
    mesmo(IMP + 'r = mail.MailReader()' + NL + 'post(r.close())')


# ── anexos ──────────────────────────────────────────────────────────────────

def test_anexo_inexistente_e_ioerror(ambos_falham):
    ambos_falham(IMP + 'm = mail.MailMessage()' + NL + 'm.attach("/nao/existe.zzz")')


def test_anexo_tipo_errado(ambos_falham):
    ambos_falham(IMP + 'm = mail.MailMessage()' + NL + 'm.attach(5)')
