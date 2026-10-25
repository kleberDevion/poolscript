# `WsConnection.send(data)`

Envia uma mensagem pro servidor WebSocket.

```assinatura
conn.send(data) -> Null
```

| Parâmetro | O que é |
|---|---|
| `data` | o que enviar — **dict/lista viram JSON automaticamente**; string vai como está |

---

## Uso

```ps sem-rodar: conecta num servidor WebSocket de fora
import request

conn = request.ws_connect("ws://chat.example.com/chat/geral")
conn.on_message(funct(msg) { post(msg) })

conn.send({"author": "ana", "body": "oi pessoal"})   # dict → JSON
conn.send("mensagem simples")                        # string crua
```

---

## Precisa estar conectado

`send` só funciona com a conexão aberta. Se a conexão ainda não subiu ou já foi
fechada, a mensagem não é enviada (e um aviso aparece). Registre o `on_message`
e envie depois de conectar.

---

## Relacionados

- [`.on_message()`](../on_message/on_message.md) — receber (obrigatório pra ver respostas)
- [`.close()`](../close/close.md) — fechar
- [`request.ws_connect()`](../../ws_connect/ws_connect.md) — abrir
