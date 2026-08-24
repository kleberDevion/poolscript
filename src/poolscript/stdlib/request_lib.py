"""
Módulo `request` da PoolScript.

Implementa get/post/put/patch/delete usando urllib (zero dependências externas).
Aplica User-Agent default automaticamente para evitar 403 em sites que bloqueiam
requests sem identificação. Body dict/list vira JSON automaticamente.
"""
from __future__ import annotations
import json as _json
import os as _os
import urllib.request
import urllib.error
import urllib.parse
from dataclasses import dataclass, field


DEFAULT_MAX_STREAM = 100 * 1024 * 1024  # 100 MB — teto padrão quando stream=true


def _parse_size(value) -> int:
    """Aceita bytes (int) ou string amigável: '100mb', '50kb', '2gb', '500b'."""
    if isinstance(value, (int, float)):
        return int(value)
    s = str(value).strip().lower().replace(" ", "")
    for suf, mult in (("gb", 1024 ** 3), ("mb", 1024 ** 2), ("kb", 1024), ("b", 1)):
        if s.endswith(suf):
            return int(float(s[: -len(suf)]) * mult)
    return int(float(s))  # sem sufixo → bytes


@dataclass
class Response:
    status: int
    headers: dict = field(default_factory=dict)
    url: str = ""
    _raw: bytes = b""

    # ── corpo ─────────────────────────────────────────────────────────
    @property
    def text(self) -> str:
        """Corpo como texto UTF-8 (tolerante). Para binário, use .content."""
        return self._raw.decode("utf-8", errors="replace")

    @property
    def content(self) -> bytes:
        """Bytes crus — use para binário (imagem, .exe, zip, pdf...)."""
        return self._raw

    @property
    def size(self) -> int:
        """Tamanho do corpo em bytes."""
        return len(self._raw)

    @property
    def status_code(self) -> int:
        """Alias de `.status` — mesmo nome que a lib `requests` do Python usa,
        pra quem tem o hábito. Só leitura (a resposta já chegou)."""
        return self.status

    @property
    def ok(self) -> bool:
        return 200 <= self.status < 300

    def decode(self, encoding: str = "utf-8") -> str:
        """Decodifica o corpo com o encoding dado (ex: .decode('latin-1'))."""
        return self._raw.decode(encoding, errors="replace")

    def content_type(self, expected: str) -> "Response":
        """Valida que o Content-Type da resposta bate com o esperado.
        Lança erro (cai no catch) se não bater — bom para downloads específicos.
        Encadeável: request.get(url).content_type('application/octet-stream')."""
        actual = ""
        for k, v in self.headers.items():
            if k.lower() == "content-type":
                actual = v
                break
        if actual.split(";")[0].strip().lower() != str(expected).split(";")[0].strip().lower():
            raise ValueError(
                f"Content-Type inesperado — esperado '{expected}', "
                f"veio '{actual or '(vazio)'}' (url={self.url})"
            )
        return self

    @property
    def filename(self) -> str:
        """Nome sugerido pelo servidor (Content-Disposition) ou o fim da URL."""
        return self._derive_filename()

    def save(self, path: str = "."):
        """Salva o corpo em disco. Se `path` for uma pasta (ex: '.'), o nome do
        arquivo vem do Content-Disposition (ou do fim da URL). Se `path` já
        incluir o nome, usa ele.

        Retorna um PoolFile do arquivo salvo — com `.name`, `.size`, `.bytes()`,
        `.move(destino)`, `.copy(destino)`, `.delete()`, `.path()`."""
        from pathlib import Path as _Path
        s = str(path)
        p = _Path(s)
        # trata como PASTA se: '.', '..', termina em barra, ou já existe como dir
        if s in (".", "..") or s.endswith(("/", "\\")) or p.is_dir():
            p = p / self._derive_filename()
        p.parent.mkdir(parents=True, exist_ok=True)
        p.write_bytes(self._raw)
        from .os_lib import PoolFile as _PoolFile
        return _PoolFile(p)

    def _derive_filename(self) -> str:
        import re as _re
        for k, v in self.headers.items():
            if k.lower() == "content-disposition":
                m = _re.search(r'filename\*?=(?:"([^"]+)"|([^;]+))', v)
                if m:
                    return (m.group(1) or m.group(2)).strip().strip('"')
        from urllib.parse import urlparse, unquote
        tail = unquote(urlparse(self.url).path.rsplit("/", 1)[-1])
        return tail or "download"

    # ── helpers de leitura (retrocompatíveis) ─────────────────────────
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
        return f"<Response status={self.status} url={self.url!r} ({self.size} bytes)>"


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


def _read_body(resp, stream: bool, max_size) -> bytes:
    """Lê o corpo. Com stream=true, lê em pedaços e aborta (raise) se passar
    do teto de memória (max_size, ou 100 MB por padrão)."""
    if not stream:
        return resp.read()
    cap = _parse_size(max_size) if max_size is not None else DEFAULT_MAX_STREAM
    clen = resp.headers.get("Content-Length")
    if clen and clen.isdigit() and int(clen) > cap:
        raise MemoryError(
            f"download de {int(clen)} bytes passa do limite de {cap} bytes — "
            f"aumente com max_size= (ex: max_size='500mb')"
        )
    chunks = []
    total = 0
    while True:
        chunk = resp.read(65536)
        if not chunk:
            break
        total += len(chunk)
        if total > cap:
            raise MemoryError(
                f"download passou do limite de {cap} bytes — "
                f"aumente com max_size= (ex: max_size='500mb')"
            )
        chunks.append(chunk)
    return b"".join(chunks)


def _resolve_arquivo(caminho: str) -> str:
    """Arquivo de ONDE o usuário quiser: absoluto, relativo à pasta do script
    em execução, ou ao diretório atual (a mesma regra da lib os/guzer)."""
    if _os.path.isabs(caminho):
        if _os.path.isfile(caminho):
            return caminho
    else:
        try:
            from .os_lib import _SCRIPT_DIR
            if _SCRIPT_DIR is not None:
                p = _os.path.join(str(_SCRIPT_DIR), caminho)
                if _os.path.isfile(p):
                    return p
        except Exception:
            pass
        if _os.path.isfile(caminho):
            return caminho
    raise FileNotFoundError(f"file=: arquivo não encontrado: {caminho}")


def _monta_multipart(fields, file) -> "tuple[bytes, str]":
    """Corpo multipart/form-data: `fields=` são os campos simples do
    formulário; `file=` é {campo: {"name": caminho}} — o arquivo é lido do
    disco e entra como parte binária (application/octet-stream)."""
    boundary = "----poolscript" + _os.urandom(16).hex()
    partes = []
    if fields is not None:
        if not isinstance(fields, dict):
            raise TypeError("fields= espera um dict {campo: valor}")
        for k, v in fields.items():
            if v is None:
                continue
            valor = v if isinstance(v, str) else str(v)
            partes.append(
                (f'--{boundary}\r\nContent-Disposition: form-data; name="{k}"\r\n\r\n').encode("utf-8")
                + valor.encode("utf-8") + b"\r\n")
    if file is not None:
        if not isinstance(file, dict):
            raise TypeError('file= espera um dict {campo: {"name": caminho}}')
        for campo, spec in file.items():
            caminho = spec.get("name") if isinstance(spec, dict) else spec
            if not caminho:
                raise ValueError(f'file=: campo "{campo}" sem "name" (o caminho do arquivo)')
            caminho = _resolve_arquivo(str(caminho))
            nome_arq = _os.path.basename(caminho)
            with open(caminho, "rb") as f:
                conteudo = f.read()
            partes.append(
                (f'--{boundary}\r\nContent-Disposition: form-data; name="{campo}"; '
                 f'filename="{nome_arq}"\r\nContent-Type: application/octet-stream\r\n\r\n').encode("utf-8")
                + conteudo + b"\r\n")
    corpo = b"".join(partes) + f"--{boundary}--\r\n".encode("utf-8")
    return corpo, f"multipart/form-data; boundary={boundary}"


def _request(method: str, url: str, headers: dict | None = None, body=None,
             timeout: int = 30, stream: bool = False, max_size=None,
             fields=None, file=None) -> Response:
    data = None
    h = _apply_default_headers(headers, body is not None or fields is not None or file is not None)
    if fields is not None or file is not None:
        if body is not None:
            raise ValueError("use body= OU fields=/file= (multipart) — não os dois juntos")
        data, ct = _monta_multipart(fields, file)
        # o Content-Type carrega o boundary gerado — um manual não serviria
        h = {k: v for k, v in h.items() if k.lower() != "content-type"}
        h["Content-Type"] = ct
    elif body is not None:
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
            raw = _read_body(resp, stream, max_size)
            return Response(
                status=resp.status, headers=dict(resp.headers),
                url=resp.geturl(), _raw=raw,
            )
    except urllib.error.HTTPError as exc:
        raw = exc.read() if exc.fp else b""
        return Response(
            status=exc.code, headers=dict(exc.headers or {}),
            url=url, _raw=raw,
        )
    except urllib.error.URLError as exc:
        raise ConnectionError(f"{exc.reason} (url={url})") from exc
    except TimeoutError:
        raise TimeoutError(f"requisição passou de {timeout}s (url={url})")


def get(url: str, headers: dict | None = None, body=None, timeout: int = 30,
        stream: bool = False, max_size=None, fields=None, file=None) -> Response:
    return _request("GET", url, headers=headers, body=body, timeout=timeout,
                    stream=stream, max_size=max_size, fields=fields, file=file)


def post(url: str, headers: dict | None = None, body=None, timeout: int = 30,
         stream: bool = False, max_size=None, fields=None, file=None) -> Response:
    return _request("POST", url, headers=headers, body=body, timeout=timeout,
                    stream=stream, max_size=max_size, fields=fields, file=file)


def put(url: str, headers: dict | None = None, body=None, timeout: int = 30,
        stream: bool = False, max_size=None, fields=None, file=None) -> Response:
    return _request("PUT", url, headers=headers, body=body, timeout=timeout,
                    stream=stream, max_size=max_size, fields=fields, file=file)


def patch(url: str, headers: dict | None = None, body=None, timeout: int = 30,
          stream: bool = False, max_size=None, fields=None, file=None) -> Response:
    return _request("PATCH", url, headers=headers, body=body, timeout=timeout,
                    stream=stream, max_size=max_size, fields=fields, file=file)


def delete(url: str, headers: dict | None = None, body=None, timeout: int = 30,
           stream: bool = False, max_size=None, fields=None, file=None) -> Response:
    return _request("DELETE", url, headers=headers, body=body, timeout=timeout,
                    stream=stream, max_size=max_size, fields=fields, file=file)


def head(url: str, headers: dict | None = None, timeout: int = 30) -> Response:
    """HEAD — só os cabeçalhos: mesmo status/headers do GET, corpo vazio.
    Serve pra checar existência/tamanho/tipo sem baixar o conteúdo."""
    return _request("HEAD", url, headers=headers, timeout=timeout)


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
    "head": head,
    "ws_connect": ws_connect,
}
