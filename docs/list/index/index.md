# `l.index(item, inicio=0, fim=len)`

Posição da primeira ocorrência do item.

## Parâmetros

| nome | tipo | default | nota |
|---|---|---|---|
| `item` | qualquer | — | o valor a procurar |
| `inicio` | int | 0 | onde começa a busca |
| `fim` | int | tamanho | onde ela para (exclusive) |

## Retorno

int — a posição, contada do **começo da lista**, não do `inicio`

## Erros

- **ValueError** — o item não está na lista

## Exemplos

```ps
post([10, 20, 30].index(20))
post([1, 2, 1, 2].index(2, 2))
post([1, 2, 1, 2].index(1, 1, 3))
```

```saida
1
3
2
```

## Bordas

- ERRA se não achar — pra só testar presença, use `contains`

[← índice](../list.md)
