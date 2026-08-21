"""
Módulo `swagger` da PoolScript — builder de OpenAPI/Swagger.

Coleta a config (título, descrição, versão, contato, saída) num builder
encadeável e, no `SwaggerGEN()`, gera o spec OpenAPI 3.0 a partir das rotas do
app-alvo (`target`), escrevendo o arquivo (ex: `docs/openapi.json`).

    import swagger

    swagger.infos(target="app.ps")
           .title("Minha API")
           .description("...")
           .version("1.0.0")
           .authorContact()
               .email("eu@mail.com")
               .name("Fulano")
               .number("(27) 90000-0000")
           .newDoc(folder_name="docs")
           .metaData(file_type=".json", name="openapi")
           .SwaggerGEN()
"""
from __future__ import annotations

import json as _json
import os as _os
import re as _re


# Swagger UI pra abrir no browser — carrega o spec ao lado (mesma pasta).
_SWAGGER_UI_HTML = """<!DOCTYPE html>
<html lang="pt-br">
<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>__TITULO__ — Swagger UI</title>
  <link rel="stylesheet" href="https://unpkg.com/swagger-ui-dist/swagger-ui.css">
</head>
<body>
  <div id="swagger-ui"></div>
  <script src="https://unpkg.com/swagger-ui-dist/swagger-ui-bundle.js"></script>
  <script>
    window.onload = function () {
      SwaggerUIBundle({ url: "./__SPEC__", dom_id: "#swagger-ui" });
    };
  </script>
</body>
</html>"""


def _rotas_do_fonte(src: str) -> dict:
    """Extrai as rotas (`@app.route("/x", methods=[...])`) do fonte de um app
    jinker e devolve o bloco `paths` do OpenAPI. Estático — não roda o app."""
    paths: dict = {}
    for m in _re.finditer(r'@\s*\w+\.route\s*\(\s*["\']([^"\']+)["\'](.*?)\)', src, _re.S):
        rota, resto = m.group(1), m.group(2)
        mm = _re.search(r'methods\s*=\s*\[([^\]]*)\]', resto)
        metodos = _re.findall(r'["\']([A-Za-z]+)["\']', mm.group(1)) if mm else ["GET"]
        # :id e <id> viram {id} (sintaxe de path OpenAPI)
        op = _re.sub(r':([A-Za-z_]\w*)', r'{\1}', rota)
        op = _re.sub(r'<([A-Za-z_]\w*)>', r'{\1}', op)
        item = paths.setdefault(op, {})
        params = [{"name": g, "in": "path", "required": True,
                   "schema": {"type": "string"}}
                  for g in _re.findall(r'\{(\w+)\}', op)]
        for mt in metodos:
            if mt.upper() in ("OPTIONS", "HEAD"):
                continue
            op_obj = {"summary": op, "responses": {"200": {"description": "OK"}}}
            if params:
                op_obj["parameters"] = params
            item[mt.lower()] = op_obj
    return paths


class SwaggerBuilder:
    """Builder encadeável do spec OpenAPI. Cada método guarda um pedaço da
    config e devolve o próprio builder; `SwaggerGEN()` gera o arquivo."""

    def __init__(self, target=None):
        self._target = target
        self._title = "API"
        self._description = ""
        self._version = "1.0.0"
        self._contact: dict = {}
        self._doc_folder = "."
        self._contracts = None
        self._bundle: dict = {}
        self._file_type = ".json"
        self._file_name = "openapi"

    # ── info (encadeável) ─────────────────────────────────────────────
    def title(self, value):
        self._title = str(value); return self

    def description(self, value):
        self._description = str(value); return self

    def version(self, value):
        self._version = str(value); return self

    def authorContact(self):
        # rótulo de seção — segue no mesmo builder (email/name/number caem no contato)
        return self

    def email(self, value):
        self._contact["email"] = str(value); return self

    def name(self, value):
        self._contact["name"] = str(value); return self

    def number(self, value):
        self._contact["x-phone"] = str(value); return self

    # ── saída (config) ────────────────────────────────────────────────
    def newDoc(self, folder_name=".", contracts=None):
        # contracts (opcional) = [pasta_dos_contratos, tipo]  (tipo: "json")
        self._doc_folder = str(folder_name)
        self._contracts = contracts
        return self

    def bundle(self, lang=None, router=None):
        # gera um cliente da API. Sem args -> default (typescript + react_router).
        self._bundle = {
            "lang": (lang or "typescript"),
            "router": (router or "react_router"),
        }
        return self

    def metaData(self, file_type=".json", name="openapi"):
        self._file_type = str(file_type); self._file_name = str(name); return self

    # ── geração ───────────────────────────────────────────────────────
    def _aplica_contratos(self, paths: dict) -> None:
        """Contratos (opcional): cada arquivo JSON tem a PRIMEIRA chave = `@rota`
        e o valor = body que a rota espera. Vira o `requestBody` da rota no spec.
        Ex: `{ "@login": { "nome": "kleber" } }` → requestBody de /login."""
        if not self._contracts:
            return
        pasta = self._contracts[0] if len(self._contracts) > 0 else None
        tipo = self._contracts[1] if len(self._contracts) > 1 else "json"
        if not pasta or not _os.path.isdir(pasta):
            return
        for fn in sorted(_os.listdir(pasta)):
            if tipo == "json" and not fn.endswith(".json"):
                continue
            try:
                with open(_os.path.join(pasta, fn), "r", encoding="utf-8") as f:
                    dados = _json.load(f)
            except (OSError, ValueError):
                continue
            if not isinstance(dados, dict) or not dados:
                continue
            chave = next(iter(dados))            # 1a chave = @rota
            rota = "/" + chave.lstrip("@").lstrip("/")
            body = dados[chave]
            rb = {"content": {"application/json": {"example": body}}}
            for p, ops in paths.items():
                if p == rota or p.rstrip("/").endswith(rota):
                    for op in ops.values():
                        op["requestBody"] = rb
                    break

    def spec(self) -> dict:
        """Monta o dict OpenAPI (info da config + rotas do target + contratos)."""
        info = {"title": self._title, "version": self._version}
        if self._description:
            info["description"] = self._description
        if self._contact:
            info["contact"] = dict(self._contact)
        paths: dict = {}
        if self._target:
            try:
                with open(self._target, "r", encoding="utf-8") as f:
                    paths = _rotas_do_fonte(f.read())
            except OSError:
                paths = {}   # target inacessível → spec só com a info
        self._aplica_contratos(paths)
        return {"openapi": "3.0.0", "info": info, "paths": paths}

    def _gen_client(self, esp: dict) -> str:
        """Gera um cliente da API a partir do spec (uma função por rota)."""
        lang = (self._bundle.get("lang") or "typescript")
        router = (self._bundle.get("router") or "react_router")
        L = ["// cliente gerado por swagger (%s / %s)" % (lang, router),
             'const BASE = "";', "",
             "async function _req(method: string, path: string, body?: any): Promise<any> {",
             "  const r = await fetch(BASE + path, {",
             "    method,",
             '    headers: body !== undefined ? { "Content-Type": "application/json" } : {},',
             "    body: body !== undefined ? JSON.stringify(body) : undefined,",
             "  });",
             "  return r.json();",
             "}", ""]
        rr = []
        for path, ops in esp["paths"].items():
            params = _re.findall(r"\{(\w+)\}", path)
            ts_path = _re.sub(r"\{(\w+)\}", r"${\1}", path)      # /users/{id} -> /users/${id}
            rr_path = _re.sub(r"\{(\w+)\}", r":\1", path)        # react_router usa :id
            for method in ops:
                segs = _re.findall(r"[A-Za-z0-9]+", path)
                fn = method.lower() + "".join(s.capitalize() for s in segs) or method.lower()
                tem_body = method.lower() in ("post", "put", "patch")
                args = ", ".join([p + ": string" for p in params] + (["body?: any"] if tem_body else []))
                L.append('export async function %s(%s) { return _req("%s", `%s`, %s); }'
                         % (fn, args, method.upper(), ts_path, "body" if tem_body else "undefined"))
                rr.append('  "%s": { %s: %s },' % (rr_path, method.lower(), fn))
        L += ["", "export const routes = {"] + rr + ["};", ""]
        return "\n".join(L)

    def SwaggerGEN(self):
        """Gera o spec em `<folder>/<name><file_type>` e, se `bundle` foi chamado,
        o cliente da API. Devolve o caminho do spec."""
        esp = self.spec()
        pasta = self._doc_folder or "."
        if pasta not in (".", ""):
            _os.makedirs(pasta, exist_ok=True)
        spec_file = self._file_name + self._file_type
        caminho = _os.path.join(pasta, spec_file)
        with open(caminho, "w", encoding="utf-8") as f:
            _json.dump(esp, f, ensure_ascii=False, indent=2)
        # index.html — Swagger UI pra abrir no browser (carrega o spec ao lado)
        with open(_os.path.join(pasta, "index.html"), "w", encoding="utf-8") as f:
            f.write(_SWAGGER_UI_HTML.replace("__TITULO__", self._title)
                                    .replace("__SPEC__", spec_file))
        if self._bundle:
            ext = ".ts" if "typescript" in str(self._bundle.get("lang", "")).lower() else ".js"
            with open(_os.path.join(pasta, "client" + ext), "w", encoding="utf-8") as f:
                f.write(self._gen_client(esp))
        return caminho

    def __repr__(self):
        return f"<SwaggerBuilder title={self._title!r} target={self._target!r}>"


def infos(target=None):
    return SwaggerBuilder(target)


EXPORTS = {
    "infos": infos,
}
