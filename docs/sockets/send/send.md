# `socket.send(data, flags)`

Manda bytes pela conexão e diz **quantos saíram**. Pode sair menos do que você
pediu: quem precisa mandar tudo usa [`sendall`](../sendall/sendall.md).

## Parâmetros

| nome | tipo | default | nota |
|---|---|---|---|
| `data` | str \| bytes | — | `str` vira UTF-8 antes de ir; `bytes` vai cru |
| `flags` | int | `0` | `sockets.MSG_OOB`, `MSG_DONTWAIT`, `MSG_DONTROUTE` — combináveis com `\|` |

`flags` que não seja `int` é ignorado (vale `0`), sem erro.

## Retorno

**`int`** — quantos **bytes** o sistema aceitou nesta chamada. Nunca é `Null`,
nunca é o socket, nunca é o texto.

Dois detalhes do número:

- é **byte**, não caractere: `s.send("ação")` devolve `6`, porque em UTF-8 são
  6 bytes para 4 caracteres;
- pode ser **menor** que `len(data)` quando o buffer do sistema enche — num
  socket não-bloqueante um `send` de 4 MB volta com alguns MB e o resto é
  problema seu. Repetir a partir do que faltou é exatamente o que o `sendall`
  faz.

## Erros

- **`TypeError`** — `send() takes at least 1 argument (0 given)` /
  `send() takes at most 2 arguments (3 given)`
- **`TypeError`** — `a bytes-like object is required, not 'int'`, quando `data`
  não é `str` nem `bytes`
- **`RuntimeError`** — `timed out`, quando o prazo estoura (ou, em socket
  não-bloqueante, quando não cabe nada agora)
- **`RuntimeError`** — o outro lado sumiu: `[Errno 32] Broken pipe`
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

post(cli.send("abcd"))
post(cli.send("ação"))          # 4 caracteres, 6 bytes
post(conn.recv(64))
conn.close(); cli.close(); srv.close()
```

```saida
4
6
b'abcdac\xcc\xa7ao'
```

## Bordas

- **confira o retorno.** Um `send` que devolve menos que o tamanho do dado
  mandou só um pedaço — o resto não vai sozinho
- num socket com prazo `0` ([`setblocking(false)`](../setblocking/setblocking.md))
  o `send` volta com o que coube e, se não coube nada, levanta `timed out`
- para UDP sem `connect`, quem manda é [`sendto`](../sendto/sendto.md):
  `send` num datagrama sem destino é `[Errno 89] Destination address required`
- no VM em C o envio **cede a fibra**: um `send` bloqueado não trava os outros
  handlers do jinker

[← índice](../sockets.md)
