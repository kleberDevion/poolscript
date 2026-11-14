# `request` — o que chegou na requisição

`request` está disponível automaticamente dentro de qualquer rota (e handler de
socket) — você não declara nem importa. É um `RequestProxy` e representa **tudo
que o cliente mandou**: corpo JSON, query string, parâmetros da URL, método,
headers e arquivos. Campos de formulário não entram no `get` (veja
[get/get.md](get/get.md)).

Os exemplos desta página pressupõem:

```ps contexto
from jinker import Jinker, cors, jsonify
Jinker app = Jinker(__name__)
```

```ps
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
| `.get(chave)` | um valor da query string **ou** do JSON; chave repetida na query devolve lista | [get/get.md](get/get.md) |
| `.get_json()` | o corpo inteiro como dict/lista | [get_json/get_json.md](get_json/get_json.md) |
| `.path_param(chave)` | valor de parâmetro dinâmico da URL (`/user/<id>`) | [path_param/path_param.md](path_param/path_param.md) |
| `.header(nome)` | **um** cabeçalho pelo nome (case-insensitive), `Null` se faltar | [header/header.md](header/header.md) |
| `.text()` | o corpo cru como texto | [text/text.md](text/text.md) |
| `.file(campo, allowed)` | **um** arquivo enviado (upload) | [file/file.md](file/file.md) |
| `.files(campo, allowed)` | **vários** arquivos enviados | [files/files.md](files/files.md) |

---

## `request` é do tipo `RequestProxy`

Dentro da rota, `type(request)` devolve `"RequestProxy"`. O nome `JinkerRequest`
também é exportado pela lib, mas é um tipo **vazio** — criável e sem membro
nenhum; `request` **não** é um `JinkerRequest`.

Veja [`RequestProxy`](../../objetos-internos/RequestProxy.md) e
[`JinkerRequest`](../../objetos-internos/JinkerRequest.md).

---

## Relacionados

- [`route`](../route/route.md) — onde `request` fica disponível
- [`JinkerResponse`](../JinkerResponse/JinkerResponse.md) — o lado de saída
