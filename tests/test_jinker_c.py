"""Módulo `jinker` em C — mesmo comportamento observável que o `jinker_lib.py`.

O servidor HTTP/WebSocket é escrito à mão sobre socket + OpenSSL (`ps_jinker.c`),
sem http.server nem websockets. Como o servidor BLOQUEIA, o diferencial roda os
dois motores em PROCESSOS separados: o mesmo `app.ps` sobe pelo binário `pool` e
pelo interpretador (`python3 -m poolscript`), e as requisições batem nos dois. O
que o cliente HTTP observa — status, corpo, headers de CORS — tem que bater.
"""
import json
import os
import socket
import subprocess
import sys
import time
import urllib.request
import urllib.error
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


APP_TMPL = """\
from jinker import Jinker, cors, jsonify, render, JinkerResponse

app = Jinker(__name__)
cors(options=["GET", "POST", "PUT", "DELETE"])

@app.route("/ping", methods=["GET"])
action ping() {
    return jsonify({"ok": true, "msg": "pong"})
}

@app.route("/user/:id", methods=["GET"])
action user() {
    return jsonify({"user": request.path_param("id")})
}

@app.route("/busca", methods=["GET"])
action busca() {
    return {"q": request.get("q"), "page": request.get("page")}
}

@app.route("/echo", methods=["POST"])
action echo() {
    return {"recebido": request.get_json()}
}

@app.route("/texto", methods=["GET"])
action texto() {
    return "conteudo puro"
}

@app.route("/num", methods=["GET"])
action num() {
    return 12345
}

@app.route("/vazio", methods=["GET"])
action vazio() {
    x = 1
}

@app.route("/comstatus", methods=["POST"])
action comstatus() {
    return {"criado": true}, 201
}

@app.route("/resp", methods=["GET"])
action resp() {
    r = JinkerResponse()
    r.json({"via": "response"}, 202)
    r.header("X-Custom", "abc")
    return r
}

@app.route("/lista", methods=["GET"])
action lista() {
    return [1, 2, 3, "quatro"]
}

@app.route("/reqinfo", methods=["GET"])
action reqinfo() {
    return {
        "method": request.method,
        "path": request.path,
        "ua": request.header("user-agent"),
        "custom": request.header("X-Meu"),
        "faltante": request.header("X-Nao-Existe")
    }
}

run_selfwith_("main") {
    app(debug=false, host="127.0.0.1", port=__PORTA__)
}
"""


def sobe(cmd, porta, cwd=None, env=None):
    proc = subprocess.Popen(cmd, cwd=cwd, env=env,
                            stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    if not espera_subir(porta):
        proc.kill()
        raise RuntimeError(f"servidor não subiu: {cmd}")
    return proc


def req(porta, metodo, caminho, corpo=None, headers=None):
    url = f"http://127.0.0.1:{porta}{caminho}"
    data = corpo.encode() if isinstance(corpo, str) else corpo
    r = urllib.request.Request(url, data=data, method=metodo, headers=headers or {})
    try:
        with urllib.request.urlopen(r, timeout=4) as resp:
            body = resp.read().decode("utf-8", "replace")
            return resp.status, dict(resp.headers), body
    except urllib.error.HTTPError as e:
        body = e.read().decode("utf-8", "replace")
        return e.code, dict(e.headers), body


# headers de CORS/resposta que fazem parte do contrato observável
CORS_H = ("Access-Control-Allow-Origin", "Access-Control-Allow-Methods",
          "Access-Control-Allow-Headers", "Content-Type", "X-Custom",
          "Retry-After")


def compara(a, b):
    sa, ha, ba = a
    sb, hb, bb = b
    assert sa == sb, f"status {sa} != {sb}"
    # corpo: normaliza JSON pra ignorar espaçamento se ambos forem JSON
    try:
        assert json.loads(ba) == json.loads(bb), f"json {ba!r} != {bb!r}"
    except (json.JSONDecodeError, ValueError):
        assert ba == bb, f"corpo {ba!r} != {bb!r}"
    for h in CORS_H:
        assert ha.get(h) == hb.get(h), f"header {h}: {ha.get(h)!r} != {hb.get(h)!r}"


@pytest.fixture(scope="module")
def servidores(tmp_path_factory):
    d = tmp_path_factory.mktemp("jinker")
    pc, pi = porta_livre(), porta_livre()
    while pi == pc:
        pi = porta_livre()

    (d / "app_c.ps").write_text(APP_TMPL.replace("__PORTA__", str(pc)))
    (d / "app_i.ps").write_text(APP_TMPL.replace("__PORTA__", str(pi)))

    env = dict(os.environ, PYTHONPATH=str(SRC))
    proc_c = sobe([str(POOL), str(d / "app_c.ps")], pc)
    proc_i = sobe([sys.executable, "-m", "poolscript", str(d / "app_i.ps")], pi, env=env)
    try:
        yield pc, pi
    finally:
        for p in (proc_c, proc_i):
            p.terminate()
            try:
                p.wait(timeout=3)
            except subprocess.TimeoutExpired:
                p.kill()


CASOS = [
    ("GET", "/ping", None, None),
    ("GET", "/user/42", None, None),
    ("GET", "/user/abc-xyz", None, None),
    ("GET", "/busca?q=poolscript&page=2", None, None),
    ("GET", "/busca?q=so", None, None),
    ("POST", "/echo", '{"a": 1, "b": [2, 3], "c": {"d": true}}', None),
    ("POST", "/echo", 'nao-e-json', None),
    ("GET", "/texto", None, None),
    ("GET", "/num", None, None),
    ("GET", "/vazio", None, None),
    ("POST", "/comstatus", "{}", None),
    ("GET", "/resp", None, None),
    ("GET", "/lista", None, None),
    # request.method / request.path / request.header(nome) (case-insensitive)
    ("GET", "/reqinfo", None, {"X-Meu": "abc123"}),
    ("GET", "/nao-existe", None, None),
    ("DELETE", "/ping", None, None),          # método não registrado -> 404
    ("OPTIONS", "/ping", None, None),         # preflight
]


@pytest.mark.parametrize("metodo,caminho,corpo,headers", CASOS)
def test_paridade(servidores, metodo, caminho, corpo, headers):
    pc, pi = servidores
    compara(req(pc, metodo, caminho, corpo, headers),
            req(pi, metodo, caminho, corpo, headers))


# ── App 2: CORS com origins, upload e WebSocket ──────────────────────────────

APP2 = """\
from jinker import Jinker, cors, jsonify

app = Jinker(__name__)
cors(options=["GET", "POST"], origins=["https://meusite.com"])

@app.route("/api", methods=cors.options(), auth=cors.origins())
action api() {
    return jsonify({"ok": true})
}

@app.route("/up", methods=["POST"])
action up() {
    f = request.file("arq", allowed=[".txt", ".png"])
    if f == Null { return {"erro": "sem arquivo"}, 400 }
    return {"nome": f.name, "tam": f.size, "ct": f.content_type}
}

@app.socket("/sala/:id", channel=true) {
    action main() {
        msg = request.get_json()
        id = request.path_param("id")
        app.socket.emit(payload=msg, room_id=id, exclude_self=false)
    }
}

run_selfwith_("main") {
    app(debug=false, host="127.0.0.1", port=__PORTA__)
}
"""


@pytest.fixture(scope="module")
def servidores2(tmp_path_factory):
    d = tmp_path_factory.mktemp("jinker2")
    pc, pi = porta_livre(), porta_livre()
    # port+1 é o WS; garante que as 4 portas não colidem
    while len({pc, pc + 1, pi, pi + 1}) != 4:
        pi = porta_livre()

    (d / "a_c.ps").write_text(APP2.replace("__PORTA__", str(pc)))
    (d / "a_i.ps").write_text(APP2.replace("__PORTA__", str(pi)))
    env = dict(os.environ, PYTHONPATH=str(SRC))
    proc_c = sobe([str(POOL), str(d / "a_c.ps")], pc)
    proc_i = sobe([sys.executable, "-m", "poolscript", str(d / "a_i.ps")], pi, env=env)
    try:
        yield pc, pi
    finally:
        for p in (proc_c, proc_i):
            p.terminate()
            try:
                p.wait(timeout=3)
            except subprocess.TimeoutExpired:
                p.kill()


def test_cors_origem_permitida(servidores2):
    pc, pi = servidores2
    h = {"Origin": "https://meusite.com", "Sec-Fetch-Site": "cross-site"}
    compara(req(pc, "GET", "/api", None, h), req(pi, "GET", "/api", None, h))


def test_cors_origem_bloqueada(servidores2):
    pc, pi = servidores2
    h = {"Origin": "https://mau.com", "Sec-Fetch-Site": "cross-site"}
    a = req(pc, "GET", "/api", None, h)
    b = req(pi, "GET", "/api", None, h)
    assert a[0] == b[0] == 403
    compara(a, b)


def _multipart(campo, nome, ctype, dados):
    b = "----poolboundary"
    corpo = (f"--{b}\r\n"
             f'Content-Disposition: form-data; name="{campo}"; filename="{nome}"\r\n'
             f"Content-Type: {ctype}\r\n\r\n{dados}\r\n--{b}--\r\n")
    return corpo.encode(), {"Content-Type": f"multipart/form-data; boundary={b}"}


def test_upload_ok(servidores2):
    pc, pi = servidores2
    corpo, h = _multipart("arq", "doc.txt", "text/plain", "conteudo do arquivo")
    compara(req(pc, "POST", "/up", corpo, h), req(pi, "POST", "/up", corpo, h))


def test_upload_extensao_bloqueada(servidores2):
    pc, pi = servidores2
    corpo, h = _multipart("arq", "virus.exe", "application/octet-stream", "MZ")
    a = req(pc, "POST", "/up", corpo, h)
    b = req(pi, "POST", "/up", corpo, h)
    assert a[0] == b[0] == 500     # exceção no handler -> 500 nos dois


websockets = pytest.importorskip("websockets")


def test_websocket_salas(servidores2):
    import asyncio

    pc, pi = servidores2

    async def cenario(base_port):
        wsp = base_port + 1
        import websockets as W
        async with W.connect(f"ws://127.0.0.1:{wsp}/sala/7") as a, \
                   W.connect(f"ws://127.0.0.1:{wsp}/sala/7") as b, \
                   W.connect(f"ws://127.0.0.1:{wsp}/sala/9") as c:
            await a.send(json.dumps({"txt": "oi"}))
            saidas = {}
            for w, nm in [(a, "a"), (b, "b")]:
                try:
                    saidas[nm] = await asyncio.wait_for(w.recv(), 2.0)
                except asyncio.TimeoutError:
                    saidas[nm] = "TIMEOUT"
            try:
                saidas["c"] = await asyncio.wait_for(c.recv(), 0.5)
            except asyncio.TimeoutError:
                saidas["c"] = None
            return saidas

    rc = asyncio.run(cenario(pc))
    ri = asyncio.run(cenario(pi))
    assert rc == ri
    # exclude_self=false: remetente e o outro da sala 7 recebem; sala 9 não
    assert rc["a"] == rc["b"] == '{"txt": "oi"}'
    assert rc["c"] is None


WSCLI = """\
import request

action recebi(m) {
    post("recebi:", m["txt"])
}

conn = request.ws_connect("ws://127.0.0.1:__WSPORT__/sala/7")
conn.on_message(recebi)
conn.send({"txt": "eco"})
sleep(0.6)
conn.close()
post("fim")
"""


def test_ws_connect_cliente(servidores2, tmp_path):
    """O cliente `request.ws_connect` (agora real na VM) fala com o servidor
    jinker e o observável — mensagens entregues ao on_message — bate nos dois
    motores."""
    pc, pi = servidores2
    env = dict(os.environ, PYTHONPATH=str(SRC))
    saidas = []
    for porta, cmd_base in [(pc, [str(POOL)]),
                            (pi, [sys.executable, "-m", "poolscript"])]:
        cli = tmp_path / f"cli_{porta}.ps"
        cli.write_text(WSCLI.replace("__WSPORT__", str(porta + 1)))
        r = subprocess.run(cmd_base + [str(cli)], env=env,
                           capture_output=True, text=True, timeout=15)
        saidas.append(r.stdout)
    assert saidas[0] == saidas[1] == "recebi: eco\nfim\n"
