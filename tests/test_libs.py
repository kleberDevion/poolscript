"""Testes das libs default."""
import os
import pytest
from poolscript.interpreter import Interpreter, PoolRuntimeError
from poolscript.parser import parse_source
from poolscript.stdlib import resolve_module, resolve_name, list_libs


def run(src):
    interp = Interpreter(source=src)
    interp.run(parse_source(src))
    return interp.output


def test_registry_has_essentials():
    libs = list_libs()
    for name in ("os", "json", "dotenv", "request"):
        assert name in libs


def test_resolve_module():
    assert resolve_module(["os"]) is not None
    assert resolve_module(["doesnt_exist"]) is None


def test_db_is_alias_de_psodbc():
    # "db" é o nome documentado em docs/PoolScript.md, docs/hash_jwt.md,
    # docs/jinker.md e no --help da CLI — mas o módulo real é psodbc_lib.py.
    # Sem esse alias, todo `import db` desses exemplos falhava em runtime.
    assert resolve_module(["db"]) is resolve_module(["psodbc"])
    assert "connect" in resolve_module(["db"])
    assert "query" in resolve_module(["db"])


def test_resolve_name():
    assert callable(resolve_name(["os"], "getenv"))
    assert resolve_name(["os"], "doesnt_exist") is None


def test_import_os_getenv(monkeypatch):
    monkeypatch.setenv("POOL_TEST_VAR", "hello")
    out = run('PUSH os GET getenv\npost(getenv("POOL_TEST_VAR"))')
    assert out == ["hello"]


def test_import_module_dot_access(monkeypatch):
    monkeypatch.setenv("FOO", "bar")
    out = run('import os\npost(os.getenv("FOO"))')
    assert out == ["bar"]


def test_from_import():
    out = run('from json import stringify\npost(stringify([1, 2, 3]))')
    assert out == ["[1, 2, 3]"]


def test_json_parse_and_get():
    out = run('''from json import parse
data = parse("{\\"name\\": \\"Pool\\"}")
post(data["name"])
''')
    assert out == ["Pool"]


def test_get_json_on_string():
    out = run('s = "{\\"x\\": 42}"\npost(s.get_json("x"))')
    assert out == ["42"]


def test_unknown_module():
    with pytest.raises(PoolRuntimeError):
        run('import foobar_nope')


def test_stub_lib_raises():
    # `flask` é stub — só deve falhar quando chamada, não na importação
    out = run('import flask\npost("ok")')
    assert out == ["ok"]
    with pytest.raises(PoolRuntimeError):
        run('from flask import Flask\nFlask("app")')
