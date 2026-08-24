# `os.exists(path)`

Diz se um caminho **existe** — seja arquivo ou pasta. Devolve `true`/`false`.

```
os.exists(path: str) -> bool
```

---

## Uso

```
import os

if (os.exists("config.json")) {
    cfg = os.loadFile("config.json")
} else {
    post("config não encontrado")
}
```

---

## `exists` vs `isfile` vs `isdir`

| Função | `true` quando… |
|---|---|
| `os.exists(x)` | existe (arquivo **ou** pasta) |
| [`os.isfile(x)`](../isfile/isfile.md) | existe **e** é arquivo |
| [`os.isdir(x)`](../isdir/isdir.md) | existe **e** é pasta |

Use `exists` quando o tipo não importa; use `isfile`/`isdir` quando importa.

---

## Relacionados

- [`os.isfile()`](../isfile/isfile.md) · [`os.isdir()`](../isdir/isdir.md)
- [`os.loadFile()`](../loadFile/loadFile.md) — ler depois de confirmar que existe
