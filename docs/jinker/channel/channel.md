# `app.channel` — enviar pros WebSockets conectados

`app.channel` gerencia as conexões WebSocket abertas e envia mensagens pra
elas, com suporte a **salas**. É o "megafone" do servidor pros clientes.

---

## Broadcast pra todos

```
app.channel(forAll=msg)      // manda `msg` pra TODAS as conexões abertas
```

## Enviar pra uma sala específica

```
app.channel.emit(msg, room_id="geral")   // só quem está na sala "geral"
```

## Status do último envio

```
if (app.channel.status == "Success") {
    post("mensagem enviada")
} else {
    post("falha no envio")
}
```

`Success` significa que a mensagem **chegou a pelo menos uma conexão**. Emit que
não alcança ninguém é `Error` — inclusive quando não há nenhuma conexão aberta,
ou quando as que existem não estão no canal.

> **A armadilha mais comum:** `@app.socket("/chat")` **sem** `channel=true` não
> põe a conexão no broadcast. O cliente conecta, o handler roda, e todo `emit`
> devolve `Error` porque não há alvo. Com `debug=true` o servidor diz isso em
> texto: `emit sem alvo: N conexao(oes) aberta(s), nenhuma no canal`.

---

## `app.channel` vs `app.socket()`

Os dois emitem, com uma diferença de padrão:

| | `app.channel` | `app.socket()` (emissor) |
|---|---|---|
| Broadcast | `app.channel(forAll=msg)` | `emit(payload=msg)` |
| Pra sala | `app.channel.emit(msg, room_id="x")` | `emit(payload=msg, room_id="x")` |
| Ecoa pro remetente? | **sim** (manda pra todos) | **não** por padrão (`exclude_self=true`) |

Use `app.socket()` dentro do handler quando **não** quer que o remetente receba
o próprio eco; use `app.channel` quando quer mandar pra todos, incluindo quem
disparou (ou quando emite de fora de um handler).

---

## Exemplo dentro de um socket

```
@app.socket("/chat/<sala>", channel=true)
reaction mensagem() {
    sala = request.path_param("sala")
    msg  = request.get_json()
    app.channel.emit(msg, room_id=sala)     // reenvia pra sala inteira
}
```

---

## Como uma conexão entra numa sala

Automático: se o caminho do socket tem parâmetro (`/chat/<sala>`), cada conexão
entra na sala igual ao valor daquele parâmetro. Ver [`socket`](../socket/socket.md).

---

## Relacionados

- [`socket`](../socket/socket.md) — registrar o handler de WebSocket
- [`request.path_param()`](../request/path_param/path_param.md) — a sala vem daqui
