# `request.put(url, headers=Null, body=Null, timeout=30, stream=False, max_size=Null, fields=Null, file=Null, save=Null)`

Faz uma requisição HTTP **PUT** — usada pra **substituir um recurso inteiro**.
Mesma assinatura e comportamento de [`request.post`](../post/post.md); muda só
o método HTTP. Devolve um [`Response`](../Response/Response.md).

```assinatura
request.put(url, headers=Null, body=Null, timeout=30, stream=False, max_size=Null,
            fields=Null, file=Null, save=Null) -> Response
```

---

## Uso

```ps sem-rodar: faz pedido HTTP a um servidor de fora
import request

# substitui TODOS os dados do usuário 10
resp = request.put(
    "https://api.x.com/users/10",
    body={"nome": "ana", "email": "ana@nova.com", "idade": 31}
)
post(resp.status)
```

`body` dict vira JSON automaticamente, igual ao `post`.

---

## PUT vs PATCH

- **PUT** — manda o recurso **completo** (o servidor troca tudo).
- **[PATCH](../patch/patch.md)** — manda só os campos que **mudaram**.

---

## Relacionados

- [`request.post()`](../post/post.md) — detalha o envio de `body`
- [`request.patch()`](../patch/patch.md) — atualização parcial
- [`Response`](../Response/Response.md) — o objeto devolvido
