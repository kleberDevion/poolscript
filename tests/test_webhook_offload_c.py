"""Webhook (request de SAÍDA) dentro de um handler jinker NÃO trava o worker.

A I/O de rede do `request` roda numa thread do pool (mesmo offload do banco), a
fibra do handler cede, e outros requests são atendidos enquanto isso. Prova por
TEMPO: 3 requests simultâneos, cada um fazendo um webhook a um alvo que dorme
0.5s — concorrente sai em ~0.5s, serial sairia em ~1.5s.

Dentro do handler `request` é a requisição de ENTRADA (RequestProxy); o módulo
de saída entra como `import request as web`.
"""
import os
import socket
import subprocess
import sys
import threading
import time
import urllib.request
from pathlib import Path

import pytest

RAIZ = Path(__file__).resolve().parent.parent
POOL = RAIZ / "pool"
SRC = RAIZ / "src"

pytestmark = pytest.mark.skipif(not POOL.exists(), reason="binário pool não compilado")

ALVO = """\
import sys, time
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
class H(BaseHTTPRequestHandler):
    def do_GET(self):
        time.sleep(0.5)
        self.send_response(200); self.send_header("Content-Type","text/plain"); self.end_headers()
        self.wfile.write(b"ok")
    def log_message(self, *a): pass
ThreadingHTTPServer(("127.0.0.1", int(sys.argv[1])), H).serve_forever()
"""

APP = """\
import request as web
from jinker import Jinker
app = Jinker()
@app.route("/proxy", methods=["GET"])
action proxy():
    r = web.get("http://127.0.0.1:%d/x")
    return {"status": r.status}
app(debug=false, host="127.0.0.1", port=%d)
"""


def _porta():
    s = socket.socket(); s.bind(("127.0.0.1", 0)); p = s.getsockname()[1]; s.close(); return p


def _espera(porta, t=6):
    fim = time.time() + t
    while time.time() < fim:
        try:
            socket.create_connection(("127.0.0.1", porta), timeout=0.2).close(); return True
        except OSError:
            time.sleep(0.05)
    return False


def test_webhook_em_handler_nao_trava_worker(tmp_path):
    pt, pp = _porta(), _porta()
    env = dict(os.environ, PYTHONPATH=str(SRC))
    alvo = subprocess.Popen([sys.executable, "-c", ALVO, str(pt)])
    app_ps = tmp_path / "app.ps"
    app_ps.write_text(APP % (pt, pp), encoding="utf-8")
    srv = subprocess.Popen([str(POOL), str(app_ps)], env=env,
                           stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    try:
        assert _espera(pt), "alvo lento não subiu"
        assert _espera(pp), "servidor jinker não subiu"
        time.sleep(0.3)

        res = {}

        def bate(i):
            t0 = time.time()
            with urllib.request.urlopen(f"http://127.0.0.1:{pp}/proxy", timeout=10) as r:
                res[i] = (r.read(), time.time() - t0)

        t0 = time.time()
        ths = [threading.Thread(target=bate, args=(i,)) for i in range(3)]
        for t in ths: t.start()
        for t in ths: t.join()
        total = time.time() - t0

        assert len(res) == 3, "algum request falhou"
        for corpo, _ in res.values():
            assert b'"status": 200' in corpo, corpo
        # serial = 3*0.5 = 1.5s; concorrente ~0.5s. Margem folgada em 1.0s.
        assert total < 1.0, f"webhook serializou (travou o worker): {total:.2f}s"
    finally:
        srv.terminate(); alvo.terminate()
        for p in (srv, alvo):
            try: p.wait(3)
            except Exception: p.kill()
