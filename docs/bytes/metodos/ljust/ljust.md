# `b.ljust(width, fillbyte)`

Enche à direita até a largura. `fillbyte` tem que ter exatamente um byte.

## Parâmetros

| nome | default |
|---|---|
| `width` | — |
| `fillbyte` | espaço |

Omitir o `fillbyte` preenche com espaço.

## Retorno

bytes

## Exemplos

```ps
import bytes
post("ab".encode().ljust(5, ".".encode()))
```

```saida
b'ab...'
```

[← índice](../../bytes.md)
