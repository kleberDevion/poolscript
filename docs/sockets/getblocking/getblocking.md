# `socket.getblocking()`

Responde se o socket **espera** ou não.

## Parâmetros

Nenhum. `getblocking(1)` é `getblocking() takes no arguments (1 given)`.

## Retorno

**`bool`** — e a regra é uma só: é `true` quando
[`gettimeout()`](../gettimeout/gettimeout.md) **não** é `0`.

| `gettimeout()` | `getblocking()` |
|---|---|
| `Null` (sem prazo) | `True` |
| `5.0` (com prazo) | `True` |
| `0.0` (não-bloqueante) | `False` |

> **Prazo não é "não-bloqueante".** Um socket com `settimeout(5)` responde
> `True` aqui: ele espera, só que com hora pra desistir. Quem precisa saber se
> existe prazo pergunta ao `gettimeout()`, que distingue `Null` de `5.0`.

## Erros

- **`TypeError`** — `getblocking() takes no arguments (1 given)`

Funciona em socket **fechado**: como só olha o prazo guardado, não levanta
`Bad file descriptor`. Um socket fechado sem prazo responde `True`.

## Exemplos

```ps
import sockets
s = sockets.socket()
post(s.getblocking(), type(s.getblocking()))
s.settimeout(5)
post(s.getblocking())            # com prazo AINDA é bloqueante
s.setblocking(false)
post(s.getblocking())
s.close()
```

```saida
True bool
True
False
```

## Bordas

- é derivado, não guardado: mudar o prazo muda a resposta na hora
- para os três estados de verdade (`sem prazo` / `com prazo` / `sem espera`) o
  método certo é o `gettimeout()`
- quem liga e desliga é [`setblocking`](../setblocking/setblocking.md)

[← índice](../sockets.md)
