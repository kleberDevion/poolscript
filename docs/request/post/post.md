# `request.post(url, headers=None, body=None, timeout=30, stream=false, max_size=None)`

Faz uma requisição HTTP **POST** — usada pra **enviar dados** (criar recursos,
fazer login, etc.). Devolve um [`Response`](../Response/Response.md).

```
request.post(url, headers=None, body=None, timeout=30, stream=false, max_size=None) -> Response
```

| Parâmetro | Padrão | O que é |
|---|---|---|
| `url` | — | endereço a chamar |
| `headers` | `None` | dict de cabeçalhos |
| `body` | `None` | o que enviar — **dict/lista viram JSON automaticamente** |
| `timeout` | `30` | segundos até desistir |
| `stream` | `false` | `true` = lê a resposta em pedaços de 64 KB com **teto de memória** — passa do teto, levanta erro em vez de engolir a RAM |
| `max_size` | `None` | o teto quando `stream=true`: bytes ou texto com unidade (`"500mb"`); `None` = 100 MB. Ignorado sem `stream` |

---

## Enviando JSON (o caso comum)

Passe um dict em `body` — ele é convertido pra JSON e o `Content-Type:
application/json` é definido sozinho:

```
import request

resp = request.post(
    "https://api.x.com/users",
    body={"nome": "ana", "email": "ana@email.com"}
)
post(resp.status)            // 201
post(resp.get_json())        // resposta do servidor
```

---

## Enviando texto ou outro formato

Se `body` for uma **string**, é enviada como está (sem virar JSON):

```
resp = request.post("https://api.x.com/webhook", body="texto cru")
```

---

## Com autenticação

```
resp = request.post(
    "https://api.x.com/protegido",
    headers={"Authorization": "Bearer token"},
    body={"acao": "salvar"}
)
```

---

## Os outros métodos que enviam dados

`put`, `patch` e `delete` têm a **mesma assinatura** e o mesmo comportamento de
`body` (dict → JSON). Muda só o método HTTP:

- [`put`](../put/put.md) — substituir um recurso inteiro
- [`patch`](../patch/patch.md) — atualizar parte de um recurso
- [`delete`](../delete/delete.md) — remover

---

## Relacionados

- [`Response`](../Response/Response.md) — o objeto devolvido
- [`request.get()`](../get/get.md) — buscar dados (detalha `.ok`, erros, headers)
