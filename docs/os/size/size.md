# `os.size(caminho)`

Devolve o **tamanho de um arquivo em bytes**.

```
os.size(caminho: str) -> int
```

---

## Uso

```
import os

bytes = os.size("video.mp4")
post(bytes, "bytes")
post(bytes / 1024 / 1024, "MB")     // converte pra megabytes
```

---

## Exemplo: recusar upload grande demais

```
if (os.size("enviado.pdf") > 5 * 1024 * 1024) {   // maior que 5 MB
    post("arquivo grande demais")
}
```

---

## Relacionados

- [`os.ls()`](../ls/ls.md) — lista com tamanho de vários itens
- [`os.isfile()`](../isfile/isfile.md) — confirmar que é arquivo antes
