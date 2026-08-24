# `Parsing.boolean(value, to_type="bool")`

Converte um valor pra **booleano** (`true`/`false`), entendendo textos comuns.

```
Parsing.boolean(value, to_type="bool") -> bool
```

`to_type` existe por uniformidade com as outras funções do `Parsing` — aqui
**não muda nada**: o resultado é sempre `bool`.

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
