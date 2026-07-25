# `open(caminho, modo="r", encoding="utf-8")`

Abre um arquivo pra leitura ou escrita. Devolve um `FileHandle`. Use com
`using`, que fecha o arquivo automaticamente.

```
open(caminho, modo="r", encoding="utf-8") -> FileHandle
```

| Parâmetro | Padrão | O que é |
|---|---|---|
| `caminho` | — | caminho do arquivo |
| `modo` | `"r"` | `"r"` ler, `"w"` escrever, `"a"` adicionar, `"rb"`/`"wb"` binário |
| `encoding` | `"utf-8"` | charset (ignorado em modo binário) |

---

## Uso com `using` (recomendado)

```
// ler
using open("dados.txt") as f {
    conteudo = f.read()
    post(conteudo)
}

// escrever (sobrescreve)
using open("log.txt", "w") as f {
    f.write("primeira linha\n")
}

// adicionar ao fim
using open("log.txt", "a", encoding="utf-8") as f {
    f.write("nova linha\n")
}
// arquivo fechado automaticamente ao sair do bloco
```

---

## Métodos do `FileHandle`

`.read()`, `.readlines()`, `.readline()`, `.write(texto)`,
`.writelines(lista)`, `.close()`.

Com `using`, você não precisa chamar `.close()` — é automático.

---

## `open` builtin vs `manpu.open`

- **`open`** (builtin) — arquivos genéricos (texto, binário), estilo Python.
- **[`manpu.open`](../../manpu/open/open.md)** — focado em CSV/XLSX estruturado.

---

## Relacionados

- `using` — ver `LANGUAGE.md`
- [`os.loadFile()`](../../os/loadFile/loadFile.md) — ler já parseando por tipo
