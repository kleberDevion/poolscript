# `l.copy()`

Cópia RASA: lista nova, itens compartilhados.

## Retorno

list — a cópia

## Exemplos

```ps
l = [1, 2]
c = l.copy()
c.append(3)
post(l, c)
```

```saida
[1, 2] [1, 2, 3]
```

```ps
dentro = [1]
l = [dentro]
c = l.copy()
dentro.append(2)
post(c)
```

```saida
[[1, 2]]
```

## Bordas

- RASA: mexer num item aninhado aparece nas duas listas (2º exemplo)

[← índice](../list.md)
