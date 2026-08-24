# `@app.route(path, methods=None, auth=None, middleware=None)`

Registra uma **rota**: liga um caminho de URL à `action` declarada logo abaixo
do decorador. Quando alguém acessa esse caminho, o jinker chama a action e usa
o que ela retorna como resposta.

```
@app.route(path, methods=..., auth=..., middleware=...)
action nome_da_rota() {
    // request disponível aqui; return vira a resposta
}
```

| Parâmetro | Tipo | Padrão | O que é |
|---|---|---|---|
| `path` | `str` | — | o path da URL (ex: `"/api/hello"`) |
| `methods` | lista | todos do `cors` | métodos HTTP aceitos — use [`cors.options([...])`](../cors/options/options.md) |
| `auth` | lista | origens do `cors` | checagem de origem — use [`cors.origins()`](../cors/origins/origins.md) |
| `middleware` | função | nenhum | roda antes da action (ver [middleware](../middleware/middleware.md)) |

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
@app.route("/user:id", methods=cors.options(["GET"]))       // /user/42
@app.route("/user/<id>", methods=cors.options(["GET"]))     // /user/42
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
    methods=cors.options(["POST"]),   // só POST
    auth=cors.origins()               // só origens configuradas no cors(...)
)
action login() {
    return jsonify({"ok": true})
}
```

`methods` e `auth` são **opcionais**. Sem `methods`, vale o conjunto global do
`cors()`. Sem `auth`, não há restrição de origem.

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

Rota que **não** aceita GET (só POST, por exemplo) devolve 404 no HEAD, igual
ao GET. Do outro lado, pra **fazer** uma requisição HEAD, use
[`request.head()`](../../request/head/head.md).

---

## O que a action pode retornar

Ver [`JinkerResponse`](../JinkerResponse/JinkerResponse.md) para a tabela
completa. Resumo:

```
return {"a": 1}                      // dict/lista → JSON 200
return "texto"                       // string → texto puro 200
return jsonify({"x": 1}), 201        // JSON com status
return render("web/index.html")      // arquivo
return None                          // 204 sem corpo
```

---

## Relacionados

- [`request`](../request/request.md) — o que chegou na requisição
- [`JinkerResponse`](../JinkerResponse/JinkerResponse.md) — o que você devolve
- [`middleware`](../middleware/middleware.md) — verificação antes da rota
- [`socket`](../socket/socket.md) — o equivalente pra WebSocket
