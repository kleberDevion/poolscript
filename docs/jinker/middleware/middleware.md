# `@app.middleware()` — verificação antes da rota

Um **middleware** é uma verificação que roda **antes** da action de uma rota —
tipicamente pra checar autenticação. Se passar, a rota executa; se barrar, a
requisição para no middleware com a resposta que ele devolver.

```
@app.middleware()
action nome() {
    // request disponível aqui
    // `continue` libera a rota; `return jsonify(...), 401` barra
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
    continue                                             // LIBERA
}
```

- **`continue`** — deixa a requisição seguir pra action da rota.
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
roda. Com token válido → o middleware faz `continue` e `dados()` executa
normalmente.

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
    continue
}
```

---

## Relacionados

- [`route`](../route/route.md) — onde `middleware=` é aplicado
- [`request`](../request/request.md) — o middleware inspeciona a requisição
- lib `jwt` — para validar tokens de verdade no middleware
