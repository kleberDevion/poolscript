# `l.reverse()`

Inverte a ordem dos itens NO LUGAR (muta).

## Retorno

null — muta a lista

## Exemplos

```ps
l = [1, 2, 3]
l.reverse()
post(l)
```

```saida
[3, 2, 1]
```

## Bordas

- não devolve a lista: `post(l.reverse())` imprime `null`
- pra uma CÓPIA invertida, use o builtin `reversed(l)`

[← índice](../list.md)
