"""route_prefix — prefixo aplicado a todas as rotas do jinker, nos 2 motores.

`Jinker(route_prefix="/api")` monta `@app.route("/hello")` em `/api/hello`.
Mesmo observável no interpretador e na VM em C.
"""
import os
import socket
import subprocess
import sys
import time
import urllib.error
import urllib.request
from pathlib import Path

import pytest

RAIZ = Path(__file__).resolve().parent.parent
POOL = RAIZ / "pool"
SRC = RAIZ / "src"

APP = """\
from jinker import Jinker
app = Jinker(route_prefix="/api")
@app.route("/hello", methods=["GET"])
action hello():
    return {"ok": true}
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


def _codigo(porta, caminho):
    try:
        with urllib.request.urlopen(f"http://127.0.0.1:{porta}{caminho}", timeout=4) as r:
            return r.status
    except urllib.error.HTTPError as e:
        return e.code


def _sobe(cmd, tmp_path, nome):
    pp = _porta()
    p = tmp_path / nome
    p.write_text(APP % pp, encoding="utf-8")
    env = dict(os.environ, PYTHONPATH=str(SRC))
    proc = subprocess.Popen(cmd + [str(p)], env=env,
                            stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    return proc, pp


def _checa(proc, pp):
    try:
        assert _espera(pp), "servidor não subiu"
        assert _codigo(pp, "/api/hello") == 200, "rota com prefixo devia dar 200"
        assert _codigo(pp, "/hello") == 404, "rota sem prefixo devia dar 404"
    finally:
        proc.terminate()
        try: proc.wait(3)
        except Exception: proc.kill()


@pytest.mark.skipif(not POOL.exists(), reason="binário pool não compilado")
def test_route_prefix_vm(tmp_path):
    proc, pp = _sobe([str(POOL)], tmp_path, "vm.ps")
    _checa(proc, pp)


def test_route_prefix_interp(tmp_path):
    proc, pp = _sobe([sys.executable, "-m", "poolscript"], tmp_path, "it.ps")
    _checa(proc, pp)
