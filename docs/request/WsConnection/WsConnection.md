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

```
import request

conn = request.ws_connect("ws://localhost:8081/chat/geral")  # 1. conecta

conn.on_message(action(msg) { post(msg) })                   # 2. receber (ANTES de esperar)

conn.send({"author": "ana", "body": "oi"})                   # 3. enviar

conn.close()                                                 # 4. fechar
```

A ordem importa: registre o `on_message` **antes** de mandar/esperar, senão as
primeiras mensagens que chegarem são perdidas.

---

## Relacionados

- [`request.ws_connect()`](../ws_connect/ws_connect.md) — cria a conexão
- [jinker/socket](../../jinker/socket/socket.md) — o servidor do outro lado
