# `jwt.check(token, secret)`

**Verifica** um token JWT: confere a assinatura e a expiração. Se estiver
válido, devolve o `payload` (os dados guardados). Se for inválido, adulterado
ou **expirado**, devolve `Null`.

```
jwt.check(token: str, secret: str) -> dict | Null
```

| Parâmetro | O que é |
|---|---|
| `token` | o token que o cliente mandou |
| `secret` | o **mesmo segredo** usado no [`jwt.gen`](../gen/gen.md) |

---

## Uso

```
import jwt

dados = jwt.check(token, "meu_segredo")

if (dados is Null) {
    post("token inválido ou expirado")
} else {
    id = dados["user_id"]
    post("usuário autenticado:", id)
}
```

**Sempre cheque `is Null`** antes de usar — é assim que você sabe se o token
presta. Um token expirado ou com assinatura errada vira `Null`, não erro.

---

## O segredo tem que ser o mesmo

`jwt.check` só valida se o `secret` for **idêntico** ao que gerou o token. É
isso que impede forjar tokens: sem o segredo, ninguém cria um token que passe
no check.

---

## Exemplo: middleware protegendo rotas (jinker)

```
@app.middleware()
action auth() {
    token = request.get("token")
    dados = jwt.check(token, os.getenv("JWT_SECRET"))
    if (dados is Null) {
        return jsonify({"erro": "não autorizado"}), 401
    }
    continue    // token válido → libera a rota
}
```

---

## Relacionados

- [`jwt.gen()`](../gen/gen.md) — criar o token
- [jinker/middleware](../../jinker/middleware/middleware.md) — usar o check pra proteger rotas
