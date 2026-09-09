# `socket.sendall(data, flags)`

Manda **tudo**: insiste em `send` atrás de `send` até o último byte sair. É o
que você quer em 99% dos casos — [`send`](../send/send.md) só existe pra quem
precisa contar os bytes na mão.

## Parâmetros

| nome | tipo | default | nota |
|---|---|---|---|
| `data` | str \| bytes | — | `str` vira UTF-8 antes de ir; `bytes` vai cru |
| `flags` | int | `0` | `sockets.MSG_OOB`, `MSG_DONTWAIT`, `MSG_DONTROUTE` — combináveis com `\|` |

`flags` que não seja `int` é ignorado (vale `0`), sem erro.

## Retorno

**`Null`** — sempre, e é de propósito.

> **`sendall` NÃO devolve a contagem de bytes.** Quem escreve
> `n = s.sendall(dado)` fica com `n = Null`, e `post(n + 1)` quebra com
> `TypeError`. A contagem é do [`send`](../send/send.md); aqui o contrato é
> "voltou sem erro = foi tudo".

## Erros

- **`TypeError`** — `sendall() takes at least 1 argument (0 given)` /
  `sendall() takes at most 2 arguments (3 given)`
- **`TypeError`** — `a bytes-like object is required, not 'int'`
- **`RuntimeError`** — `timed out`, quando o prazo estoura no meio
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

post(cli.sendall("mensagem inteira"))   # Null, não 16
post(conn.recv(64).decode())
conn.close(); cli.close(); srv.close()
```

```saida
Null
mensagem inteira
```

Volume que não cabe num `send` só:

```ps
import sockets
srv = sockets.socket()
srv.bind(("127.0.0.1", 0))
srv.listen(1)
cli = sockets.socket()
cli.connect(("127.0.0.1", srv.getsockname()[1]))
conn = srv.accept()[0]

cli.sendall("x" * 5000)
total = 0
while total < 5000 {
    total = total + len(conn.recv(4096))
}
post(total)
conn.close(); cli.close(); srv.close()
```

```saida
5000
```

## Bordas

- **erro no meio não diz quanto já foi.** Se o prazo estoura depois de metade
  do dado, o `timed out` sobe e a outra ponta ficou com um pedaço — em socket
  não-bloqueante com buffer cheio isso acontece de verdade
- por isso `sendall` combina com socket **bloqueante** ou com prazo folgado;
  em não-bloqueante o certo é `send` num laço, olhando o retorno
- no VM em C o laço inteiro roda no pool de threads e **cede a fibra**

[← índice](../sockets.md)
