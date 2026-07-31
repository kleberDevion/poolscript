"""Fixes de segurança: CORS allowlist, host default e os.run sem shell."""
import inspect
import sys

from poolscript.stdlib.jinker_lib import Jinker, _build_acao_origin, render
from poolscript.stdlib.os_lib import run


# ── CORS: a allowlist tem que BLOQUEAR origem não permitida ───────────────────

def test_cors_disallowed_origin_is_not_wildcard():
    allowed = ["https://meusite.com"]
    val = _build_acao_origin("https://evil.com", allowed)
    assert val != "*"                    # nunca libera geral
    assert val != "https://evil.com"     # não reflete o atacante -> browser bloqueia


def test_cors_allowed_origin_is_reflected():
    allowed = ["https://meusite.com"]
    assert _build_acao_origin("https://meusite.com", allowed) == "https://meusite.com"


def test_cors_no_allowlist_stays_wildcard():
    # sem allowlist (ex: assets estáticos públicos) o comportamento é '*' mesmo
    assert _build_acao_origin("https://qualquer.com", None) == "*"


# ── host padrão seguro: localhost, não 0.0.0.0 ────────────────────────────────

def test_server_default_host_is_localhost():
    default_host = inspect.signature(Jinker.__call__).parameters["host"].default
    assert default_host == "127.0.0.1"


# ── os.run: sem shell — injeção neutralizada ──────────────────────────────────

def test_run_shell_metachars_are_literal():
    # ';' e '&' chegam como TEXTO no argv, não como separadores de comando
    payload = "a; rm -rf ~ & b"
    out = run([sys.executable, "-c", "import sys; print(sys.argv[1])", payload], capture=True)
    assert out == payload


def test_run_basic_capture():
    assert run([sys.executable, "-c", "print('ok')"], capture=True) == "ok"


# ── render(): path traversal não pode escapar da pasta ────────────────────────

def test_render_blocks_path_traversal(tmp_path, monkeypatch):
    monkeypatch.chdir(tmp_path)
    (tmp_path / "assets").mkdir()
    (tmp_path / "assets" / "app.js").write_text("ok", encoding="utf-8")
    (tmp_path / "secret.txt").write_text("SENHA", encoding="utf-8")   # FORA de assets

    # arquivo legítimo dentro da pasta -> serve (200)
    assert render("assets", "app.js").status_code == 200

    # tentativa de escapar com '../' -> 404, NÃO serve o secret
    escaped = render("assets", "../secret.txt")
    assert escaped.status_code == 404
    assert "SENHA" not in escaped._body
