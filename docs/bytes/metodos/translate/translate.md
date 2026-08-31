# `b.translate(tabela, delete=Null)`

Traduz byte a byte pela tabela de 256 bytes. `Null` no lugar da tabela só apaga; `delete` diz quais bytes somem.

## Parâmetros

| nome | default |
|---|---|
| `tabela` | — |
| `delete` | `Null` |

## Retorno

bytes

## Exemplos

```ps
import bytes
t = "".encode().maketrans("abc".encode(), "xyz".encode())
post("abcabc".encode().translate(t))
post("abcabc".encode().translate(Null, "b".encode()))
```

```saida
b'xyzxyz'
b'acac'
```

[← índice](../../bytes.md)
