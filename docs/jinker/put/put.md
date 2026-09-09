# `Jinker.put(path, auth=None, middleware=None, model=None)`

Registra uma rota que responde **só a PUT** — o
[`@app.route(...)`](../route/route.md) com o método decidido pelo nome. Não
tem `methods=`: o verbo é o membro.

É um membro do objeto [`Jinker`](../Jinker/Jinker.md), então na prática se
escreve `@app.put(...)`, com `app` sendo a sua aplicação.

```ps
model Perfil() {
    nome: str(length=40)
    bio: str
}

@app.put("/perfil", model=Perfil)
funct troca() {
    # PUT substitui o recurso inteiro: o model garante que veio inteiro
    return jsonify({"trocou": request.get("nome")})
}
```

## Parâmetros

| nome | tipo | default | nota |
|---|---|---|---|
| `path` | `str` | — | o path da URL (`"/perfil"`, `"/user/<id>"`); **o único obrigatório** |
| `auth` | lista de `str` | origens do `cors` | checagem de origem — [`cors.origins()`](../cors/origins/origins.md) |
| `middleware` | funct | nenhum | roda antes da funct — [middleware](../middleware/middleware.md) |
| `model` | `model` | nenhum | valida o corpo JSON **antes** da funct; torto vira `422` |

Os nomes são estes quatro. Não existe `methods=` aqui — o verbo é o nome do
membro.

## Retorno

**`_RouteRegistrar`** — o registrador da rota.

```ps
reg = app.put("/perfil")
post(type(reg))     # _RouteRegistrar
post(reg)           # <RouteRegistrar ['PUT'] /perfil>
```

Ele tem **um membro só**, `.register(handler)`, chamado pelo próprio
decorador. Detalhe em
[`RouteRegistrar`](../RouteRegistrar/RouteRegistrar.md).

---

Qualquer **outro** método no mesmo path é `405` com `Allow: PUT` — e se o
path também tem um `get`, `Allow: GET, PUT`.

Os parâmetros e o `405` estão explicados em [`post`](../post/post.md), e são
os mesmos nos cinco verbos: [`get`](../get/get.md), [`post`](../post/post.md),
`put`, [`patch`](../patch/patch.md), [`delete`](../delete/delete.md).
