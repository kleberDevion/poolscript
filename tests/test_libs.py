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


# ── shield(): erro de biblioteca != bug do interpretador ─────────────────────
# Antes, TODA exceção fora do _MAP virava PoolInternalError, que diz "erro
# inesperado no interpretador" e manda abrir issue no GitHub — mesmo quando a
# causa era do usuário (host errado, arquivo corrompido) e o erro tinha vindo
# de uma lib por baixo (pyodbc, zipfile, sqlite3...).

def test_shield_lib_error_is_not_internal_error():
    from poolscript.ps_errors import shield, PoolLibraryError, PoolInternalError, _FakeNode
    import sqlite3
    try:
        sqlite3.connect(":memory:").execute("SELECT * FROM tabela_inexistente")
    except Exception as e:
        err = shield(e, _FakeNode(1, 1), "x = 1", "s.ps")
    assert isinstance(err, PoolLibraryError)
    assert not isinstance(err, PoolInternalError)
    msg = err.pool_message()
    assert "sqlite3" in msg          # diz de qual lib veio
    assert "github.com" not in msg   # não manda reportar issue contra o interpretador
    assert "interpretador" not in msg


def test_shield_lib_error_shows_root_frames():
    """O 'log raiz' da lib deve aparecer — frames da lib, nunca do interpretador."""
    from poolscript.ps_errors import shield, _FakeNode
    import zipfile, io
    try:
        zipfile.ZipFile(io.BytesIO(b"nao e um zip"))
    except Exception as e:
        err = shield(e, _FakeNode(1, 1), "x = 1", "s.ps")
    assert err.lib == "zipfile"
    assert err.origin_frames, "deveria trazer os frames de origem da lib"
    # nenhum frame do próprio interpretador pode vazar como se fosse da lib
    assert not any("interpreter.py" in f for f in err.origin_frames)


def test_shield_real_interpreter_bug_still_internal_error():
    """Erro sem origem de lib continua sendo InternalError (bug de verdade)."""
    from poolscript.ps_errors import shield, PoolInternalError, _FakeNode
    err = shield(AttributeError("objeto interno sem atributo"), _FakeNode(1, 1), "x = 1", "s.ps")
    assert isinstance(err, PoolInternalError)
    assert "github.com" in err.pool_message()


def test_shield_accepts_fake_node_when_compiled():
    """shield() usa _FakeNode por padrão; construir o erro não pode estourar.

    Compilado com mypyc a anotação de tipo vira checagem em runtime — com
    `node: "Node | None"` isso levantava TypeError DENTRO do tratamento de erro
    e escondia o erro real do usuário.
    """
    from poolscript.ps_errors import shield, _FakeNode
    err = shield(TypeError("tipos incompatíveis"), _FakeNode(2, 3), "y = 2", "s.ps")
    assert err.pool_message()  # não pode levantar
