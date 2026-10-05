# `os.isdir(path)`

Diz se o caminho existe **e é uma pasta** (não um arquivo). Devolve
`True`/`False`.

```
os.isdir(path: str) -> bool
```

---

## Uso

```
import os

if (os.isdir("uploads")) {
    for each item in os.ls("uploads") {
        post(item["name"])
    }
} else {
    os.mkdir("uploads")
}
```

Retorna `False` pra caminho inexistente ou pra um **arquivo** — só `True` pra
pasta.

---

## Relacionados

- [`os.isfile()`](../isfile/isfile.md) — testa se é arquivo
- [`os.exists()`](../exists/exists.md) — existe, tanto faz o tipo
- [`os.ls()`](../ls/ls.md) — listar o conteúdo da pasta
