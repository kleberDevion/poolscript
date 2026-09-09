# `socket.bind(address)`

Prende o socket a um endereço **local**. É o primeiro passo de um servidor
(antes de [`listen`](../listen/listen.md)) e o jeito de escolher a porta de
origem de um cliente ou de um UDP.

## Parâmetros

| nome | tipo | default | nota |
|---|---|---|---|
| `address` | tup | — | `AF_INET`: `(str host, int porta)`. `AF_INET6`: `(str host, int porta, int flowinfo, int scope_id)`. `AF_UNIX`: **`str`** com o caminho |

O nome é este: `s.bind(address=("127.0.0.1", 0))` funciona; `s.bind(endereco=…)`
é `'endereco' is an invalid keyword argument for bind()`.

Uma `list` vale onde a `tup` vale: `s.bind(["127.0.0.1", 0])` prende igual.

O host é resolvido pelo sistema com `AI_PASSIVE` — vale IP numérico
(`"127.0.0.1"`), nome (`"localhost"`) e o texto vazio `""`, que significa
**todas as interfaces** (`0.0.0.0`).

## Retorno

**`Null`** — sempre, e em qualquer família.

`bind` **não** devolve o endereço que passou a valer. Quem devolve é
[`getsockname()`](../getsockname/getsockname.md), e é por ele que se lê a porta
quando você pediu `0` (o sistema escolhe uma livre).

## Erros

- **`TypeError`** — `bind() takes exactly one argument (0 given)`
- **`TypeError`** — endereço na forma errada:
  `bind(): AF_INET address must be tuple, not str`
- **`TypeError`** — host que não é texto:
  `str, bytes or bytearray expected, not int`
- **`TypeError`** — porta que não é `int`:
  `'str' object cannot be interpreted as an integer`
- **`RuntimeError`** — recusa do sistema, com o número na frente:
  `[Errno 98] Address already in use`
- **`OSError`** — socket já fechado (ou depois de `detach()`):
  `[Errno 9] Bad file descriptor`

## Exemplos

```ps
import sockets
s = sockets.socket()
post(s.bind(("127.0.0.1", 0)))     # o retorno é Null
post(s.getsockname()[0], s.getsockname()[1] > 0)
s.close()
```

```saida
Null
127.0.0.1 True
```

```ps
import sockets
s = sockets.socket()
s.bind(("", 0))                    # host vazio = todas as interfaces
post(s.getsockname()[0])
s.close()
```

```saida
0.0.0.0
```

## Bordas

- **porta `0`** = o sistema escolhe uma livre; leia qual com `getsockname()`
- `SO_REUSEADDR` antes do `bind` é o que evita `Address already in use` ao
  reiniciar um servidor — ver [`setsockopt`](../setsockopt/setsockopt.md)
- em `AF_UNIX` o endereço é **`str`**, não tupla, e o arquivo do caminho fica
  no disco depois do `close()` — apagar é por sua conta
- prender duas vezes o mesmo socket é recusado pelo sistema
  (`[Errno 22] Invalid argument`)

[← índice](../sockets.md)
