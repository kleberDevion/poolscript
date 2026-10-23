# `os.chdir(path)`

Muda o **diretório de trabalho atual** — daí em diante, caminhos relativos
partem da nova pasta.

```assinatura
os.chdir(path: str) -> Null
```

Os exemplos desta página pressupõem a pasta `dados`:

```ps contexto
import os
os.mkdir("dados")
```

---

## Uso

```ps
import os

post(os.cwd())          # "C:\projeto"
os.chdir("dados")
post(os.cwd())          # "C:\projeto\dados"
```

---

## Relacionados

- [`os.cwd()`](../cwd/cwd.md) — ver o diretório atual
