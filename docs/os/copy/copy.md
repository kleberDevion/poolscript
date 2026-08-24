# `os.copy(src, dst)`

Copia um arquivo de um caminho pra outro. O original permanece.

```
os.copy(src: str, dst: str) -> None
```

| Parâmetro | O que é |
|---|---|
| `src` | caminho do arquivo de origem |
| `dst` | caminho de destino |

---

## Uso

```
import os

os.copy("dados.db", "backup/dados.db")     // duplica o arquivo
os.copy("config.json", "config.bak.json")  // cópia de segurança
```

Depois de copiar, os **dois** arquivos existem.

---

## `os.copy` vs `PoolFile.copy`

- **`os.copy(src, dst)`** — trabalha por **caminho**, sem carregar o conteúdo
  na memória. Bom pra arquivos grandes.
- **[`PoolFile.copy()`](../PoolFile/PoolFile.md)** — quando você já carregou o
  arquivo com `loadFile`.

---

## Relacionados

- [`os.move()`](../move/move.md) — mover (não deixa o original)
- [`os.rename()`](../rename/rename.md) — renomear
