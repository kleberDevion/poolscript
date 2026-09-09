# `socket.listen(backlog)`

Vira servidor: o socket para de ser um ponta-solta e passa a **enfileirar**
conexões que chegam, para o [`accept`](../accept/accept.md) ir tirando da fila.
Só faz sentido depois de [`bind`](../bind/bind.md), e só em socket de fluxo
(`SOCK_STREAM`).

## Parâmetros

| nome | tipo | default | nota |
|---|---|---|---|
| `backlog` | int | `sockets.SOMAXCONN` | tamanho da fila de conexões ainda não aceitas |

Omitir o argumento usa `SOMAXCONN`, o maior valor que o sistema aceita. Um
argumento que **não** seja `int` é ignorado do mesmo jeito — `s.listen("a")`
não levanta erro, cai no default.

## Retorno

**`Null`** — sempre.

## Erros

- **`TypeError`** — `listen() takes at most 1 argument (2 given)`
- **`RuntimeError`** — em socket que não é de fluxo (UDP, por exemplo):
  `[Errno 95] Operation not supported`
- **`OSError`** — socket fechado: `[Errno 9] Bad file descriptor`

## Exemplos

```ps
import sockets
s = sockets.socket()
s.bind(("127.0.0.1", 0))
post(s.listen(4))
post(s.listen())        # sem argumento: SOMAXCONN
s.close()
```

```saida
Null
Null
```

## Bordas

- a fila do `backlog` é o que faz um `connect` do próprio script completar
  **antes** de qualquer `accept` — é assim que dá pra testar cliente e servidor
  no mesmo arquivo, sem fibra
- `accept()` num socket que nunca ouviu dá `[Errno 22] Invalid argument`
- `listen` pode ser chamado de novo pra mudar o tamanho da fila
- `sockets.SOMAXCONN` é o valor do sistema, não um número fixo da linguagem

[← índice](../sockets.md)
