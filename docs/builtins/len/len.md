# `len(x)`

Devolve o **tamanho** de uma lista, string, tupla ou dict.

```
len(x) -> int
```

---

## Uso

```
len([1, 2, 3])              // 3   (itens da lista)
len("poolscript")          // 10  (caracteres)
len({"a": 1, "b": 2})      // 2   (chaves do dict)
len((1, 2))                // 2   (itens da tupla)
```

---

## Uso comum: checar se está vazio

```
if (len(lista) == 0) {
    post("lista vazia")
}
```

---

## Relacionados

- [`range()`](../range/range.md) — gerar sequências de números
- [`enumerate()`](../enumerate/enumerate.md) — item + índice ao iterar
