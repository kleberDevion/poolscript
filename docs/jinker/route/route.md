# `@app.route(path, methods=None, auth=None, middleware=None, model=None)`

Registra uma **rota**: liga um caminho de URL à `action` declarada logo abaixo
do decorador. Quando alguém acessa esse caminho, o jinker chama a action e usa
o que ela retorna como resposta.

```
@app.route(path, methods=..., auth=..., middleware=...)
action nome_da_rota() {
    # request disponível aqui; return vira a resposta
}
```

| Parâmetro | Tipo | Padrão | O que é |
|---|---|---|---|
| `path` | `str` | — | o path da URL (ex: `"/api/hello"`) |
| `methods` | lista | todos do `cors` | métodos HTTP aceitos — use [`cors.options([...])`](../cors/options/options.md) |
| `auth` | lista | origens do `cors` | checagem de origem — use [`cors.origins()`](../cors/origins/origins.md) |
| `middleware` | função | nenhum | roda antes da action (ver [middleware](../middleware/middleware.md)) |
| `model` | `model` | nenhum | valida o corpo JSON **antes** da action; torto vira `422` |

---

## Atalhos por verbo: `get`, `post`, `put`, `patch`, `delete`

Quando a rota responde a **um** método só — que é a maioria — o verbo pode ir
no nome do membro, e aí `methods=` não existe:

```ps
from jinker import Jinker, jsonify, request, cors

app = Jinker(__name__)

model Login() {
    email: str(length=60)
    senha: str
}

@app.post("/login", model=Login, auth=cors.origins())
action entrar() {
    return jsonify({"ok": true, "email": request.get("email")})
}

@app.get("/perfil")
action perfil() {
    return jsonify({"quem": "ana"})
}
```

Os cinco recebem os mesmos `auth=`, `middleware=` e `model=` desta página.
Estão explicados em **[post/post.md](../post/post.md)** — inclusive o `405`
abaixo, que é a razão de eles existirem.

## Método errado é `405`, não `404`

Se o **path** está registrado mas o **método** não, a resposta é
`405 Method Not Allowed`, com `Allow:` listando o que a rota (ou as rotas)
daquele path aceita:

```
GET /login          (rota declarada só com POST)
→ 405   Allow: POST
  {"error": true, "code": 405, "message": "método inválido: GET /login — a rota aceita POST"}
```

`404` fica reservado pra path que **não existe**. A diferença é o que o front
precisa saber: `405` = "a URL está certa, troque o método"; `404` = "a URL
está errada".

---

## `model=` — o corpo validado antes da action

A linguagem já tem [`model`](../../linguagem/08-model-e-enum.md) e o `==` que
valida um dict contra ele. Passar o model na rota liga isso na porta de
entrada: o corpo é conferido **antes** de a action rodar, e corpo torto vira
`422` dizendo qual campo e por quê.

```ps
model Usuario() {
    nome: str(length=20)
    idade: int
}

@app.route("/user", methods=["POST"], model=Usuario)
action cria()
{
    # chegou aqui: nome e idade EXISTEM e são do tipo certo.
    # Nenhum `if` de validação neste corpo.
    return {"criado": request.get("nome")}
}
```

O que o cliente recebe:

| corpo enviado | resposta |
|---|---|
| `{"nome": "ana", "idade": 30}` | `200` — a action rodou |
| `{"nome": "ana"}` | `422` · `campo 'idade': faltando` |
| `{"nome": "ana", "idade": "x"}` | `422` · `campo 'idade': esperava int, veio str` |
| `{"nome": "<21 letras>", "idade": 1}` | `422` · `campo 'nome': no maximo 20 caracteres, veio 21` |
| `{isto nao e json` | `400` · `corpo nao e JSON valido` |

**JSON quebrado é `400`, não `422`**, e a diferença não é decorativa: `422`
significa "entendi o que você mandou e ele não serve", `400` significa "não
consegui nem ler". Quem recebe `422` corrige um campo; quem recebe `400`
corrige o cliente.

A validação é a MESMA do operador `==` — a rota não tem uma segunda noção do
que o model aceita. A única coisa que ela acrescenta é dizer **qual** campo
reprovou, que o `==` não tem como devolver.

Rota sem `model=` não muda em nada.

---

## A action é capturada automaticamente

Você **não** envolve a action em chaves extras nem a chama. O `@app.route(...)`
pega a próxima `action` como o handler da rota:

```
@app.route("/", methods=cors.options(["GET"]))
action inicio() {
    return jsonify({"msg": "olá"})
}
```

A action **não recebe parâmetros** — `request` já está disponível dentro dela
de graça. E você nunca escreve `inicio()`: o jinker chama quando a URL `/` é
acessada.

---

## Caminho fixo vs. com parâmetro dinâmico

**Fixo** — casa exatamente:

```
@app.route("/api/status", methods=cors.options(["GET"]))
```

**Com parâmetro** — parte do caminho vira variável. Duas sintaxes:

```
@app.route("/user:id", methods=cors.options(["GET"]))       # /user/42
@app.route("/user/<id>", methods=cors.options(["GET"]))     # /user/42
```

Pegue o valor dentro da action com
[`request.path_param("id")`](../request/path_param/path_param.md):

```
@app.route("/user/<id>", methods=cors.options(["GET"]))
action perfil() {
    id = request.path_param("id")
    return jsonify({"user_id": id})
}
```

> A forma com **barra** (`/user/<id>`) é a recomendada: casa `/user/42`
> naturalmente. A forma `/user:id` (sem barra) exige a URL colada
> (`/user42`) — comportamento verificado.

---

## Restringindo método e origem

```
@app.route(
    "/api/login",
    methods=cors.options(["POST"]),   # só POST
    auth=cors.origins()               # só origens configuradas no cors(...)
)
action login() {
    return jsonify({"ok": true})
}
```

`methods` e `auth` são **opcionais**. Sem `methods`, vale o conjunto global do
`cors()`. Sem `auth`, não há restrição de origem.

Pra um método só, o atalho diz a mesma coisa sem a lista:
`@app.post("/api/login", auth=cors.origins())` — ver
[post/post.md](../post/post.md).

### `HEAD` vem junto com o `GET`

Uma rota que aceita **GET** responde **HEAD** automaticamente: mesmo status e
mesmos cabeçalhos (`Content-Length` inclusive), **sem o corpo** — é o que a
RFC 9110 manda e serve pra quem só quer checar existência/tamanho:

```
@app.route("/relatorio.pdf", methods=cors.options(["GET"]))
action relatorio() {
    return render("arquivos/relatorio.pdf")
}
```

```bash
curl -I http://localhost:2000/relatorio.pdf
# HTTP/1.1 200 OK
# Content-Length: 184320     ← o tamanho que o GET traria
# (sem corpo)
```

Rota que **não** aceita GET (só POST, por exemplo) devolve `405` no HEAD, igual
ao GET — o path existe, o método não. Do outro lado, pra **fazer** uma
requisição HEAD, use [`request.head()`](../../request/head/head.md).

---

## O que a action pode retornar

Ver [`JinkerResponse`](../JinkerResponse/JinkerResponse.md) para a tabela
completa. Resumo:

```
return {"a": 1}                      # dict/lista → JSON 200
return "texto"                       # string → texto puro 200
return jsonify({"x": 1}), 201        # JSON com status
return render("web/index.html")      # arquivo
return None                          # 204 sem corpo
```

---

## Relacionados

- [`request`](../request/request.md) — o que chegou na requisição
- [`JinkerResponse`](../JinkerResponse/JinkerResponse.md) — o que você devolve
- [`middleware`](../middleware/middleware.md) — verificação antes da rota
- [`socket`](../socket/socket.md) — o equivalente pra WebSocket
