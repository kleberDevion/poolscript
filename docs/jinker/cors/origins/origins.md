# `cors.origins()`

Devolve a lista de origens (domínios) configuradas no `cors(origins=[...])`
global. Usado no `auth=` de uma rota pra aplicar a checagem de origem.

```
cors.origins() -> list
```

---

## Uso

```
cors(origins=["https://meusite.com"])

@app.route("/api/dados", auth=cors.origins(), methods=cors.options(["GET"]))
action dados() {
    return jsonify({"ok": true})
}
```

`auth=cors.origins()` diz "essa rota só aceita requisições das origens
configuradas". Requisição de um domínio fora da lista recebe **403 Forbidden**
(exceto origens locais e clientes sem `Origin` — ver [`cors`](../cors.md)).

---

## Sem restrição

Se `cors(origins=[...])` não foi chamado (ou foi com lista vazia),
`cors.origins()` devolve `[]` — e `auth=cors.origins()` na prática libera todas
as origens. Ou seja, deixar o `auth=` mesmo assim não atrapalha; ele passa a
valer automaticamente quando você configurar origens.

---

## `permiser()` é a mesma coisa

`cors.permiser()` é um **apelido legado** de `cors.origins()` — devolve a mesma
lista. Existe por compatibilidade, mas prefira `cors.origins()`.

---

## Relacionados

- [`cors`](../cors.md) — configuração global (onde `origins=` é definido)
- [`cors.options()`](../options/options.md) — métodos permitidos
- [`route`](../../route/route.md) — onde `auth=` é usado
