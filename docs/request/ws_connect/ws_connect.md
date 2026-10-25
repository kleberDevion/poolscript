# `request.ws_connect(url)`

Conecta como **cliente** num servidor WebSocket. Devolve uma
[`WsConnection`](../WsConnection/WsConnection.md) — a conexão aberta, por onde
você envia e recebe mensagens em tempo real.

```assinatura
request.ws_connect(url: str) -> WsConnection
```


| Parâmetro | O que é |
|---|---|
| `url` | endereço `ws://...` do servidor (ex: `"ws://chat.example.com/chat/geral"`) |

Os exemplos desta página pressupõem:

```ps contexto
import request
```

---

## Uso completo

```ps sem-rodar: conecta num servidor WebSocket de fora
import request

conn = request.ws_connect("ws://chat.example.com/chat/geral")

# 1. registre o que fazer com CADA mensagem que chegar
conn.on_message(funct(msg) { post(msg) })

# 2. envie mensagens
conn.send({"author": "ana", "body": "oi"})

# 3. feche quando terminar
conn.close()
```

---

## O erro nº 1: esquecer o `on_message`

Sem `conn.on_message(...)`, as mensagens que o servidor manda **chegam e são
descartadas em silêncio** — nenhum erro, nada aparece. É o motivo mais comum
de "conectei mas não recebo nada". **Sempre** registre um callback antes de
esperar receber:

```ps sem-rodar: conecta num servidor WebSocket de fora
conn = request.ws_connect("ws://chat.example.com/chat/sala")
conn.on_message(funct(msg) { post(f"chegou: {msg}") })   # sem isto, você não vê nada
```

Ver [`WsConnection.on_message`](../WsConnection/on_message/on_message.md).

---

## Do lado do servidor

O servidor do outro lado é de fora: a jinker não tem mais servidor de
WebSocket (saiu no 19.0.0). Pra um servidor de rede próprio, a lib
[`sockets`](../../sockets/sockets.md) dá o socket cru.

---

## Relacionados

- [`WsConnection`](../WsConnection/WsConnection.md) — a conexão devolvida
- [`.on_message()`](../WsConnection/on_message/on_message.md) — receber (obrigatório)
- [`sockets`](../../sockets/sockets.md) — o socket cru, pra um servidor próprio
