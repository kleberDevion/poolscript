# `os.cwd()`

Devolve o **diretório de trabalho atual** (current working directory) — a pasta
de onde o programa está rodando.

```
os.cwd() -> str
```

---

## Uso

```
import os

post(os.cwd())     # ex: "C:\Users\ana\projeto"
```

Útil pra montar caminhos ou entender de onde caminhos relativos partem.

---

## Relacionados

- [`os.chdir()`](../chdir/chdir.md) — mudar o diretório atual
- [`os.ls()`](../ls/ls.md) — listar o conteúdo (usa `"."` = cwd por padrão)
