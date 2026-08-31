# `b.rsplit(sep=Null, maxsplit=-1)`

Como `split`, mas contando do fim quando há `maxsplit`.

## Parâmetros

| nome | default |
|---|---|
| `sep` | `Null` |
| `maxsplit` | `-1` |

## Retorno

list

## Exemplos

```ps
import bytes
post("a-b-c".encode().rsplit("-".encode(), 1))
```

```saida
[b'a-b', b'c']
```

[← índice](../../bytes.md)
