# `Parsing.boolean(value, to_type="bool")`

Converte um valor pra **booleano** (`True`/`False`), entendendo textos comuns.

```
Parsing.boolean(value, to_type="bool") -> bool
```

`to_type` existe por uniformidade com as outras funções do `Parsing` — aqui
**não muda nada**: o resultado é sempre `bool`.

---

## Uso

```
import Parsing

Parsing.boolean("sim")       # True
Parsing.boolean("true")      # True
Parsing.boolean("1")         # True
Parsing.boolean("não")       # False
Parsing.boolean("0")         # False
Parsing.boolean("")          # False
```

Textos que representam "vazio/falso" (`""`, `"0"`, `"false"`, `"null"`,
`"none"`) viram `False`; o resto vira `True`.

---

## Relacionados

- [visão geral do Parsing](../Parsing.md)
