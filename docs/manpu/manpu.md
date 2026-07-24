# manpu — Manipulação de arquivos (CSV, XLSX, texto…)

Lib pra **ler e escrever arquivos** de forma estruturada: planilhas CSV/XLSX,
XML, HTML, JSON e texto puro. Enquanto `os.loadFile` é genérico, o `manpu` é
focado em **dados tabulares** (linhas e colunas) e edições pontuais.

```
import manpu as mp
// (mp é o apelido comum; import manpu também funciona)
```

---

## Referência

| Membro | O que faz | Página |
|---|---|---|
| `mp.read(arquivo)` | lê o arquivo e devolve o conteúdo (dicts, texto…) | [read/read.md](read/read.md) |
| `mp.load(arquivo)` | carrega o arquivo em bytes (ex: pra anexar em email) | [load/load.md](load/load.md) |
| `mp.write(...)` | escreve um valor numa célula/posição do arquivo | [write/write.md](write/write.md) |
| `mp.remove(...)` | remove conteúdo do arquivo | [remove/remove.md](remove/remove.md) |
| `mp.open(target, encoding)` | abre pra várias operações (use com `using`) | [open/open.md](open/open.md) |
| `mp.src(arquivo)` | caminho absoluto do arquivo | [src/src.md](src/src.md) |
| `ManpuFile` | o objeto aberto por `open()` (`.read`/`.write`/`.save`) | [ManpuFile/ManpuFile.md](ManpuFile/ManpuFile.md) |

---

## Dois estilos de uso

**Operação única** — funções soltas (`read`, `write`, `remove`): abrem, fazem a
coisa, e fecham.

```
import manpu as mp
dados = mp.read("clientes.csv")     // lista de dicts
```

**Várias operações no mesmo arquivo** — `open()` com `using`: abre uma vez, faz
tudo, salva/fecha automático ao sair do bloco.

```
using mp.open(target="saida.csv") as arq {
    arq.write(column=0, cell=full, content=lista)
}
// salvo e fechado aqui
```

---

## Exemplo rápido

```
import manpu as mp

// lê uma planilha como lista de dicts
clientes = mp.read("clientes.xlsx")
for each c in clientes {
    post(c["nome"], c["email"])
}

// escreve um relatório, salvando automático
lista = mp.load("compras.txt")
using mp.open(target="relatorio.csv") as arq {
    arq.write(column=0, cell=full, content=lista)
}
```
