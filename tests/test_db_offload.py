"""I/O de banco não-bloqueante (offload pra thread pool) no binário `pool`.

Um `SELECT` lento roda dentro do driver em C (libpq) fazendo `recv` bloqueante.
Sem offload, ele travaria o worker inteiro; com o offload, a fibra cede a uma
thread e o worker atende outras requisições. Prova por TEMPO: N consultas com
`pg_sleep(S)` concorrentes terminam em ~S (sobrepostas), não em N*S.

Auto-contido: sobe um Postgres DESCARTÁVEL via initdb (sem sudo, trust). Pula se
os binários do Postgres não existirem.
"""
import asyncio
import getpass
import glob
import os
import socket
import subprocess
import time
from pathlib import Path

import pytest

RAIZ = Path(__file__).resolve().parent.parent
POOL = RAIZ / "pool"
SRC = RAIZ / "src"


def _pgbin(nome):
    cands = sorted(glob.glob(f"/usr/lib/postgresql/*/bin/{nome}"))
    return cands[-1] if cands else None


pytestmark = pytest.mark.skipif(
    not POOL.exists() or not _pgbin("initdb") or not _pgbin("pg_ctl"),
    reason="precisa do binário pool e dos binários do Postgres",
)


def porta_livre():
    s = socket.socket()
    s.bind(("127.0.0.1", 0))
    p = s.getsockname()[1]
    s.close()
    return p


def espera_porta(porta, timeout=8.0):
    fim = time.time() + timeout
    while time.time() < fim:
        try:
            socket.create_connection(("127.0.0.1", porta), timeout=0.2).close()
            return True
        except OSError:
            time.sleep(0.05)
    return False


@pytest.fixture(scope="module")
def pg(tmp_path_factory):
    d = tmp_path_factory.mktemp("pg")
    data = d / "data"
    user = getpass.getuser()
    r = subprocess.run([_pgbin("initdb"), "-D", str(data), "-A", "trust", "-U", user],
                       capture_output=True, text=True)
    if r.returncode != 0:
        pytest.skip("initdb falhou: " + r.stderr[-300:])
    porta = porta_livre()
    log = d / "pg.log"
    r = subprocess.run(
        [_pgbin("pg_ctl"), "-D", str(data),
         "-o", f"-p {porta} -k /tmp -c listen_addresses=127.0.0.1",
         "-l", str(log), "-w", "start"],
        capture_output=True, text=True, timeout=30)
    if r.returncode != 0 or not espera_porta(porta):
        pytest.skip("postgres não subiu: " + (log.read_text()[-300:] if log.exists() else ""))
    try:
        yield porta, user
    finally:
        subprocess.run([_pgbin("pg_ctl"), "-D", str(data), "-m", "immediate", "stop"],
                       capture_output=True, text=True, timeout=15)


APP = """\
from jinker import Jinker, jsonify
import psodbc

app = Jinker(__name__)

@app.route("/q")
action q() {
    conn = psodbc.connect(driver="postgres", host="127.0.0.1", port=__PGPORT__, user="__USER__", password="", database="postgres")
    cur = conn.cursor()
    cur.execute("SELECT pg_sleep(0.15)")
    cur.fetchall()
    conn.close()
    return jsonify({"ok": true})
}

app(debug=false, host="127.0.0.1", port=__PORT__, workers=1)
"""


async def _uma(porta, res, i):
    try:
        r, w = await asyncio.open_connection("127.0.0.1", porta)
        w.write(b"GET /q HTTP/1.1\r\nHost: h\r\nConnection: close\r\n\r\n")
        await w.drain()
        data = b""
        while True:
            c = await asyncio.wait_for(r.read(4096), timeout=15)
            if not c:
                break
            data += c
        w.close()
        res[i] = b'"ok": true' in data
    except Exception as e:  # noqa: BLE001
        res[i] = "ERRO:" + str(e)


def test_query_lenta_nao_serializa(pg, tmp_path):
    """8 handlers com pg_sleep(0.15) concorrentes terminam sobrepostos (bem menos
    que 8*0.15=1.2s), provando que a query no banco não trava o worker."""
    pgport, user = pg
    porta = porta_livre()
    src = tmp_path / "app.ps"
    src.write_text(APP.replace("__PGPORT__", str(pgport)).replace("__USER__", user)
                      .replace("__PORT__", str(porta)), encoding="utf-8")
    env = dict(os.environ, PYTHONPATH=str(SRC))
    proc = subprocess.Popen([str(POOL), str(src)], env=env,
                            stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    try:
        assert espera_porta(porta), "servidor não subiu"

        async def roda():
            res = [None] * 8
            t0 = time.monotonic()
            await asyncio.gather(*[_uma(porta, res, i) for i in range(8)])
            return time.monotonic() - t0, res
        elapsed, res = asyncio.run(roda())
        assert all(r is True for r in res), res
        # serializado seria >= 1.2s; sobreposto ~0.3-0.6s. Margem folgada.
        assert elapsed < 0.9, f"query serializou o worker? {elapsed:.2f}s para 8x pg_sleep(0.15)"
    finally:
        proc.terminate()
        try:
            proc.wait(timeout=3)
        except subprocess.TimeoutExpired:
            proc.kill()
