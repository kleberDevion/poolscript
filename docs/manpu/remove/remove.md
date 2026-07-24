# `manpu.remove(value, column, celula, amount, target)`

Remove conteúdo de um arquivo — por posição (coluna/célula) em CSV, ou por
texto encontrado em HTML/txt.

```
manpu.remove(value=..., column=..., celula=..., amount=..., target="arquivo") -> ManpuResult
```

| Parâmetro | O que é |
|---|---|
| `value` | o texto a remover (em HTML/txt) |
| `column` / `celula` | posição a limpar (em CSV) |
| `amount` | `full` = remove tudo que encontrar; `mei` = remove metade |
| `target` | caminho do arquivo |

---

## Remover por posição (CSV)

```
import manpu as mp

x = mp.remove(
    column=2,
    celula=5,
    amount=full,
    target="planilha.csv"
)
```

## Remover por texto (HTML / txt)

```
x = mp.remove(
    value="Texto alvo",
    amount=full,
    target="pagina.html"
)
```

## `amount`

| Valor | Efeito |
|---|---|
| `full` | remove **todas** as ocorrências encontradas |
| `mei` | remove **metade** do texto encontrado |

---

## Relacionados

- [`manpu.write()`](../write/write.md) — escrever
- [`manpu.read()`](../read/read.md) — ler
