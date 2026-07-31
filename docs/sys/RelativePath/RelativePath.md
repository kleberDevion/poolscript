# `sys.RelativePath(nome)`

Busca um arquivo pelo nome, **recursivamente**, a partir do diretório de
trabalho atual, e devolve o caminho encontrado.

```
sys.RelativePath(nome: str) -> str
```

---

## Uso

```
import sys

caminho = sys.RelativePath("config.json")
post(caminho)     // acha o config.json em qualquer subpasta
```

Diferente de [`os.pathFile`](../../os/pathFile/pathFile.md), que busca a partir
da pasta do `.ps` e do cwd, o `RelativePath` faz uma **busca recursiva** a
partir do cwd — útil quando você não sabe exatamente em qual subpasta o arquivo
está.

---

## Relacionados

- [`os.pathFile()`](../../os/pathFile/pathFile.md) — busca por nome (raízes fixas)
