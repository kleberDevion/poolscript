# `b.isspace()`

True se tem pelo menos um byte e todos são branco ASCII (espaço, `\t`, `\n`, `\r`, `\v`, `\f`).

## Parâmetros

Nenhum.

## Retorno

bool

## Exemplos

```ps
import bytes
post(" \t\n".encode().isspace())
```

```saida
True
```

[← índice](../../bytes.md)
