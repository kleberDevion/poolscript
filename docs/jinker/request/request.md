# `request` — o que chegou na requisição

`request` está disponível automaticamente dentro de qualquer rota (e handler de
socket) — você não declara nem importa. É uma instância de `JinkerRequest` e
representa **tudo que o cliente mandou**: corpo, campos, parâmetros da URL,
método, headers, arquivos.

```
@app.route("/api/dados", methods=cors.options(["POST"]))
funct receber() {
    data = request.get_json()     # corpo JSON inteiro (dict)
    nome = request.get("nome")    # um campo (JSON ou query string)
    return jsonify({"ola": nome})
}
```

---

## Propriedades diretas

| Acesso | O que é |
|---|---|
| `request.method` | método HTTP (`"GET"`, `"POST"`…) |
| `request.path` | caminho da URL (`"/api/dados"`) |
| `request.headers` | dict com os cabeçalhos da requisição |

---

## Métodos

| Método | O que devolve | Página |
|---|---|---|
| `.get(chave)` | um campo do JSON **ou** da query string | [get/get.md](get/get.md) |
| `.get_json()` | o corpo inteiro como dict/lista | [get_json/get_json.md](get_json/get_json.md) |
| `.path_param(chave)` | valor de parâmetro dinâmico da URL (`/user/<id>`) | [path_param/path_param.md](path_param/path_param.md) |
| `.header(nome)` | **um** cabeçalho pelo nome (case-insensitive), `Null` se faltar | [header/header.md](header/header.md) |
| `.text()` | o corpo cru como texto | [text/text.md](text/text.md) |
| `.file(campo, allowed)` | **um** arquivo enviado (upload) | [file/file.md](file/file.md) |
| `.files(campo, allowed)` | **vários** arquivos enviados | [files/files.md](files/files.md) |

---

## `request` é do tipo `JinkerRequest`

Exportado pela lib caso você queira checar o tipo:

```
from jinker import JinkerRequest
# request is JinkerRequest  → true dentro de uma rota
```

---

## Relacionados

- [`route`](../route/route.md) — onde `request` fica disponível
- [`JinkerResponse`](../JinkerResponse/JinkerResponse.md) — o lado de saída
