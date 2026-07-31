# `s.strip(chars=Null)`

Remove espaços (ou os chars dados) das duas pontas.

## Parâmetros

| nome | tipo | default | nota |
|---|---|---|---|
| `chars` | str | Null | conjunto de caracteres a remover |

## Retorno

str

## Exemplos

```ps
post("  oi  ".strip() + "!")
```

```saida
oi!
```

```ps
post("xxoixx".strip("x"))
```

```saida
oi
```

[← índice](../string.md)
