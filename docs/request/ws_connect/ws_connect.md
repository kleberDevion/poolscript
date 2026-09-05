# `request.ws_connect(url)`

Conecta como **cliente** num servidor WebSocket. Devolve uma
[`WsConnection`](../WsConnection/WsConnection.md) — a conexão aberta, por onde
você envia e recebe mensagens em tempo real.

```
request.ws_connect(url: str) -> WsConnection
```


| Parâmetro | O que é |
|---|---|
| `url` | endereço `ws://...` do servidor (ex: `"ws://localhost:8081/chat/geral"`) |

---

## Uso completo

```
import request

conn = request.ws_connect("ws://localhost:8081/chat/geral")

# 1. registre o que fazer com CADA mensagem que chegar
conn.on_message(action(msg) { post(msg) })

# 2. envie mensagens
conn.send({"author": "ana", "body": "oi"})

# 3. feche quando terminar
conn.close()
```

---

## ⚠️ O erro nº 1: esquecer o `on_message`

Sem `conn.on_message(...)`, as mensagens que o servidor manda **chegam e são
descartadas em silêncio** — nenhum erro, nada aparece. É o motivo mais comum
de "conectei mas não recebo nada". **Sempre** registre um callback antes de
esperar receber:

```
conn = request.ws_connect("ws://localhost:8081/chat/sala")
conn.on_message(action(msg) { post(f"chegou: {msg}") })   # sem isto, você não vê nada
```

Ver [`WsConnection.on_message`](../WsConnection/on_message/on_message.md).

---

## Do lado do servidor

O servidor WebSocket é feito com jinker (`@app.socket(...)`), que sobe na porta
**HTTP + 1**. Ver [jinker/socket](../../jinker/socket/socket.md).

---

## Relacionados

- [`WsConnection`](../WsConnection/WsConnection.md) — a conexão devolvida
- [`.on_message()`](../WsConnection/on_message/on_message.md) — receber (obrigatório)
- [jinker/socket](../../jinker/socket/socket.md) — o servidor do outro lado
