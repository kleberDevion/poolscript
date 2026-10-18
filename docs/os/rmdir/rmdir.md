# `os.rmdir(path, force=False)`

Remove uma pasta.

```assinatura
os.rmdir(path: str, force: bool = False) -> Null
```

| Parâmetro | Padrão | O que é |
|---|---|---|
| `path` | — | pasta a remover |
| `force` | `False` | `True` = remove **mesmo com conteúdo** dentro |

Os exemplos desta página pressupõem:

```ps contexto
import os
```

---

## Uso

```ps
import os

os.rmdir("temp")                   # só remove se estiver VAZIA
os.rmdir("cache", force=True)      # remove a pasta e tudo dentro dela
```

---

## `force` é destrutivo — cuidado

Com `force=False` (padrão), remover uma pasta que **tem arquivos** dá erro — é
uma proteção. Com `force=True`, a pasta e **todo o conteúdo** são apagados
recursivamente, sem confirmação e **sem volta**, em qualquer profundidade
(antes desistia acima de 32 níveis de pasta, com "Directory not empty" e nada
apagado).

Um **link** dentro da pasta (link simbólico; no Windows também a junção de
pasta) sai como link: o `force` não entra nele e não apaga o que está do
outro lado — como o `rm -rf`. Se o próprio `path` é um link, sai só o link.

Confirme o caminho antes de usar `force=True`:

```ps
if (os.isdir("temp") and os.exists("temp")) {
    os.rmdir("temp", force=True)
}
```

---

## Relacionados

- [`os.mkdir()`](../mkdir/mkdir.md) — criar pasta
- [`os.exists()`](../exists/exists.md) — checar antes de remover
