# `socket.sendto(data, address, flags)`

Manda um datagrama para um endereço **escolhido nesta chamada** — sem
`connect`, sem conexão. É o envio do UDP (`SOCK_DGRAM`).

## Parâmetros

| nome | tipo | default | nota |
|---|---|---|---|
| `data` | str \| bytes | — | `str` vira UTF-8 antes de ir; `bytes` vai cru |
| `address` | tup | — | `AF_INET`: `(str host, int porta)`. `AF_INET6`: 4 posições. `AF_UNIX`: **`str`** com o caminho |
| `flags` | int | `0` | `sockets.MSG_DONTWAIT`, `MSG_DONTROUTE` — combináveis com `\|` |

A ordem é a da assinatura, posicional ou por nome: `s.sendto(dado, addr, 0)`
e `s.sendto(data=d, address=a, flags=0)` são a mesma chamada. `flags` que não
é `int` é `TypeError: 'str' object cannot be interpreted as an integer`.

## Retorno

**`int`** — quantos **bytes** saíram, contados em UTF-8 quando `data` é `str`:
`s.sendto("ação", destino)` devolve `6`.

Em UDP o datagrama é atômico: ou vai inteiro, ou levanta erro — o retorno é o
tamanho do dado, não um pedaço dele.

## Erros

- **`TypeError`** — `sendto() takes at least 2 arguments (1 given)` /
  `sendto() takes at most 3 arguments (4 given)`
- **`TypeError`** — `a bytes-like object is required, not 'int'` (dado errado)
- **`TypeError`** — `sendto(): AF_INET address must be tuple, not int`
  (endereço que não é a tupla `(host, porta)`)
- **`RuntimeError`** — dado maior do que o datagrama aceita:
  `[Errno 90] Message too long`
- **`OSError`** — socket fechado: `[Errno 9] Bad file descriptor`

## Exemplos

```ps
import sockets
u1 = sockets.socket(sockets.AF_INET, sockets.SOCK_DGRAM)
u1.bind(("127.0.0.1", 0))
porta = u1.getsockname()[1]

u2 = sockets.socket(sockets.AF_INET, sockets.SOCK_DGRAM)
post(u2.sendto("datagrama", ("127.0.0.1", porta)))
post(u2.sendto("ação", ("127.0.0.1", porta)))       # 4 caracteres, 6 bytes
post(u1.recvfrom(64)[0].decode())
u1.close(); u2.close()
```

```saida
9
6
datagrama
```

Com `flags`, na ordem da assinatura:

```ps
import sockets
u1 = sockets.socket(sockets.AF_INET, sockets.SOCK_DGRAM)
u1.bind(("127.0.0.1", 0))
u2 = sockets.socket(sockets.AF_INET, sockets.SOCK_DGRAM)
post(u2.sendto("com flags", ("127.0.0.1", u1.getsockname()[1]), 0))
post(u1.recv(64).decode())
u1.close(); u2.close()
```

```saida
9
com flags
```

## Bordas

- num socket **conectado** o destino já está fixo: use
  [`send`](../send/send.md) ou [`sendall`](../sendall/sendall.md)
- `SO_BROADCAST` precisa estar ligado antes de mandar pra endereço de difusão —
  ver [`setsockopt`](../setsockopt/setsockopt.md)
- quem recebe do outro lado com o remetente junto é
  [`recvfrom`](../recvfrom/recvfrom.md)
- o host é resolvido a cada chamada; mandar em rajada para um **nome** paga a
  resolução toda vez

[← índice](../sockets.md)
