# `socket.gettimeout()`

Lê o prazo que está valendo neste socket.

## Parâmetros

Nenhum. `gettimeout(1)` é `gettimeout() takes no arguments (1 given)`.

## Retorno

**`flo`** ou **`Null`** — os dois casos, e nada além disso:

| retorno | modo |
|---|---|
| `Null` | **bloqueante**: nunca foi posto prazo, ou foi `settimeout(null)` |
| `0.0` | **não-bloqueante** (`settimeout(0)` ou `setblocking(false)`) |
| `> 0.0` | o prazo em segundos |

O número sai sempre como `flo`, mesmo tendo entrado como `int`:
`settimeout(5)` depois lê `5.0`.

## Erros

- **`TypeError`** — `gettimeout() takes no arguments (1 given)`

Este é um dos poucos métodos que **funcionam em socket fechado**: depois do
`close()` ele continua devolvendo o último prazo, sem levantar
`Bad file descriptor`.

## Exemplos

```ps
import sockets
s = sockets.socket()
post(s.gettimeout())              # nasce bloqueante
s.settimeout(5)
post(s.gettimeout(), type(s.gettimeout()))
s.setblocking(false)
post(s.gettimeout())
s.close()
post(s.gettimeout())              # fechado: continua respondendo
```

```saida
Null
5.0 flo
0.0
0.0
```

## Bordas

- `Null` e `0.0` são coisas **opostas** — `Null` espera para sempre, `0.0` não
  espera nada. Testar com `if s.gettimeout()` confunde os dois
- [`getblocking()`](../getblocking/getblocking.md) responde a mesma pergunta em
  `bool`: é `true` para `Null` e para prazo `> 0`
- o socket que sai do [`accept`](../accept/accept.md) e do
  [`dup`](../dup/dup.md) herda esse valor

[← índice](../sockets.md)
