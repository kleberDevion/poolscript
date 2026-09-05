# `manpu.read(filepath)`

Lê um arquivo e devolve o conteúdo já no formato certo, **detectado pela
extensão**.

```
manpu.read(filepath: str) -> list | dict | str
```

---

## O que devolve, por tipo

| Extensão | Devolve |
|---|---|
| `.csv` | lista de dicts — `[{"coluna": "valor"}, ...]` (cabeçalho = chaves) |
| `.xlsx` / `.xls` | lista de dicts (igual ao CSV) |
| `.json` | dict ou lista |
| `.xml` | dict com `tag`, `attrs`, `text`, `children` |
| `.html` | texto limpo, sem as tags |
| `.txt`, `.py`, `.ps`, código | texto puro (string) |

---

## Uso

```
import manpu as mp

# CSV/XLSX → lista de dicts, uma linha por dict
clientes = mp.read("clientes.csv")
post(clientes[0]["nome"])          # primeira linha, coluna "nome"

for each c in clientes {
    post(c["nome"], "-", c["email"])
}
```

---

## `read` vs `load`

- **`read`** — devolve o **conteúdo estruturado** (dicts, texto) pra você usar
  no código.
- **[`load`](../load/load.md)** — devolve os **bytes** do arquivo (pra anexar
  em email, enviar por rede, etc.).

---

## Relacionados

- [`manpu.load()`](../load/load.md) — bytes crus
- [`manpu.open()`](../open/open.md) — pra ler E escrever no mesmo arquivo
- [`os.loadFile()`](../../os/loadFile/loadFile.md) — leitura genérica (não-tabular)
