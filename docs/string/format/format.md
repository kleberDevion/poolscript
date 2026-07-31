# `s.format(a, b, ...)`

Preenche {} posicionais e {nome} nomeados.

## Parâmetros

| nome | tipo | default | nota |
|---|---|---|---|
| `a, b...` | qualquer | — |  |

## Retorno

str

## Erros

- **SomeValueUnexpected** — chave/índice ausente

## Exemplos

```ps
post("{} e {}".format(1, "x"))
```

```saida
1 e x
```

## Bordas

- `Null` formata como null, não None

[← índice](../string.md)
