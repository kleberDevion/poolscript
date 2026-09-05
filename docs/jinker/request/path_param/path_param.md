# `request.path_param(key)`

Devolve o valor de um **parâmetro dinâmico da URL** — a parte do caminho que
você marcou como variável na rota (`/user/<id>` ou `/user:id`).

```
request.path_param(key: str) -> valor | Null
```

---

## Uso

Rota com parâmetro `id`:

```
@app.route("/user/<id>", methods=cors.options(["GET"]))
action perfil() {
    id = request.path_param("id")    # acessando /user/42 → "42"
    return jsonify({"user_id": id})
}
```

---

## Diferença de `.get()`

- **`.path_param("id")`** — vem do **caminho da URL** (`/user/42`).
- **`.get("id")`** — vem da **query string** (`?id=42`) ou do **corpo JSON**.

São fontes diferentes:

```
# GET /user/42?fmt=json
id  = request.path_param("id")   # "42"  (do caminho)
fmt = request.get("fmt")         # "json" (da query string)
```

---

## Vários parâmetros

```
@app.route("/loja/<loja>/item/<item>", methods=cors.options(["GET"]))
action item() {
    loja = request.path_param("loja")
    item = request.path_param("item")
    return jsonify({"loja": loja, "item": item})
}
```

---

## Também funciona em sockets

Handlers de WebSocket com path dinâmico (`/chat/<sala>`) usam o mesmo método —
ver [`socket`](../../socket/socket.md).

---

## Relacionados

- [`route`](../../route/route.md) — onde os parâmetros dinâmicos são definidos
- [`.get()`](../get/get.md) — valor da query string / corpo
