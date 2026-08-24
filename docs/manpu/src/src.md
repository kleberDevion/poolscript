# `manpu.src(filepath)`

Resolve e devolve o **caminho absoluto** de um arquivo.

```
manpu.src(filepath: str) -> str
```

---

## Uso

```
import manpu as mp

caminho = mp.src("planilha.xlsx")
post(caminho)     // "C:\projeto\planilha.xlsx"
```

Não lê o arquivo — só resolve onde ele está.

---

## Relacionados

- [`os.pathFile()`](../../os/pathFile/pathFile.md) — o equivalente na lib `os`
- [`manpu.read()`](../read/read.md) — ler o conteúdo
