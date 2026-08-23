# `os.writeFile(caminho, conteudo, encoding="utf-8")`

**Escreve** `conteudo` (str ou bytes) num arquivo, **criando a pasta pai** se não
existir. Sobrescreve se já existir. Devolve o caminho escrito.

## Parâmetros

| nome | tipo | default | nota |
|---|---|---|---|
| `caminho` | str | — | onde gravar (pastas são criadas) |
| `conteudo` | str \| bytes | — | o que gravar |
| `encoding` | str | `"utf-8"` | usado quando `conteudo` é str |

## Retorno

`str` — o caminho gravado.

## Exemplo

```ps
import os
caminho = os.writeFile("docs/saida.txt", "conteudo")
post(caminho)
post(os.readFile("docs/saida.txt"))
```

```saida
docs/saida.txt
conteudo
```

Par com [`readFile`](../readFile/readFile.md).

[← índice](../os.md)
