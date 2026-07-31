# `jwt.gen(payload, secret, algorithm="HS256")`

**Cria** um token JWT a partir de um dict de dados (`payload`), assinado com um
segredo. Devolve o token como string.

```
jwt.gen(payload: dict, secret: str, algorithm: str = "HS256") -> str
```

| Parâmetro | Padrão | O que é |
|---|---|---|
| `payload` | — | dict com os dados a guardar no token (id, expiração…) |
| `secret` | — | a **chave secreta** que assina o token (guarde no `.env`) |
| `algorithm` | `"HS256"` | algoritmo de assinatura |

---

## Uso

```
import jwt
import date

payload = {
    "user_id": 42,
    "exp": date.timestamp() + date.hora(hours=24)   // expira em 24h
}
token = jwt.gen(payload, "meu_segredo")
post(token)     // "eyJhbGciOi..." (string longa)
```

---

## O campo `exp` — expiração

Se você incluir `exp` (um timestamp) no payload, o [`jwt.check`](../check/check.md)
recusa o token automaticamente depois desse instante. É como você faz o token
"vencer":

```
"exp": date.timestamp() + date.hora(days=7)     // vale 7 dias
"exp": date.timestamp() + date.hora(hours=1)    // vale 1 hora
```

Sem `exp`, o token nunca expira (não recomendado).

---

## O que colocar no payload

O necessário pra identificar o usuário depois — tipicamente o id. **Não coloque
senha nem dados sensíveis**: o conteúdo do token é legível por quem o tem (só
não é *forjável* sem o segredo).

```
payload = {"user_id": 42, "papel": "admin", "exp": ...}
```

---

## Relacionados

- [`jwt.check()`](../check/check.md) — verificar o token depois
- [`date.timestamp()`](../../date/timestamp/timestamp.md) — montar o `exp`
