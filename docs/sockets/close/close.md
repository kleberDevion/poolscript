# `socket.close()`

Fecha o descritor e desliga o objeto. Depois disso o socket não serve pra mais
nada — os métodos que precisam do descritor recusam com
`[Errno 9] Bad file descriptor`.

## Parâmetros

Nenhum. `close(1)` é `close() takes no arguments (1 given)`.

## Retorno

**`Null`** — sempre, inclusive na segunda chamada.

## Erros

- **`TypeError`** — `close() takes no arguments (1 given)`

`close` é o único método do socket que **nunca** falha por causa do estado: em
socket já fechado ele não faz nada e devolve `Null` do mesmo jeito.

## Exemplos

```ps
import sockets
s = sockets.socket()
post(s.close())
post(s.close())        # de novo: continua Null
post(s.fileno())       # -1 = não tem mais descritor
s.close()
```

```saida
Null
Null
-1
```

O `using` fecha sozinho no fim do bloco:

```ps
import sockets
using sockets.socket() as s {
    s.bind(("127.0.0.1", 0))
    post(s.getsockname()[0])
}
```

```saida
127.0.0.1
```

Depois de fechado, o resto recusa:

```ps
import sockets
s = sockets.socket()
s.close()
try {
    s.recv(16)
} catch (OSError e) {
    post(str(e))
}
```

```saida
[Errno 9] Bad file descriptor
```

## Bordas

- **idempotente**: fechar duas vezes é legal e não levanta nada
- depois do `close`, [`fileno()`](../fileno/fileno.md) devolve `-1`,
  [`gettimeout()`](../gettimeout/gettimeout.md) continua respondendo e
  [`detach()`](../detach/detach.md) devolve `-1`
- em `AF_UNIX` o arquivo do caminho **continua no disco** depois do `close` —
  apagar é por sua conta
- o GC fecha o que sobrar, mas tarde; num servidor, feche cada conexão
  atendida na hora
- o socket que saiu de [`accept`](../accept/accept.md) e o que escuta são dois
  objetos: fechar um não fecha o outro

[← índice](../sockets.md)
