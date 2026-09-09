# `socket.detach()`

Solta a **posse** do descritor: o objeto devolve o número e passa a agir como
se estivesse fechado, mas o descritor continua **aberto** no processo.

## Parâmetros

Nenhum. `detach(1)` é `detach() takes no arguments (1 given)`.

## Retorno

**`int`** — o descritor que o objeto tinha:

| valor | quer dizer |
|---|---|
| `> 0` | o descritor, agora sem dono; o objeto largou |
| `-1` | não havia descritor (o socket já estava fechado ou já tinha sido soltado) |

Depois da chamada, [`fileno()`](../fileno/fileno.md) devolve `-1` e todo método
que precisa do descritor recusa com `[Errno 9] Bad file descriptor`.

## Erros

- **`TypeError`** — `detach() takes no arguments (1 given)`

Nunca levanta por estado: em socket fechado devolve `-1` e pronto.

## Exemplos

```ps
import sockets
s = sockets.socket()
fd = s.detach()
post(type(fd), fd > 0)
post(s.fileno())
post(s.detach())          # de novo: já não tem nada
```

```saida
int True
-1
-1
```

O descritor **continua aberto** — a porta segue presa:

```ps
import sockets
a = sockets.socket()
a.bind(("127.0.0.1", 0))
porta = a.getsockname()[1]
a.detach()                       # o objeto largou, o sistema não

b = sockets.socket()
try {
    b.bind(("127.0.0.1", porta))
} catch (e) {
    post("Errno 98" in str(e))
}
b.close()
```

```saida
True
```

## Bordas

- **`detach` não fecha nada.** O `close()` depois dele é um no-op, e o
  descritor solto só some quando o processo termina — chamar `detach` em laço
  vaza descritores até bater no limite do sistema
- quem quer **fechar** usa [`close()`](../close/close.md); `detach` é para
  entregar o número cru a quem vai cuidar dele
- o prazo e os campos `family`/`type`/`proto` do objeto continuam legíveis
  depois do `detach` — só o descritor foi embora
- para ter **outro** descritor para a mesma conexão, sem largar este, use
  [`dup()`](../dup/dup.md)

[← índice](../sockets.md)
