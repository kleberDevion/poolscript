# `os.rmdir(path, force=false)`

Remove uma pasta.

```
os.rmdir(path: str, force: bool = false) -> Null
```

| Parâmetro | Padrão | O que é |
|---|---|---|
| `path` | — | pasta a remover |
| `force` | `false` | `true` = remove **mesmo com conteúdo** dentro |

---

## Uso

```
import os

os.rmdir("temp")                   # só remove se estiver VAZIA
os.rmdir("cache", force=true)      # remove a pasta e tudo dentro dela
```

---

## `force` é destrutivo — cuidado

Com `force=false` (padrão), remover uma pasta que **tem arquivos** dá erro — é
uma proteção. Com `force=true`, a pasta e **todo o conteúdo** são apagados
recursivamente, sem confirmação e **sem volta**.

Um **link** dentro da pasta (link simbólico; no Windows também a junção de
pasta) sai como link: o `force` não entra nele e não apaga o que está do
outro lado — como o `rm -rf`. Se o próprio `path` é um link, sai só o link.

Confirme o caminho antes de usar `force=true`:

```
if (os.isdir("temp") and os.exists("temp")) {
    os.rmdir("temp", force=true)
}
```

---

## Relacionados

- [`os.mkdir()`](../mkdir/mkdir.md) — criar pasta
- [`os.exists()`](../exists/exists.md) — checar antes de remover
