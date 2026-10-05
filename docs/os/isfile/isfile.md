# `os.isfile(path)`

Diz se o caminho existe **e é um arquivo** (não uma pasta). Devolve
`True`/`False`.

```
os.isfile(path: str) -> bool
```

---

## Uso

```
import os

if (os.isfile("dados.csv")) {
    tabela = os.loadFile("dados.csv")
}
```

Retorna `False` tanto pra caminho inexistente quanto pra uma **pasta** — só é
`True` pra arquivo de verdade.

---

## Relacionados

- [`os.isdir()`](../isdir/isdir.md) — testa se é pasta
- [`os.exists()`](../exists/exists.md) — existe, tanto faz o tipo
