# `os.readFile(path, encoding="utf-8")`

Lê um arquivo de **texto** e devolve o conteúdo como `str` (UTF-8 por padrão).
Para binário (imagem, zip…), use [`loadFile`](../loadFile/loadFile.md) (devolve `PoolFile`).

## Parâmetros

| nome | tipo | default | nota |
|---|---|---|---|
| `path` | str | — | caminho do arquivo |
| `encoding` | str | `"utf-8"` | charset do ARQUIVO; o texto devolvido é sempre UTF-8 |

O `encoding` diz em que charset o arquivo **está gravado** — a leitura decodifica
dele para o texto da linguagem:

```ps
post(os.readFile("legado.txt", encoding="latin-1"))   # café, com o acento certo
```

Vale **qualquer charset que a linguagem conhece** — os mesmos de
[`.decode()`](../../bytes/metodos/decode/decode.md): `utf-8`, `latin-1`,
`ascii`, `utf-16`/`utf-16be`, `utf-32`/`utf-32be`, com os apelidos
(`iso-8859-1`, `cp819`, `u8`, …). Não é uma lista à parte: a leitura **chama o
mesmo `.decode()`**, então o que funciona num funciona no outro.

Erros são os de lá: byte que não cabe no charset é `UnicodeDecodeError`, nome
que não existe é `LookupError: unknown encoding: …`.

## Retorno

`str` — o conteúdo do arquivo.

## Exemplo

```ps
import os
os.writeFile("nota.txt", "oi\nmundo")
post(os.readFile("nota.txt"))
```

```saida
oi
mundo
```

Par com [`writeFile`](../writeFile/writeFile.md).

[← índice](../os.md)
