"""
Módulo `jinker` da PoolScript — servidor HTTP nativo.

Zero dependências externas (usa http.server + threading da stdlib do Python).

Uso em PoolScript:
    import jinker
    from jinker import Jinker, cors, jsonify

    server = Jinker(__name__)
    cors(options=['POST', 'GET'], permiser=["*/api", "allowed.all/Users-Agent"])

    @server.route("/api/hello", auth=cors.permiser(), methods=cors.options()) {
        action handler() {
            # código aqui
        }
    }

    run_selfwith_("main") {
        server(debug=True, host='0.0.0.0', port=2000)
    }

WebSocket com salas:
    from jinker import Jinker, cors

    sk = Jinker(__name__)
    cors(options=["POST"], origins=["https://nome.com"])

    # Cada conexão em /sala:id entra automaticamente na sala = valor de :id
    @sk.socket("/sala:id", channel=true) {
        action main() {
            msg = request.get_json()
            id = request.path_param('id')

            # com instância
            send = sk.socket()
            send.emit(payload=msg, room_id=id)
            state = send.status_send()

            # sem instância — equivalente
            sk.socket.emit(payload=msg, room_id=id)
        }
    }
"""
from __future__ import annotations

import json as _json
import os.path as _os_path
from typing import Any, Callable
from urllib.parse import urlparse, parse_qs

# heavy imports — só carregados quando o servidor for iniciado
def _get_server_classes():
    import threading
    from http.server import BaseHTTPRequestHandler, HTTPServer
    from socketserver import ThreadingMixIn

    class ThreadingHTTPServer(ThreadingMixIn, HTTPServer):
        """HTTPServer com suporte a múltiplas threads."""
        daemon_threads = True

    return BaseHTTPRequestHandler, ThreadingHTTPServer


# ── CORS global ────────────────────────────────────────────────────────

# Domínios locais — sempre permitidos automaticamente
_LOCAL_ORIGINS = {
    "localhost", "127.0.0.1", "127.1.0.1", "0.0.0.0", "::1",
}

def _is_local(origin: str) -> bool:
    """Verifica se a origem é um domínio local (auto-permitido)."""
    if not origin:
        return False
    try:
        parsed = urlparse(origin)
        host = parsed.hostname or origin.lower()
    except Exception:
        host = origin.lower()
    if host in _LOCAL_ORIGINS:
        return True
    # 127.x.x.x range
    if host.startswith("127."):
        return True
    return False


class CorsConfig:
    """Configuração global de CORS. Instância única por aplicação."""

    def __init__(self):
        self._options: list[str] = ["GET", "POST", "PUT", "PATCH", "DELETE"]
        self._origins: list[str] = []   # origens permitidas; vazio = permite tudo

    def __call__(self, options: list[str] | None = None,
                 origins: list[str] | None = None,
                 # legado — ignorado silenciosamente
                 permiser: list[str] | None = None) -> "CorsConfig":
        """cors(options=[...], origins=[...]) — define config global."""
        if options is not None:
            self._options = [m.upper() for m in options]
        if origins is not None:
            self._origins = [o.rstrip("/") for o in origins]
        return self

    def options(self, subset: list[str] | None = None) -> list[str]:
        """cors.options()           → todos os métodos configurados
           cors.options(["POST"])   → filtra/sobrescreve para a rota
        """
        if subset is not None:
            return [m.upper() for m in subset]
        return list(self._options)

    def origins(self) -> list[str]:
        """cors.origins() → lista de origens configuradas (usado em auth=)."""
        return list(self._origins)

    # ── verificação de origem ──────────────────────────────────────────

    def _origin_allowed(self, origin: str, allowed: list[str] | None,
                        sec_fetch_site: str = "") -> bool:
        """Verifica se a origem da requisição é permitida.

        Regras (em ordem):
        1. Sem lista configurada (allowed vazio/None) → permite tudo
        2. Cliente sem Origin e sem Sec-Fetch-Site → tool client (Insomnia/Postman) → permite
        3. Origem local (localhost, 127.x.x.x etc.) → sempre permite
        4. Origem está na lista → permite
        5. Caso contrário → bloqueia com 403
        """
        # sem restrição configurada
        if not allowed:
            return True
        # sem Origin e sem Sec-Fetch-Site = tool client, não browser
        if not origin and not sec_fetch_site:
            return True
        # origens locais são sempre permitidas
        if _is_local(origin):
            return True
        # verifica contra a lista
        origin_clean = (origin or "").rstrip("/")
        for allowed_origin in allowed:
            if allowed_origin.rstrip("/") == origin_clean:
                return True
        return False

    def _allowed_method(self, method: str) -> bool:
        return method.upper() in self._options

    # legado — mantido para não quebrar código antigo
    def permiser(self) -> list[str]:
        return list(self._origins)

    def _user_agent_allowed(self, *_) -> bool:
        return True


# Instância global — importada diretamente pelo usuário
cors = CorsConfig()


# ── Request / Response ─────────────────────────────────────────────────

# ── Extensões permitidas por categoria ───────────────────────────────────────
_SAFE_EXTENSIONS = {
    "image":    {".jpg", ".jpeg", ".png", ".gif", ".bmp", ".webp", ".svg"},
    "document": {".pdf", ".docx", ".xlsx", ".txt", ".csv", ".md"},
    "video":    {".mp4", ".webm", ".mov"},
    "audio":    {".mp3", ".wav", ".ogg"},
    "any":      None,   # None = permite tudo
}

# Extensões sempre bloqueadas — scripts e executáveis
_BLOCKED_EXTENSIONS = {
    ".exe", ".bat", ".sh", ".ps1", ".cmd", ".msi", ".dll",
    ".php", ".py", ".rb", ".js", ".ts", ".jar", ".vbs",
    ".scr", ".pif", ".com", ".reg", ".ws", ".wsf",
}


class PoolFileUpload:
    """Arquivo recebido via upload — vive em memória, sem path temporário."""

    def __init__(self, name: str, content_type: str, data: bytes):
        self.name         = name
        self.content_type = content_type
        self.size         = len(data)
        self.ext          = _os_path.splitext(name)[-1].lower()
        self._data        = data   # bytes em memória

    def save(self, destino: str) -> "PoolFileUpload":
        """Salva os bytes em disco no caminho especificado."""
        from pathlib import Path as _Path
        dest = _Path(destino)
        dest.parent.mkdir(parents=True, exist_ok=True)
        dest.write_bytes(self._data)
        return self

    def bytes(self) -> bytes:
        """Retorna os bytes brutos."""
        return self._data

    def move(self, destino: str) -> "PoolFileUpload":
        """Alias de save() — semântica de mover da memória pro disco."""
        return self.save(destino)

    def __repr__(self):
        return f"<PoolFileUpload '{self.name}' ({self.size} bytes)>"


def _parse_multipart(body: bytes, boundary: str) -> dict:
    """Parse simples de multipart/form-data.
    Retorna dict: {field_name: value_or_PoolFileUpload_or_list}
    """
    import re as _re
    result: dict = {}

    sep = ("--" + boundary).encode()
    parts = body.split(sep)

    for part in parts[1:]:  # pula o primeiro (vazio)
        if part.strip() in (b"", b"--", b"--\r\n"):
            continue
        # separa headers do corpo
        if b"\r\n\r\n" in part:
            raw_headers, _, raw_body = part.partition(b"\r\n\r\n")
        elif b"\n\n" in part:
            raw_headers, _, raw_body = part.partition(b"\n\n")
        else:
            continue

        # remove trailing boundary marker
        raw_body = raw_body.rstrip(b"\r\n--")

        # parse Content-Disposition
        headers_text = raw_headers.decode("utf-8", errors="replace")
        cd_match = _re.search(r'Content-Disposition:[^\n]+', headers_text, _re.IGNORECASE)
        if not cd_match:
            continue

        cd = cd_match.group()
        name_match = _re.search(r'name="([^"]+)"', cd)
        if not name_match:
            continue
        field_name = name_match.group(1)

        filename_match = _re.search(r'filename="([^"]*)"', cd)

        if filename_match:
            # é um arquivo
            filename = filename_match.group(1)
            ct_match = _re.search(r'Content-Type:\s*([^\r\n]+)', headers_text, _re.IGNORECASE)
            ct = ct_match.group(1).strip() if ct_match else "application/octet-stream"
            upload = PoolFileUpload(name=filename, content_type=ct, data=raw_body)
            # suporta múltiplos arquivos no mesmo campo
            if field_name in result:
                if isinstance(result[field_name], list):
                    result[field_name].append(upload)
                else:
                    result[field_name] = [result[field_name], upload]
            else:
                result[field_name] = upload
        else:
            # campo de texto
            result[field_name] = raw_body.decode("utf-8", errors="replace")

    return result


class JinkerRequest:
    """Objeto de requisição disponível dentro da action do handler."""

    def __init__(self, method: str, path: str, headers: dict,
                 body: bytes, query: dict):
        self.method = method
        self.path = path
        self.headers = headers
        self.body_raw = body
        self.query = query

    def json(self) -> Any:
        try:
            return _json.loads(self.body_raw.decode("utf-8"))
        except Exception:
            return None

    def text(self) -> str:
        return self.body_raw.decode("utf-8", errors="replace")

    def path_param(self, key: str) -> Any:
        """Retorna parâmetro dinâmico da URL. Ex: /user:<id> → request.path_param('id')"""
        return getattr(self, "_path_params", {}).get(key)

    def get(self, key: str) -> Any:
        """Pega do query string ou do body JSON."""
        if key in self.query:
            vals = self.query[key]
            return vals[0] if len(vals) == 1 else vals
        data = self.json()
        if isinstance(data, dict):
            return data.get(key)
        return None

    def _parsed_multipart(self) -> dict:
        ct = self.headers.get("Content-Type", "")
        if "multipart/form-data" not in ct:
            return {}
        boundary_match = __import__("re").search(r"boundary=([^;\s]+)", ct)
        if not boundary_match:
            return {}
        return _parse_multipart(self.body_raw, boundary_match.group(1))

    def file(self, field: str, allowed: list = None) -> "PoolFileUpload | None":
        """Retorna um único arquivo do upload.

        allowed: lista de extensões permitidas ex: [".jpg", ".png"]
                 None = aceita qualquer extensão segura
        """
        parts = self._parsed_multipart()
        upload = parts.get(field)
        if upload is None:
            return None
        # se vier lista, pega o primeiro
        if isinstance(upload, list):
            upload = upload[0]
        if not isinstance(upload, PoolFileUpload):
            return None
        # valida extensão
        if upload.ext in _BLOCKED_EXTENSIONS:
            raise ValueError(f"extensão bloqueada por segurança: {upload.ext}")
        if allowed is not None:
            allowed_lower = {e.lower() for e in allowed}
            if upload.ext not in allowed_lower:
                raise ValueError(
                    f"extensão '{upload.ext}' não permitida. Permitidas: {allowed}"
                )
        return upload

    def files(self, field: str, allowed: list = None) -> "list[PoolFileUpload]":
        """Retorna lista de arquivos do upload (múltiplos arquivos no mesmo campo)."""
        parts = self._parsed_multipart()
        uploads = parts.get(field)
        if uploads is None:
            return []
        if not isinstance(uploads, list):
            uploads = [uploads]
        result = []
        for upload in uploads:
            if not isinstance(upload, PoolFileUpload):
                continue
            if upload.ext in _BLOCKED_EXTENSIONS:
                raise ValueError(f"extensão bloqueada: {upload.ext}")
            if allowed is not None:
                allowed_lower = {e.lower() for e in allowed}
                if upload.ext not in allowed_lower:
                    raise ValueError(
                        f"extensão '{upload.ext}' não permitida. Permitidas: {allowed}"
                    )
            result.append(upload)
        return result


class JinkerResponse:
    """Objeto de resposta que a action pode montar e retornar."""

    def __init__(self):
        self.status_code = 200
        self._body: str = ""
        self._content_type = "text/plain; charset=utf-8"
        self._headers: dict[str, str] = {}

    def send(self, text: str, status: int = 200) -> "JinkerResponse":
        self.status_code = status
        self._body = str(text)
        self._content_type = "text/plain; charset=utf-8"
        return self

    def json(self, data: Any, status: int = 200) -> "JinkerResponse":
        self.status_code = status
        self._body = _json.dumps(data, ensure_ascii=False)
        self._content_type = "application/json; charset=utf-8"
        return self

    def status(self, code: int) -> "JinkerResponse":
        self.status_code = code
        return self

    def header(self, key: str, value: str) -> "JinkerResponse":
        self._headers[key] = value
        return self


def render(folder_or_file: str, file: str = None) -> "JinkerResponse":
    """Serve um arquivo estático com MIME type correto.

    Uso:
        render("dist", "index.html")          # serve dist/index.html
        render("dist", "assets/app.js")       # serve dist/assets/app.js
        render("templates/login.html")        # serve arquivo direto
    """
    import mimetypes as _mimetypes
    from pathlib import Path as _Path

    from .os_lib import _search_roots

    if file is not None:
        # folder + file (ex: render("static", nome_do_user)): confina o arquivo
        # DENTRO da pasta base. Bloqueia path traversal — tanto '../../etc/passwd'
        # quanto caminho absoluto '/etc/passwd' escapam e viram 404.
        # file_path só é setado se um arquivo VÁLIDO (dentro da base) for achado;
        # senão fica None -> 404 (nunca cai num is_file() relativo que reabriria
        # o traversal via CWD).
        base_rel = _Path(folder_or_file)
        file_path = None
        for root in _search_roots():
            base = (root / base_rel).resolve()
            candidate = (base / file).resolve()
            if candidate.is_relative_to(base) and candidate.is_file():
                file_path = candidate
                break
    else:
        # arquivo único (caminho fixo do dev) — resolve relativo ao script/cwd
        file_path = _Path(folder_or_file)
        if not file_path.is_absolute():
            for root in _search_roots():
                candidate = root / file_path
                if candidate.is_file():
                    file_path = candidate
                    break

    r = JinkerResponse()
    if file_path is None or not file_path.is_file():
        r.status_code = 404
        r._body = f"<h1>404 — arquivo não encontrado: {file_path}</h1>"
        r._content_type = "text/html; charset=utf-8"
        return r

    mime, _ = _mimetypes.guess_type(str(file_path))
    mime = mime or "application/octet-stream"
    is_binary = not mime.startswith("text") and "javascript" not in mime and "json" not in mime

    r.status_code = 200
    r._content_type = mime
    if is_binary:
        r._body_bytes = file_path.read_bytes()
        r._is_binary = True
    else:
        r._body = file_path.read_text(encoding="utf-8")
    return r


def jsonify(data: Any) -> JinkerResponse:
    """Atalho: jsonify(dict) → JinkerResponse com JSON."""
    r = JinkerResponse()
    r.json(data)
    return r


# ── Route ──────────────────────────────────────────────────────────────

class Route:
    """Uma rota registrada no Jinker."""

    def __init__(self, path: str, methods: list[str],
                 auth: list[str] | None, handler: Callable | None,
                 middleware: Callable | None = None):
        self.path = path
        self.methods = [m.upper() for m in methods]
        self.auth = auth
        self.handler = handler
        self.middleware = middleware


class MiddlewareRegistrar:
    """Registrador do middleware."""

    def __init__(self, app: "Jinker"):
        self._app = app

    def register(self, handler: Callable) -> None:
        self._app._middleware_handler = handler

    def __repr__(self):
        return "<MiddlewareRegistrar>"


class ChannelStatus:
    """Status de envio do channel."""
    def __init__(self, success: bool):
        self._success = success
        self.status = "Success" if success else "Error"

    def __eq__(self, other):
        if other == "Success":
            return self._success
        if other == "Error":
            return not self._success
        return False

    def __bool__(self):
        return self._success

    def __repr__(self):
        return self.status


class ChannelManager:
    """
    app.channel — gerencia conexões websocket abertas, com suporte a salas.

    Uma conexão entra automaticamente numa "sala" quando o path do socket tem
    parâmetro dinâmico (ex: /sala:id → sala = valor de :id).

    app.channel(forAll=msg)               → broadcast pra todos
    app.channel.emit(msg, room_id="123")  → manda só pra sala "123"
    app.channel.status                    → status do último envio
    """

    def __init__(self):
        self._connections: set = set()
        self._rooms: dict[str, set] = {}
        self._ws_room: dict = {}
        self.status = ChannelStatus(True)
        self._loop = None

    def _register(self, ws, room_id: Any = None) -> None:
        self._connections.add(ws)
        if room_id is not None:
            room_key = str(room_id)
            self._rooms.setdefault(room_key, set()).add(ws)
            self._ws_room[ws] = room_key

    def _unregister(self, ws) -> None:
        self._connections.discard(ws)
        room_key = self._ws_room.pop(ws, None)
        if room_key is not None:
            room = self._rooms.get(room_key)
            if room is not None:
                room.discard(ws)
                if not room:
                    del self._rooms[room_key]

    def _targets(self, room_id: Any = None) -> list:
        if room_id is None:
            return list(self._connections)
        return list(self._rooms.get(str(room_id), ()))

    def emit(self, payload: Any = None, room_id: Any = None, exclude: "set | None" = None) -> "ChannelStatus":
        """Envia payload pros conectados. room_id filtra pra uma sala específica;
        sem room_id, faz broadcast geral (igual __call__(forAll=...)).
        exclude: set de conexões a pular (usado pra não ecoar pro remetente)."""
        if payload is None:
            self.status = ChannelStatus(False)
            return self.status

        import asyncio
        msg = payload if isinstance(payload, str) else _json.dumps(payload, ensure_ascii=False)
        targets = self._targets(room_id)
        if exclude:
            targets = [t for t in targets if t not in exclude]

        try:
            sent = 0
            dead = set()
            for ws in targets:
                try:
                    if self._loop and not self._loop.is_closed():
                        asyncio.run_coroutine_threadsafe(ws.send(msg), self._loop)
                        sent += 1
                    else:
                        dead.add(ws)
                except Exception:
                    dead.add(ws)
            for ws in dead:
                self._unregister(ws)
            self.status = ChannelStatus(sent > 0 or len(targets) == 0)
        except Exception:
            self.status = ChannelStatus(False)

        return self.status

    def __call__(self, forAll: Any = None) -> "ChannelStatus":
        """Broadcast pra todos os conectados. Atalho de emit(payload, room_id=None)."""
        return self.emit(forAll, room_id=None)

    def __repr__(self):
        return f"<Channel connections={len(self._connections)} rooms={len(self._rooms)}>"


def _current_ws():
    """Websocket da conexão que está sendo processada agora (dentro de um
    handler de socket), se houver. Usado pra excluir o remetente do emit."""
    current = request._current
    return getattr(current, "_ws", None) if current is not None else None


class SocketEmitter:
    """Emissor obtido em runtime via `app.socket()` (sem args), dentro de um
    handler — usado pra mandar mensagens a uma sala específica ou broadcast.

    Por padrão, quem disparou o handler (o remetente) NÃO recebe o próprio
    emit de volta — evita a mensagem aparecer duplicada pro remetente.
    Passe exclude_self=false pra ecoar de volta também pro remetente.

    Uso:
        send = sk.socket()
        send.emit(payload=msg, room_id=identify["id"])
        state = send.status_send()
    """

    def __init__(self, app: "Jinker"):
        self._app = app

    def emit(self, payload: Any = None, room_id: Any = None, exclude_self: bool = True) -> "ChannelStatus":
        exclude = None
        if exclude_self:
            ws = _current_ws()
            if ws is not None:
                exclude = {ws}
        return self._app.channel.emit(payload, room_id=room_id, exclude=exclude)

    def status_send(self) -> "ChannelStatus":
        return self._app.channel.status

    def __repr__(self):
        return f"<SocketEmitter {self._app.name}>"


class _SocketRegistrar:
    """Intermediário do @app.socket(...)."""

    def __init__(self, app: "Jinker", path: str, channel: bool):
        self._app = app
        self._path = path
        self._channel = channel

    def register(self, handler: Callable) -> None:
        self._app._register_socket(self._path, self._channel, handler)

    def __repr__(self):
        return f"<SocketRegistrar {self._path}>"


class SocketNamespace:
    """app.socket — dupla função:

    1. Decorator de registro:  @sk.socket("/sala:id", channel=true)
    2. Emissor em runtime, com ou sem instância:
         send = sk.socket()               # instancia um emissor
         send.emit(payload=msg, room_id=identify["id"])
         state = send.status_send()

         sk.socket.emit(payload=msg, room_id=identify["id"])  # direto, sem instanciar
    """

    def __init__(self, app: "Jinker"):
        self._app = app

    def __call__(self, path: str = None, channel: bool = False):
        if path is not None:
            return _SocketRegistrar(self._app, path, channel)
        return SocketEmitter(self._app)

    def emit(self, payload: Any = None, room_id: Any = None, exclude_self: bool = True) -> "ChannelStatus":
        exclude = None
        if exclude_self:
            ws = _current_ws()
            if ws is not None:
                exclude = {ws}
        return self._app.channel.emit(payload, room_id=room_id, exclude=exclude)

    def status_send(self) -> "ChannelStatus":
        return self._app.channel.status

    def __repr__(self):
        return f"<SocketNamespace {self._app.name}>"


def _start_ws_server(app: "Jinker", host: str, port: int, debug: bool) -> None:
    """Inicia servidor websocket em thread separada."""
    try:
        import asyncio
        import threading
        import websockets
    except ImportError:
        print("[jinker] Error: instale websockets — pip install websockets")
        return

    channel = app.channel

    async def ws_handler(websocket):
        path = websocket.request.path if hasattr(websocket, 'request') else getattr(websocket, 'path', '/')
        # Encontra o socket registrado pra esse path — suporta path params
        socket_cfg, path_params = app._find_socket(path)
        if socket_cfg is None:
            await websocket.close(1008, "path não encontrado")
            return

        # Sala automática = valor do(s) parâmetro(s) dinâmico(s) do path.
        # Ex: /sala:id conectado em /sala/42 → entra na sala "42"
        room_id = None
        if path_params:
            if len(path_params) == 1:
                room_id = next(iter(path_params.values()))
            else:
                room_id = "|".join(f"{k}={v}" for k, v in sorted(path_params.items()))

        if socket_cfg["channel"]:
            channel._register(websocket, room_id=room_id)
            if debug:
                room_info = f" sala={room_id}" if room_id is not None else ""
                print(f"[jinker-ws] cliente conectou: {path}{room_info} ({len(channel._connections)} total)")

        try:
            async for raw_msg in websocket:
                # Parseia a mensagem
                try:
                    msg_data = _json.loads(raw_msg)
                except Exception:
                    msg_data = raw_msg

                # Injeta o proxy do request com os dados da mensagem
                from .jinker_lib import request as req_proxy

                class WsRequest:
                    _ws = websocket  # conexão que enviou esta mensagem — usado por exclude_self
                    def get_json(self):
                        return msg_data
                    def json(self):
                        return msg_data
                    def get(self, key: str):
                        if isinstance(msg_data, dict):
                            return msg_data.get(key)
                        return None
                    def path_param(self, key: str):
                        """Igual JinkerRequest.path_param — parâmetro dinâmico
                        do path do socket, ex: /chat:<sala> → path_param('sala')"""
                        return path_params.get(key)
                    @property
                    def headers(self):
                        return {}
                    @property
                    def method(self):
                        return "WS"
                    @property
                    def path(self):
                        return socket_cfg["path"]
                    def text(self):
                        return str(msg_data)

                req_proxy._current = WsRequest()

                # Executa o handler
                try:
                    socket_cfg["handler"]()
                except Exception as e:
                    if debug:
                        print(f"[jinker-ws] erro no handler: {e}")

        except Exception:
            pass
        finally:
            if socket_cfg["channel"]:
                channel._unregister(websocket)
                if debug:
                    print(f"[jinker-ws] cliente desconectou ({len(channel._connections)} total)")

    async def run():
        loop = asyncio.get_event_loop()
        channel._loop = loop
        async with websockets.serve(ws_handler, host, port):
            await asyncio.Future()

    def thread_target():
        asyncio.run(run())

    t = threading.Thread(target=thread_target, daemon=True)
    t.start()


# ── Jinker (app principal) ─────────────────────────────────────────────
import time as _time
import threading as _threading
import ipaddress as _ipaddress

# ── PoolIp — Rate limit + Ban ─────────────────────────────────────────────────

class PoolIp:
    """Gerencia rate limit e ban de IPs por instância Jinker."""

    def __init__(self, rate: int = 100, window: int = 60, bloq: int = 1):
        self.rate   = rate           # max req por janela
        self.window = window         # janela em segundos
        self.bloq   = bloq           # dias de ban
        self._hits: dict  = {}       # {ip: [timestamps]}
        self._banned: dict = {}      # {ip: ban_until_timestamp}
        self._lock = _threading.Lock()

    def check(self, ip: str) -> tuple:
        """Verifica se o IP pode fazer requisição.
        Retorna (allowed: bool, reason: str)
        """
        now = _time.time()
        with self._lock:
            # verifica ban
            if ip in self._banned:
                until = self._banned[ip]
                if now < until:
                    remaining = int((until - now) / 86400)
                    return False, f"IP bloqueado por {remaining} dia(s)"
                else:
                    del self._banned[ip]

            # registra hit
            if ip not in self._hits:
                self._hits[ip] = []

            # limpa hits fora da janela
            self._hits[ip] = [t for t in self._hits[ip] if now - t < self.window]
            self._hits[ip].append(now)

            count = len(self._hits[ip])

            # ultrapassou rate — ban imediato
            if count > self.rate:
                ban_until = now + (self.bloq * 86400)
                self._banned[ip] = ban_until
                self._hits[ip] = []
                return False, f"Rate limit excedido — IP banido por {self.bloq} dia(s)"

            return True, "ok"

    def is_banned(self, ip: str) -> bool:
        now = _time.time()
        with self._lock:
            if ip in self._banned:
                if now < self._banned[ip]:
                    return True
                del self._banned[ip]
        return False

    def unban(self, ip: str):
        with self._lock:
            self._banned.pop(ip, None)

    def banned_list(self) -> list:
        now = _time.time()
        with self._lock:
            return [
                {"ip": ip, "until": int(until - now)}
                for ip, until in self._banned.items()
                if now < until
            ]


# ── TLS — Certificado ─────────────────────────────────────────────────────────

def _setup_tls(cert_path: str = None) -> tuple:
    """Configura TLS. Retorna (ssl_context, cert_file, key_file).
    
    Ordem:
    1. cert_path explícito
    2. .jinkerTls no diretório do projeto (busca recursiva)
    3. Gera self-signed automaticamente
    """
    import ssl
    import subprocess
    from pathlib import Path

    RED   = "\033[91m"
    RESET = "\033[0m"
    YELLOW = "\033[93m"

    cert_file = None
    key_file  = None

    # 1. cert_path explícito
    if cert_path:
        p = Path(cert_path)
        if p.is_file():
            cert_file = str(p)
            # tenta achar key no mesmo dir
            key_candidate = p.parent / (p.stem + ".key")
            if key_candidate.is_file():
                key_file = str(key_candidate)

    # 2. busca .jinkerTls
    if not cert_file:
        for found in Path.cwd().rglob(".jinkerTls"):
            cert_file = str(found)
            key_candidate = found.parent / ".jinkerTls.key"
            if key_candidate.is_file():
                key_file = str(key_candidate)
            break

    # 3. self-signed
    if not cert_file:
        print(f"{RED}[Jinker:warn] Certificado TLS não encontrado.{RESET}")
        print(f"{YELLOW}[Jinker:warn] Gerando certificado self-signed para uso temporário.{RESET}")
        print(f"{YELLOW}[Jinker:warn] Para produção, forneça um certificado em .jinkerTls{RESET}")

        tmp_cert = Path.cwd() / ".jinkerTls"
        tmp_key  = Path.cwd() / ".jinkerTls.key"

        try:
            # -addext subjectAltName: sem SAN o cert é REJEITADO por cliente
            # moderno (browser, curl, urllib) mesmo depois de confiar na CA —
            # CN sozinho não vale mais. CA:TRUE permite adicioná-lo ao trust.
            subprocess.run([
                "openssl", "req", "-x509", "-newkey", "rsa:2048",
                "-keyout", str(tmp_key),
                "-out", str(tmp_cert),
                "-days", "365",
                "-nodes",
                "-subj", "/CN=localhost",
                "-addext", "subjectAltName=DNS:localhost,IP:127.0.0.1,IP:::1",
                "-addext", "basicConstraints=critical,CA:TRUE",
            ], check=True, capture_output=True)

            cert_file = str(tmp_cert)
            key_file  = str(tmp_key)
            print(f"{YELLOW}[Jinker:warn] Certificado self-signed gerado em .jinkerTls{RESET}")
            print(f"{YELLOW}[Jinker:warn] Providencie um certificado real (ex: Let\'s Encrypt){RESET}")
        except Exception as e:
            print(f"{RED}[Jinker:erro] Falha ao gerar certificado: {e}{RESET}")
            return None, None, None

    ctx = ssl.SSLContext(ssl.PROTOCOL_TLS_SERVER)
    ctx.load_cert_chain(certfile=cert_file, keyfile=key_file)
    return ctx, cert_file, key_file



def _build_acao_origin(origin: str, allowed: list[str] | None) -> str:
    """Retorna o valor correto para Access-Control-Allow-Origin.
    
    Se a origem está na lista de permitidas, ecoa ela (necessário para credenciais).
    Caso contrário retorna *.
    """
    if not allowed:
        return "*"
    if _is_local(origin):
        return origin or "*"
    origin_clean = (origin or "").rstrip("/")
    for o in allowed:
        if o.rstrip("/") == origin_clean:
            return origin
    # origem NÃO permitida: nunca devolver "*" (isso liberava geral e tornava a
    # allowlist decorativa). Devolve uma origem permitida qualquer — que não bate
    # com a do atacante, então o browser BLOQUEIA a resposta cross-origin.
    return allowed[0]


class Jinker:
    """
    Aplicação HTTP da PoolScript.

    Uso:
        server = Jinker(__name__)
    """

    def __init__(self, name: str = "__main__", oauth: dict = None,
                 static_folder: str = None, static_url: str = "/"):
        self.name = name
        self._routes: list[Route] = []
        self._debug = False
        self._middleware_handler: Callable | None = None
        self._sockets: list[dict] = []
        self.channel = ChannelManager()
        self.socket = SocketNamespace(self)
        # SPA / static
        self.static_folder = static_folder   # ex: "frontend/dist"
        self.static_url    = static_url      # ex: "/"
        # oauth config
        self._oauth   = oauth or {}
        self._poolip  = PoolIp(
            rate=self._oauth.get("rate", 100),
            window=60,
            bloq=self._oauth.get("bloq", 1),
        ) if self._oauth.get("poolip") else None
        self._use_tls = bool(self._oauth.get("tls"))
        self._cert    = self._oauth.get("cert", None)

    @property
    def middleware(self):
        return self._middleware_handler

    def _middleware_decorator(self):
        return MiddlewareRegistrar(self)

    def route(self, path: str, methods: list[str] | None = None,
              auth: list[str] | None = None, middleware=None):
        resolved_methods = methods if methods is not None else cors.options()
        resolved_auth = auth if auth is not None else cors.permiser()
        # middleware pode ser None, uma função, ou o próprio app (resolve depois)
        return _RouteRegistrar(self, path, resolved_methods, resolved_auth, middleware)

    def _register_socket(self, path: str, channel: bool, handler: Callable) -> None:
        self._sockets.append({"path": path, "channel": channel, "handler": handler})
        if self._debug:
            print(f"[jinker] socket registrado: {path} (channel={channel})")

    def _find_socket(self, path: str) -> "tuple[dict, dict] | tuple[None, None]":
        """Retorna (socket_cfg, path_params) ou (None, None).
        Usa EXATAMENTE as mesmas convenções de _find_route, para consistência:
          - /chat:sala       → (?P<sala>[^/]+)   (sem barra de separação)
          - /chat/<sala>     → /(?P<sala>.+)     (estilo Flask, recomendado)
        """
        import re as _re
        for socket_cfg in self._sockets:
            spath = socket_cfg["path"]
            # rota exata
            if spath == path:
                return socket_cfg, {}
            pattern = spath
            # :param (sem <>) — mesmo regex de _find_route
            params_found = _re.findall(r":([A-Za-z_][A-Za-z0-9_]*)", spath)
            for p in params_found:
                pattern = pattern.replace(f":{p}", f"(?P<{p}>[^/]+)")
            # /<param> catch-all — estilo Flask
            params_found2 = _re.findall(r"/<([A-Za-z_][A-Za-z0-9_]*)>", spath)
            for p in params_found2:
                pattern = pattern.replace(f"/<{p}>", f"/(?P<{p}>.+)")
            m = _re.fullmatch(pattern, path)
            if m:
                return socket_cfg, m.groupdict()
        return None, None

    def _register_route(self, path: str, methods: list[str],
                        auth: list[str] | None, handler: Callable,
                        middleware: Callable | None = None) -> None:
        self._routes.append(Route(path, methods, auth, handler, middleware))
        if self._debug:
            print(f"[jinker] rota registrada: {methods} {path}")

    def _find_route(self, method: str, path: str) -> "tuple[Route, dict] | tuple[None, None]":
        """Retorna (route, path_params) ou (None, None).
        Suporta parâmetros dinâmicos: /user:<id> → id="123"
        """
        import re as _re
        for route in self._routes:
            if method.upper() not in route.methods:
                continue
            # rota exata
            if route.path == path:
                return route, {}
            # rota com parâmetros: /user:<id> → /user/123
            # converte :<param> em grupo nomeado regex
            rpath = route.path
            import re as _re2
            params_found = _re2.findall(r":([A-Za-z_][A-Za-z0-9_]*)", rpath)
            pattern = rpath
            for p in params_found:
                pattern = pattern.replace(f":{p}", f"(?P<{p}>[^/]+)")
            # /<param> catch-all
            params_found2 = _re2.findall(r"/<([A-Za-z_][A-Za-z0-9_]*)>", route.path)
            for p in params_found2:
                pattern = pattern.replace(f"/<{p}>", f"/(?P<{p}>.+)")
            m = _re.fullmatch(pattern, path)
            if m:
                return route, m.groupdict()
        return None, None

    def __call__(self, debug: bool = False, host: str = "127.0.0.1",
                 port: int = 2000, reload: bool = False, workers: int = 1) -> None:
        """Inicia o servidor. Chamado no run_selfwith_.

        host padrão é 127.0.0.1 (só a própria máquina) — mais seguro. Para expor
        na rede/LAN, passe host="0.0.0.0" EXPLICITAMENTE, ciente do risco.

        `workers` é a paralelização multi-processo do binário `pool` (fork de N
        workers pra usar todos os núcleos). O interpretador é o runtime de
        referência/dev e roda sempre em UM processo — aceita `workers` só pra o
        mesmo `.ps` rodar nos dois motores sem mudar."""
        self._debug = debug
        app = self

        # Inicia websocket server em thread separada se tiver sockets
        if self._sockets:
            ws_port = port + 1
            _start_ws_server(app, host, ws_port, debug)
            print(f"[jinker] websocket rodando em ws://{host}:{ws_port}")

        BaseHTTPRequestHandler, _ = _get_server_classes()
        class Handler(BaseHTTPRequestHandler):
            # HTTP/1.1 → conexões persistentes (keep-alive): não abre um TCP
            # novo a cada request. Seguro aqui porque TODA resposta com corpo
            # manda Content-Length (o cliente sabe onde o corpo termina).
            protocol_version = "HTTP/1.1"
            # TCP_NODELAY: desliga o Nagle — corta os atrasos de dezenas/centenas
            # de ms (às vezes segundos) que aparecem em respostas pequenas.
            disable_nagle_algorithm = True
            # fecha conexão keep-alive ociosa após 30s (não segura thread eterna)
            timeout = 30
            # Suprime headers que revelam tecnologia
            server_version = ""
            sys_version    = ""

            def version_string(self):
                return "Jinker"

            def log_message(self, format, *args):
                if debug:
                    super().log_message(format, *args)
            def log_message(self, fmt, *args):
                if app._debug:
                    print(f"[jinker] {self.address_string()} - {fmt % args}")

            def _handle(self):
                import mimetypes
                from pathlib import Path as _Path

                # ── PoolIp check ──────────────────────────────────────
                if _poolip is not None:
                    client_ip = self.client_address[0]
                    allowed, reason = _poolip.check(client_ip)
                    if not allowed:
                        body = _json.dumps({
                            "error": True,
                            "code": 429,
                            "message": reason
                        }, ensure_ascii=False).encode("utf-8")
                        self.send_response(429)
                        self.send_header("Content-Type", "application/json; charset=utf-8")
                        self.send_header("Content-Length", str(len(body)))
                        self.send_header("Retry-After", str(_poolip.bloq * 86400))
                        self.end_headers()
                        self.wfile.write(body)
                        return

                parsed = urlparse(self.path)
                path = parsed.path
                query = parse_qs(parsed.query)
                method = self.command
                headers = dict(self.headers)
                user_agent = headers.get("User-Agent", "")

                length = int(self.headers.get("Content-Length", 0))
                body = self.rfile.read(length) if length > 0 else b""

                # Extrai origem da requisição (usado em auth e CORS headers)
                origin = headers.get("Origin", headers.get("Referer", ""))

                # Resposta de erro padrão
                def send_error_response(code: int, msg: str):
                    body_bytes = _json.dumps({"error": True, "code": code, "message": msg}, ensure_ascii=False).encode("utf-8")
                    self.send_response(code)
                    self.send_header("Content-Type", "application/json; charset=utf-8")
                    self.send_header("Content-Length", str(len(body_bytes)))
                    self.send_header("Access-Control-Allow-Origin", "*")
                    self.end_headers()
                    self.wfile.write(body_bytes)

                # ── Tier 1: Rotas de API (maior prioridade) ──────────────
                route, path_params = app._find_route(method, path)
                path_params = path_params or {}

                # ── Tier 2: Arquivos físicos em /static/ ─────────────────
                if route is None and path.startswith("/static/"):
                    static_file = _Path.cwd() / path.lstrip("/")
                    if static_file.is_file():
                        mime, _ = mimetypes.guess_type(str(static_file))
                        mime = mime or "application/octet-stream"
                        data = static_file.read_bytes()
                        self.send_response(200)
                        self.send_header("Content-Type", mime)
                        self.send_header("Content-Length", str(len(data)))
                        self.send_header("Access-Control-Allow-Origin", _build_acao_origin(origin, None))
                        self.end_headers()
                        self.wfile.write(data)
                    else:
                        send_error_response(404, f"arquivo não encontrado: {path}")
                    return

                # ── Tier 2b: SPA static_folder — arquivos físicos do dist ─
                if route is None and app.static_folder:
                    from .os_lib import _search_roots
                    from pathlib import Path as _SPath
                    static_root = None
                    for root in _search_roots():
                        candidate = root / app.static_folder
                        if candidate.is_dir():
                            static_root = candidate
                            break

                    if static_root:
                        file_path = static_root / path.lstrip("/")
                        if file_path.is_file():
                            # arquivo físico existe — serve direto
                            mime, _ = mimetypes.guess_type(str(file_path))
                            mime = mime or "application/octet-stream"
                            is_text = mime.startswith("text") or "javascript" in mime or "json" in mime
                            data = file_path.read_bytes()
                            self.send_response(200)
                            self.send_header("Content-Type", mime)
                            self.send_header("Content-Length", str(len(data)))
                            self.send_header("Access-Control-Allow-Origin", _build_acao_origin(origin, None))
                            self.end_headers()
                            self.wfile.write(data)
                            return

                        # ── Tier 3: SPA fallback — serve index.html ───────
                        index = static_root / "index.html"
                        if index.is_file():
                            data = index.read_bytes()
                            self.send_response(200)
                            self.send_header("Content-Type", "text/html; charset=utf-8")
                            self.send_header("Content-Length", str(len(data)))
                            self.send_header("Access-Control-Allow-Origin", _build_acao_origin(origin, None))
                            self.end_headers()
                            self.wfile.write(data)
                            return

                if route is None:
                    send_error_response(404, f"rota não encontrada: {method} {path}")
                    return

                # Verifica origem da requisição (substitui User-Agent)
                sec_fetch_site = headers.get("Sec-Fetch-Site", "")
                if not cors._origin_allowed(origin, route.auth, sec_fetch_site):
                    send_error_response(403, f"Origem não autorizada: {origin or '(sem origin)'}")
                    return

                req = JinkerRequest(method, path, headers, body, query)
                res = JinkerResponse()

                try:
                    # Resolve middleware na hora da requisição
                    mw = route.middleware
                    if mw is None and app._middleware_handler is not None:
                        mw = None  # só usa se explicitamente passado
                    if mw is not None and callable(mw):
                        mw_result = mw(req, res)
                        if isinstance(mw_result, JinkerResponse):
                            body_bytes = mw_result._body.encode("utf-8")
                            self.send_response(mw_result.status_code)
                            self.send_header("Content-Type", mw_result._content_type)
                            self.send_header("Content-Length", str(len(body_bytes)))
                            self.send_header("Access-Control-Allow-Origin", "*")
                            self.send_header("Access-Control-Allow-Methods", "GET, POST, PUT, PATCH, DELETE, OPTIONS")
                            self.send_header("Access-Control-Allow-Headers", "Content-Type, Authorization")
                            self.end_headers()
                            self.wfile.write(body_bytes)
                            return

                    # injeta path_params no escopo da rota
                    if path_params:
                        for _k, _v in path_params.items():
                            req._path_params = path_params
                    result = route.handler(req, res)
                    if result is None:
                        # return None/Null — Jinker infere erro pelo contexto
                        body_bytes = _json.dumps({
                            "error": True,
                            "code": 400,
                            "message": "Requisição inválida ou sem dados"
                        }, ensure_ascii=False).encode("utf-8")
                        self.send_response(400)
                        self.send_header("Content-Type", "application/json; charset=utf-8")
                        self.send_header("Content-Length", str(len(body_bytes)))
                        self.send_header("Access-Control-Allow-Origin", _build_acao_origin(origin, route.auth))
                        self.end_headers()
                        self.wfile.write(body_bytes)
                        return
                    if isinstance(result, JinkerResponse):
                        res = result
                    elif isinstance(result, dict):
                        res.json(result)
                    elif isinstance(result, str):
                        res.send(result)
                except Exception as exc:
                    import traceback
                    # sempre loga no terminal — nunca expõe pro browser
                    traceback.print_exc()
                    send_error_response(500, "Erro interno do servidor")
                    return

                body_bytes = getattr(res, "_body_bytes", None) or res._body.encode("utf-8")
                self.send_response(res.status_code)
                self.send_header("Content-Type", res._content_type)
                self.send_header("Content-Length", str(len(body_bytes)))
                self.send_header("Access-Control-Allow-Origin", _build_acao_origin(origin, route.auth))
                self.send_header("Access-Control-Allow-Methods", ", ".join(route.methods))
                self.send_header("Access-Control-Allow-Headers", "Content-Type, Authorization")
                for k, v in res._headers.items():
                    self.send_header(k, v)
                self.end_headers()
                self.wfile.write(body_bytes)

            # Mapeia todos os métodos HTTP para _handle
            def do_GET(self): self._handle()
            def do_POST(self): self._handle()
            def do_PUT(self): self._handle()
            def do_PATCH(self): self._handle()
            def do_DELETE(self): self._handle()
            def do_OPTIONS(self):
                self.send_response(204)
                self.send_header("Access-Control-Allow-Origin", "*")
                self.send_header("Access-Control-Allow-Methods",
                                 ", ".join(cors.options()))
                self.send_header("Access-Control-Allow-Headers",
                                 "Content-Type, Authorization")
                self.end_headers()

        # ── PoolIp — injeta no Handler ────────────────────────────────
        _poolip = self._poolip

        # ── TLS ────────────────────────────────────────────────────────
        _ssl_ctx = None
        protocol = "http"
        if self._use_tls:
            _ssl_ctx, _, _ = _setup_tls(self._cert)
            if _ssl_ctx:
                protocol = "https"

        _, ThreadingHTTPServer = _get_server_classes()
        server = ThreadingHTTPServer((host, port), Handler)
        # threads de conexão morrem junto com o processo (sem travar o shutdown)
        server.daemon_threads = True

        if _ssl_ctx:
            import ssl as _ssl_mod
            server.socket = _ssl_ctx.wrap_socket(server.socket, server_side=True)

        print(f"[jinker] servidor rodando em {protocol}://{host}:{port}")
        if self._poolip:
            print(f"[jinker] PoolIp ativo — rate: {self._poolip.rate} req/min, ban: {self._poolip.bloq} dia(s)")
        if debug:
            print(f"[jinker] modo debug ativado")
        if reload:
            import sys
            import os
            import signal
            # o .ps de entrada — NÃO `sys.argv[0]` (que é o entry do interpretador
            # e nunca muda: era esse o bug que fazia o reload nunca disparar).
            from .os_lib import _SCRIPT_FILE
            watched_file = _SCRIPT_FILE or (sys.argv[0] if sys.argv else None)
            print(f"[jinker] auto-reload ativado (vigiando {watched_file})")
            last_mtime = os.path.getmtime(watched_file) if watched_file and os.path.isfile(watched_file) else None

            def _watch_reload():
                import time
                while True:
                    time.sleep(1)
                    try:
                        if watched_file and os.path.isfile(watched_file):
                            mtime = os.path.getmtime(watched_file)
                            if last_mtime and mtime != last_mtime:
                                print(f"\n[jinker] arquivo alterado — recarregando...")
                                server.shutdown()
                                os.execv(sys.executable, [sys.executable] + sys.argv)
                    except Exception:
                        pass

            t = threading.Thread(target=_watch_reload, daemon=True)
            t.start()

        try:
            server.serve_forever()
        except KeyboardInterrupt:
            print("\n[jinker] servidor encerrado")
            server.server_close()


class _RouteRegistrar:
    """
    Intermediário entre @server.route(...) e a action registrada.
    O interpretador chama .register(handler) após executar o bloco.
    """

    def __init__(self, app: Jinker, path: str,
                 methods: list[str], auth: list[str] | None,
                 middleware: Callable | None = None):
        self._app = app
        self._path = path
        self._methods = methods
        self._auth = auth
        self._middleware = middleware

    def register(self, handler: Callable) -> None:
        self._app._register_route(self._path, self._methods, self._auth, handler, self._middleware)

    def __repr__(self):
        return f"<RouteRegistrar {self._methods} {self._path}>"


class RequestProxy:
    """
    Proxy do `request` para uso com `import request` na PoolScript.
    Os métodos delegam pro JinkerRequest atual injetado no escopo pela rota.
    O interpretador injeta o objeto real em _current antes de chamar a action.

    Usa threading.local() para garantir isolamento entre requisições concorrentes:
    cada thread tem seu próprio _current, evitando race condition quando rotas
    fazem chamadas internas (ex: webhook que chama outra rota do mesmo servidor).
    """
    def __init__(self):
        self._local = __import__("threading").local()

    @property
    def _current(self) -> "JinkerRequest | None":
        return getattr(self._local, "current", None)

    @_current.setter
    def _current(self, value):
        self._local.current = value

    def _set(self, req: JinkerRequest):
        self._local.current = req

    def get_json(self):
        if self._current is None:
            return None
        return self._current.json()

    def json(self):
        """Alias de get_json() — o corpo do POST como dict. O JinkerRequest usa
        `.json()`, então o proxy aceita os dois nomes (request.json() e
        request.get_json()) pra não pegar ninguém de surpresa."""
        return self.get_json()

    def get(self, key: str):
        if self._current is None:
            return None
        return self._current.get(key)

    @property
    def method(self):
        return self._current.method if self._current else None

    @property
    def path(self):
        return self._current.path if self._current else None

    @property
    def headers(self):
        return self._current.headers if self._current else {}

    def header(self, key: str):
        """request.header('User-Agent') → valor do header (case-insensitive),
        ou Null se não existir. Atalho pra request.headers sem precisar do dict
        inteiro nem se preocupar com maiúsculas/minúsculas."""
        if self._current is None:
            return None
        h = self._current.headers or {}
        kl = key.lower()
        for k, v in h.items():
            if k.lower() == kl:
                return v
        return None

    def text(self):
        return self._current.text() if self._current else None

    def path_param(self, key: str):
        """request.path_param('id') → valor do parâmetro dinâmico da rota/socket.
        Ex: /sala:id ou /user/<id>"""
        if self._current is None:
            return None
        return self._current.path_param(key)

    def file(self, field: str, allowed: list = None):
        """request.file('campo', allowed=['.jpg', '.png']) → PoolFileUpload"""
        if self._current is None:
            return None
        return self._current.file(field, allowed=allowed)

    def files(self, field: str, allowed: list = None):
        """request.files('campo') → lista de PoolFileUpload"""
        if self._current is None:
            return []
        return self._current.files(field, allowed=allowed)

    def __repr__(self):
        return "<jinker.request>"


# Instância global do proxy — importada com `import request`
request = RequestProxy()


EXPORTS = {
    "Jinker": Jinker,
    "cors": cors,
    "jsonify": jsonify,
    "render": render,
    "JinkerRequest": JinkerRequest,
    "JinkerResponse": JinkerResponse,
    "request": request,
}
