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

Valem `utf-8`, `latin-1` (e apelidos: `iso-8859-1`, `cp819`…) e `ascii`. Byte
fora do ASCII com `encoding="ascii"` é `UnicodeDecodeError`. Nome desconhecido é
`LookupError: unknown encoding: …`. Charset de largura fixa (`utf-16`,
`utf-32`) **não** é aplicado aqui, e a mensagem diz o caminho: leia os bytes
(`open(caminho, "rb")`) e use `.decode("utf-16")`.

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
