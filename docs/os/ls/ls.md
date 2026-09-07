# `os.ls(path=".")`

Lista o conteúdo de uma pasta. Devolve uma **lista de dicts**, um por item, com
nome, tipo e tamanho.

```
os.ls(path: str = ".") -> list
```

| Parâmetro | Padrão | O que é |
|---|---|---|
| `path` | `"."` (pasta atual) | a pasta a listar |

---

## Formato do retorno

Cada item é um dict:

```
{"name": "foto.png", "type": "file", "size": 20481}
{"name": "uploads",  "type": "dir",  "size": 0}
```

| Chave | O que é |
|---|---|
| `name` | nome do arquivo ou pasta |
| `type` | `"file"` ou `"dir"` |
| `size` | tamanho em bytes (`0` para pastas) |

---

## Uso

```
import os

for each item in os.ls("uploads") {
    if (item["type"] == "file") {
        post(item["name"], "-", item["size"], "bytes")
    } else {
        post("[pasta]", item["name"])
    }
}
```

Listar a pasta atual (sem argumento):

```
for each item in os.ls() {
    post(item["name"])
}
```

---

## Só os arquivos, só as pastas

Filtre pelo `type`:

```
arquivos = filter(os.ls("dados"), funct(i) { return i["type"] == "file" })
pastas   = filter(os.ls("dados"), funct(i) { return i["type"] == "dir" })
```

---

## Relacionados

- [`os.exists()`](../exists/exists.md) / [`os.isdir()`](../isdir/isdir.md) — checar antes de listar
- [`os.size()`](../size/size.md) — tamanho de um arquivo específico
