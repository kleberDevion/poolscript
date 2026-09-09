# `socket.getsockopt(level, optname, buflen)`

Lê uma opção do socket. **O tipo do retorno muda com o número de argumentos**:
com dois você recebe um `int`, com três recebe `bytes`.

## Parâmetros

| nome | tipo | default | nota |
|---|---|---|---|
| `level` | int | — | a camada: `sockets.SOL_SOCKET`, `sockets.IPPROTO_TCP`, `sockets.IPPROTO_IP` |
| `optname` | int | — | a opção: `sockets.SO_ERROR`, `SO_TYPE`, `SO_RCVBUF`, `TCP_NODELAY`… |
| `buflen` | int | — (sem ele, o retorno é `int`) | **1 a 1024**: quantos bytes reservar para a resposta crua |

## Retorno

Depende de você ter passado `buflen`:

| chamada | devolve | formato |
|---|---|---|
| `s.getsockopt(level, optname)` | **`int`** | o valor da opção como inteiro do sistema |
| `s.getsockopt(level, optname, buflen)` | **`bytes`** | os bytes crus, **na ordem da máquina**, com o tamanho que o sistema escreveu (≤ `buflen`) |

Um exemplo do formato cru: `SO_REUSEADDR` ligado, lido com `buflen=4` numa
máquina little-endian, é `b'\x01\x00\x00\x00'` — 4 bytes, e o `1` está no
primeiro.

Booleana ligada devolve `1`, desligada devolve `0` — nunca `true`/`false`.

## Erros

- **`TypeError`** — `getsockopt() takes at least 2 arguments (1 given)` /
  `getsockopt() takes at most 3 arguments (4 given)`
- **`TypeError`** — `getsockopt: level/optname devem ser int`
- **`TypeError`** — `getsockopt: buflen invalido`, para `buflen` ≤ 0 ou > 1024
- **`TypeError`** — `buflen` que não é `int`:
  `'str' object cannot be interpreted as an integer`
- **`RuntimeError`** — opção que a camada não conhece:
  `[Errno 92] Protocol not available`
- **`OSError`** — socket fechado: `[Errno 9] Bad file descriptor`

## Exemplos

```ps
import sockets
s = sockets.socket()
s.setsockopt(sockets.SOL_SOCKET, sockets.SO_REUSEADDR, 1)

v = s.getsockopt(sockets.SOL_SOCKET, sockets.SO_REUSEADDR)
post(type(v), v)

b = s.getsockopt(sockets.SOL_SOCKET, sockets.SO_REUSEADDR, 4)
post(type(b), len(b), b)
s.close()
```

```saida
int 1
bytes 4 b'\x01\x00\x00\x00'
```

`SO_TYPE` diz com que tipo o socket nasceu:

```ps
import sockets
t = sockets.socket(sockets.AF_INET, sockets.SOCK_STREAM)
u = sockets.socket(sockets.AF_INET, sockets.SOCK_DGRAM)
post(t.getsockopt(sockets.SOL_SOCKET, sockets.SO_TYPE) == sockets.SOCK_STREAM)
post(u.getsockopt(sockets.SOL_SOCKET, sockets.SO_TYPE) == sockets.SOCK_DGRAM)
t.close(); u.close()
```

```saida
True
True
```

## Bordas

- `SO_ERROR` **lê e limpa** o erro pendente: a segunda leitura seguida devolve
  `0`
- `buflen` é o teto do buffer, não o tamanho garantido do resultado — confira
  com `len()`
- o campo `.type` do objeto guarda o que foi pedido no `sockets.socket(...)`;
  `SO_TYPE` é o que o sistema registrou. São dois caminhos diferentes pro mesmo
  número
- não existe `getsockopt` "de tudo": cada opção é uma chamada

[← índice](../sockets.md)
