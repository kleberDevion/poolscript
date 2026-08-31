# `b.isalpha()`

True se tem pelo menos um byte e TODOS são letras ASCII.

## Parâmetros

Nenhum.

## Retorno

bool

## Exemplos

```ps
import bytes
post("abc".encode().isalpha())
post("a1".encode().isalpha())
post("".encode().isalpha())
```

```saida
True
False
False
```

[← índice](../../bytes.md)
