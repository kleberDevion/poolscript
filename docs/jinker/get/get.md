# `@app.get(path, auth=None, middleware=None, model=None)`

Registra uma rota que responde **só a GET** — o
[`@app.route(...)`](../route/route.md) com o método decidido pelo nome. Não
tem `methods=`: o verbo é o membro.

```ps
@app.get("/perfil")
action perfil() {
    return jsonify({"quem": "ana"})
}

@app.get("/user/<id>")
action um(id) {                      # o path param entra no argumento, se pedir
    return jsonify({"id": id})
}
```

`HEAD` numa rota `get` é atendido por ela (RFC 9110: mesmos cabeçalhos, sem
corpo). Qualquer **outro** método no mesmo path é `405` com `Allow: GET`.

Parâmetros (`auth=`, `middleware=`, `model=`) e o `405` estão explicados em
[`post`](../post/post.md), e são os mesmos nos cinco verbos: `get`,
[`post`](../post/post.md), [`put`](../put/put.md), [`patch`](../patch/patch.md),
[`delete`](../delete/delete.md).
