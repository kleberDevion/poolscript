# jwt — Tokens de autenticação (JWT)

Lib pra criar e verificar **tokens JWT** — o crachá que o usuário carrega depois
de fazer login. Em vez de checar usuário/senha a cada requisição, você emite um
token no login e o cliente o apresenta nas próximas chamadas.

```
import jwt
```

| Membro | O que faz | Página |
|---|---|---|
| `jwt.gen(payload, secret)` | **cria** um token a partir de dados | [gen/gen.md](gen/gen.md) |
| `jwt.check(token, secret)` | **verifica** um token e devolve os dados | [check/check.md](check/check.md) |

---

## Como funciona (o modelo)

1. Usuário faz login (você valida com [`hash.check`](../hash/check/check.md)).
2. Você **gera** um token com `jwt.gen(...)` contendo o id do usuário e uma
   expiração, assinado com um **segredo** só seu.
3. O cliente guarda o token e o manda nas próximas requisições.
4. Você **verifica** com `jwt.check(...)` — se o token é válido e não expirou,
   devolve os dados; senão, `Null`.

O segredo (`secret`) é o que impede forjar um token. Guarde-o num
`.env`, nunca no código.

---

## Fluxo completo

```
import jwt
import date

# LOGIN — gera o token (expira em 24h)
payload = {
    "user_id": 42,
    "exp": date.timestamp() + date.hora(hours=24)
}
token = jwt.gen(payload, "meu_segredo")

# DEPOIS — verifica o token que o cliente mandou
dados = jwt.check(token, "meu_segredo")
if (dados is Null) {
    post("token inválido ou expirado")
} else {
    post("usuário:", dados["user_id"])
}
```

---

## Relacionados

- lib `hash` — validar a senha antes de emitir o token
- [`date.timestamp()`](../date/timestamp/timestamp.md) + [`date.hora()`](../date/hora/hora.md) — montar o `exp`
- [jinker/middleware](../jinker/middleware/middleware.md) — verificar o token nas rotas
