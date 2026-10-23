# `os.rename(src, dst)`

Renomeia um arquivo ou pasta.

```assinatura
os.rename(src: str, dst: str) -> Null
```

| Parâmetro | O que é |
|---|---|
| `src` | nome/caminho atual |
| `dst` | novo nome/caminho |

Os exemplos desta página pressupõem o arquivo `rascunho.txt` e a pasta
`fotos`:

```ps contexto
import os
os.writeFile("rascunho.txt", "texto do rascunho")
os.mkdir("fotos")
```

---

## Uso

```ps
import os

os.rename("rascunho.txt", "final.txt")       # troca o nome
os.rename("fotos", "imagens")                # renomeia a pasta
```

Se já existe um **arquivo** com o novo nome, ele é substituído, nos três
sistemas (no Windows também: o `rename` do sistema lá recusava).

---

## Relacionados

- [`os.move()`](../move/move.md) — mover pra outra pasta (também renomeia)
- [`os.copy()`](../copy/copy.md) — duplicar
