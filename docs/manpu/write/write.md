# `manpu.write(content="", column=0, celula=0, target="")`

Escreve um valor numa posição (coluna/célula) de um arquivo CSV/XLSX/texto, em
**uma operação única** (abre, escreve, salva).

```
manpu.write(content=..., column=0, celula=..., target="arquivo.csv") -> ManpuResult
```

| Parâmetro | O que é |
|---|---|
| `content` | o valor a escrever |
| `column` | número da coluna (0 = primeira); `full` = todas |
| `celula` (ou `cell`) | índice da linha/célula; `full` = descendo linha a linha |
| `target` | caminho do arquivo |

Devolve um `ManpuResult` — dá pra checar se deu certo com `==`.

---

## Uso

```
import manpu as mp

nm = mp.write(
    content="Kleber",
    column=0,
    celula=3,
    target="planilha.csv"
)

if (nm == "Success") {
    post("escrito")
} else {
    post("erro:", nm.status)
}
```

Para `.txt` e formatos sem estrutura, o conteúdo é **adicionado ao final** do
arquivo.

---

## `celula=full` — preencher descendo

Com `celula=full` (ou `cell=full`), o conteúdo é dividido por quebra de linha e
cada parte vai numa célula, descendo na coluna:

```
// content com "Arroz\nFeijão\nMacarrão" e cell=full:
// coluna 0, linha 0 → Arroz
// coluna 0, linha 1 → Feijão
// coluna 0, linha 2 → Macarrão
```

---

## Uma escrita vs. várias

`mp.write(...)` abre e fecha o arquivo a cada chamada. Se você vai escrever
**várias** vezes no mesmo arquivo, use [`mp.open()`](../open/open.md) com
`using` — abre uma vez só e é mais eficiente.

---

## Relacionados

- [`manpu.open()`](../open/open.md) — várias operações no mesmo arquivo
- [`manpu.remove()`](../remove/remove.md) — remover conteúdo
- [`ManpuFile.write()`](../ManpuFile/write/write.md) — a versão dentro do `using`
