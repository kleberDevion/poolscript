# `os.mkdir(path, exist_ok=false)`

Cria uma pasta (diretório).

```
os.mkdir(path: str, exist_ok: bool = false) -> None
```

| Parâmetro | Padrão | O que é |
|---|---|---|
| `path` | — | caminho da pasta a criar |
| `exist_ok` | `false` | `true` = não dá erro se a pasta já existir |

---

## Uso

```
import os

os.mkdir("uploads")                      # cria "uploads"
os.mkdir("dados/2026", exist_ok=true)    # cria; se já existir, tudo bem
```

Com `exist_ok=false` (padrão), criar uma pasta que já existe **levanta erro**.
Use `exist_ok=true` quando "garantir que a pasta exista" for a intenção:

```
os.mkdir("cache", exist_ok=true)   # roda quantas vezes quiser, sem erro
```

---

## Padrão comum: criar só se não existe

```
if (not os.exists("uploads")) {
    os.mkdir("uploads")
}
# ou, mais curto:
os.mkdir("uploads", exist_ok=true)
```

---

## Relacionados

- [`os.rmdir()`](../rmdir/rmdir.md) — remover pasta
- [`os.exists()`](../exists/exists.md) / [`os.isdir()`](../isdir/isdir.md) — checar
