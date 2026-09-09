# `t.index(item, inicio=0, fim=len)`

Posição da primeira ocorrência do item na tupla.

## Parâmetros

| nome | tipo | default | nota |
|---|---|---|---|
| `item` | qualquer | — | o valor a procurar |
| `inicio` | int | 0 | onde começa a busca; negativo conta do fim |
| `fim` | int | tamanho | onde ela para (exclusive); negativo conta do fim |

## Retorno

int — a posição, contada do **começo da tupla**, não do `inicio`.

## Erros

- **ValueError** — o item não está no trecho procurado:
  `tup.index(x): x not in tup`. A frase diz `tup`, não `list`: é o mesmo código
  da lista, com um ramo próprio pro nome do tipo.
- **TypeError** — `inicio` ou `fim` que não é `int`:
  `slice indices must be integers or have an __index__ method`.
- **TypeError** — sem argumento nenhum:
  `index() takes at least 1 argument (0 given)`.

## Exemplos

```ps
t = (10, 20, 30, 20)
post(t.index(20))
post(t.index(20, 2))
post(t.index(20, -3))
post(t.index(20, 0, 2))
```

```saida
1
3
1
1
```

## Bordas

- ERRA se não achar — pra só testar presença, use
  [`contains`](../contains/contains.md)
- `fim` é **exclusive**: `t.index(30, 0, 2)` não enxerga a posição 2
- `inicio` maior que o tamanho não é erro de índice — a busca simplesmente não
  encontra nada e o resultado é o **ValueError**
- `inicio` negativo é somado ao tamanho antes da busca: numa tupla de 4 itens,
  `-3` começa na posição 1

[← índice](../tup.md)
