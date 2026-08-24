# `request.get(url, headers=None, body=None, timeout=30, stream=false, max_size=None, fields=None, file=None)`

Faz uma requisição HTTP **GET** para uma URL. Devolve um
[`Response`](../Response/Response.md).

```
request.get(url, headers=None, body=None, timeout=30, stream=false, max_size=None,
            fields=None, file=None) -> Response
```

| Parâmetro | Padrão | O que é |
|---|---|---|
| `url` | — | endereço a chamar (ex: `"https://api.x.com/users"`) |
| `headers` | `None` | dict de cabeçalhos a enviar |
| `body` | `None` | corpo da requisição (dict vira JSON automaticamente) |
| `timeout` | `30` | segundos até desistir |
| `stream` | `false` | `true` = lê o corpo em pedaços de 64 KB com **teto de memória** — passa do teto, levanta erro em vez de engolir a RAM (pra download grande) |
| `max_size` | `None` | o teto quando `stream=true`: bytes ou texto com unidade (`"500mb"`); `None` = 100 MB. Ignorado sem `stream` |
| `fields` / `file` | `None` | corpo **multipart/form-data** (formulário + arquivo) — ver a seção Multipart em [`request.post`](../post/post.md) |

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
