# `request.patch(url, headers=Null, body=Null, timeout=30, stream=False, max_size=Null, fields=Null, file=Null, save=Null)`

Faz uma requisição HTTP **PATCH** — usada pra **atualizar parte** de um recurso
(só os campos que mudaram). Mesma assinatura de
[`request.post`](../post/post.md). Devolve um [`Response`](../Response/Response.md).

```assinatura
request.patch(url, headers=Null, body=Null, timeout=30, stream=False, max_size=Null,
              fields=Null, file=Null, save=Null) -> Response
```

---

## Uso

```ps sem-rodar: faz pedido HTTP a um servidor de fora
import request

# muda SÓ o email do usuário 10, o resto fica como está
resp = request.patch(
    "https://api.x.com/users/10",
    body={"email": "ana@nova.com"}
)
post(resp.status)
```

---

## PATCH vs PUT

- **PATCH** — só os campos a mudar.
- **[PUT](../put/put.md)** — o recurso inteiro.

---

## Relacionados

- [`request.put()`](../put/put.md) — substituição completa
- [`request.post()`](../post/post.md) — detalha o envio de `body`
- [`Response`](../Response/Response.md) — o objeto devolvido
