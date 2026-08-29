# `l.index(item, inicio=0, fim=len)`

Posição da primeira ocorrência do item.

## Parâmetros

| nome | tipo | default | nota |
|---|---|---|---|
| `item` | qualquer | — | o valor a procurar |

## Retorno

int — a posição

## Erros

- **ValueError** — o item não está na lista

## Exemplos

```ps
post([10, 20, 30].index(20))
```

```saida
1
```

## Bordas

- ERRA se não achar — pra só testar presença, use `contains`

[← índice](../list.md)
