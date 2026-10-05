# `socket.setblocking(flag)`

Atalho de duas posições do [`settimeout`](../settimeout/settimeout.md):
`setblocking(True)` é `settimeout(Null)`, `setblocking(False)` é
`settimeout(0)`.

## Parâmetros

| nome | tipo | default | nota |
|---|---|---|---|
| `flag` | bool | — | **obrigatório**. Qualquer valor **verdadeiro** liga o modo bloqueante; qualquer **falso** (`False`, `0`, `""`, `Null`) desliga |

O argumento não precisa ser `bool`: o motor olha só a verdade do valor.

| chamada | equivale a | `gettimeout()` depois |
|---|---|---|
| `s.setblocking(True)` | `s.settimeout(Null)` | `Null` |
| `s.setblocking(False)` | `s.settimeout(0)` | `0.0` |

## Retorno

**`Null`** — sempre.

## Erros

- **`TypeError`** — `setblocking() takes exactly one argument (0 given)`
- **`OSError`** — socket fechado: `[Errno 9] Bad file descriptor`

## Exemplos

```ps
import sockets
s = sockets.socket()
post(s.setblocking(False))
post(s.gettimeout(), s.getblocking())
s.setblocking(True)
post(s.gettimeout(), s.getblocking())
s.close()
```

```saida
Null
0.0 False
Null True
```

Em modo não-bloqueante, quem não tem dado levanta na hora:

```ps
import sockets
u = sockets.socket(sockets.AF_INET, sockets.SOCK_DGRAM)
u.bind(("127.0.0.1", 0))
u.setblocking(False)
try {
    u.recv(16)
} catch (e) {
    post("timed out" in str(e))
}
u.close()
```

```saida
True
```

## Bordas

- **`setblocking(True)` apaga o prazo.** Um socket com `settimeout(5)` que
  recebe `setblocking(True)` volta a `Null` — passa a esperar para sempre
- não existe meio-termo aqui: para prazo em segundos é `settimeout`
- em modo não-bloqueante, [`send`](../send/send.md) devolve o que coube e
  [`sendall`](../sendall/sendall.md) pode levantar `timed out` no meio, com
  parte do dado já entregue

[← índice](../sockets.md)
