# `request.delete(url, headers=None, body=None, timeout=30)`

Faz uma requisição HTTP **DELETE** — usada pra **remover** um recurso. Mesma
assinatura de [`request.post`](../post/post.md). Devolve um
[`Response`](../Response/Response.md).

```
request.delete(url, headers=None, body=None, timeout=30) -> Response
```

---

## Uso

```
import request

resp = request.delete("https://api.x.com/users/10")
if (resp.ok) {
    post("removido")
} else {
    post("falhou:", resp.status)
}
```

`headers` e `body` funcionam igual aos outros métodos (a maioria dos DELETE não
usa `body`, mas dá pra passar).

---

## Relacionados

- [`request.get()`](../get/get.md) — buscar (detalha `.ok`, erros)
- [`Response`](../Response/Response.md) — o objeto devolvido
