# `Jinker.get(path, auth=None, middleware=None, model=None)`

Registra uma rota que responde **só a GET** — o
[`@app.route(...)`](../route/route.md) com o método decidido pelo nome. Não
tem `methods=`: o verbo é o membro.

É um membro do objeto [`Jinker`](../Jinker/Jinker.md), então na prática se
escreve `@app.get(...)`, com `app` sendo a sua aplicação.

```ps
@app.get("/perfil")
funct perfil() {
    return jsonify({"quem": "ana"})
}

@app.get("/user/<id>")
funct um(id) {                       # o path param entra no argumento, se pedir
    return jsonify({"id": id})
}
```

## Parâmetros

| nome | tipo | default | nota |
|---|---|---|---|
| `path` | `str` | — | o path da URL (`"/perfil"`, `"/user/<id>"`); **o único obrigatório** |
| `auth` | lista de `str` | origens do `cors` | checagem de origem — [`cors.origins()`](../cors/origins/origins.md) |
| `middleware` | funct | nenhum | roda antes da funct — [middleware](../middleware/middleware.md) |
| `model` | `model` | nenhum | valida o corpo JSON **antes** da funct; torto vira `422` |

Os nomes são estes quatro: `get(caminho=...)` é
`'caminho' is an invalid keyword argument for get()`. Não existe `methods=`
aqui — o verbo é o nome do membro. `model=` que não seja um `model` é
`TypeError`.

## Retorno

**`_RouteRegistrar`** — o registrador da rota. Não é a rota, não é a `funct`, e
não é o `app`.

```ps
reg = app.get("/x")
post(type(reg))     # _RouteRegistrar
post(reg)           # <RouteRegistrar ['GET'] /x>
```

Ele tem **um membro só**, `.register(handler)`, e no uso normal quem chama é o
próprio decorador: `@app.get("/x")` avalia `app.get("/x")`, define a funct
logo abaixo e chama `.register(funct)`. Por isso o valor devolvido some sem
você ver. Detalhe em
[`RouteRegistrar`](../RouteRegistrar/RouteRegistrar.md).

---

`HEAD` numa rota `get` é atendido por ela (RFC 9110: mesmos cabeçalhos, sem
corpo). Qualquer **outro** método no mesmo path é `405` com `Allow: GET`.

Os parâmetros e o `405` estão explicados em [`post`](../post/post.md), e são
os mesmos nos cinco verbos: `get`, [`post`](../post/post.md),
[`put`](../put/put.md), [`patch`](../patch/patch.md),
[`delete`](../delete/delete.md).
