# `app.POOLHTMLElements`

O **registro dos elementos** do app (campo, sem parênteses). É por ele que o
seu código acha qualquer elemento pelo atributo `name=` — em qualquer nível da
árvore — e lê o valor:

```
nome = app.POOLHTMLElements.getitemByIdentify("nome").value
```

Ver [getitemByIdentify](../getitemByIdentify/getitemByIdentify.md) e
[value](../value/value.md).

[← índice](../guzer.md)
