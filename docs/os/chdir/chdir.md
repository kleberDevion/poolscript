# `os.chdir(path)`

Muda o **diretório de trabalho atual** — daí em diante, caminhos relativos
partem da nova pasta.

```
os.chdir(path: str) -> None
```

---

## Uso

```
import os

post(os.cwd())          # "C:\projeto"
os.chdir("dados")
post(os.cwd())          # "C:\projeto\dados"
```

---

## Relacionados

- [`os.cwd()`](../cwd/cwd.md) — ver o diretório atual
