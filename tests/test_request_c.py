"""Módulo `request` em C — mesma resposta que o `request_lib.py`.

O HTTP é escrito à mão sobre socket + OpenSSL (`ps_http.c`), sem libcurl.
Para comparar contra o interpretador sem depender da internet, um servidor
HTTP sobe num **processo separado** e os dois motores batem nele.

O processo separado não é capricho: a extensão C faz I/O de rede segurando o
GIL, então um servidor rodando na mesma thread do teste travaria — o handler
Python nunca pegaria o GIL pra responder. O binário `pool` real não tem GIL e
não sofre disso; é artefato de rodar a VM *dentro* do CPython no teste.
"""
import io
import json
import os
import socket
import subprocess
import sys
import time
from contextlib import redirect_stdout

import pytest

from poolscript.interpreter import Interpreter
from poolscript.parser import parse_source

vm = pytest.importorskip(
    "poolscript.vm.poolscript_vm",
    reason="extensão C não compilada — rode: ./rebuild_vm.sh",
)

NL = chr(10)

_SERVIDOR = r'''
import json, sys
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
class H(BaseHTTPRequestHandler):
    protocol_version = "HTTP/1.1"
    def log_message(self, *a): pass
    def _send(self, status, body, ctype="text/plain", extra=None):
        if isinstance(body, str): body = body.encode("utf-8")
        self.send_response(status)
        self.send_header("Content-Type", ctype)
        self.send_header("Content-Length", str(len(body)))
        self.send_header("Connection", "close")
        for k, v in (extra or {}).items(): self.send_header(k, v)
        self.end_headers()
        try: self.wfile.write(body)
        except BrokenPipeError: pass
        self.close_connection = True
    def _resp(self):
        p = self.path
        n = int(self.headers.get("Content-Length", 0) or 0)
        body = self.rfile.read(n) if n else b""
        if p == "/json":
            self._send(200, json.dumps({"a": 1, "b": [2, 3], "nome": "ana"}), "application/json")
        elif p == "/echo":
            self._send(200, json.dumps({"method": self.command,
                "ua": self.headers.get("User-Agent", ""),
                "ctype": self.headers.get("Content-Type", ""),
                "body": body.decode("utf-8", "replace")}), "application/json")
        elif p == "/text":
            self._send(200, "olá ção", "text/plain; charset=utf-8")
        elif p == "/404":
            self._send(404, "nao achei", "text/plain")
        elif p == "/bin":
            self._send(200, bytes(range(64)), "application/octet-stream",
                       {"Content-Disposition": 'attachment; filename="arq.dat"'})
        elif p == "/big":
            self._send(200, b"x" * 5000, "application/octet-stream")
        elif p == "/redir":
            self.send_response(302); self.send_header("Location", "/json")
            self.send_header("Content-Length", "0"); self.send_header("Connection", "close")
            self.end_headers(); self.close_connection = True
        else:
            self._send(200, "ok")
    do_GET = do_POST = do_PUT = do_PATCH = do_DELETE = _resp
ThreadingHTTPServer(("127.0.0.1", int(sys.argv[1])), H).serve_forever()
'''


@pytest.fixture(scope="module")
def servidor(tmp_path_factory):
    d = tmp_path_factory.mktemp("srv")
    script = d / "srv.py"
    script.write_text(_SERVIDOR, encoding="utf-8")
    s = socket.socket()
    s.bind(("127.0.0.1", 0))
    porta = s.getsockname()[1]
    s.close()
    proc = subprocess.Popen([sys.executable, str(script), str(porta)])
    # espera a porta abrir
    for _ in range(50):
        try:
            with socket.create_connection(("127.0.0.1", porta), timeout=0.2):
                break
        except OSError:
            time.sleep(0.1)
    yield "http://127.0.0.1:%d" % porta
    proc.terminate()
    proc.wait(timeout=5)


def via_interpretador(src):
    buf = io.StringIO()
    i = Interpreter(source=src, filename="<t>")
    with redirect_stdout(buf):
        i.run(parse_source(src, "<t>"))
    return buf.getvalue().splitlines()


def via_c(src):
    sys.stdout.flush()
    r, w = os.pipe()
    original = os.dup(1)
    try:
        os.dup2(w, 1)
        os.close(w)
        try:
            vm.executa_fonte(src)
        finally:
            sys.stdout.flush()
            os.dup2(original, 1)
    finally:
        os.close(original)
    with os.fdopen(r, "rb") as f:
        return f.read().decode("utf-8", "replace").splitlines()


IMP = "import request" + NL


def mesmo(src):
    assert via_c(src) == via_interpretador(src)


# ── GET e o objeto Response ──────────────────────────────────────────────────

@pytest.mark.parametrize("frag", [
    'post(r.status, r.ok)',
    'post(r.json())',
    'post(r.get_json("a"))',
    'post(r.get_json("b"))',
    'post(r.get("a"))',                 # get() cai no JSON quando não é header
    'post(r.get("Content-Type"))',      # get() acha header, case-insensitive
    'post(type(r))',
])
def test_get_json(frag, servidor):
    mesmo(IMP + 'r = request.get("%s/json")' % servidor + NL + frag)


@pytest.mark.parametrize("frag", [
    'post(r.text)',
    'post(r.size)',
    'post(r.decode())',
])
def test_get_texto(frag, servidor):
    """Corpo UTF-8 com acento volta inteiro — a iteração e o tamanho são em
    bytes aqui (é `content`/`text`, não caractere)."""
    mesmo(IMP + 'r = request.get("%s/text")' % servidor + NL + frag)


def test_status_de_erro_vira_response(servidor):
    """404 não é exceção: volta como Response com status 404 e o corpo."""
    mesmo(IMP + 'r = request.get("%s/404")' % servidor + NL
          + 'post(r.status, r.ok, r.text)')


# ── verbos e corpo ───────────────────────────────────────────────────────────

@pytest.mark.parametrize("verbo", ["post", "put", "patch", "delete"])
def test_verbos(verbo, servidor):
    mesmo(IMP + 'r = request.%s("%s/echo")' % (verbo, servidor) + NL
          + 'post(r.get_json("method"))')


def test_body_dict_vira_json(servidor):
    """dict/list no body vira JSON e liga Content-Type application/json."""
    mesmo(IMP + 'r = request.post("%s/echo", body={"x": 1})' % servidor + NL
          + 'post(r.get_json("ctype"), r.get_json("body"))')


def test_body_str_cru(servidor):
    mesmo(IMP + 'r = request.post("%s/echo", body="cru")' % servidor + NL
          + 'post(r.get_json("body"))')


# ── headers ──────────────────────────────────────────────────────────────────

def test_user_agent_default(servidor):
    mesmo(IMP + 'r = request.get("%s/echo")' % servidor + NL
          + 'post(r.get_json("ua").contains("PoolScript"))')


def test_header_do_usuario_ganha(servidor):
    mesmo(IMP + 'r = request.get("%s/echo", headers={"User-Agent": "meu-agente"})' % servidor
          + NL + 'post(r.get_json("ua"))')


# ── content_type ─────────────────────────────────────────────────────────────

def test_content_type_confere_e_encadeia(servidor):
    mesmo(IMP + 'r = request.get("%s/json")' % servidor + NL
          + 'post(r.content_type("application/json").status)')


def test_content_type_errado_estoura(servidor):
    mesmo(IMP + 'r = request.get("%s/json")' % servidor + NL
          + 'try { r.content_type("text/html") } catch (e) { post("errado") }')


# ── binário, filename, save ──────────────────────────────────────────────────

def test_binario_e_filename(servidor):
    mesmo(IMP + 'r = request.get("%s/bin")' % servidor + NL
          + 'post(r.filename, r.size, type(r.content))')


def test_save_devolve_poolfile(servidor, tmp_path):
    """`.save()` grava o corpo e devolve um PoolFile — com name/ext/size."""
    src = (IMP + 'r = request.get("%s/bin")' % servidor + NL
           + 'f = r.save("%s/baixado.dat")' % tmp_path + NL
           + 'post(f.name, f.size, f is PoolFile)')
    # o interpretador e a VM gravam em caixas próprias; comparo a saída
    a = via_interpretador(src)
    b = via_c(src)
    assert a == b == ["baixado.dat 64 True"]


# ── redirecionamento ─────────────────────────────────────────────────────────

def test_segue_redirect_preservando_porta(servidor):
    """O bug clássico: Location relativo perdia a porta e caía na 80."""
    mesmo(IMP + 'r = request.get("%s/redir")' % servidor + NL
          + 'post(r.status, r.get_json("a"))')


# ── max_size só com stream ───────────────────────────────────────────────────

def test_max_size_sem_stream_e_ignorado(servidor):
    """Sem `stream=true`, `max_size` não limita — o interpretador lê tudo."""
    mesmo(IMP + 'r = request.get("%s/big", max_size=1000)' % servidor + NL
          + 'post(r.size)')


def test_max_size_com_stream_corta(servidor):
    mesmo(IMP + 'try {' + NL
          + ' request.get("%s/big", stream=true, max_size=1000)' % servidor + NL
          + '} catch (e) { post("estourou") }')


# ── erro de rede ─────────────────────────────────────────────────────────────

def test_host_invalido_e_networkerror():
    mesmo(IMP + 'try {' + NL
          + ' request.get("http://nao.existe.zzz.invalid")' + NL
          + '} catch (NetworkError e) { post("rede") }')
