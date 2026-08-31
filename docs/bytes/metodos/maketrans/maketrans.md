# `b.maketrans(de, para)`

Monta a tabela de 256 bytes que o `translate` usa. Os dois argumentos precisam ter o mesmo tamanho.

## Parâmetros

| nome | default |
|---|---|
| `de` | — |
| `para` | — |

## Retorno

bytes

## Exemplos

```ps
import bytes
t = "".encode().maketrans("abc".encode(), "xyz".encode())
post("abcabc".encode().translate(t))
```

```saida
b'xyzxyz'
```

[← índice](../../bytes.md)
