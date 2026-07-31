"""Servidor jinker: performance — HTTP/1.1 com keep-alive."""
import http.client
import socket
import threading
import time

from poolscript.stdlib.jinker_lib import Jinker, Route


def _free_port() -> int:
    s = socket.socket()
    s.bind(("127.0.0.1", 0))
    port = s.getsockname()[1]
    s.close()
    return port


def _start_app(port: int) -> None:
    app = Jinker("test")
    app._routes.append(Route("/ping", ["GET"], [], lambda req, res: {"ok": True}))
    threading.Thread(target=lambda: app(host="127.0.0.1", port=port), daemon=True).start()
    for _ in range(100):                      # espera o socket subir
        try:
            socket.create_connection(("127.0.0.1", port), timeout=0.2).close()
            return
        except OSError:
            time.sleep(0.02)
    raise RuntimeError("servidor não subiu")


def test_server_speaks_http_1_1_and_keeps_alive():
    port = _free_port()
    _start_app(port)

    conn = http.client.HTTPConnection("127.0.0.1", port)
    conn.request("GET", "/ping")
    r = conn.getresponse()
    r.read()

    assert r.status == 200
    assert r.version == 11          # 11 = HTTP/1.1 (antes era 10 = HTTP/1.0)
    assert r.will_close is False    # keep-alive: a conexão NÃO fecha após a resposta

    # segunda request reaproveita a mesma conexão TCP
    conn.request("GET", "/ping")
    r2 = conn.getresponse()
    assert r2.status == 200
    r2.read()
    conn.close()
