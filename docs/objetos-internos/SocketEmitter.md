<!-- gerado: gera_doc_gaps.py — pode regenerar -->
# `SocketEmitter`

> **Objeto interno da linguagem** — você não cria `SocketEmitter` na mão:
> é o TIPO de um objeto que a lib `jinker` te entrega pronto.
> Confira com `type(obj)`, que mostra exatamente este nome.

Emissor obtido em runtime via `app.socket()` (sem args), dentro de um
handler — usado pra mandar mensagens a uma sala específica ou broadcast.

Por padrão, quem disparou o handler (o remetente) NÃO recebe o próprio
emit de volta — evita a mensagem aparecer duplicada pro remetente.
Passe exclude_self=false pra ecoar de volta também pro remetente.

Uso:
    send = sk.socket()
    send.emit(payload=msg, room_id=identify["id"])
    state = send.status_send()

## Métodos e propriedades

| Acesso | O que faz |
|---|---|
| `.emit(payload=None, room_id=None, exclude_self=True)` |  |
| `.status_send()` |  |

[← índice](objetos-internos.md)
