# `socket.connect_ex(address)`

O mesmo que [`connect`](../connect/connect.md), só que a falha de rede **não
vira exceção**: ela vira o número que o sistema devolveu. É o jeito de varrer
portas ou tentar vários endereços sem `try/catch` em volta de cada um.

## Parâmetros

| nome | tipo | default | nota |
|---|---|---|---|
| `address` | tup | — | `AF_INET`: `(str host, int porta)`. `AF_INET6`: 4 posições. `AF_UNIX`: **`str`** com o caminho |

## Retorno

**`int`** — o código do sistema:

| valor | significa |
|---|---|
| `0` | conectou |
| `111` | `Connection refused` — nada escutando ali |
| `110` | o prazo do [`settimeout`](../settimeout/settimeout.md) estourou |
| outro | o `errno` que o sistema devolveu (`113` sem rota, e assim por diante) |

Só isso: nunca uma tupla, nunca `Null`. O número é o `errno` cru — para
comparar, compare com o número.

## Erros

O erro de **rede** não é levantado (é o retorno). O que ainda levanta:

- **`TypeError`** — `connect_ex() takes exactly one argument (0 given)`
- **`TypeError`** — endereço na forma errada:
  `connect_ex(): AF_INET address must be tuple, not str`
- **`RuntimeError`** — nome que não resolve:
  `[Errno -5] No address associated with hostname` (a resolução acontece antes
  da conexão, e essa parte continua levantando)
- **`OSError`** — socket fechado: `[Errno 9] Bad file descriptor`

## Exemplos

```ps
import sockets
srv = sockets.socket()
srv.bind(("127.0.0.1", 0))
srv.listen(1)

cli = sockets.socket()
post(cli.connect_ex(("127.0.0.1", srv.getsockname()[1])))
cli.close(); srv.close()

fechado = sockets.socket()
post(fechado.connect_ex(("127.0.0.1", 1)))
fechado.close()
```

```saida
0
111
```

Varredura simples, sem `try` em volta de cada porta:

```ps
import sockets
srv = sockets.socket()
srv.bind(("127.0.0.1", 0))
srv.listen(1)
aberta = srv.getsockname()[1]

for each p in [aberta, 1] {
    s = sockets.socket()
    s.settimeout(1)
    if s.connect_ex(("127.0.0.1", p)) == 0 {
        post(p == aberta, "aberta")
    } else {
        post(p == aberta, "fechada")
    }
    s.close()
}
srv.close()
```

```saida
True aberta
False fechada
```

## Bordas

- prazo estourado devolve **`110`**, não levanta `timed out` — a diferença
  central em relação ao `connect`
- `0` é o único valor que quer dizer sucesso; qualquer outro é falha
- o socket continua utilizável depois de um retorno diferente de `0` só para
  ser fechado: tentar de novo no mesmo objeto é recusado pelo sistema

[← índice](../sockets.md)
