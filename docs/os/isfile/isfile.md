# `os.isfile(caminho)`

Diz se o caminho existe **e é um arquivo** (não uma pasta). Devolve
`true`/`false`.

```
os.isfile(caminho: str) -> bool
```

---

## Uso

```
import os

if (os.isfile("dados.csv")) {
    tabela = os.loadFile("dados.csv")
}
```

Retorna `false` tanto pra caminho inexistente quanto pra uma **pasta** — só é
`true` pra arquivo de verdade.

---

## Relacionados

- [`os.isdir()`](../isdir/isdir.md) — testa se é pasta
- [`os.exists()`](../exists/exists.md) — existe, tanto faz o tipo
