# `jwt.check(token, secret)`

**Verifica** um token JWT: confere a assinatura e a expiração. Se estiver
válido, devolve o `payload` (os dados guardados). Se for inválido, adulterado
ou **expirado**, devolve `Null`.

```assinatura
jwt.check(token: str, secret: str) -> dict | Null
```

| Parâmetro | O que é |
|---|---|
| `token` | o token que o cliente mandou |
| `secret` | o **mesmo segredo** usado no [`jwt.gen`](../gen/gen.md) |

Os exemplos desta página pressupõem:

```ps contexto
import jwt
import os
from jinker import Jinker, jsonify
Jinker app = Jinker(__name__)
str token = jwt.gen({"user_id": 42}, "meu_segredo")
```

---

## Uso

```ps
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
isso que impede forjar tokens: sem o segredo não se cria um token que passe
no check.

---

## Exemplo: middleware protegendo rotas (jinker)

```ps
@app.middleware()
funct auth() {
    token = request.get("token")
    dados = jwt.check(token, os.getenv("JWT_SECRET"))
    if (dados is Null) {
        return jsonify({"erro": "não autorizado"}), 401
    }
    pass    # token válido → libera a rota
}
```

---

## Relacionados

- [`jwt.gen()`](../gen/gen.md) — criar o token
- [jinker/middleware](../../jinker/middleware/middleware.md) — usar o check pra proteger rotas
