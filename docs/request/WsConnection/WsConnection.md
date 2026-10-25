# `WsConnection` — conexão WebSocket (cliente)

`WsConnection` é o que [`request.ws_connect(url)`](../ws_connect/ws_connect.md)
devolve: a conexão aberta com um servidor WebSocket. Por ela você **envia**
mensagens, **recebe** (via callback) e **fecha** quando termina.

---

## Métodos

| Método | O que faz | Página |
|---|---|---|
| `.on_message(callback)` | registra o que rodar a cada mensagem recebida | [on_message/on_message.md](on_message/on_message.md) |
| `.send(dados)` | envia uma mensagem pro servidor | [send/send.md](send/send.md) |
| `.close()` | fecha a conexão | [close/close.md](close/close.md) |

---

## Ciclo de vida

```ps sem-rodar: conecta num servidor WebSocket de fora
import request

conn = request.ws_connect("ws://chat.example.com/chat/geral")  # 1. conecta

conn.on_message(funct(msg) { post(msg) })                    # 2. receber (ANTES de esperar)

conn.send({"author": "ana", "body": "oi"})                   # 3. enviar

conn.close()                                                 # 4. fechar
```

A ordem importa: registre o `on_message` **antes** de mandar/esperar, senão as
primeiras mensagens que chegarem são perdidas.

---

## Relacionados

- [`request.ws_connect()`](../ws_connect/ws_connect.md) — cria a conexão
- [`sockets`](../../sockets/sockets.md) — o socket cru, pra um servidor próprio
