# `os.loadFile(nome, encoding=None)`

Lê um arquivo e devolve o conteúdo já no formato certo, **detectado pela
extensão**: texto vira `str`, JSON vira dict/lista, CSV vira lista de dicts, e
arquivos binários (imagem, PDF, docx…) viram um [`PoolFile`](../PoolFile/PoolFile.md).

```
os.loadFile(nome: str, encoding: str = None) -> str | dict | list | PoolFile
```

| Parâmetro | O que é |
|---|---|
| `nome` | nome/caminho do arquivo (buscado a partir da pasta do `.ps` e do cwd) |
| `encoding` | força o modo — ver abaixo. `None` = automático pela extensão |

---

## O retorno depende da extensão (modo automático)

| Extensão | Devolve |
|---|---|
| `.json` | dict ou lista (JSON parseado) |
| `.csv` | lista de dicts (uma linha = um dict, cabeçalho = chaves) |
| `.txt`, `.html`, `.md`, código… | `str` (o texto) |
| `.pdf`, `.jpg`, `.png`, `.docx`, binários | [`PoolFile`](../PoolFile/PoolFile.md) |

```
config  = os.loadFile("config.json")     // dict → config["versao"]
tabela  = os.loadFile("dados.csv")       // lista de dicts
texto   = os.loadFile("leiame.txt")      // str
imagem  = os.loadFile("logo.png")        // PoolFile (bytes)
```

---

## Forçando o modo com `encoding`

O segundo argumento força a leitura:

- **`encoding="rb"`** — força **binário** (devolve `PoolFile`). Só aceita
  extensões binárias.
- **`encoding="utf-8"`** (ou `"latin-1"`, etc.) — força **texto** com aquele
  charset. Só aceita extensões de texto.

```
img = os.loadFile("recibo.dat", encoding="rb")        // trata como binário
txt = os.loadFile("legado.csv", encoding="latin-1")   // texto em charset antigo
```

Forçar um modo incompatível com a extensão levanta erro claro (ex: `"rb"` num
`.txt`).

---

## Exemplo: config + envio de arquivo

```
import os
import mail

// JSON vira dict direto
cfg = os.loadFile("config.json")
post(cfg["destinatario"])

// binário vira PoolFile, pronto pra anexar
pdf = os.loadFile("relatorio.pdf")       // PoolFile
```

---

## Relacionados

- [`PoolFile`](../PoolFile/PoolFile.md) — o tipo devolvido pra binários
- [`pathFile`](../pathFile/pathFile.md) — só o caminho, sem ler o conteúdo
- [`exists`](../exists/exists.md) — checar antes de carregar
