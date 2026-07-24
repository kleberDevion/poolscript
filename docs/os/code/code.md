# `os.code(caminho=".")`

Abre o **editor de código** do sistema (ex: VS Code) no caminho indicado.

```
os.code(caminho: str = ".") -> None
```

| Parâmetro | Padrão | O que é |
|---|---|---|
| `caminho` | `"."` (pasta atual) | arquivo ou pasta a abrir no editor |

---

## Uso

```
import os

os.code()                  // abre o editor na pasta atual
os.code("config.json")     // abre esse arquivo
os.code("frontend")        // abre essa pasta
```

Depende de o editor estar instalado e disponível no `PATH` (ex: o comando
`code` do VS Code).

---

## Relacionados

- [`os.cmd()`](../cmd/cmd.md) — rodar qualquer comando do terminal
