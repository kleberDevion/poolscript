# `@app.put(path, auth=None, middleware=None, model=None)`

Registra uma rota que responde **só a PUT** — o
[`@app.route(...)`](../route/route.md) com o método decidido pelo nome. Não
tem `methods=`: o verbo é o membro.

```ps
model Perfil() {
    nome: str(length=40)
    bio: str
}

@app.put("/perfil", model=Perfil)
action troca() {
    # PUT substitui o recurso inteiro: o model garante que veio inteiro
    return jsonify({"trocou": request.get("nome")})
}
```

Qualquer **outro** método no mesmo path é `405` com `Allow: PUT` — e se o
path também tem um `get`, `Allow: GET, PUT`.

Parâmetros (`auth=`, `middleware=`, `model=`) e o `405` estão explicados em
[`post`](../post/post.md), e são os mesmos nos cinco verbos:
[`get`](../get/get.md), [`post`](../post/post.md), `put`,
[`patch`](../patch/patch.md), [`delete`](../delete/delete.md).
