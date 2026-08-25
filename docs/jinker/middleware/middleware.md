# `@app.middleware()` — verificação antes da rota

Um **middleware** é uma verificação que roda **antes** da action de uma rota —
tipicamente pra checar autenticação. Se passar, a rota executa; se barrar, a
requisição para no middleware com a resposta que ele devolver.

```
@app.middleware()
action nome() {
    // request disponível aqui
    // `pass` libera a rota; `return jsonify(...), 401` barra
}
```

---

## Declarando

```
@app.middleware()
action verificar() {
    token = request.get("token")
    if (not token) {
        return jsonify({"msg": "sem permissão"}), 401   // BARRA
    }
    pass                                                 // LIBERA
}
```

- **`pass`** — deixa a requisição seguir pra action da rota.
- **`return jsonify(...), <status>`** — para aqui e devolve isso ao cliente
  (a action da rota nem roda).

---

## Aplicando numa rota

O middleware **não** roda automaticamente em tudo — você escolhe as rotas
protegidas passando `middleware=app.middleware`:

```
// rota livre — sem middleware
@app.route("/api/login", methods=cors.options(["POST"]))
action login() {
    return jsonify({"ok": true})
}

// rota protegida — o middleware roda primeiro
@app.route("/api/dados", methods=cors.options(["GET"]), middleware=app.middleware)
action dados() {
    return jsonify({"msg": "área protegida"})
}
```

Acessar `/api/dados` sem token → o middleware responde `401` e `dados()` nunca
roda. Com token válido → o middleware chega no `pass` e `dados()` executa
normalmente.

`middleware=` aceita duas formas, com o mesmo efeito:

| Forma | Quando usar |
|---|---|
| `middleware=app.middleware` | o único middleware da app, o do `@app.middleware()` |
| `middleware=nome_da_action` | uma action qualquer, quando você quer mais de um |

Nos dois casos a action é chamada **sem argumentos** — a requisição vem do
`request`, igual numa rota — e o que ela devolve decide:

| Devolve | Efeito |
|---|---|
| `pass` / nada / `null` | libera: a rota roda |
| `jsonify(...), <status>` (tupla) | barra: essa é a resposta |
| `JinkerResponse` | barra: essa é a resposta |

---

## Padrão típico: autenticação por token

```
@app.middleware()
action auth() {
    token = request.get("token")
    if (not token) {
        return jsonify({"erro": "não autorizado"}), 401
    }
    // aqui você validaria o token (jwt.check, consulta no banco, etc.)
    pass
}
```

---

## Relacionados

- [`route`](../route/route.md) — onde `middleware=` é aplicado
- [`request`](../request/request.md) — o middleware inspeciona a requisição
- lib `jwt` — para validar tokens de verdade no middleware
