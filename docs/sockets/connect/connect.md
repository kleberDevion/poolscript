# `socket.connect(address)`

Conecta o socket a um endereço **remoto**. É o lado cliente: depois que volta
sem erro, dá pra [`send`](../send/send.md) e [`recv`](../recv/recv.md).

## Parâmetros

| nome | tipo | default | nota |
|---|---|---|---|
| `address` | tup | — | `AF_INET`: `(str host, int porta)`. `AF_INET6`: `(str host, int porta, int flowinfo, int scope_id)`. `AF_UNIX`: **`str`** com o caminho |

O host pode ser IP numérico ou nome — a resolução é do sistema, e acontece
dentro do próprio `connect`. Uma `list` vale onde a `tup` vale.

## Retorno

**`Null`** — sempre. `connect` informa o resultado **levantando erro**; quem
quiser o código numérico em vez da exceção usa
[`connect_ex`](../connect_ex/connect_ex.md), que devolve `int`.

## Erros

- **`TypeError`** — `connect() takes exactly one argument (0 given)`
- **`TypeError`** — endereço na forma errada:
  `connect(): AF_INET address must be tuple, not str`; porta não-`int` dá
  `'str' object cannot be interpreted as an integer`
- **`RuntimeError`** — `timed out`, quando o prazo do
  [`settimeout`](../settimeout/settimeout.md) estoura antes de completar
- **`RuntimeError`** — recusa/falha do sistema:
  `[Errno 111] Connection refused`, `[Errno 113] No route to host`
- **`RuntimeError`** — nome que não resolve:
  `[Errno -5] No address associated with hostname`
- **`OSError`** — socket fechado: `[Errno 9] Bad file descriptor`

## Exemplos

```ps
import sockets
srv = sockets.socket()
srv.bind(("127.0.0.1", 0))
srv.listen(1)

cli = sockets.socket()
cli.settimeout(5)
post(cli.connect(("127.0.0.1", srv.getsockname()[1])))
post(cli.getpeername()[0])
cli.close(); srv.close()
```

```saida
Null
127.0.0.1
```

Recusa vira erro capturável:

```ps
import sockets
s = sockets.socket()
s.settimeout(2)
try {
    s.connect(("127.0.0.1", 1))
} catch (e) {
    post("Errno 111" in str(e))
}
s.close()
```

```saida
True
```

## Bordas

- com **prazo** (`settimeout(t)`) o motor usa conexão não-bloqueante e espera
  até `t` segundos; sem prazo, espera o que o sistema quiser esperar
- num socket UDP (`SOCK_DGRAM`) `connect` não abre conexão nenhuma: só fixa o
  destino padrão, e aí `send`/`recv` passam a valer
- conectar duas vezes o mesmo socket é recusado pelo sistema
- no VM em C a espera **cede a fibra** — um `connect` lento não trava os outros
  handlers do jinker

[← índice](../sockets.md)
