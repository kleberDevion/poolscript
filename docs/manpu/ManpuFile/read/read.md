# `ManpuFile.read()`

Lê o conteúdo atual do arquivo aberto. Mesmo formato de retorno da
[`manpu.read()`](../../read/read.md): CSV/XLSX viram lista de dicts, texto vira
string.

```
arq.read() -> list | str
```

---

## Uso

```
import manpu as mp

using mp.open(target="clientes.csv") as arq {
    dados = arq.read()             // lista de dicts
    for each c in dados {
        post(c["nome"])
    }
}
```

---

## Relacionados

- [`ManpuFile.write()`](../write/write.md) — escrever
- [`manpu.read()`](../../read/read.md) — versão de operação única
