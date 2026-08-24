"""jinker servindo HEAD — DIFERENCIAL nos 2 motores.

Uma rota que aceita GET tem que responder HEAD com os MESMOS status e headers
(Content-Length inclusive) e NENHUM corpo (RFC 9110). Antes: o interpretador
devolvia 501 (o BaseHTTPRequestHandler não tinha `do_HEAD`) e o VM devolvia 404
(o método não casava com a rota de GET) — divergência dupla, e nenhum dos dois
servia HEAD.
"""
import os
import socket
import subprocess
import sys
import time
from pathlib import Path

import pytest

RAIZ = Path(__file__).resolve().parent.parent
POOL = RAIZ / "pool"
SRC = RAIZ / "src"

# VM em C NÃO se pula: sem o binário o teste FALHA (skip = falso verde).
assert POOL.exists(), "binário 'pool' não compilado — rode ./rebuild_vm.sh."

INTERP = [sys.executable, "-m", "poolscript"]
VM = [str(POOL)]

APP = '''
from jinker import Jinker, cors, jsonify
app = Jinker(__name__)

@app.route("/ping", methods=cors.options(["GET"]))
action ping() {
    return jsonify({"pong": true})
}

@app.route("/so_post", methods=cors.options(["POST"]))
action so_post() {
    return jsonify({"ok": true})
}

run_selfwith_("main"):
    app(debug=false, host="127.0.0.1", port=__PORT__)
'''


def porta_livre() -> int:
    with socket.socket() as s:
        s.bind(("127.0.0.1", 0))
        return s.getsockname()[1]


def espera_porta(porta, timeout=15.0) -> bool:
    fim = time.time() + timeout
    while time.time() < fim:
        try:
            with socket.create_connection(("127.0.0.1", porta), timeout=0.5):
                return True
        except OSError:
            time.sleep(0.05)
    return False


def _cru(porta, metodo, caminho):
    """Fala HTTP na unha — nada de cliente que esconda o corpo do HEAD."""
    with socket.create_connection(("127.0.0.1", porta), timeout=10) as s:
        s.sendall(f"{metodo} {caminho} HTTP/1.1\r\nHost: 127.0.0.1\r\n"
                  f"Connection: close\r\n\r\n".encode())
        dados = b""
        while True:
            pedaco = s.recv(65536)
            if not pedaco:
                break
            dados += pedaco
    cabecalho, _, corpo = dados.partition(b"\r\n\r\n")
    linhas = cabecalho.decode("latin-1").split("\r\n")
    status = int(linhas[0].split()[1])
    headers = {}
    for ln in linhas[1:]:
        k, _, v = ln.partition(":")
        headers[k.strip().lower()] = v.strip()
    return status, headers, corpo


@pytest.fixture(params=["interp", "vm"])
def servidor(request, tmp_path_factory):
    cmd = INTERP if request.param == "interp" else VM
    porta = porta_livre()
    d = tmp_path_factory.mktemp("head")
    src = d / "app.ps"
    src.write_text(APP.replace("__PORT__", str(porta)), encoding="utf-8")
    env = dict(os.environ, PYTHONPATH=str(SRC))
    proc = subprocess.Popen(cmd + [str(src)], env=env,
                            stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    try:
        assert espera_porta(porta), f"servidor {request.param} não subiu"
        yield porta
    finally:
        proc.terminate()
        try:
            proc.wait(timeout=10)
        except subprocess.TimeoutExpired:
            proc.kill()


def test_head_na_rota_de_get(servidor):
    """HEAD /ping: 200, mesmos headers do GET, corpo VAZIO."""
    st_g, h_g, corpo_g = _cru(servidor, "GET", "/ping")
    st_h, h_h, corpo_h = _cru(servidor, "HEAD", "/ping")

    assert st_g == 200 and corpo_g, "o GET tem que trazer corpo"
    assert st_h == 200, f"HEAD devia responder 200, veio {st_h}"
    assert corpo_h == b"", f"HEAD NÃO pode ter corpo, veio {corpo_h!r}"
    # Content-Length anuncia o tamanho que o GET traria — não zero
    assert h_h.get("content-length") == h_g.get("content-length")
    assert h_h.get("content-type") == h_g.get("content-type")


def test_head_em_rota_que_nao_tem_get(servidor):
    """A rota só aceita POST — HEAD nela é 404, como o GET seria."""
    st_get, _, _ = _cru(servidor, "GET", "/so_post")
    st_head, _, corpo = _cru(servidor, "HEAD", "/so_post")
    assert st_get == 404
    assert st_head == 404
    assert corpo == b""


def test_head_em_rota_inexistente(servidor):
    st, _, corpo = _cru(servidor, "HEAD", "/nao_existe")
    assert st == 404
    assert corpo == b""
