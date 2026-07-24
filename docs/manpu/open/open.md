# `manpu.open(target, encoding="utf-8")`

Abre um arquivo pra **várias operações** (ler + escrever) e o mantém aberto.
Devolve um [`ManpuFile`](../ManpuFile/ManpuFile.md). Feito pra usar com `using`,
que **salva e fecha automaticamente** ao sair do bloco.

```
manpu.open(target: str, encoding: str = "utf-8") -> ManpuFile
```

| Parâmetro | Padrão | O que é |
|---|---|---|
| `target` | — | caminho do arquivo |
| `encoding` | `"utf-8"` | charset pra CSV/texto puro (ignorado em xlsx/xls) |

---

## Uso com `using` (recomendado)

`using` fecha e salva o arquivo sozinho ao terminar o bloco — mesmo se der erro
no meio:

```
import manpu as mp

lista = mp.load("compras.txt")

using mp.open(target="compras.xlsx") as arq {
    arq.write(column=0, cell=full, content=lista)
}
// aqui o arquivo já está salvo e fechado
```

O `using` aceita os dois estilos de bloco — chaves `{ }` (acima) ou `:` com
indentação:

```
using mp.open(target="compras.xlsx") as arq:
    arq.write(column=0, cell=full, content=lista)
```

---

## `encoding` — arquivos legados

O padrão `utf-8` cobre a maioria. Pra CSVs/textos antigos noutro charset:

```
using mp.open(target="legado.csv", encoding="latin-1") as arq:
    dados = arq.read()
    post(dados)
```

O `encoding` vale pra **CSV e texto puro**; em `.xlsx`/`.xls` é ignorado (o
formato já cuida disso).

---

## `open` vs funções soltas

- **`open()` + `using`** — várias operações no mesmo arquivo, abre uma vez.
- **[`read`](../read/read.md) / [`write`](../write/write.md)** — operação única,
  abre e fecha a cada chamada.

Se você só lê **ou** só escreve uma vez, as funções soltas bastam. Pra um
fluxo (ler, transformar, escrever de volta), `open` é melhor.

---

## Relacionados

- [`ManpuFile`](../ManpuFile/ManpuFile.md) — o objeto devolvido (`.read`/`.write`/`.save`)
- [`manpu.read()`](../read/read.md) / [`manpu.write()`](../write/write.md) — operação única
