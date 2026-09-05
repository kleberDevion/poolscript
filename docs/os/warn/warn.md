# `os.warn(text="", color="yellow")`

Imprime uma mensagem **colorida** no terminal. Útil pra destacar avisos, erros
ou informações nos logs do seu programa.

```
os.warn(text: str = "", color: str = "yellow") -> None
```

| Parâmetro | Padrão | O que é |
|---|---|---|
| `text` | `""` | a mensagem |
| `color` | `"yellow"` | cor do texto |

---

## Uso

```
import os

os.warn("cuidado: arquivo será sobrescrito")          # amarelo (padrão)
os.warn("erro ao conectar", "red")                    # vermelho
os.warn("tudo certo", "green")                        # verde
```

---

## Cores disponíveis

`red`, `green`, `yellow`, `blue`, `magenta`, `cyan`, `white`.

---

## `os.warn` vs `post` colorido

- **`os.warn(text, cor)`** — atalho pra uma mensagem colorida no stderr,
  pensado pra avisos/logs.
- **`post(<red>"texto")`** — cor inline em qualquer `post`, pra saída normal.

Use `os.warn` pra logs de aviso; use `post` com cor pra saída comum colorida.

---

## Relacionados

- `post` — saída normal (aceita cor inline `<red>"..."`)
- Cores da linguagem — ver a seção de strings coloridas em `../LANGUAGE.md`
