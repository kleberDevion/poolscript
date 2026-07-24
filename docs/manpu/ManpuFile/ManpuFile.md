# `ManpuFile` — arquivo aberto para operações

`ManpuFile` é o objeto que [`manpu.open()`](../open/open.md) devolve. Enquanto
está aberto (dentro do `using`), você lê e escreve nele; ao sair do bloco, é
**salvo e fechado automaticamente**.

```
using mp.open(target="dados.csv") as arq {
    // arq é um ManpuFile
}
```

---

## Métodos

| Método | O que faz | Página |
|---|---|---|
| `.read()` | lê o conteúdo atual do arquivo | [read/read.md](read/read.md) |
| `.write(...)` | escreve numa posição (coluna/célula) | [write/write.md](write/write.md) |
| `.save()` | salva as mudanças no disco | [save/save.md](save/save.md) |

> Com `using`, você **não precisa** chamar `.save()` — ele roda sozinho ao sair
> do bloco. `.save()` explícito serve pra salvar no meio do fluxo.

---

## Exemplo — ler, transformar, escrever

```
import manpu as mp

using mp.open(target="dados.csv") as arq {
    linhas = arq.read()                    // lê o que já tem
    for each l in linhas {
        post(l["nome"])
    }
    arq.write(column=0, cell=full, content="novo\nvalores")
}
// salvo e fechado automaticamente
```

---

## Relacionados

- [`manpu.open()`](../open/open.md) — o que cria um `ManpuFile`
- [`manpu.write()`](../write/write.md) — a versão de operação única
