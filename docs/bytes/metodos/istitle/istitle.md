# `b.istitle()`

True se está no formato de título: cada letra depois de uma não-letra é maiúscula, o resto minúscula.

## Parâmetros

Nenhum.

## Retorno

bool

## Exemplos

```ps
import bytes
post("Hello World".encode().istitle())
post("HELLO".encode().istitle())
```

```saida
True
False
```

[← índice](../../bytes.md)
