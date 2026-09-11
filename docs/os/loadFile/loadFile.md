# `os.loadFile(path, mode=Null)`

Lê um arquivo e devolve o conteúdo já no formato certo: texto vira `str`, JSON
vira dict/lista, CSV vira lista de dicts, e arquivo binário vira um
[`PoolFile`](../PoolFile/PoolFile.md).

```
os.loadFile(path: str, mode: str = Null) -> str | dict | list | PoolFile
```

| Parâmetro | O que é |
|---|---|
| `path` | nome/caminho do arquivo (buscado a partir da pasta do `.ps` e do cwd) |
| `mode` | `"r"` ou `"rb"`. Omitido = decidido pelo conteúdo do arquivo |

---

## `mode` manda, e não existe extensão proibida

Com `mode`, o arquivo abre do jeito que você pediu, **seja qual for o nome
dele**:

- **`"rb"`** — devolve `PoolFile`. Qualquer caminho: `.xlsm`, `.7z`, `.tar`,
  `.parquet`, arquivo sem extensão nenhuma.
- **`"r"`** — devolve o texto. Também em qualquer caminho.

```ps
import os

a = os.loadFile("planilha.xlsm", mode="rb")   # PoolFile
b = os.loadFile("dump", mode="rb")            # sem extensão: PoolFile
c = os.loadFile("logo.png", mode="r")         # o texto dos bytes
```

> **Isto mudou na versão 15.90.14.** Havia uma lista fixa de doze extensões
> binárias, e pedir `mode="rb"` fora dela era recusado com
> `TypeError: loadFile: mode='rb' nao aceita extensao '.xlsm'`. A lista saiu: a
> linguagem não tem tipos de arquivo que se recusa a abrir.

---

## Sem `mode`, quem decide é o CONTEÚDO

Omitindo o `mode`, o motor lê os primeiros 8 KB: **byte NUL presente = binário**.
É o mesmo critério das ferramentas de linha de comando, e vale pra arquivo que
ninguém previu — não há lista consultada.

| Conteúdo | Devolve |
|---|---|
| tem byte NUL nos primeiros 8 KB | [`PoolFile`](../PoolFile/PoolFile.md) |
| não tem, e o nome termina em `.json` | dict ou lista (JSON parseado) |
| não tem, e o nome termina em `.csv` | lista de dicts (cabeçalho = chaves) |
| não tem | `str` (o texto) |

```ps
config = os.loadFile("config.json")     # dict → config["versao"]
tabela = os.loadFile("dados.csv")       # lista de dicts
texto  = os.loadFile("leiame.txt")      # str
imagem = os.loadFile("logo.png")        # PoolFile
```

A extensão só entra na **última** decisão, entre as três formas de texto. Um
arquivo chamado `.png` que contém `"nao sou png"` volta como `str`, porque não
é binário — e um `.dat` com bytes crus volta como `PoolFile`.

---

## Erros

**Não é charset.** Passar um nome de codificação no `mode` é erro, e a mensagem
diz onde ele vale:

```
os.loadFile("legado.csv", mode="latin-1")
ValueError: loadFile: 'latin-1' e um charset, o 2 argumento e o mode ('r' ou 'rb').
            Pra ler com charset: os.readFile(path, encoding="latin-1")
```

Valor que não é modo nem charset:

```
os.loadFile("x.txt", mode="xyz")
ValueError: loadFile: 'xyz' nao e um mode valido — use 'r' ou 'rb'
```

Arquivo inexistente é `FileNotFoundError`, que a árvore de exceções põe sob
`OSError` — `catch (OSError e)` pega.

---

## Exemplo: config + envio de arquivo

```
import os
import mail

# JSON vira dict direto
cfg = os.loadFile("config.json")
post(cfg["destinatario"])

# binário vira PoolFile, pronto pra anexar
pdf = os.loadFile("relatorio.pdf")       # PoolFile
```

---

## Relacionados

- [`PoolFile`](../PoolFile/PoolFile.md) — o tipo devolvido pra binários
- [`pathFile`](../pathFile/pathFile.md) — só o caminho, sem ler o conteúdo
- [`exists`](../exists/exists.md) — checar antes de carregar
