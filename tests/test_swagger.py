"""swagger (lib em PoolScript) — gera OpenAPI + cliente + Swagger UI, IGUAL nos
dois motores (reusa os/regex/json, que existem em ambos)."""
import json
import os
import shutil
import subprocess
import sys
from pathlib import Path

import pytest

RAIZ = Path(__file__).resolve().parent.parent
POOL = RAIZ / "pool"
assert POOL.exists(), "binário pool não compilado — rode ./rebuild_vm.sh (VM em C não se pula: skip = falso verde)"
SRC = RAIZ / "src"
SWAGGER_PS = SRC / "poolscript" / "stdlib" / "swagger.ps"

APP = """from jinker import Jinker
app = Jinker()
@app.route("/login", methods=["POST"])
action login():
    return {"ok": true}
@app.route("/users/:id", methods=["GET"])
action um():
    return {"ok": true}
"""

GEN = """import swagger
d = swagger.infos(target="__APP__").title("API X").description("d").version("2.0.0").authorContact().email("e@x.com").name("K").number("(27)0")
d.newDoc(folder_name="__DOCS__", contracts=["__CONTR__", "json"])
d.bundle(lang="typescript", router="react_router")
d.metaData(file_type=".json", name="openapi")
post(d.SwaggerGEN())
"""


def _setup(tmp_path):
    shutil.copy(SWAGGER_PS, tmp_path / "swagger.ps")     # import relativo -> self-contained
    (tmp_path / "app.ps").write_text(APP, encoding="utf-8")
    (tmp_path / "contratos").mkdir()
    (tmp_path / "contratos" / "login.json").write_text(
        '{"@login": {"nome": "kleber"}}', encoding="utf-8")
    docs = tmp_path / "docs"
    gen = tmp_path / "gen.ps"
    gen.write_text(GEN.replace("__APP__", str(tmp_path / "app.ps"))
                      .replace("__DOCS__", str(docs))
                      .replace("__CONTR__", str(tmp_path / "contratos")),
                   encoding="utf-8")
    return gen, docs


def _run(cmd, gen):
    env = dict(os.environ, PYTHONPATH=str(SRC))
    return subprocess.run(cmd + [str(gen)], capture_output=True, text=True,
                          env=env, timeout=20, check=False)


def _confere(docs):
    spec = json.loads((docs / "openapi.json").read_text(encoding="utf-8"))
    assert spec["openapi"] == "3.0.0"
    assert spec["info"]["title"] == "API X"
    assert spec["info"]["version"] == "2.0.0"
    assert spec["info"]["contact"]["email"] == "e@x.com"
    assert "post" in spec["paths"]["/login"]
    ex = spec["paths"]["/login"]["post"]["requestBody"]["content"]["application/json"]["example"]
    assert ex == {"nome": "kleber"}
    assert spec["paths"]["/users/{id}"]["get"]["parameters"][0]["name"] == "id"
    cli = (docs / "client.ts").read_text(encoding="utf-8")
    assert "postLogin" in cli and "getUsersId" in cli
    assert "swagger-ui" in (docs / "index.html").read_text(encoding="utf-8")


def test_swagger_interp(tmp_path):
    gen, docs = _setup(tmp_path)
    r = _run([sys.executable, "-m", "poolscript"], gen)
    assert (docs / "openapi.json").exists(), r.stderr
    _confere(docs)


def test_swagger_vm(tmp_path):
    gen, docs = _setup(tmp_path)
    r = _run([str(POOL)], gen)
    assert (docs / "openapi.json").exists(), r.stderr
    _confere(docs)
