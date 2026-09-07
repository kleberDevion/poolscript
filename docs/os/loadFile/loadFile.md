# `os.loadFile(name, modo=Null)`

Lê um arquivo e devolve o conteúdo já no formato certo, **detectado pela
extensão**: texto vira `str`, JSON vira dict/lista, CSV vira lista de dicts, e
arquivos binários (imagem, PDF, docx…) viram um [`PoolFile`](../PoolFile/PoolFile.md).

```
os.loadFile(name: str, modo: str = Null) -> str | dict | list | PoolFile
```

| Parâmetro | O que é |
|---|---|
| `name` | nome/caminho do arquivo (buscado a partir da pasta do `.ps` e do cwd) |
| `modo` | força o modo, `"r"` ou `"rb"` — ver abaixo. Omitido = automático pela extensão |

---

## O retorno depende da extensão (modo automático)

| Extensão | Devolve |
|---|---|
| `.json` | dict ou lista (JSON parseado) |
| `.csv` | lista de dicts (uma linha = um dict, cabeçalho = chaves) |
| `.txt`, `.html`, `.md`, código… | `str` (o texto) |
| `.pdf`, `.jpg`, `.png`, `.docx`, binários | [`PoolFile`](../PoolFile/PoolFile.md) |

```
config  = os.loadFile("config.json")     # dict → config["versao"]
tabela  = os.loadFile("dados.csv")       # lista de dicts
texto   = os.loadFile("leiame.txt")      # str
imagem  = os.loadFile("logo.png")        # PoolFile (bytes)
```

---

## Forçando o modo com o 2º argumento

O segundo argumento é o **modo**, e só dois valores existem:

- **`"rb"`** — força **binário** (devolve `PoolFile`). Só aceita extensões
  binárias: `os.loadFile("recibo.dat", "rb")` é erro, porque `.dat` não está na
  lista de extensões binárias.
- **`"r"`** — força **texto**.

```
img = os.loadFile("recibo.pdf", "rb")     # binário: devolve PoolFile
txt = os.loadFile("dados.csv", "r")       # texto
```

**Não é charset.** Passar um nome de codificação aqui é erro, e a mensagem diz
onde ele vale:

```
os.loadFile("legado.csv", encoding="latin-1")
ValueError: loadFile(): o 2o argumento e o MODO ('r' ou 'rb'), nao um charset.
            Pra ler '.csv' com charset use os.readFile(caminho, encoding="latin-1")
```

Forçar um modo incompatível com a extensão levanta erro claro (ex: `"rb"` num
`.txt`).

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
