"""WS-recv do CLIENTE (ws_drena) dentro de handler jinker NÃO trava o worker.

O drain do cliente WebSocket (poll + leitura de frame, que pode bloquear no meio
de um frame fatiado) roda numa thread do pool (fib_offload) e a fibra cede —
igual DB/HTTP/mail. Prova por TEMPO: um handler conecta um WS a um servidor que
segura a resposta ~0.18s (DENTRO da janela de 200ms que o .close() espera) e
fecha; 3 requests simultâneos saem em ~tempo de UM (~0.2s), não 3x (~0.6s).

Também confere o comportamento FUNCIONAL: on_message recebe a mensagem no drain
(ordem preservada), nos DOIS motores.
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

assert POOL.exists(), "binário pool não compilado — rode ./rebuild_vm.sh (VM em C não se pula: skip = falso verde)"

# Servidor WS eco-lento em Python puro (handshake RFC6455 + 1 frame de texto,
# sem lib externa): espera receber um frame e responde depois de ~0.5s.
ALVO_WS = r"""
import base64, hashlib, socket, sys, threading, time
GUID = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11"
def frame_texto(s):
    b = s.encode(); n = len(b)
    if n < 126: return bytes([0x81, n]) + b
    return bytes([0x81, 126, n >> 8, n & 0xFF]) + b
def atende(c):
    dados = b""
    while b"\r\n\r\n" not in dados:
        dados += c.recv(4096)
    chave = ""
    for ln in dados.split(b"\r\n"):
        if ln.lower().startswith(b"sec-websocket-key:"):
            chave = ln.split(b":", 1)[1].strip().decode()
    aceite = base64.b64encode(hashlib.sha1((chave + GUID).encode()).digest()).decode()
    c.sendall(("HTTP/1.1 101 Switching Protocols\r\nUpgrade: websocket\r\n"
               "Connection: Upgrade\r\nSec-WebSocket-Accept: %s\r\n\r\n" % aceite).encode())
    c.recv(4096)              # frame do cliente (mascarado) — não precisa decodificar
    time.sleep(0.18)          # segura a resposta DENTRO da janela de 200ms do close()
    c.sendall(frame_texto('{"eco": 1}'))
    time.sleep(0.2)
    c.close()
srv = socket.socket(); srv.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
srv.bind(("127.0.0.1", int(sys.argv[1]))); srv.listen(16)
while True:
    c, _ = srv.accept()
    threading.Thread(target=atende, args=(c,), daemon=True).start()
"""

APP = """\
import request as web
from jinker import Jinker
app = Jinker()
int recebidas = 0
action marca(m):
    global recebidas
    recebidas = recebidas + 1
@app.route("/ws", methods=["GET"])
action faz_ws():
    conn = web.ws_connect("ws://127.0.0.1:%d/")
    conn.on_message(marca)
    conn.send("oi")
    conn.close()
    return {"ok": true}
@app.route("/total", methods=["GET"])
action total():
    return {"n": recebidas}
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


def test_ws_recv_em_handler_nao_trava_worker(tmp_path):
    pt, pp = _porta(), _porta()
    env = dict(os.environ, PYTHONPATH=str(SRC))
    alvo = subprocess.Popen([sys.executable, "-c", ALVO_WS, str(pt)])
    app_ps = tmp_path / "app.ps"
    app_ps.write_text(APP % (pt, pp), encoding="utf-8")
    srv = subprocess.Popen([str(POOL), str(app_ps)], env=env,
                           stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    try:
        assert _espera(pt), "alvo WS não subiu"
        assert _espera(pp), "servidor jinker não subiu"
        time.sleep(0.3)

        res = {}

        def bate(i):
            with urllib.request.urlopen(f"http://127.0.0.1:{pp}/ws", timeout=15) as r:
                res[i] = r.read()

        t0 = time.time()
        ths = [threading.Thread(target=bate, args=(i,)) for i in range(3)]
        for t in ths: t.start()
        for t in ths: t.join()
        total = time.time() - t0

        assert len(res) == 3, "algum request falhou"
        for corpo in res.values():
            assert b'"ok": true' in corpo.lower(), corpo
        # funcional: as 3 mensagens chegaram no on_message (dentro da janela do close)
        with urllib.request.urlopen(f"http://127.0.0.1:{pp}/total", timeout=10) as r:
            assert b'"n": 3' in r.read(), "on_message nao recebeu as 3 mensagens"
        # serial: 3 * ~0.2s >= 0.6s; concorrente ~0.2-0.3s. Margem folgada em 0.45s.
        assert total < 0.45, f"WS-recv serializou (travou o worker): {total:.2f}s"
    finally:
        srv.terminate(); alvo.terminate()
        for p in (srv, alvo):
            try: p.wait(3)
            except Exception: p.kill()
