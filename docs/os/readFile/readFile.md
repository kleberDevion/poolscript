# `os.readFile(caminho, encoding="utf-8")`

Lê um arquivo de **texto** e devolve o conteúdo como `str` (UTF-8 por padrão).
Para binário (imagem, zip…), use [`loadFile`](../loadFile/loadFile.md) (devolve `PoolFile`).

## Parâmetros

| nome | tipo | default | nota |
|---|---|---|---|
| `caminho` | str | — | caminho do arquivo |
| `encoding` | str | `"utf-8"` | codificação do texto |

## Retorno

`str` — o conteúdo do arquivo.

## Exemplo

![exemplo 1](../../assets/os__readFile__readFile_ex1.png)

<details><summary>código</summary>

```ps
import os
os.writeFile("nota.txt", "oi\nmundo")
post(os.readFile("nota.txt"))
```

</details>

```saida
oi
mundo
```

Par com [`writeFile`](../writeFile/writeFile.md).

[← índice](../os.md)
