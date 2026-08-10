# `SocketNamespace`

app.socket — dupla função:

1. Decorator de registro:  @sk.socket("/sala:id", channel=true)
2. Emissor em runtime, com ou sem instância:
     send = sk.socket()               # instancia um emissor
     send.emit(payload=msg, room_id=identify["id"])
     state = send.status_send()

     sk.socket.emit(payload=msg, room_id=identify["id"])  # direto, sem instanciar

## Métodos e propriedades

| Acesso | O que faz |
|---|---|
| `.emit(payload=None, room_id=None, exclude_self=True)` |  |
| `.status_send()` |  |

[← índice](../jinker.md)
