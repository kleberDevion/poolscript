# `s.format(a, b, ...)`

Preenche {} posicionais e {nome} nomeados.

## Parâmetros

| nome | tipo | default | nota |
|---|---|---|---|
| `a, b...` | qualquer | — |  |

## Retorno

str

## Erros

- **TypeError** — a chave ou o índice citado no formato não existe

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
