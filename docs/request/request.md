# request — Cliente HTTP e WebSocket

A lib `request` é o **lado cliente**: você usa pra *chamar* outras APIs
(fazer `GET`/`POST` pra um servidor) e pra *conectar* como cliente num
WebSocket. **Zero dependências externas** nos dois casos.

```
import request
```

> **Não confunda** com o `request` de dentro de uma rota jinker. Aquele é o
> que **chegou** no *seu* servidor (ver [jinker/request](../jinker/request/request.md)).
> Este aqui é pra *você* chamar servidores dos outros.

---

## Requisições HTTP

| Método | O que faz | Página |
|---|---|---|
| `request.get(url, ...)` | requisição GET | [get/get.md](get/get.md) |
| `request.post(url, ...)` | requisição POST | [post/post.md](post/post.md) |
| `request.put(url, ...)` | requisição PUT | [put/put.md](put/put.md) |
| `request.patch(url, ...)` | requisição PATCH | [patch/patch.md](patch/patch.md) |
| `request.delete(url, ...)` | requisição DELETE | [delete/delete.md](delete/delete.md) |
| `request.head(url, ...)` | requisição HEAD — status/headers sem baixar o corpo | [head/head.md](head/head.md) |

Todas devolvem um [`Response`](Response/Response.md).

### Baixar arquivo (binário)

Pra baixar binário (`.exe`, imagem, zip, pdf), use os bytes crus e o `stream`:

| Recurso | O que faz |
|---|---|
| `resp.content` | bytes crus (não decodifica — não corrompe) |
| `resp.save(pasta_ou_caminho)` | grava em disco e devolve um [`PoolFile`](../os/PoolFile/PoolFile.md) (`.name`/`.size`/`.move()`) |
| `resp.decode('utf-8')` | corpo como texto num encoding específico |
| `stream=true` | baixa em pedaços; aborta com raise se passar de `max_size` (padrão 100MB) |
| `resp.content_type('...')` | valida o MIME; raise se não bater |

Ver a página de [download](download/download.md) e [`Response`](Response/Response.md).

## WebSocket (cliente)

| Membro | O que faz | Página |
|---|---|---|
| `request.ws_connect(url)` | conecta como cliente num WebSocket | [ws_connect/ws_connect.md](ws_connect/ws_connect.md) |
| `WsConnection` | a conexão devolvida (`.send`/`.on_message`/`.close`) | [WsConnection/WsConnection.md](WsConnection/WsConnection.md) |

---

## Exemplo rápido — HTTP

```
import request

resp = request.get("https://api.exemplo.com/users")
if (resp.ok) {
    dados = resp.get_json()      # corpo parseado como dict/lista
    post(dados)
}

resp2 = request.post(
    "https://api.exemplo.com/users",
    body={"nome": "ana", "email": "ana@email.com"}
)
post(resp2.status)               # 201, por exemplo
```

## Exemplo rápido — WebSocket

```
import request

conn = request.ws_connect("ws://localhost:8081/chat/geral")
conn.on_message(action(msg) { post(msg) })   # OBRIGATÓRIO pra ver o que chega
# O uso de uma action/reaction dentro dos () do modulo e opcional, pois ele devolve sozinho
conn.send({"author": "ana", "body": "oi"})
```
