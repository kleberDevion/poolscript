# `d.get(chave, default=Null)`

Valor da chave — NÃO erra se a chave não existir.

## Parâmetros

| nome | tipo | default | nota |
|---|---|---|---|
| `chave` | qualquer | — | a chave a buscar |
| `default` | qualquer | Null | o que devolver quando a chave falta |

## Retorno

o valor, ou o `default`

## Exemplos

```ps
d = { "a": 1 }
post(d.get("a"), d.get("z"), d.get("z", 0))
```

```saida
1 null 0
```

## Bordas

- `d["z"]` inexistente dá KeyError; `d.get("z")` devolve `Null`

[← índice](../dict.md)
