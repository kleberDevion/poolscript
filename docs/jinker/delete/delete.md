# `Jinker.delete(path, auth=None, middleware=None, model=None)`

Registra uma rota que responde **só a DELETE** — o
[`@app.route(...)`](../route/route.md) com o método decidido pelo nome. Não
tem `methods=`: o verbo é o membro.

É um membro do objeto [`Jinker`](../Jinker/Jinker.md), então na prática se
escreve `@app.delete(...)`, com `app` sendo a sua aplicação.

```ps
@app.delete("/perfil/<id>")
funct apaga(id) {
    return jsonify({"apagou": id})
}
```

## Parâmetros

| nome | tipo | default | nota |
|---|---|---|---|
| `path` | `str` | — | o path da URL (`"/perfil/<id>"`); **o único obrigatório** |
| `auth` | lista de `str` | origens do `cors` | checagem de origem — [`cors.origins()`](../cors/origins/origins.md) |
| `middleware` | funct | nenhum | roda antes da funct — [middleware](../middleware/middleware.md) |
| `model` | `model` | nenhum | valida o corpo JSON **antes** da funct; torto vira `422` |

Os nomes são estes quatro. Não existe `methods=` aqui — o verbo é o nome do
membro.

## Retorno

**`_RouteRegistrar`** — o registrador da rota.

```ps
reg = app.delete("/perfil/7")
post(type(reg))     # _RouteRegistrar
post(reg)           # <RouteRegistrar ['DELETE'] /perfil/7>
```

Ele tem **um membro só**, `.register(handler)`, chamado pelo próprio
decorador. Detalhe em
[`RouteRegistrar`](../RouteRegistrar/RouteRegistrar.md).

---

Qualquer **outro** método no mesmo path é `405` com `Allow: DELETE`.

Os parâmetros e o `405` estão explicados em [`post`](../post/post.md), e são
os mesmos nos cinco verbos: [`get`](../get/get.md), [`post`](../post/post.md),
[`put`](../put/put.md), [`patch`](../patch/patch.md), `delete`.
