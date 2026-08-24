# `request.head(url, headers=None, timeout=30)`

Faz uma requisição HTTP **HEAD** — o servidor devolve o **status e os
cabeçalhos** que devolveria num GET, mas **sem o corpo**. Serve pra checar se
algo existe, o tamanho (`Content-Length`) ou o tipo (`Content-Type`) **sem
baixar o conteúdo**. Devolve um [`Response`](../Response/Response.md) com
`.content` vazio.

```
request.head(url, headers=None, timeout=30) -> Response
```

| Parâmetro | Padrão | O que é |
|---|---|---|
| `url` | — | endereço a checar |
| `headers` | `None` | dict de cabeçalhos a enviar |
| `timeout` | `30` | segundos até desistir |

HEAD não tem corpo de requisição — por isso não existe `body=` aqui.

---

## Uso

```
import request

r = request.head("https://exemplo.com/arquivo.zip")
if (r.ok) {
    post("existe;", r.headers["Content-Length"], "bytes")
} else {
    post("não existe:", r.status)
}
```

O corpo vem **vazio de verdade** (`len(r.content)` → `0`), mesmo que o
`Content-Length` anuncie o tamanho do recurso — é assim que o HEAD funciona:
o header diz quanto o GET traria.

---

## HEAD vs GET

| | `head()` | `get()` |
|---|---|---|
| status + headers | sim | sim |
| corpo baixado | **não** | sim |
| uso típico | "existe? qual o tamanho/tipo?" | buscar o conteúdo |

Pra baixar algo grande com teto de memória, o par certo é
[`get(stream=true, max_size=...)`](../get/get.md).

---

## Do outro lado: servindo HEAD

Um servidor [`jinker`](../../jinker/jinker.md) responde HEAD sozinho em toda
rota que aceita GET — mesmos headers, sem corpo. Ver
[`@app.route`](../../jinker/route/route.md).

---

## Relacionados

- [`Response`](../Response/Response.md) — o objeto devolvido (`.status`, `.ok`, `.headers`)
- [`request.get()`](../get/get.md) — buscar o conteúdo de verdade
- [`@app.route`](../../jinker/route/route.md) — o lado servidor do HEAD

[← índice](../request.md)
