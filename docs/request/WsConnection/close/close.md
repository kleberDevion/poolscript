# `WsConnection.close()`

Fecha a conexão WebSocket. Depois disso não dá pra enviar nem receber.

```assinatura
conn.close() -> Null
```

Os exemplos desta página pressupõem:

```ps contexto
import request
```

---

## Uso

```ps sem-rodar: conecta num servidor WebSocket de fora
import request

conn = request.ws_connect("ws://chat.example.com/chat/geral")
conn.on_message(funct(msg) { post(msg) })

# ... usa a conexão ...

conn.close()          # encerra quando terminar
```

---

## Exemplo: loop de chat até "sair"

```ps sem-rodar: conecta num servidor WebSocket de fora
conn = request.ws_connect("ws://chat.example.com/chat/geral")
conn.on_message(funct(msg) { post(f"\n{msg}") })

while (True) {
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
