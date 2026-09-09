# `socket.fileno()`

O descritor de arquivo cru que o sistema deu para este socket.

## Parâmetros

Nenhum. `fileno(1)` é `fileno() takes no arguments (1 given)`.

## Retorno

**`int`** — o número do descritor:

| valor | quer dizer |
|---|---|
| `> 0` | o descritor aberto (`3`, `4`, … — o número que o sistema escolheu) |
| `-1` | **não tem descritor**: o socket foi fechado com [`close()`](../close/close.md) ou soltou o seu com [`detach()`](../detach/detach.md) |

`-1` é o único valor sentinela. Não devolve `Null`, e não levanta erro por
estar fechado.

## Erros

- **`TypeError`** — `fileno() takes no arguments (1 given)`

Funciona em socket fechado — é justamente para isso que serve o `-1`.

## Exemplos

```ps
import sockets
s = sockets.socket()
post(type(s.fileno()), s.fileno() > 0)
s.close()
post(s.fileno())
```

```saida
int True
-1
```

Testar se o socket ainda vale:

```ps
import sockets
s = sockets.socket()
if s.fileno() >= 0 {
    post("aberto")
}
s.close()
if s.fileno() < 0 {
    post("fechado")
}
```

```saida
aberto
fechado
```

## Bordas

- dois sockets diferentes nunca compartilham o número ao mesmo tempo, mas o
  sistema **reaproveita** descritores fechados: guardar o número de um socket
  morto e comparar com o de um vivo dá falso positivo
- [`dup()`](../dup/dup.md) devolve um socket com descritor **diferente**
  apontando pra mesma conexão
- depois de [`detach()`](../detach/detach.md) o `fileno()` é `-1`, mas o
  descritor que ele devolveu continua aberto no processo

[← índice](../sockets.md)
