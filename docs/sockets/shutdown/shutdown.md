# `socket.shutdown(how)`

Encerra **um lado** da conexão sem fechar o descritor. Serve pra dizer "acabei
de falar" e ainda continuar ouvindo — coisa que
[`close`](../close/close.md) não permite, porque ele desliga tudo.

## Parâmetros

| nome | tipo | default | nota |
|---|---|---|---|
| `how` | int | — | **obrigatório**: `sockets.SHUT_RD`, `sockets.SHUT_WR` ou `sockets.SHUT_RDWR` |

| constante | fecha | efeito no outro lado |
|---|---|---|
| `SHUT_RD` | a leitura deste socket | nenhum aviso |
| `SHUT_WR` | a escrita deste socket | o `recv` dele passa a devolver `b""` |
| `SHUT_RDWR` | as duas | idem |

## Retorno

**`Null`** — sempre.

## Erros

- **`TypeError`** — `shutdown() takes exactly one argument (0 given)`
- **`TypeError`** — `'str' object cannot be interpreted as an integer`
- **`RuntimeError`** — socket sem conexão:
  `[Errno 107] Transport endpoint is not connected`
- **`OSError`** — socket fechado: `[Errno 9] Bad file descriptor`

## Exemplos

`SHUT_WR` avisa o outro lado que o fluxo acabou:

```ps
import sockets
srv = sockets.socket()
srv.bind(("127.0.0.1", 0))
srv.listen(1)
cli = sockets.socket()
cli.connect(("127.0.0.1", srv.getsockname()[1]))
conn = srv.accept()[0]

cli.sendall("tchau")
post(cli.shutdown(sockets.SHUT_WR))
post(conn.recv(16))
post(conn.recv(16))          # b"" = fim de fluxo
conn.close(); cli.close(); srv.close()
```

```saida
Null
b'tchau'
b''
```

## Bordas

- depois de `SHUT_WR`, um `send` **no próprio socket** é
  `[Errno 32] Broken pipe`
- `SHUT_RD` não impede o outro lado de mandar; ele só descarta o que chega aqui
- `shutdown` **não** libera o descritor: `fileno()` continua o mesmo e o
  `close()` ainda é obrigatório
- é o jeito certo de terminar um pedido HTTP `1.0` na unha: mandar o cabeçalho,
  dar `SHUT_WR` e ler a resposta até `b""`

[← índice](../sockets.md)
