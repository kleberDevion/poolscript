# `socket.recvfrom(bufsize, flags)`

Recebe um datagrama **e diz de quem veio**. É o [`recv`](../recv/recv.md) do
UDP: sem conexão, o remetente muda a cada pacote, e é ele que você precisa pra
responder.

## Parâmetros

| nome | tipo | default | nota |
|---|---|---|---|
| `bufsize` | int | — | **obrigatório e positivo**: o máximo de bytes desta leitura |
| `flags` | int | `0` | `sockets.MSG_PEEK`, `MSG_WAITALL`, `MSG_DONTWAIT`, `MSG_TRUNC` — combináveis com `\|` |

`flags` que não seja `int` é ignorado (vale `0`), sem erro.

## Retorno

**`tup`** de 2 posições — `(bytes dados, endereco)`:

| posição | tipo | o que é |
|---|---|---|
| `[0]` | `bytes` | o conteúdo, de `0` a `bufsize` bytes |
| `[1]` | `tup` \| `str` | o remetente, no formato da família |

O endereço da posição `[1]` é `(str host, int porta)` em `AF_INET`,
`(host, porta, flowinfo, scope_id)` em `AF_INET6` e `str` (o caminho) em
`AF_UNIX`.

Nunca é só o `bytes`, nunca é só o endereço: é sempre a tupla de dois. Quem
quer só os dados escreve `s.recvfrom(64)[0]`.

## Erros

- **`TypeError`** — `recvfrom() takes at least 1 argument (0 given)` /
  `recvfrom() takes at most 2 arguments (3 given)`
- **`TypeError`** — `'str' object cannot be interpreted as an integer`
- **`TypeError`** — `recvfrom: bufsize deve ser int positivo`, para `0` e
  negativos
- **`RuntimeError`** — `timed out`: o prazo estourou, ou o socket é
  não-bloqueante e não havia datagrama
- **`OSError`** — socket fechado: `[Errno 9] Bad file descriptor`

## Exemplos

```ps
import sockets
u1 = sockets.socket(sockets.AF_INET, sockets.SOCK_DGRAM)
u1.bind(("127.0.0.1", 0))
u2 = sockets.socket(sockets.AF_INET, sockets.SOCK_DGRAM)
u2.sendto("datagrama", ("127.0.0.1", u1.getsockname()[1]))

r = u1.recvfrom(64)
post(type(r), len(r))
post(type(r[0]), r[0].decode())
post(r[1][0], r[1][1] > 0)
u1.close(); u2.close()
```

```saida
tup 2
bytes datagrama
127.0.0.1 True
```

Responder para quem mandou:

```ps
import sockets
srv = sockets.socket(sockets.AF_INET, sockets.SOCK_DGRAM)
srv.bind(("127.0.0.1", 0))
cli = sockets.socket(sockets.AF_INET, sockets.SOCK_DGRAM)
cli.bind(("127.0.0.1", 0))
cli.sendto("ping", ("127.0.0.1", srv.getsockname()[1]))

par = srv.recvfrom(64)
srv.sendto("pong", par[1])          # o endereço volta cru pro sendto
post(cli.recv(64).decode())
srv.close(); cli.close()
```

```saida
pong
```

## Bordas

- o endereço de `[1]` entra **como está** no
  [`sendto`](../sendto/sendto.md) — é o mesmo formato nos dois
- `bufsize` menor que o datagrama **corta** o pacote, e o resto se perde: em
  UDP não existe "ler o resto depois"
- num socket TCP `recvfrom` funciona, mas o endereço não acrescenta nada — use
  [`recv`](../recv/recv.md)
- no VM em C a espera **cede a fibra**

[← índice](../sockets.md)
