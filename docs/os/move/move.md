# `os.move(src, dst)`

Move um arquivo ou pasta pra outro lugar. Diferente de `copy`, o original
**deixa de existir** no lugar antigo.

```
os.move(src: str, dst: str) -> None
```

| Parâmetro | O que é |
|---|---|
| `src` | caminho de origem (arquivo ou pasta) |
| `dst` | caminho de destino |

---

## Uso

```
import os

os.move("temp/foto.png", "final/foto.png")   # move o arquivo
os.move("rascunhos", "arquivados")           # move a pasta inteira
```

Também serve como "renomear + mudar de lugar" ao mesmo tempo (é só o destino
ter nome/pasta diferente).

---

## `move` vs `rename` vs `copy`

| Função | Efeito |
|---|---|
| [`os.move()`](../move/move.md) | tira do lugar antigo e põe no novo (pode mudar de pasta) |
| [`os.rename()`](../rename/rename.md) | troca o nome no mesmo lugar |
| [`os.copy()`](../copy/copy.md) | duplica — o original continua |

---

## Relacionados

- [`os.copy()`](../copy/copy.md) — copiar sem remover o original
- [`os.rename()`](../rename/rename.md) — só trocar o nome
