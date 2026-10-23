# `os.size(path)`

Devolve o **tamanho de um arquivo em bytes**.

```assinatura
os.size(path: str) -> int
```

Os exemplos desta página pressupõem os arquivos `video.mp4` e `enviado.pdf`:

```ps contexto
import os
os.writeFile("video.mp4", "x" * 2048)
os.writeFile("enviado.pdf", "y" * 512)
```

---

## Uso

```ps
import os

bytes = os.size("video.mp4")
post(bytes, "bytes")
post(bytes / 1024 / 1024, "MB")     # converte pra megabytes
```

---

## Exemplo: recusar upload grande demais

```ps
if (os.size("enviado.pdf") > 5 * 1024 * 1024) {   # maior que 5 MB
    post("arquivo grande demais")
}
```

---

## Relacionados

- [`os.ls()`](../ls/ls.md) — lista com tamanho de vários itens
- [`os.isfile()`](../isfile/isfile.md) — confirmar que é arquivo antes
