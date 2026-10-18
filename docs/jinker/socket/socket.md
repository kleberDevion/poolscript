# `@app.socket(path=Null, channel=False)`

Registra um handler de **WebSocket** — conexão persistente de mão-dupla, boa
pra chat, notificações, dados ao vivo. A `funct` logo abaixo roda
**a cada mensagem recebida** naquela conexão.

Os exemplos desta página pressupõem (`path` é o caminho do socket; `msg` e
`sala` são o que chega num handler, usados fora dele nos trechos curtos):

```ps contexto
from jinker import Jinker
Jinker app = Jinker(__name__)
str path = "/chat/<sala>"
str sala = "geral"
dict msg = {"author": "ana", "body": "oi"}
```

```ps
@app.socket(path, channel=True)
funct nome() {
    # request disponível; roda a cada mensagem que chega
}
```

O servidor WebSocket sobe automaticamente na porta **HTTP + 1** (se o HTTP é `8080`, o WS é `8081`).

**O handler cede, como a rota.** Cada mensagem é atendida numa fibra própria:
`sleep`, banco, `request`, mail e `os.run` esperam sem segurar o servidor. Um
handler que demora 3 segundos não impede as rotas HTTP de responderem nem
outra conexão de entrar. Enquanto uma mensagem da MESMA conexão está sendo
atendida, a seguinte espera a vez — a ordem de quem mandou é preservada.

---

## Salas automáticas por parâmetro dinâmico

Se o caminho tem parâmetro (`/chat/<sala>`), cada conexão entra automaticamente
na **sala** = valor do parâmetro. `/chat/geral` → sala `"geral"`.

```ps
@app.socket("/chat/<sala>", channel=True)
funct mensagem() {
    sala = request.path_param("sala")     # "geral"
    msg  = request.get_json()             # a mensagem que chegou (dict)

    # reenvia pra todo mundo na MESMA sala
    send = app.socket()
    send.emit(payload=msg, room_id=sala, exclude_self=False)
}
```

Path que **não** bate com nenhum `@app.socket` recebe uma resposta HTTP
`404` normal (`{"error": true, "code": 404, "message": "rota não encontrada:
GET /x"}`) e a conexão fecha — o handshake nunca acontece, como a RFC 6455
manda para recusa.

**Origem.** O handshake confere o `Origin` com a lista de
`cors(app, origins=[...])`, a mesma regra das rotas: site fora da lista recebe
`403` (`{"error": true, "code": 403, "message": "Origem não autorizada:
https://outro.com"}`, sem `Access-Control-Allow-Origin`) e a conexão fecha.
No modo padrão origem local e cliente sem `Origin` (um programa, outro
backend) passam; com `cors(app, ..., local=False)` só a lista vale, e sem
`Origin` é `403`. Sem `cors(app, ...)`, qualquer origem abre o socket. Ver
[`cors`](../cors/cors.md).

---

## Enviando mensagens: `app.socket()` (emissor)

Dentro do handler, `app.socket()` (sem argumentos) devolve um **emissor**:

```ps
send = app.socket()
send.emit(payload=msg, room_id=sala)                    # manda pra sala
send.emit(payload=msg, room_id=sala, exclude_self=False) # inclui o remetente
state = send.status_send()                               # status do envio
```

- `payload` — o que enviar (dict vira JSON automaticamente);
- `room_id` — a sala alvo (omitir = broadcast pra todos);
- `exclude_self` — por padrão `True` (não ecoa pro remetente); `False` inclui.

Também dá pra chamar direto: `app.socket.emit(payload=msg, room_id=sala)`.

---

## Respondendo só a quem mandou: o `return` do handler

O que o handler **devolve** volta pro remetente daquela mensagem — e só pra ele.
String sai crua; dict/lista viram JSON.

```ps
@app.socket("/eco")
funct eco() {
    return "recebi: " + request.text()      # volta só pra quem mandou
}
```

É o único jeito de responder num socket **sem** `channel=True`, porque o `emit`
só alcança conexão que está no canal. Devolver `Null` (ou não devolver nada)
não manda coisa alguma.

Cuidado com o eco dobrado: se o handler faz `emit(..., exclude_self=False)`
**e** devolve algo, o remetente recebe as duas mensagens.

---

## Sem "hook" de conectar/desconectar

O handler só reage a **mensagem recebida** — não há callback separado de
"cliente conectou" / "cliente saiu". Se você quer avisar a sala que alguém
entrou, o próprio cliente manda uma mensagem de "entrou" e o handler
reage a ela.

---

## Cliente WebSocket em Jinga

Do outro lado, conecta com `request.ws_connect` (lib `request`) e registra
`on_message` pra ver o que chega:

```ps
import request

conn = request.ws_connect("ws://localhost:8081/chat/geral")
conn.on_message(funct(msg) { post(msg) })   # OBRIGATÓRIO pra ver mensagens
conn.send({"author": "ana", "body": "oi"})
```

> Sem `on_message`, as mensagens que chegam são descartadas em silêncio — é o
> erro mais comum. Ver a doc da lib `request`.

---

## Relacionados

- [`channel`](../channel/channel.md) — o outro jeito de emitir (broadcast/sala)
- [`request.path_param()`](../request/path_param/path_param.md) — pega a sala do caminho
- [`route`](../route/route.md) — o equivalente pra HTTP
