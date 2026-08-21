"""swagger — builder gera OpenAPI (info + rotas do target + contratos),
cliente TS e Swagger UI."""
import json

from poolscript.stdlib import swagger_lib

APP = """from jinker import Jinker
app = Jinker()
@app.route("/login", methods=["POST"])
action login():
    return {"ok": true}
@app.route("/users/:id", methods=["GET"])
action um():
    return {"ok": true}
"""


def test_swagger_gera_spec_client_ui(tmp_path):
    target = tmp_path / "app.ps"; target.write_text(APP, encoding="utf-8")
    contratos = tmp_path / "contratos"; contratos.mkdir()
    (contratos / "login.json").write_text('{"@login": {"nome": "kleber"}}', encoding="utf-8")
    docs = tmp_path / "docs"

    b = (swagger_lib.infos(target=str(target))
         .title("API X").description("d").version("2.0.0")
         .authorContact().email("e@x.com").name("K").number("(27) 0"))
    b.newDoc(folder_name=str(docs), contracts=[str(contratos), "json"])
    b.bundle(lang="typescript", router="react_router")
    b.metaData(file_type=".json", name="openapi")
    b.SwaggerGEN()

    spec = json.loads((docs / "openapi.json").read_text(encoding="utf-8"))
    assert spec["openapi"] == "3.0.0"
    assert spec["info"]["title"] == "API X"
    assert spec["info"]["version"] == "2.0.0"
    assert spec["info"]["contact"]["email"] == "e@x.com"
    # rotas do target
    assert "post" in spec["paths"]["/login"]
    assert "/users/{id}" in spec["paths"]
    # contrato -> requestBody
    ex = spec["paths"]["/login"]["post"]["requestBody"]["content"]["application/json"]["example"]
    assert ex == {"nome": "kleber"}
    # parâmetro de path
    assert spec["paths"]["/users/{id}"]["get"]["parameters"][0]["name"] == "id"
    # cliente + UI
    cli = (docs / "client.ts").read_text(encoding="utf-8")
    assert "postLogin" in cli and "getUsersId" in cli
    assert (docs / "index.html").read_text(encoding="utf-8").count("swagger-ui") >= 1


def test_swagger_bundle_default(tmp_path):
    target = tmp_path / "a.ps"; target.write_text(APP, encoding="utf-8")
    docs = tmp_path / "d"
    (swagger_lib.infos(target=str(target)).title("t").newDoc(folder_name=str(docs)).bundle())
    swagger_lib.infos(target=str(target)).title("t").newDoc(folder_name=str(docs)).bundle().SwaggerGEN()
    assert (docs / "client.ts").exists()


def test_swagger_sem_target(tmp_path):
    # target inacessível -> spec só com info, paths vazio, sem crashar
    docs = tmp_path / "d"
    swagger_lib.infos(target="/nao/existe.ps").title("t").newDoc(folder_name=str(docs)).SwaggerGEN()
    spec = json.loads((docs / "openapi.json").read_text(encoding="utf-8"))
    assert spec["paths"] == {}
    assert spec["info"]["title"] == "t"
