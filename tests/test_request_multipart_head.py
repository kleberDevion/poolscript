"""request: multipart (`fields=`/`file=`) e `head()` — DIFERENCIAL nos 2
motores contra um servidor de eco local.

A API multipart veio do uso real: `post(url, fields={...}, file={"file":
{"name": caminho}})` — a lib lê o arquivo do disco (absoluto | pasta do script
| cwd), monta o multipart/form-data com boundary próprio e põe o Content-Type
certo (um manual é descartado, senão o boundary não bate). `head()` devolve
status/headers com corpo vazio — o cliente C já sabia não esperar corpo em
HEAD; faltava só o wrapper.
"""
import hashlib
import json as _json
import os
import socket
import subprocess
import sys
import threading
from email import message_from_bytes
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
from pathlib import Path

import pytest

RAIZ = Path(__file__).resolve().parent.parent
POOL = RAIZ / "pool"
SRC = RAIZ / "src"

# VM em C NÃO se pula: sem o binário o teste FALHA (skip = falso verde).
assert POOL.exists(), "binário 'pool' não compilado — rode ./rebuild_vm.sh."

INTERP = [sys.executable, "-m", "poolscript"]
VM = [str(POOL)]

CORPO_GET = b"corpo-do-get"


class _Eco(BaseHTTPRequestHandler):
    def log_message(self, *a):
        pass

    def _cabs(self, tamanho):
        self.send_response(200)
        self.send_header("Content-Type", "text/plain")
        self.send_header("Content-Length", str(tamanho))
        self.send_header("X-Eco", "sim")
        self.end_headers()

    def do_GET(self):
        self._cabs(len(CORPO_GET))
        self.wfile.write(CORPO_GET)

    def do_HEAD(self):
        self._cabs(len(CORPO_GET))     # mesmos headers do GET, corpo NENHUM

    def do_POST(self):
        length = int(self.headers.get("Content-Length", 0))
        body = self.rfile.read(length)
        ct = self.headers.get("Content-Type", "")
        msg = message_from_bytes(b"Content-Type: " + ct.encode() + b"\r\n\r\n" + body)
        campos, arquivos = {}, {}
        if msg.is_multipart():
            for parte in msg.get_payload():
                nome = parte.get_param("name", header="content-disposition")
                fn = parte.get_param("filename", header="content-disposition")
                dados = parte.get_payload(decode=True) or b""
                if fn is not None:
                    arquivos[nome] = [fn, parte.get_content_type(), len(dados),
                                      hashlib.sha256(dados).hexdigest()]
                else:
                    campos[nome] = dados.decode("utf-8")
        out = _json.dumps({"campos": campos, "arquivos": arquivos},
                          sort_keys=True, ensure_ascii=False).encode("utf-8")
        self.send_response(200)
        self.send_header("Content-Type", "application/json; charset=utf-8")
        self.send_header("Content-Length", str(len(out)))
        self.end_headers()
        self.wfile.write(out)


@pytest.fixture(scope="module")
def servidor():
    srv = ThreadingHTTPServer(("127.0.0.1", 0), _Eco)
    t = threading.Thread(target=srv.serve_forever, daemon=True)
    t.start()
    yield srv.server_address[1]
    srv.shutdown()


def _run(cmd, tmp_path, prog):
    ps = tmp_path / "t.ps"
    ps.write_text(prog, encoding="utf-8")
    env = dict(os.environ, PYTHONPATH=str(SRC))
    return subprocess.run(cmd + [str(ps)], capture_output=True, text=True,
                          env=env, timeout=30)


def _ambos_ok(tmp_path, prog):
    a = _run(INTERP, tmp_path, prog)
    b = _run(VM, tmp_path, prog)
    assert a.returncode == 0, a.stderr
    assert b.returncode == 0, b.stderr
    assert a.stdout == b.stdout, (
        f"DIVERGÊNCIA interp x VM\n--- interp\n{a.stdout}\n--- VM\n{b.stdout}")
    return a.stdout


def _ambos_falham(tmp_path, prog, trecho):
    a = _run(INTERP, tmp_path, prog)
    b = _run(VM, tmp_path, prog)
    assert a.returncode != 0 and b.returncode != 0, (a.stdout, b.stdout)
    assert trecho in a.stderr, a.stderr
    assert trecho in b.stderr, b.stderr


# ── head() ───────────────────────────────────────────────────────────────────

def test_head_sem_corpo(servidor, tmp_path):
    prog = (
        'import request\n'
        f'h = request.head("http://127.0.0.1:{servidor}/x")\n'
        'post(h.status, len(h.content))\n'
        f'g = request.get("http://127.0.0.1:{servidor}/x")\n'
        'post(g.status, len(g.content))\n'
    )
    assert _ambos_ok(tmp_path, prog) == "200 0\n200 12\n"


def test_head_existe_nos_aliases(tmp_path):
    # request e requests são o MESMO módulo; head tem que estar nos dois
    prog = 'import requests\npost(type(requests.head))\n'
    saida = _ambos_ok(tmp_path, prog)
    assert saida == _ambos_ok(tmp_path, 'import request\npost(type(request.head))\n')


# ── multipart: fields= / file= ───────────────────────────────────────────────

def _arquivo_binario(tmp_path):
    # binário de verdade (0..255 repetido) — pega corrupção de encoding
    dados = bytes(range(256)) * 17 + "acentuação-Ã¡".encode("utf-8")
    arq = tmp_path / "audio_teste.bin"
    arq.write_bytes(dados)
    return arq, dados


def test_multipart_fields_e_file(servidor, tmp_path):
    arq, dados = _arquivo_binario(tmp_path)
    prog = (
        'import request\n'
        f'r = request.post("http://127.0.0.1:{servidor}/up",\n'
        '    fields={"model": "whisper-large-v3", "language": "pt", "n": 3},\n'
        f'    file={{"file": {{"name": "{arq}"}}}})\n'
        'd = r.get_json()\n'
        'post(d["campos"]["model"], d["campos"]["language"], d["campos"]["n"])\n'
        'post(d["arquivos"]["file"])\n'
    )
    saida = _ambos_ok(tmp_path, prog)
    linhas = saida.splitlines()
    assert linhas[0] == "whisper-large-v3 pt 3"
    # nome do arquivo (basename), content-type fixo, tamanho e sha256 EXATOS —
    # o binário atravessou os dois clientes intacto
    assert "audio_teste.bin" in linhas[1]
    assert "application/octet-stream" in linhas[1]
    assert str(len(dados)) in linhas[1]
    assert hashlib.sha256(dados).hexdigest() in linhas[1]


def test_multipart_body_junto_erra_igual(servidor, tmp_path):
    prog = (
        'import request\n'
        f'request.post("http://127.0.0.1:{servidor}/up", body={{"a": 1}}, fields={{"b": "2"}})\n'
    )
    _ambos_falham(tmp_path, prog, "OU fields=")


def test_multipart_arquivo_inexistente_erra_igual(servidor, tmp_path):
    prog = (
        'import request\n'
        f'request.post("http://127.0.0.1:{servidor}/up", file={{"file": {{"name": "nao_existe_xyz.bin"}}}})\n'
    )
    _ambos_falham(tmp_path, prog, "arquivo não encontrado")


def test_multipart_file_sem_name_erra_igual(servidor, tmp_path):
    prog = (
        'import request\n'
        f'request.post("http://127.0.0.1:{servidor}/up", file={{"file": {{}}}})\n'
    )
    _ambos_falham(tmp_path, prog, 'sem "name"')
