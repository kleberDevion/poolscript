# `post(v1, v2, ...)`

Imprime os valores no stdout, separados por espaço, com quebra de linha no final.

## Parâmetros

| nome | tipo | default | nota |
|---|---|---|---|
| `v...` | qualquer | — | zero ou mais valores |

## Retorno

Null

## Exemplos

```ps
post(1, "a", true)
```

```saida
1 a True
```

```ps
post([1, Null], {"k": 2})
```

```saida
[1, null] {'k': 2}
```

## Bordas

- `post()` sem argumento nenhum não imprime nada — nem a quebra de linha; linha em branco é `post("")`
- `Null` imprime `Null`, inclusive aninhado (`[Null]` → `[Null]`)

[← índice](../builtins.md)
