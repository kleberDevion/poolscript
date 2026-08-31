# `b.removeprefix(p)`

Tira o prefixo se ele estiver lá; senão devolve igual. Não é conjunto: é o prefixo inteiro.

## Parâmetros

| nome | default |
|---|---|
| `p` | — |

## Retorno

bytes

## Exemplos

```ps
import bytes
b = "Hello".encode()
post(b.removeprefix("He".encode()))
post(b.removeprefix("zz".encode()))
```

```saida
b'llo'
b'Hello'
```

[← índice](../../bytes.md)
