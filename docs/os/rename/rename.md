# `os.rename(src, dst)`

Renomeia um arquivo ou pasta.

```
os.rename(src: str, dst: str) -> None
```

| Parâmetro | O que é |
|---|---|
| `src` | nome/caminho atual |
| `dst` | novo nome/caminho |

---

## Uso

```
import os

os.rename("rascunho.txt", "final.txt")       # troca o nome
os.rename("fotos", "imagens")                # renomeia a pasta
```

---

## Relacionados

- [`os.move()`](../move/move.md) — mover pra outra pasta (também renomeia)
- [`os.copy()`](../copy/copy.md) — duplicar
