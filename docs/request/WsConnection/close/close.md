# `WsConnection.close()`

Fecha a conexão WebSocket. Depois disso não dá pra enviar nem receber.

```
conn.close() -> None
```

---

## Uso

```
import request

conn = request.ws_connect("ws://localhost:8081/chat/geral")
conn.on_message(action(msg) { post(msg) })

# ... usa a conexão ...

conn.close()          # encerra quando terminar
```

---

## Exemplo: loop de chat até "sair"

```
conn = request.ws_connect("ws://localhost:8081/chat/geral")
conn.on_message(action(msg) { post(f"\n{msg}") })

while (true) {
    texto = input("")
    if (texto == "sair") {
        conn.close()
        break
    }
    conn.send({"author": "eu", "body": texto})
}
```

---

## Relacionados

- [`request.ws_connect()`](../../ws_connect/ws_connect.md) — abrir
- [`.send()`](../send/send.md) · [`.on_message()`](../on_message/on_message.md)
