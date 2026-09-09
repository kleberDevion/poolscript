# `socket.setsockopt(level, optname, value)`

Liga, desliga ou ajusta uma opção do socket — reuso de endereço, keepalive,
tamanho de buffer, Nagle. Quase sempre é a primeira linha depois do
`sockets.socket()`, antes do [`bind`](../bind/bind.md).

## Parâmetros

| nome | tipo | default | nota |
|---|---|---|---|
| `level` | int | — | a camada da opção: `sockets.SOL_SOCKET` para as `SO_*`, `sockets.IPPROTO_TCP` para as `TCP_*`, `sockets.IPPROTO_IP` para as `IP_*` |
| `optname` | int | — | a opção: `sockets.SO_REUSEADDR`, `SO_KEEPALIVE`, `SO_RCVBUF`, `TCP_NODELAY`… |
| `value` | int \| bool \| bytes \| str | — | `int`/`bool` vão como inteiro do sistema (`true` = `1`); `bytes`/`str` vão **crus**, do tamanho que têm |

As três posições são obrigatórias — não há default nenhum.

## Retorno

**`Null`** — sempre. Para ler de volta, use
[`getsockopt`](../getsockopt/getsockopt.md).

## Erros

- **`TypeError`** — `setsockopt() takes exactly 3 arguments (2 given)`
- **`TypeError`** — `level`/`optname` que não são `int`:
  `'str' object cannot be interpreted as an integer`
- **`TypeError`** — valor de tipo que não serve:
  `a bytes-like object is required, not 'flo'` (um `flo`, uma `list`…)
- **`RuntimeError`** — opção que a camada não conhece:
  `[Errno 92] Protocol not available`
- **`OSError`** — socket fechado: `[Errno 9] Bad file descriptor`

## Exemplos

```ps
import sockets
s = sockets.socket()
post(s.setsockopt(sockets.SOL_SOCKET, sockets.SO_REUSEADDR, 1))
post(s.getsockopt(sockets.SOL_SOCKET, sockets.SO_REUSEADDR))
post(s.setsockopt(sockets.SOL_SOCKET, sockets.SO_KEEPALIVE, true))
post(s.getsockopt(sockets.SOL_SOCKET, sockets.SO_KEEPALIVE))
s.close()
```

```saida
Null
1
Null
1
```

O par que evita `Address already in use` ao reiniciar um servidor:

```ps
import sockets
srv = sockets.socket()
srv.setsockopt(sockets.SOL_SOCKET, sockets.SO_REUSEADDR, 1)
srv.bind(("127.0.0.1", 0))
srv.listen(4)
post(srv.getsockname()[0])
srv.close()
```

```saida
127.0.0.1
```

Desligar o Nagle (manda o pacote pequeno na hora):

```ps
import sockets
s = sockets.socket()
s.setsockopt(sockets.IPPROTO_TCP, sockets.TCP_NODELAY, 1)
post(s.getsockopt(sockets.IPPROTO_TCP, sockets.TCP_NODELAY))
s.close()
```

```saida
1
```

## Bordas

- `SO_REUSEADDR` só tem efeito **antes** do `bind`
- o sistema pode guardar um valor diferente do que você pediu: `SO_RCVBUF` e
  `SO_SNDBUF` costumam voltar dobrados no `getsockopt`
- `bool` é aceito e vira `1`/`0`; `flo` **não** é aceito
- opção de multicast (`IP_ADD_MEMBERSHIP`) espera uma estrutura binária — é o
  caso de passar `bytes`, não `int`
- as opções que o sistema propaga chegam junto no socket devolvido pelo
  [`accept`](../accept/accept.md)

[← índice](../sockets.md)
