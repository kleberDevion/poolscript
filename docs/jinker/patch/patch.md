# `@app.patch(path, auth=None, middleware=None, model=None)`

Registra uma rota que responde **só a PATCH** — o
[`@app.route(...)`](../route/route.md) com o método decidido pelo nome. Não
tem `methods=`: o verbo é o membro.

```ps
@app.patch("/perfil/<id>")
action remenda(id) {
    # PATCH altera parte do recurso: o corpo traz só o que muda
    return jsonify({"id": id, "mudou": request.get_json()})
}
```

Qualquer **outro** método no mesmo path é `405` com `Allow: PATCH`.

Parâmetros (`auth=`, `middleware=`, `model=`) e o `405` estão explicados em
[`post`](../post/post.md), e são os mesmos nos cinco verbos:
[`get`](../get/get.md), [`post`](../post/post.md), [`put`](../put/put.md),
`patch`, [`delete`](../delete/delete.md).
