# `os.mkdir(path, exist_ok=False)`

Cria uma pasta (diretório).

```
os.mkdir(path: str, exist_ok: bool = False) -> Null
```

| Parâmetro | Padrão | O que é |
|---|---|---|
| `path` | — | caminho da pasta a criar |
| `exist_ok` | `False` | `True` = não dá erro se a pasta já existir |

---

## Uso

```
import os

os.mkdir("uploads")                      # cria "uploads"
os.mkdir("dados/2026", exist_ok=True)    # cria; se já existir, tudo bem
```

Com `exist_ok=False` (padrão), criar uma pasta que já existe **levanta erro**.
Use `exist_ok=True` quando "garantir que a pasta exista" for a intenção:

```
os.mkdir("cache", exist_ok=True)   # roda quantas vezes quiser, sem erro
```

---

## Padrão comum: criar só se não existe

```
if (not os.exists("uploads")) {
    os.mkdir("uploads")
}
# ou, mais curto:
os.mkdir("uploads", exist_ok=True)
```

---

## Relacionados

- [`os.rmdir()`](../rmdir/rmdir.md) — remover pasta
- [`os.exists()`](../exists/exists.md) / [`os.isdir()`](../isdir/isdir.md) — checar
