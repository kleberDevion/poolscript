# `socket.accept()`

Tira a próxima conexão da fila do [`listen`](../listen/listen.md) e devolve
**um socket novo** para conversar com aquele cliente. O socket que escuta
continua escutando: quem transporta os bytes é o que sai daqui.

## Parâmetros

Nenhum.

`accept(1)` é `accept() takes no arguments (1 given)`, e
`accept(x=1)` é `socket.accept() takes no keyword arguments`.

## Retorno

**`tup`** de 2 posições — `(socket conexao, endereco)`:

| posição | tipo | o que é |
|---|---|---|
| `[0]` | `socket` | a conexão nova, já pronta pra `recv`/`send` |
| `[1]` | `tup` \| `str` | o endereço do OUTRO lado, no formato da família |

O endereço da posição `[1]` é `(str host, int porta)` em `AF_INET`,
`(host, porta, flowinfo, scope_id)` em `AF_INET6` e `str` (o caminho) em
`AF_UNIX` — o mesmo formato de
[`getpeername`](../getpeername/getpeername.md).

O socket devolvido **herda** `family`, `type`, `proto` e o **prazo**
(`gettimeout()`) do socket que aceitou; as opções de
[`setsockopt`](../setsockopt/setsockopt.md) que o sistema propaga
(`SO_KEEPALIVE` e `SO_RCVBUF`, entre outras) chegam junto.

## Erros

- **`TypeError`** — `accept() takes no arguments (1 given)`
- **`RuntimeError`** — `timed out`, quando o prazo do
  [`settimeout`](../settimeout/settimeout.md) estoura sem chegar conexão
- **`RuntimeError`** — sem `listen` antes: `[Errno 22] Invalid argument`
- **`RuntimeError`** — em socket que não é de fluxo:
  `[Errno 95] Operation not supported`
- **`OSError`** — socket fechado: `[Errno 9] Bad file descriptor`

## Exemplos

```ps
import sockets
srv = sockets.socket()
srv.bind(("127.0.0.1", 0))
srv.listen(1)
porta = srv.getsockname()[1]

cli = sockets.socket()
cli.connect(("127.0.0.1", porta))

par = srv.accept()
post(type(par), len(par))
post(type(par[0]), par[1][0])
par[0].close(); cli.close(); srv.close()
```

```saida
tup 2
socket 127.0.0.1
```

Desempacotar as duas pontas na hora:

```ps
import sockets
srv = sockets.socket()
srv.bind(("127.0.0.1", 0))
srv.listen(1)
cli = sockets.socket()
cli.connect(("127.0.0.1", srv.getsockname()[1]))

par = srv.accept()
conn = par[0]
quem = par[1]
cli.sendall("oi")
post(conn.recv(16).decode(), quem[0])
conn.close(); cli.close(); srv.close()
```

```saida
oi 127.0.0.1
```

## Bordas

- é o socket de `[0]` que se fecha no fim do atendimento; fechar o que escuta
  derruba o servidor inteiro
- no VM em C a espera **cede a fibra**: um `accept` parado não trava os outros
  handlers do jinker
- num servidor com prazo, o prazo vale pro `accept` **e** vai junto pra cada
  conexão aceita

[← índice](../sockets.md)
