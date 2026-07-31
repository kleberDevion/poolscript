# `Parsing.boolean(valor)`

Converte um valor pra **booleano** (`true`/`false`), entendendo textos comuns.

```
Parsing.boolean(valor) -> bool
```

---

## Uso

```
Parsing.boolean("sim")       // true
Parsing.boolean("true")      // true
Parsing.boolean("1")         // true
Parsing.boolean("não")       // false
Parsing.boolean("0")         // false
Parsing.boolean("")          // false
```

Textos que representam "vazio/falso" (`""`, `"0"`, `"false"`, `"null"`,
`"none"`) viram `false`; o resto vira `true`.

---

## Relacionados

- [visão geral do Parsing](../Parsing.md)
