"""Fibras (green-threads) do jinker no binário `pool`.

Um handler que cede numa I/O (aqui: `sleep`) NÃO trava mais o worker: várias
requisições correm concorrentes num único processo/worker. O teste prova isso
por TEMPO (N requisições que dormem S cada terminam em ~S, não N*S) e prova a
correção do GC rodando com fibras suspensas (handler que aloca muito + dorme +
usa os dados: se o GC liberar algo vivo de uma fibra suspensa, o checksum diverge).

Só o binário C tem fibras; o interpretador (referência) já sobrepõe I/O-bound
via threads, então o observável — o RESULTADO — é o mesmo nos dois.
"""
import asyncio
import os
import socket
import subprocess
import time
from pathlib import Path

import pytest

RAIZ = Path(__file__).resolve().parent.parent
POOL = RAIZ / "pool"
SRC = RAIZ / "src"

pytestmark = pytest.mark.skipif(not POOL.exists(), reason="binário pool não compilado")


def porta_livre():
    s = socket.socket()
    s.bind(("127.0.0.1", 0))
    p = s.getsockname()[1]
    s.close()
    return p


def espera_subir(porta, timeout=8.0):
    fim = time.time() + timeout
    while time.time() < fim:
        try:
            socket.create_connection(("127.0.0.1", porta), timeout=0.2).close()
            return True
        except OSError:
            time.sleep(0.05)
    return False


APP = """\
from jinker import Jinker, jsonify

app = Jinker(__name__)

@app.route("/slow", methods=["GET"])
action slow() {
    sleep(0.2)
    return jsonify({"ok": true})
}

@app.route("/calc", methods=["GET"])
action calc() {
    d = {}
    for each i in range(1000) {
        d[str(i)] = [i, i + 1]
    }
    sleep(0.1)
    total = 0
    for each i in range(1000) {
        total = total + d[str(i)][0]
    }
    return jsonify({"total": total})
}

app(debug=false, host="127.0.0.1", port=__PORTA__, workers=1)
"""


def sobe(tmp_path):
    porta = porta_livre()
    src = tmp_path / "app.ps"
    src.write_text(APP.replace("__PORTA__", str(porta)), encoding="utf-8")
    env = dict(os.environ, PYTHONPATH=str(SRC))
    proc = subprocess.Popen([str(POOL), str(src)], env=env,
                            stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    if not espera_subir(porta):
        proc.kill()
        raise RuntimeError("servidor não subiu")
    return proc, porta


async def _uma(porta, caminho, res, idx):
    try:
        r, w = await asyncio.open_connection("127.0.0.1", porta)
        w.write(f"GET {caminho} HTTP/1.1\r\nHost: h\r\nConnection: close\r\n\r\n".encode())
        await w.drain()
        data = b""
        while True:
            c = await asyncio.wait_for(r.read(4096), timeout=10)
            if not c:
                break
            data += c
        w.close()
        res[idx] = data.decode("utf-8", "replace")
    except Exception as e:  # noqa: BLE001
        res[idx] = "ERRO: " + str(e)


def em_paralelo(porta, caminho, n):
    """Dispara n conexões NOVAS de uma vez (asyncio, concorrência real) e mede o
    tempo total de parede."""
    async def roda():
        res = [None] * n
        t0 = time.monotonic()
        await asyncio.gather(*[_uma(porta, caminho, res, i) for i in range(n)])
        return time.monotonic() - t0, res
    return asyncio.run(roda())


def test_sleep_nao_serializa(tmp_path):
    """10 handlers dormindo 0.2s terminam CONCORRENTES (~0.2s), não em 2.0s."""
    proc, porta = sobe(tmp_path)
    try:
        elapsed, res = em_paralelo(porta, "/slow", 10)
        assert all(r and '"ok": true' in r for r in res), res
        # serializado seria >= 2.0s; concorrente ~0.2-0.4s. Margem folgada p/ não piscar.
        assert elapsed < 1.0, f"serializou? {elapsed:.2f}s para 10x sleep(0.2)"
    finally:
        proc.terminate()
        try:
            proc.wait(timeout=3)
        except subprocess.TimeoutExpired:
            proc.kill()


def test_gc_sob_fibras_suspensas(tmp_path):
    """Handler que aloca 1000 entradas + dorme + soma: com 16 concorrentes o GC
    roda com fibras suspensas; todo checksum tem que bater (1000*999/2 = 499500)."""
    proc, porta = sobe(tmp_path)
    try:
        elapsed, res = em_paralelo(porta, "/calc", 16)
        assert all(r and '"total": 499500' in r for r in res), res
        assert elapsed < 1.0, f"serializou? {elapsed:.2f}s"
    finally:
        proc.terminate()
        try:
            proc.wait(timeout=3)
        except subprocess.TimeoutExpired:
            proc.kill()
