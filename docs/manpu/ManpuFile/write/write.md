# `ManpuFile.write(content, column=0, cell=None, init=0)`

Escreve num arquivo aberto (dentro do `using`). Mesma ideia da
[`manpu.write()`](../../write/write.md) solta, mas sem reabrir o arquivo a cada
chamada — você pode chamar várias vezes no mesmo bloco.

```
arq.write(content=..., column=0, cell=..., init=0) -> ManpuResult
```

| Parâmetro | O que é |
|---|---|
| `content` | o valor a escrever |
| `column` | coluna alvo (número, ou `full` pra todas) |
| `cell` (ou `celula`) | índice da célula, ou `full` pra descer linha a linha |
| `init` | de onde começar no conteúdo (índice; padrão `0`) |

---

## Uso

```
import manpu as mp

lista = mp.load("compras.txt")

using mp.open(target="compras.csv") as arq {
    arq.write(column=0, cell=full, content=lista)
    // pode escrever mais de uma vez no mesmo arquivo:
    arq.write(column=1, cell=0, content="cabeçalho")
}
// salvo e fechado automaticamente
```

---

## `cell=full` — preencher descendo

Divide o `content` por quebra de linha; cada parte vai numa célula descendo na
coluna. Ver o exemplo em [`manpu.write()`](../../write/write.md).

---

## Relacionados

- [`ManpuFile.read()`](../read/read.md) — ler
- [`ManpuFile.save()`](../save/save.md) — salvar no meio do fluxo
- [`manpu.write()`](../../write/write.md) — versão de operação única
