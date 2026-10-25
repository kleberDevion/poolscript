# `os.mkdir(path, exist_ok=False)`

Cria uma pasta (diretório), e as pastas do caminho que faltarem, como o
`os.makedirs` do Python. A barra no fim não muda nada: `"dados/2026/"` cria
`dados/2026`.

```assinatura
os.mkdir(path: str, exist_ok: bool = False) -> Null
```

| Parâmetro | Padrão | O que é |
|---|---|---|
| `path` | — | caminho da pasta a criar |
| `exist_ok` | `False` | `True` = não dá erro se a pasta já existir |

Os exemplos desta página pressupõem:

```ps contexto
import os
```

---

## Uso

```ps
import os

os.mkdir("uploads")                      # cria "uploads"
os.mkdir("dados/2026", exist_ok=True)    # cria; se já existir, tudo bem
```

Com `exist_ok=False` (padrão), criar uma pasta que já existe **levanta erro**.
Use `exist_ok=True` quando "garantir que a pasta exista" for a intenção:

```ps
os.mkdir("cache", exist_ok=True)   # roda quantas vezes quiser, sem erro
```

O `exist_ok` aceita o que já existe só se for **pasta**. Sobre um arquivo, a
pasta pedida não existe, e o erro diz isso (antes passava calado):

```ps
import os
os.writeFile("nota.txt", "x")
try {
    os.mkdir("nota.txt", exist_ok=True)
} catch (FileExistsError e) {
    post(e)
}
```

```saida
[Errno 17] File exists: 'nota.txt' (linha 4)
```

Uma pasta do caminho que não pode ser criada (um arquivo no meio dele) é o
erro, com o caminho **até ela**, e nada depois dela é criado, como no
`os.makedirs`:

```ps
import os
os.writeFile("relatorio", "x")
try {
    os.mkdir("relatorio/2026/out", exist_ok=True)
} catch (NotADirectoryError e) {
    post(e)
}
```

```saida
[Errno 20] Not a directory: 'relatorio/2026' (linha 4)
```

---

## Padrão comum: criar só se não existe

O exemplo pressupõe:

```ps contexto
import os
```

```ps
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
