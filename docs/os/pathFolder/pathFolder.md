# `os.pathFolder(name)`

Devolve o **caminho absoluto** de uma pasta, buscando-a pelo nome a partir da
pasta do `.ps` e do diretório atual. Erro se não encontrar.

```
os.pathFolder(name: str) -> str
```

---

## Uso

```
import os

pasta = os.pathFolder("uploads")
post(pasta)     # "C:\projeto\uploads"
```

Não lê nem lista — só te dá o caminho absoluto da pasta.

---

## Relacionados

- [`os.pathFile()`](../pathFile/pathFile.md) — o equivalente pra arquivos
- [`os.isdir()`](../isdir/isdir.md) — checar se a pasta existe antes
- [`os.ls()`](../ls/ls.md) — listar o conteúdo
