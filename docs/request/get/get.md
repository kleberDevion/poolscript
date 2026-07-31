# `request.get(url, headers=None, body=None, timeout=30)`

Faz uma requisição HTTP **GET** para uma URL. Devolve um
[`Response`](../Response/Response.md).

```
request.get(url, headers=None, body=None, timeout=30) -> Response
```

| Parâmetro | Padrão | O que é |
|---|---|---|
| `url` | — | endereço a chamar (ex: `"https://api.x.com/users"`) |
| `headers` | `None` | dict de cabeçalhos a enviar |
| `body` | `None` | corpo da requisição (dict vira JSON automaticamente) |
| `timeout` | `30` | segundos até desistir |

---

## Uso básico

```
import request

resp = request.get("https://api.exemplo.com/users")
post(resp.status)          // 200
dados = resp.get_json()    // corpo parseado (dict ou lista)
```

---

## Com query string

Coloque os parâmetros na própria URL:

```
resp = request.get("https://api.x.com/buscar?termo=poolscript&limite=10")
```

---

## Com headers (ex: token de autenticação)

```
resp = request.get(
    "https://api.x.com/privado",
    headers={"Authorization": "Bearer meu_token"}
)
```

Um `User-Agent` padrão é adicionado sozinho (evita `403` de sites que barram
requisições sem identificação) — você pode sobrescrever passando o seu.

---

## Tratando o resultado

`request.get` sempre devolve um `Response`, mesmo em erro HTTP (404, 500) —
cheque `.ok` ou `.status`:

```
resp = request.get("https://api.x.com/item/999")
if (resp.ok) {                       // true se status 2xx
    post(resp.get_json())
} else {
    post("falhou:", resp.status)     // ex: 404
}
```

**Falha de conexão** (host inexistente, sem rede) levanta erro em vez de
devolver `Response` — envolva em `try/catch` se precisar:

```
try {
    resp = request.get("https://host.que.nao.existe")
} catch (e) {
    post("sem conexão:", e)
}
```

---

## Relacionados

- [`Response`](../Response/Response.md) — o objeto devolvido (`.ok`, `.get_json()`, …)
- [`request.post()`](../post/post.md) — enviar dados
- [`request.delete()`](../delete/delete.md) — apagar
