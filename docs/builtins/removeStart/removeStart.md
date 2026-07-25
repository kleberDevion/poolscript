# `removeStart(lista)`

Remove o **primeiro** item da lista e o devolve. Modifica a lista original.

```
removeStart(lista) -> valor | Null
```

---

## Uso

```
fila = ["a", "b", "c"]
primeiro = removeStart(fila)
post(primeiro)          // "a"
post(fila)              // ["b", "c"]
```

Em lista **vazia**, devolve `Null`.

---

## Uso comum: fila (FIFO)

`addEnd` + `removeStart` = fila (primeiro a entrar, primeiro a sair):

```
fila = []
addEnd(fila, "tarefa1")     // entra no fim
addEnd(fila, "tarefa2")
proxima = removeStart(fila) // sai do início → "tarefa1"
```

---

## Relacionados

- [`addStart()`](../addStart/addStart.md) — o oposto
- [`removeEnd()`](../removeEnd/removeEnd.md) — remover do fim (pilha/LIFO)
