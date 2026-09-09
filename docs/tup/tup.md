# Métodos de tup

Chamados direto no valor: `t.metodo()`. A tupla é **imutável**, então aqui só
existem os métodos de **leitura**: nenhum deles muta nada e nenhum devolve
`null` — todos devolvem um valor (`int` ou `bool`).

A lista sai da tabela `METODOS_TUPLA` de `vm/poolscript_vm.c`, publicada por
`pool --metadata`; cada página traz um exemplo executável.

| nome | assinatura | o que faz |
|---|---|---|
| [`contains`](contains/contains.md) | `t.contains(item)` | O item está na tupla? (o mesmo que `item in t`) |
| [`count`](count/count.md) | `t.count(item)` | Quantas vezes o item aparece. |
| [`has`](has/has.md) | `t.has(item)` | Apelido de `contains` — as duas grafias existem. |
| [`index`](index/index.md) | `t.index(item, inicio=0, fim=len)` | Posição da primeira ocorrência do item. |
| [`len`](len/len.md) | `t.len()` | Quantidade de itens — forma de método do builtin `len`. |

## São CINCO, e só esses

A tabela da tupla é um subconjunto da tabela de `list`: os cinco de leitura, e
mais nada. Os métodos que **mudam** a sequência — `append`, `extend`, `insert`,
`pop`, `remove`, `clear`, `reverse`, `sort` — **não existem** na tupla, e
chamá-los dá `AttributeError`:

```ps
post((1, 2, 3).len())
```

```saida
3
```

`(1, 2, 3).append(9)` responde
`AttributeError: 'tup' object has no attribute 'append'`. O mesmo para `sort`,
`pop` e `remove`, trocando o nome no fim da frase.

Os cinco métodos de leitura são os **mesmos** de `list` por dentro (a tupla
reusa `met_l_index`, `met_l_count`, `met_l_contains` e `met_l_len`), então
comparação de item, tratamento de índice negativo e formato de retorno são
idênticos aos da lista. A única diferença de comportamento visível está na
mensagem de erro do [`index`](index/index.md), que diz `tup` e não `list`.

## Onde `copy` estaria

`list` tem `copy`; a tupla não, e não precisa: como ela é imutável, `t2 = t`
já é seguro — não há como um lado alterar o outro.

## Para ordenar ou inverter, use os builtins

`sorted(t)` e `reversed(t)` funcionam com tupla, mas devolvem **`list`**, não
`tup`:

```ps
t = (10, 20, 30, 20)
post(sorted(t), reversed(t), type(sorted(t)))
```

```saida
[10, 20, 20, 30] [20, 30, 20, 10] list
```

Fatia e concatenação, ao contrário, continuam `tup`:

```ps
t = (10, 20, 30, 20)
post(t[1:3], t + (40,), type(t[1:3]))
```

```saida
(20, 30) (10, 20, 30, 20, 40) tup
```

[← índice geral](../INDEX.md)
