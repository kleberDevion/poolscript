# `socket.getpeername()`

O endereço do **outro lado** da conexão — quem está do outro fio. Só existe em
socket conectado.

## Parâmetros

Nenhum. `getpeername(1)` é `getpeername() takes no arguments (1 given)`.

## Retorno

O mesmo formato do [`getsockname()`](../getsockname/getsockname.md), só que do
lado de lá:

| família | devolve | forma |
|---|---|---|
| `AF_INET` | **`tup`** de 2 | `(str host, int porta)` |
| `AF_INET6` | **`tup`** de 4 | `(str host, int porta, int flowinfo, int scope_id)` |
| `AF_UNIX` | **`str`** | o caminho do arquivo |

É o mesmo valor que o [`accept()`](../accept/accept.md) já entrega na posição
`[1]` da tupla dele — quem aceitou a conexão não precisa perguntar de novo.

## Erros

- **`TypeError`** — `getpeername() takes no arguments (1 given)`
- **`RuntimeError`** — socket sem conexão:
  `[Errno 107] Transport endpoint is not connected`. Ao contrário do
  `getsockname()`, aqui **não** existe resposta neutra
- **`OSError`** — socket fechado: `[Errno 9] Bad file descriptor`

## Exemplos

```ps
import sockets
srv = sockets.socket()
srv.bind(("127.0.0.1", 0))
srv.listen(1)
porta = srv.getsockname()[1]

cli = sockets.socket()
cli.connect(("127.0.0.1", porta))
par = cli.getpeername()
post(type(par), par[0], par[1] == porta)
cli.close(); srv.close()
```

```saida
tup 2
127.0.0.1 True
```

Sem conexão, levanta:

```ps
import sockets
s = sockets.socket()
try {
    s.getpeername()
} catch (e) {
    post("Errno 107" in str(e))
}
s.close()
```

```saida
True
```

## Bordas

- num socket que **escuta** não há par: `getpeername()` no socket do
  `listen` é `[Errno 107]`; quem tem par é o socket que saiu do `accept`
- em UDP só responde depois de um `connect` — sem ele, o remetente vem no
  `[1]` do [`recvfrom`](../recvfrom/recvfrom.md), pacote a pacote
- depois de `shutdown` a conexão ainda existe e o par continua legível; depois
  de `close` não

[← índice](../sockets.md)
