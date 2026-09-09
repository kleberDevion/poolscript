# `socket.recv(bufsize, flags)`

Recebe até `bufsize` bytes da conexão. É a leitura crua: o que chegar, chega —
não existe "linha", não existe "mensagem", quem monta o protocolo é você.

## Parâmetros

| nome | tipo | default | nota |
|---|---|---|---|
| `bufsize` | int | — | **obrigatório e positivo**: o máximo de bytes desta leitura |
| `flags` | int | `0` | `sockets.MSG_PEEK`, `MSG_WAITALL`, `MSG_DONTWAIT`, `MSG_OOB`, `MSG_TRUNC` — combináveis com `\|` |

`flags` que não seja `int` é ignorado (vale `0`), sem erro.

## Retorno

**`bytes`** — sempre, mesmo quando você mandou `str` do outro lado. Nunca é
`str`: para texto, chame `.decode()`.

O tamanho do resultado vai de `0` a `bufsize`:

| resultado | quer dizer |
|---|---|
| `b"…"` com 1..`bufsize` bytes | chegou isso; **pode ser menos** do que o outro lado mandou numa tacada |
| `b""` (vazio, `len` = 0) | **fim de fluxo**: o outro lado fechou ou deu `shutdown(SHUT_WR)`. Não é erro, e nunca vai chegar mais nada |

`b""` é o sinal de fim — um laço `while true { d = s.recv(4096) }` que não testa
`len(d) == 0` gira para sempre.

## Erros

- **`TypeError`** — `recv() takes at least 1 argument (0 given)` /
  `recv() takes at most 2 arguments (3 given)`
- **`TypeError`** — `'str' object cannot be interpreted as an integer`
- **`TypeError`** — `recv: bufsize deve ser int positivo`, para `0` e negativos
- **`RuntimeError`** — `timed out`: o prazo estourou, ou o socket é
  não-bloqueante e não havia nada pronto
- **`OSError`** — socket fechado: `[Errno 9] Bad file descriptor`

## Exemplos

```ps
import sockets
srv = sockets.socket()
srv.bind(("127.0.0.1", 0))
srv.listen(1)
cli = sockets.socket()
cli.connect(("127.0.0.1", srv.getsockname()[1]))
conn = srv.accept()[0]

cli.sendall("olá servidor")
d = conn.recv(64)
post(type(d), len(d))
post(d.decode())
conn.close(); cli.close(); srv.close()
```

```saida
bytes 13
olá servidor
```

`b""` marcando o fim:

```ps
import sockets
srv = sockets.socket()
srv.bind(("127.0.0.1", 0))
srv.listen(1)
cli = sockets.socket()
cli.connect(("127.0.0.1", srv.getsockname()[1]))
conn = srv.accept()[0]

cli.sendall("tchau")
cli.close()
post(conn.recv(16))
post(conn.recv(16), len(conn.recv(16)))
conn.close(); srv.close()
```

```saida
b'tchau'
b'' 0
```

`MSG_PEEK` espia sem consumir:

```ps
import sockets
srv = sockets.socket()
srv.bind(("127.0.0.1", 0))
srv.listen(1)
cli = sockets.socket()
cli.connect(("127.0.0.1", srv.getsockname()[1]))
conn = srv.accept()[0]

cli.sendall("peek")
post(conn.recv(8, sockets.MSG_PEEK))
post(conn.recv(8))
conn.close(); cli.close(); srv.close()
```

```saida
b'peek'
b'peek'
```

## Bordas

- `bufsize` é **teto**, não pedido: pedir `4096` não espera encher 4096 bytes
- TCP é fluxo, não mensagem: duas chamadas de `send` podem chegar num `recv` só,
  e uma pode chegar picada em duas
- prazo estourado é `RuntimeError` com `"timed out"` na mensagem — não é
  `TimeoutError`
- em UDP quem também devolve o remetente é
  [`recvfrom`](../recvfrom/recvfrom.md); o `recv` joga o endereço fora
- no VM em C a espera **cede a fibra**: um `recv` parado não trava os outros
  handlers do jinker

[← índice](../sockets.md)
