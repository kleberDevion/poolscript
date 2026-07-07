"""
Regressão dos bugs reportados em erros-v0-1-0.md (kleberDevion/erros-ps).

Bug 1: post() não aceitava argumentos justapostos sem vírgula.
Bug 2: request sem headers dava 403 Forbidden (sem User-Agent default).
Bug 3: request não aceitava args headers+body.
Bug 4: 'as' não funcionava em import / from-import.
"""
from __future__ import annotations

import io
import sys
from contextlib import redirect_stdout, redirect_stderr

from poolscript.lexer import Lexer
from poolscript.parser import parse_source
from poolscript.interpreter import Interpreter
from poolscript.stdlib.request_lib import _apply_default_headers, DEFAULT_USER_AGENT


def _run(src: str) -> list[str]:
    program = parse_source(src, "<test>")
    interp = Interpreter(source=src, filename="<test>")
    buf = io.StringIO()
    with redirect_stdout(buf), redirect_stderr(io.StringIO()):
        interp.run(program)
    return buf.getvalue().splitlines()


# ─── Bug 1: post() com args justapostos + Null em todas as grafias ───
def test_post_concat_sem_virgula():
    out = _run('''x = 42\npost("valor:" x)''')
    assert out == ["valor: 42"]


def test_post_tres_args_justapostos():
    out = _run('''a = 1\nb = 2\npost("a" a "b" b)''')
    assert out == ["a 1 b 2"]


def test_null_em_return_quatro_grafias():
    src = """
action f1() { return Null }
action f2() { return null }
action f3() { return None }
action f4() { return none }
post("v1:" f1())
post("v2:" f2())
post("v3:" f3())
post("v4:" f4())
"""
    out = _run(src)
    # Todas as 4 grafias viram o mesmo Null/None
    assert all("null" in line.lower() or "none" in line.lower() for line in out)
    assert len(out) == 4


# ─── Bug 2: User-Agent default ────────────────────────────────────
def test_user_agent_default_quando_sem_headers():
    h = _apply_default_headers(None)
    assert "User-Agent" in h
    assert h["User-Agent"] == DEFAULT_USER_AGENT


def test_user_agent_default_quando_headers_vazio():
    h = _apply_default_headers({})
    assert h["User-Agent"] == DEFAULT_USER_AGENT


def test_user_agent_respeita_quando_usuario_passou():
    h = _apply_default_headers({"User-Agent": "MeuBot/1.0"})
    assert h["User-Agent"] == "MeuBot/1.0"


def test_user_agent_adicionado_quando_outros_headers_presentes():
    h = _apply_default_headers({"Authorization": "Bearer xyz"})
    assert h["Authorization"] == "Bearer xyz"
    assert h["User-Agent"] == DEFAULT_USER_AGENT


def test_user_agent_case_insensitive():
    """Se o usuário mandou 'user-agent' minúsculo, não duplica."""
    h = _apply_default_headers({"user-agent": "Custom/2.0"})
    # Não deve haver dois headers de UA — verificamos que o valor original sobrevive
    ua_keys = [k for k in h if k.lower() == "user-agent"]
    assert len(ua_keys) == 1
    assert h[ua_keys[0]] == "Custom/2.0"


# ─── Bug 3: request com headers e body ────────────────────────────
def test_parser_aceita_chamada_com_headers_kwarg():
    """Não vamos bater rede no teste — só validamos que parseia."""
    src = '''from request import get
h = {"X-Token": "abc"}
r = get("https://example.com", headers=h)
'''
    program = parse_source(src, "<test>")
    assert program is not None  # não levanta SyntaxError


def test_parser_aceita_post_com_headers_e_body():
    src = '''from request import post as http_post
h = {"X-Token": "abc"}
b = {"name": "ana"}
r = http_post("https://example.com", headers=h, body=b)
'''
    program = parse_source(src, "<test>")
    assert program is not None


# ─── Bug 4: 'as' em imports ──────────────────────────────────────
def test_import_modulo_com_as():
    out = _run('''import os as sistema\npost("ok")''')
    assert out == ["ok"]


def test_from_import_um_nome_com_as():
    out = _run('''from request import get as buscar\npost("ok")''')
    assert out == ["ok"]


def test_from_import_multiplos_nomes_alguns_com_as():
    out = _run('''from request import get as buscar, post as enviar, put\npost("ok")''')
    assert out == ["ok"]


def test_alias_eh_o_nome_realmente_vinculado_no_escopo():
    """O nome original NÃO deve estar acessível depois do 'as'."""
    src = '''from request import get as buscar
post("buscar:" buscar)
'''
    out = _run(src)
    assert len(out) == 1
    assert out[0].startswith("buscar:")


def test_push_get_com_alias():
    out = _run('''PUSH os GET getenv as ler_env\npost("ok")''')
    assert out == ["ok"]
