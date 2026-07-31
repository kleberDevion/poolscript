# `cors.options([subset])`

Resolve a lista de métodos HTTP de uma rota. Usado no `methods=` do
`@app.route(...)`.

```
cors.options() -> list           // todos os métodos configurados globalmente
cors.options(["POST"]) -> list   // filtra/sobrescreve pra esta rota
```

---

## Dois usos

**Sem argumento** — devolve todos os métodos definidos no `cors(options=[...])`
global:

```
cors(options=["GET", "POST", "DELETE"])
// ...
methods=cors.options()    // GET, POST, DELETE
```

**Com uma lista** — define exatamente os métodos daquela rota (ignora o global):

```
@app.route("/api/item", methods=cors.options(["POST"]))   // só POST
action criar() { ... }

@app.route("/api/item", methods=cors.options(["GET", "DELETE"]))
action ler_ou_apagar() { ... }
```

---

## Por que isso existe

O `cors(options=...)` global diz o "teto" de métodos da API. Cada rota
normalmente aceita **só um ou dois** desses. `cors.options(["POST"])` deixa
explícito, na própria rota, o que ela responde — quem chamar com outro método
recebe erro.

```
@app.route("/api/login", methods=cors.options(["POST"]))
action login() {
    // só responde POST /api/login
    return jsonify({"ok": true})
}
```

---

## Relacionados

- [`cors`](../cors.md) — configuração global
- [`cors.origins()`](../origins/origins.md) — origens permitidas (usado em `auth=`)
- [`route`](../../route/route.md) — onde `methods=` é usado
