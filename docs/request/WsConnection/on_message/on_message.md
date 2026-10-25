# `WsConnection.on_message(callback)`

Registra uma função que roda **a cada mensagem que o servidor manda**. É como
você *recebe* dados num WebSocket.

```assinatura
conn.on_message(callback) -> Null
```

| Parâmetro | O que é |
|---|---|
| `callback` | uma função que recebe a mensagem (`funct(msg) { ... }`) |

Os exemplos desta página pressupõem:

```ps contexto
import request
WsConnection conn = request.ws_connect("ws://chat.example.com/chat/sala")
```

---

## Sem isto, você não recebe NADA

Este é o ponto mais importante da lib. Um WebSocket recebe mensagens numa
thread em segundo plano. Se você **não** registrou um `on_message`, essas
mensagens chegam e são **descartadas em silêncio** — nenhum erro, nada
impresso. É o motivo nº 1 de "conectei mas não recebo nada".

```ps sem-rodar: conecta num servidor WebSocket de fora
conn = request.ws_connect("ws://chat.example.com/chat/sala")

# SEM esta linha, as mensagens somem sem aviso:
conn.on_message(funct(msg) { post(msg) })
```

---

## O callback

A função recebe **um** argumento: a mensagem. Se o servidor mandou JSON, ela
já chega como dict; senão, chega como string.

```ps sem-rodar: conecta num servidor WebSocket de fora
conn.on_message(funct(msg) {
    autor = msg["author"]
    corpo = msg["body"]
    post(f"{autor}: {corpo}")
})
```

Você também pode passar uma `funct` nomeada:

```ps sem-rodar: conecta num servidor WebSocket de fora
funct aoReceber(msg) {
    post(f"chegou: {msg}")
}
conn.on_message(aoReceber)
```

---

## Registre ANTES de esperar

Registre o `on_message` logo depois de conectar, antes de `send` ou de entrar
num loop de espera — senão as primeiras mensagens que chegarem se perdem.

---

## Relacionados

- [`request.ws_connect()`](../../ws_connect/ws_connect.md) — abrir a conexão
- [`.send()`](../send/send.md) — enviar
- [`.close()`](../close/close.md) — fechar
