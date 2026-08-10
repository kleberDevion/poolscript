"""Servidor LSP — testado pelo PROTOCOLO, como um editor de verdade.

Sobe `poolscript.lsp.server` num subprocesso e fala JSON-RPC/stdio com ele
(Content-Length framing, igual VS Code/IntelliJ fazem). Os cenários espelham
os testes da extensão VS Code: cadeia type-aware de DB, PoolFile, os 3
`request`, self., argumento nomeado, from-import sem builtin vazando,
não-usado apagado e erro de sintaxe do parser real.
"""
import json
import os
import subprocess
import sys
from pathlib import Path

import pytest

pytest.importorskip("pygls", reason="pygls não instalado — pip install '.[lsp]'")

RAIZ = Path(__file__).resolve().parent.parent
NL = chr(10)


class ClienteLSP:
    """Cliente mínimo: manda requests/notifications e coleta respostas."""

    def __init__(self, workspace: Path):
        env = dict(os.environ)
        env["PYTHONPATH"] = str(RAIZ / "src")
        self.proc = subprocess.Popen(
            [sys.executable, "-m", "poolscript.lsp.server"],
            stdin=subprocess.PIPE, stdout=subprocess.PIPE,
            stderr=subprocess.DEVNULL, env=env,
        )
        self._id = 0
        self.notificacoes = []
        self._responde("initialize", {
            "processId": None,
            "rootUri": workspace.as_uri(),
            "capabilities": {},
            "workspaceFolders": [{"uri": workspace.as_uri(), "name": "ws"}],
        })
        self._notifica("initialized", {})

    # ── protocolo ─────────────────────────────────────────────────────

    def _envia(self, payload: dict):
        corpo = json.dumps(payload).encode("utf-8")
        self.proc.stdin.write(b"Content-Length: %d\r\n\r\n" % len(corpo) + corpo)
        self.proc.stdin.flush()

    def _le_mensagem(self) -> dict:
        headers = {}
        while True:
            linha = self.proc.stdout.readline()
            if not linha:
                raise RuntimeError("servidor fechou o stdout")
            linha = linha.strip()
            if not linha:
                break
            k, _, v = linha.partition(b":")
            headers[k.strip().lower()] = v.strip()
        tam = int(headers[b"content-length"])
        return json.loads(self.proc.stdout.read(tam).decode("utf-8"))

    def _notifica(self, metodo: str, params: dict):
        self._envia({"jsonrpc": "2.0", "method": metodo, "params": params})

    def _responde(self, metodo: str, params: dict):
        self._id += 1
        rid = self._id
        self._envia({"jsonrpc": "2.0", "id": rid, "method": metodo, "params": params})
        while True:
            msg = self._le_mensagem()
            if msg.get("id") == rid:
                if "error" in msg:
                    raise RuntimeError(f"{metodo}: {msg['error']}")
                return msg.get("result")
            if "method" in msg and "id" not in msg:
                self.notificacoes.append(msg)
            # requests do servidor pro cliente (registerCapability etc): ignora
            elif "method" in msg:
                self._envia({"jsonrpc": "2.0", "id": msg["id"], "result": None})

    # ── operações de editor ───────────────────────────────────────────

    def abre(self, path: Path, texto: str) -> str:
        uri = path.as_uri()
        self._notifica("textDocument/didOpen", {"textDocument": {
            "uri": uri, "languageId": "poolscript", "version": 1, "text": texto}})
        return uri

    def diagnosticos(self, uri: str) -> list:
        """Espera o publishDiagnostics do uri (didOpen é assíncrono)."""
        for n in self.notificacoes:
            if n["method"] == "textDocument/publishDiagnostics" \
                    and n["params"]["uri"] == uri:
                return n["params"]["diagnostics"]
        # ainda não chegou: força um roundtrip (hover barato) e olha de novo
        for _ in range(20):
            self._responde("textDocument/hover", {
                "textDocument": {"uri": uri},
                "position": {"line": 0, "character": 0}})
            for n in self.notificacoes:
                if n["method"] == "textDocument/publishDiagnostics" \
                        and n["params"]["uri"] == uri:
                    return n["params"]["diagnostics"]
        raise AssertionError("publishDiagnostics não chegou")

    def completa(self, uri: str, linha: int, col: int) -> list[str]:
        r = self._responde("textDocument/completion", {
            "textDocument": {"uri": uri},
            "position": {"line": linha, "character": col}})
        itens = r["items"] if isinstance(r, dict) else (r or [])
        return [i["label"] for i in itens]

    def paira(self, uri: str, linha: int, col: int) -> str:
        r = self._responde("textDocument/hover", {
            "textDocument": {"uri": uri},
            "position": {"line": linha, "character": col}})
        if not r:
            return ""
        c = r.get("contents")
        if isinstance(c, dict):
            return c.get("value", "")
        return str(c)

    def fecha(self):
        try:
            self._responde("shutdown", {})
            self._notifica("exit", {})
            self.proc.wait(timeout=5)
        except Exception:
            self.proc.kill()


@pytest.fixture(scope="module")
def ws(tmp_path_factory):
    return tmp_path_factory.mktemp("ws")


@pytest.fixture(scope="module")
def cliente(ws):
    c = ClienteLSP(ws)
    yield c
    c.fecha()


# ── cenários ─────────────────────────────────────────────────────────────────

def test_cadeia_db_type_aware(cliente, ws):
    """conn = psodbc.connect() -> DbConnection -> cursor() -> DbCursor."""
    src = ('import psodbc' + NL + 'conn = psodbc.connect(base="d")' + NL
           + 'cur = conn.cursor()' + NL + 'cur.' + NL)
    uri = cliente.abre(ws / "db.ps", src)
    nomes = cliente.completa(uri, 3, 4)
    for esperado in ("fetchall", "fetchone", "fetchmany", "rowcount"):
        assert esperado in nomes, f"faltou {esperado} em {nomes[:15]}"


def test_tipo_desconhecido_nao_sugere(cliente, ws):
    """Variável de tipo desconhecido: NADA de método falso."""
    src = 'x = misterio()' + NL + 'x.' + NL
    uri = cliente.abre(ws / "des.ps", src)
    assert cliente.completa(uri, 1, 2) == []


def test_poolfile_com_save(cliente, ws):
    src = ('import os' + NL + 'f = os.loadFile("a.png")' + NL + 'f.' + NL)
    uri = cliente.abre(ws / "pf.ps", src)
    nomes = cliente.completa(uri, 2, 2)
    for esperado in ("save", "move", "copy", "delete", "name", "ext", "size"):
        assert esperado in nomes, f"faltou {esperado} em {nomes}"


def test_request_do_jinker_e_proxy(cliente, ws):
    """Com jinker importado, `request.` é o proxy (method/path/header)."""
    src = ('import jinker' + NL + 'app = jinker.Jinker()' + NL + 'request.' + NL)
    uri = cliente.abre(ws / "jk.ps", src)
    nomes = cliente.completa(uri, 2, 8)
    for esperado in ("method", "path", "header", "headers"):
        assert esperado in nomes, f"faltou {esperado} em {nomes}"


def test_request_lib_e_modulo(cliente, ws):
    """Sem jinker, `import request` -> request. são os verbos HTTP."""
    src = 'import request' + NL + 'request.' + NL
    uri = cliente.abre(ws / "rq.ps", src)
    nomes = cliente.completa(uri, 1, 8)
    for esperado in ("get", "post", "put", "delete"):
        assert esperado in nomes, f"faltou {esperado} em {nomes}"


def test_self_na_entity(cliente, ws):
    src = ('Entity Conta() {' + NL
           + '    action __init__(self) {' + NL
           + '        self.saldo = 0' + NL
           + '    }' + NL
           + '    action ver(self) {' + NL
           + '        post(self.)' + NL
           + '    }' + NL
           + '}' + NL)
    uri = cliente.abre(ws / "ent.ps", src)
    nomes = cliente.completa(uri, 5, len('        post(self.'))
    assert "saldo" in nomes, f"faltou saldo em {nomes}"
    assert "ver" in nomes, f"faltou ver em {nomes}"


def test_argumento_nomeado(cliente, ws):
    src = 'import psodbc' + NL + 'psodbc.connect(' + NL
    uri = cliente.abre(ws / "arg.ps", src)
    nomes = cliente.completa(uri, 1, len('psodbc.connect('))
    assert any(n.startswith("driver") for n in nomes), f"faltou driver= em {nomes[:15]}"


def test_from_import_sem_builtins(cliente, ws):
    (ws / "_conn.ps").write_text('action conectar() { return 1 }' + NL, encoding="utf-8")
    src = 'from ._conn import '
    uri = cliente.abre(ws / "main.ps", src)
    nomes = cliente.completa(uri, 0, len(src))
    assert "conectar" in nomes, f"faltou conectar em {nomes}"
    assert "post" not in nomes, f"builtin post vazou: {nomes}"


def test_erro_de_sintaxe_vira_diagnostico(cliente, ws):
    uri = cliente.abre(ws / "err.ps", 'action f( {' + NL)
    diags = cliente.diagnosticos(uri)
    assert diags, "erro de sintaxe não virou diagnóstico"
    assert diags[0]["severity"] == 1   # Error


def test_import_nao_usado_e_apagado(cliente, ws):
    uri = cliente.abre(ws / "nu.ps", 'import os' + NL + 'post("oi")' + NL)
    diags = cliente.diagnosticos(uri)
    naousado = [d for d in diags if "não é usado" in d["message"]]
    assert naousado, f"import não usado não marcado: {diags}"
    assert 1 in (naousado[0].get("tags") or []), "sem DiagnosticTag.Unnecessary"
    # sublinha o `os`, NÃO a keyword `import` (col 7, 0-based)
    assert naousado[0]["range"]["start"]["character"] == 7, naousado[0]["range"]


def test_var_tipada_marca_o_nome_nao_o_tipo(cliente, ws):
    """`int x = 1` sem uso: o apagado fica no `x` (col 4), não no `int`."""
    uri = cliente.abre(ws / "vt.ps", 'int x = 1' + NL + 'post("fim")' + NL)
    diags = cliente.diagnosticos(uri)
    naousado = [d for d in diags if "não é usado" in d["message"]]
    assert naousado, f"var tipada não usada não marcada: {diags}"
    assert naousado[0]["range"]["start"]["character"] == 4, naousado[0]["range"]
    assert naousado[0]["range"]["end"]["character"] == 5, naousado[0]["range"]


def test_variavel_usada_nao_marca(cliente, ws):
    uri = cliente.abre(ws / "uso.ps", 'x = 1' + NL + 'post(x)' + NL)
    diags = cliente.diagnosticos(uri)
    assert not [d for d in diags if "não é usado" in d["message"]], diags


def test_hover_keyword_rico(cliente, ws):
    src = 'try { raise "x" } catch (e) { post(e) }' + NL
    uri = cliente.abre(ws / "hv.ps", src)
    texto = cliente.paira(uri, 0, 20)   # em cima do catch
    assert "catch" in texto, f"hover pobre: {texto!r}"


def test_hover_membro_de_cadeia(cliente, ws):
    src = ('import os' + NL + 'f = os.loadFile("a.png")' + NL + 'f.save()' + NL)
    uri = cliente.abre(ws / "hm.ps", src)
    texto = cliente.paira(uri, 2, 3)    # em cima do save
    assert "save" in texto and "PoolFile" in texto, f"hover: {texto!r}"
