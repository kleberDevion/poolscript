# `os.writeFile(path, content, encoding="utf-8")`

**Escreve** `content` (str ou bytes) num arquivo, **criando a pasta pai** se não
existir. Sobrescreve se já existir. Devolve o caminho escrito.

## Parâmetros

| nome | tipo | default | nota |
|---|---|---|---|
| `path` | str | — | onde gravar (pastas são criadas) |
| `content` | str \| bytes | — | o que gravar |
| `encoding` | str | `"utf-8"` | usado quando `content` é str |

## Retorno

`str` — o caminho gravado.

## Exemplo

```ps
import os
path = os.writeFile("docs/saida.txt", "content")
post(caminho)
post(os.readFile("docs/saida.txt"))
```

```saida
docs/saida.txt
conteudo
```

Par com [`readFile`](../readFile/readFile.md).

[← índice](../os.md)
