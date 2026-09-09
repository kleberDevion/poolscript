# `socket.getsockname()`

O endereço **deste lado** — o que o [`bind`](../bind/bind.md) prendeu, ou o que
o sistema escolheu sozinho no `connect`. É por aqui que se descobre a porta
quando você pediu `0`.

## Parâmetros

Nenhum. `getsockname(1)` é `getsockname() takes no arguments (1 given)`.

## Retorno

O formato depende da **família** do socket:

| família | devolve | forma |
|---|---|---|
| `AF_INET` | **`tup`** de 2 | `(str host, int porta)` — ex.: `('127.0.0.1', 37867)` |
| `AF_INET6` | **`tup`** de 4 | `(str host, int porta, int flowinfo, int scope_id)` — ex.: `('::1', 52249, 0, 0)` |
| `AF_UNIX` | **`str`** | o caminho do arquivo — ex.: `'/tmp/meu.sock'` |

O host vem sempre **numérico**, mesmo que você tenha prendido por nome:
`bind(("localhost", 0))` depois lê `'127.0.0.1'`.

É a mesma forma que o [`bind`](../bind/bind.md) e o
[`connect`](../connect/connect.md) recebem — o valor volta cru pra eles.

## Erros

- **`TypeError`** — `getsockname() takes no arguments (1 given)`
- **`OSError`** — socket fechado: `[Errno 9] Bad file descriptor`

Num socket ainda **sem** `bind` não é erro: devolve `('0.0.0.0', 0)`.

## Exemplos

```ps
import sockets
s = sockets.socket()
s.bind(("localhost", 0))
end = s.getsockname()
post(type(end), len(end))
post(end[0], end[1] > 0)
s.close()
```

```saida
tup 2
127.0.0.1 True
```

Descobrir a porta que o sistema escolheu:

```ps
import sockets
srv = sockets.socket()
srv.bind(("127.0.0.1", 0))
srv.listen(1)
porta = srv.getsockname()[1]

cli = sockets.socket()
cli.connect(("127.0.0.1", porta))
post(cli.getpeername()[1] == porta)
cli.close(); srv.close()
```

```saida
True
```

IPv6 tem quatro posições:

```ps
import sockets
v6 = sockets.socket(sockets.AF_INET6, sockets.SOCK_STREAM)
v6.bind(("::1", 0))
end = v6.getsockname()
post(len(end), end[0], end[2], end[3])
v6.close()
```

```saida
4 ::1 0 0
```

## Bordas

- socket recém-criado, sem `bind`, responde `('0.0.0.0', 0)` — não levanta erro
- num socket que fez `connect` sem `bind`, o sistema já escolheu IP e porta de
  origem, e é isso que aparece aqui
- o outro lado é [`getpeername()`](../getpeername/getpeername.md)
- em `AF_UNIX` é **`str`**, não tupla: `end[0]` daria a primeira letra do
  caminho, não o caminho

[← índice](../sockets.md)
