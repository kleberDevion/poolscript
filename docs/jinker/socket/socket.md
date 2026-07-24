# `@app.socket(caminho, channel=true)`

Registra um handler de **WebSocket** — conexão persistente de mão-dupla, boa
pra chat, notificações, dados ao vivo. A `reaction`/`action` logo abaixo roda
**a cada mensagem recebida** naquela conexão.

```
@app.socket(caminho, channel=true)
reaction nome() {
    // request disponível; roda a cada mensagem que chega
}
```

Precisa de `pip install websockets`. O servidor WebSocket sobe automaticamente
na porta **HTTP + 1** (se o HTTP é `8080`, o WS é `8081`).

---

## Salas automáticas por parâmetro dinâmico

Se o caminho tem parâmetro (`/chat/<sala>`), cada conexão entra automaticamente
na **sala** = valor do parâmetro. `/chat/geral` → sala `"geral"`.

```
@app.socket("/chat/<sala>", channel=true)
reaction mensagem() {
    sala = request.path_param("sala")     // "geral"
    msg  = request.get_json()             // a mensagem que chegou (dict)

    // reenvia pra todo mundo na MESMA sala
    send = app.socket()
    send.emit(payload=msg, room_id=sala, exclude_self=false)
}
```

---

## Enviando mensagens: `app.socket()` (emissor)

Dentro do handler, `app.socket()` (sem argumentos) devolve um **emissor**:

```
send = app.socket()
send.emit(payload=msg, room_id=sala)                    // manda pra sala
send.emit(payload=msg, room_id=sala, exclude_self=false) // inclui o remetente
state = send.status_send()                               // status do envio
```

- `payload` — o que enviar (dict vira JSON automaticamente);
- `room_id` — a sala alvo (omitir = broadcast pra todos);
- `exclude_self` — por padrão `true` (não ecoa pro remetente); `false` inclui.

Também dá pra chamar direto: `app.socket.emit(payload=msg, room_id=sala)`.

---

## Sem "hook" de conectar/desconectar

O handler só reage a **mensagem recebida** — não há callback separado de
"cliente conectou" / "cliente saiu". Se você quer avisar a sala que alguém
entrou, o próprio cliente manda uma mensagem de "entrou" e o handler
reage a ela.

---

## Cliente WebSocket em PoolScript

Do outro lado, conecta com `request.ws_connect` (lib `request`) e registra
`on_message` pra ver o que chega:

```
import request

conn = request.ws_connect("ws://localhost:8081/chat/geral")
conn.on_message(action(msg) { post(msg) })   // OBRIGATÓRIO pra ver mensagens
conn.send({"author": "ana", "body": "oi"})
```

> Sem `on_message`, as mensagens que chegam são descartadas em silêncio — é o
> erro mais comum. Ver a doc da lib `request`.

---

## Relacionados

- [`channel`](../channel/channel.md) — o outro jeito de emitir (broadcast/sala)
- [`request.path_param()`](../request/path_param/path_param.md) — pega a sala do caminho
- [`route`](../route/route.md) — o equivalente pra HTTP
