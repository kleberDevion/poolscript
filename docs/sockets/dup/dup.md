# `socket.dup()`

Duplica o descritor: devolve **outro objeto socket**, com número de descritor
diferente, apontando para a **mesma** conexão.

## Parâmetros

Nenhum. `dup(1)` é `dup() takes no arguments (1 given)`.

## Retorno

**`socket`** — um objeto novo, e `type()` dele é `"socket"`.

O que vem junto no objeto devolvido:

| do original | copiado? |
|---|---|
| `family`, `type`, `proto` | sim, os mesmos números |
| o prazo (`gettimeout()`) | sim |
| o descritor (`fileno()`) | **não** — é um número novo |
| o endereço (`getsockname()`) | é o mesmo, porque a conexão é a mesma |

Os dois objetos são independentes: fechar um **não** fecha o outro, e o
original continua servindo.

## Erros

- **`TypeError`** — `dup() takes no arguments (1 given)`
- **`OSError`** — socket fechado (ou depois de `detach()`):
  `[Errno 9] Bad file descriptor`
- **`RuntimeError`** — recusa do sistema, com o número na frente
  (`[Errno N] …`)

## Exemplos

```ps
import sockets
s = sockets.socket()
s.settimeout(3)
s.bind(("127.0.0.1", 0))

d = s.dup()
post(type(d))
post(d.gettimeout(), d.family == s.family, d.type == s.type)
post(d.fileno() == s.fileno())              # descritor DIFERENTE
post(d.getsockname() == s.getsockname())    # mesma conexão
d.close()
post(s.getsockname()[0])                    # o original continua vivo
s.close()
```

```saida
socket
3.0 True True
False
True
127.0.0.1
```

## Bordas

- **um `close()` não basta.** Cada objeto tem o seu descritor; a conexão só
  acaba quando o último for fechado
- o que os dois compartilham é o estado do **sistema** (o mesmo socket aberto):
  ler num objeto consome o dado para o outro também
- o prazo é copiado **no momento do `dup`**: mudar o prazo de um depois não
  mexe no outro objeto
- para largar o descritor em vez de duplicá-lo, use
  [`detach()`](../detach/detach.md)

[← índice](../sockets.md)
