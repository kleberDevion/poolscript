# `socket.settimeout(timeout)`

Define o prazo das operações que esperam — `connect`, `accept`, `recv`,
`recvfrom`, `send`, `sendall`. Sem prazo, um socket parado trava o script até o
sistema desistir.

## Parâmetros

| nome | tipo | default | nota |
|---|---|---|---|
| `timeout` | flo \| int \| Null | — | **obrigatório**. Segundos (aceita fração). `Null` = bloqueante, sem prazo. `0` = não-bloqueante |

Os três modos, e é só isso que existe:

| valor | modo | o que acontece quando não há dado |
|---|---|---|
| `null` | bloqueante | espera o tempo que for |
| `0` | não-bloqueante | levanta `timed out` **na hora** |
| `> 0` | com prazo | espera até esse tempo e aí levanta `timed out` |

## Retorno

**`Null`** — sempre. Para ler o prazo de volta, use
[`gettimeout()`](../gettimeout/gettimeout.md).

## Erros

- **`TypeError`** — `settimeout() takes exactly one argument (0 given)`
- **`TypeError`** — valor que não é número nem `Null`:
  `'str' object cannot be interpreted as an integer`
- **`ValueError`** — `Timeout value out of range`, para prazo negativo
- **`OSError`** — socket fechado: `[Errno 9] Bad file descriptor`

## Exemplos

```ps
import sockets
s = sockets.socket()
post(s.settimeout(5))
post(s.gettimeout())
s.settimeout(0.25)
post(s.gettimeout())
s.settimeout(null)
post(s.gettimeout())
s.close()
```

```saida
Null
5.0
0.25
Null
```

Prazo estourado é capturável:

```ps
import sockets
u = sockets.socket(sockets.AF_INET, sockets.SOCK_DGRAM)
u.bind(("127.0.0.1", 0))
u.settimeout(0.2)
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

- o erro de prazo é **`RuntimeError`** com `"timed out"` na mensagem — não é
  `TimeoutError`, e `catch (TimeoutError e)` não pega
- um `int` vira `flo` no caminho: `settimeout(5)` e `gettimeout()` = `5.0`
- o prazo vale por **chamada**, não pela operação inteira: um `sendall` com
  prazo de 1 s pode demorar mais, porque cada `send` de dentro ganha o seu
- o socket que sai do [`accept`](../accept/accept.md) e o que sai do
  [`dup`](../dup/dup.md) **herdam** o prazo
- `sockets.setdefaulttimeout(t)` define o prazo dos sockets **novos**; este
  método mexe só neste

[← índice](../sockets.md)
