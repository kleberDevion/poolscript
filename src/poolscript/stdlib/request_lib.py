"""
Módulo `request` da PoolScript.

Implementa get/post/put/patch/delete usando urllib (zero dependências externas).
Aplica User-Agent default automaticamente para evitar 403 em sites que bloqueiam
requests sem identificação. Body dict/list vira JSON automaticamente.
"""
from __future__ import annotations
import json as _json
import urllib.request
import urllib.error
import urllib.parse
from dataclasses import dataclass, field


@dataclass
class Response:
    status: int
    text: str
    headers: dict = field(default_factory=dict)
    url: str = ""

    @property
    def ok(self) -> bool:
        return 200 <= self.status < 300

    def get(self, key: str):
        """Header (case-insensitive) ou chave do JSON parseado."""
        for k, v in self.headers.items():
            if k.lower() == key.lower():
                return v
        try:
            data = _json.loads(self.text)
            if isinstance(data, dict) and key in data:
                return data[key]
        except (ValueError, TypeError):
            pass
        return None

    def get_json(self, key: str | None = None):
        try:
            data = _json.loads(self.text)
        except ValueError:
            return None
        if key is None:
            return data
        if isinstance(data, dict):
            return data.get(key)
        return None

    def json(self):
        """Retorna o body parseado como dict/list Python (None se inválido)."""
        return self.get_json()

    def __repr__(self):
        return f"<Response status={self.status} url={self.url!r}>"


DEFAULT_USER_AGENT = (
    "Mozilla/5.0 (Windows NT 10.0; Win64; x64) "
    "AppleWebKit/537.36 (KHTML, like Gecko) "
    "Chrome/120.0.0.0 Safari/537.36 PoolScript/0.3"
)
DEFAULT_ACCEPT = "application/json, text/plain, */*"


def _apply_default_headers(headers: dict | None, has_body: bool = False) -> dict:
    """User-Agent + Accept default. Content-Type só se houver body JSON."""
    h = dict(headers) if headers else {}
    if not any(k.lower() == "user-agent" for k in h):
        h["User-Agent"] = DEFAULT_USER_AGENT
    if not any(k.lower() == "accept" for k in h):
        h["Accept"] = DEFAULT_ACCEPT
    return h


def _request(method: str, url: str, headers: dict | None = None, body=None,
             timeout: int = 30) -> Response | dict:
    data = None
    h = _apply_default_headers(headers, body is not None)
    if body is not None:
        if isinstance(body, (dict, list)):
            data = _json.dumps(body).encode("utf-8")
            if not any(k.lower() == "content-type" for k in h):
                h["Content-Type"] = "application/json"
        elif isinstance(body, str):
            data = body.encode("utf-8")
        elif isinstance(body, bytes):
            data = body
        else:
            data = str(body).encode("utf-8")

    req = urllib.request.Request(url, data=data, headers=h, method=method.upper())
    try:
        with urllib.request.urlopen(req, timeout=timeout) as resp:
            text = resp.read().decode("utf-8", errors="replace")
            return Response(
                status=resp.status, text=text,
                headers=dict(resp.headers), url=resp.geturl(),
            )
    except urllib.error.HTTPError as exc:
        text = exc.read().decode("utf-8", errors="replace") if exc.fp else ""
        return Response(
            status=exc.code, text=text,
            headers=dict(exc.headers or {}), url=url,
        )
    except urllib.error.URLError as exc:
        return {"error": "connection_error", "message": str(exc.reason), "url": url}
    except TimeoutError:
        return {"error": "timeout", "message": f"requisição passou de {timeout}s", "url": url}


def get(url: str, headers: dict | None = None, body=None, timeout: int = 30):
    return _request("GET", url, headers=headers, body=body, timeout=timeout)


def post(url: str, headers: dict | None = None, body=None, timeout: int = 30):
    return _request("POST", url, headers=headers, body=body, timeout=timeout)


def put(url: str, headers: dict | None = None, body=None, timeout: int = 30):
    return _request("PUT", url, headers=headers, body=body, timeout=timeout)


def patch(url: str, headers: dict | None = None, body=None, timeout: int = 30):
    return _request("PATCH", url, headers=headers, body=body, timeout=timeout)


def delete(url: str, headers: dict | None = None, body=None, timeout: int = 30):
    return _request("DELETE", url, headers=headers, body=body, timeout=timeout)


class WsConnection:
    """Conexão WebSocket do lado cliente."""

    def __init__(self, url: str):
        self._url = url
        self._ws = None
        self._loop = None
        self._thread = None
        self._connected = False
        self._on_message = None
        self._connect()

    def _connect(self):
        import asyncio
        import threading
        try:
            import websockets
        except ImportError:
            print("Error: instale websockets — pip install websockets")
            return

        async def _run():
            async with websockets.connect(self._url) as ws:
                self._ws = ws
                self._connected = True
                async for raw in ws:
                    import json as _json
                    try:
                        msg = _json.loads(raw)
                    except Exception:
                        msg = raw
                    if self._on_message:
                        self._on_message(msg)

        def _thread_fn():
            self._loop = asyncio.new_event_loop()
            asyncio.set_event_loop(self._loop)
            self._loop.run_until_complete(_run())

        self._thread = threading.Thread(target=_thread_fn, daemon=True)
        self._thread.start()

        # Aguarda conexão
        import time
        for _ in range(50):
            if self._connected:
                break
            time.sleep(0.1)

    def send(self, data) -> None:
        """Envia uma mensagem pro servidor."""
        import asyncio
        import json as _json
        if not self._connected or self._ws is None:
            print("Error: não conectado")
            return
        msg = data if isinstance(data, str) else _json.dumps(data, ensure_ascii=False)
        asyncio.run_coroutine_threadsafe(self._ws.send(msg), self._loop)

    def on_message(self, callback) -> None:
        """Define callback chamado quando chega mensagem."""
        self._on_message = callback

    def close(self) -> None:
        """Fecha a conexão."""
        if self._ws and self._loop:
            import asyncio
            asyncio.run_coroutine_threadsafe(self._ws.close(), self._loop)
        self._connected = False

    def __repr__(self):
        status = "conectado" if self._connected else "desconectado"
        return f"<WsConnection {self._url} [{status}]>"


def ws_connect(url: str) -> WsConnection:
    """
    Conecta a um servidor WebSocket.

    conn = request.ws_connect("ws://localhost:7701/chat")
    conn.send({"user_name": "joao", "body_msg": "oi!"})
    """
    return WsConnection(url)


EXPORTS = {
    "get": get,
    "post": post,
    "put": put,
    "patch": patch,
    "delete": delete,
    "ws_connect": ws_connect,
}
