# `@app.delete(path, auth=None, middleware=None, model=None)`

Registra uma rota que responde **só a DELETE** — o
[`@app.route(...)`](../route/route.md) com o método decidido pelo nome. Não
tem `methods=`: o verbo é o membro.

```ps
@app.delete("/perfil/<id>")
action apaga(id) {
    return jsonify({"apagou": id})
}
```

Qualquer **outro** método no mesmo path é `405` com `Allow: DELETE`.

Parâmetros (`auth=`, `middleware=`, `model=`) e o `405` estão explicados em
[`post`](../post/post.md), e são os mesmos nos cinco verbos:
[`get`](../get/get.md), [`post`](../post/post.md), [`put`](../put/put.md),
[`patch`](../patch/patch.md), `delete`.
