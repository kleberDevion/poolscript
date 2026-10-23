# `os.readFile(path, encoding="utf-8")`

Lê um arquivo de **texto** e devolve o conteúdo como `str` (UTF-8 por padrão).
Para binário (imagem, zip…), use [`loadFile`](../loadFile/loadFile.md) (devolve `PoolFile`).

## Parâmetros

| nome | tipo | default | nota |
|---|---|---|---|
| `path` | str | — | caminho do arquivo |
| `encoding` | str | `"utf-8"` | charset do ARQUIVO; o texto devolvido é sempre UTF-8 |

Os exemplos desta página pressupõem o `legado.txt`, gravado em latin-1, como
um arquivo antigo do Windows:

```ps contexto
import os
Object h = open("legado.txt", "wb")
h.write("café".encode("latin-1"))
h.close()
```

O `encoding` diz em que charset o arquivo **está gravado** — a leitura decodifica
dele para o texto da linguagem:

```ps
import os
post(os.readFile("legado.txt", encoding="latin-1"))   # café, com o acento certo
```

```saida
café
```

Vale **qualquer charset que a linguagem conhece** — os mesmos de
[`.decode()`](../../byte/decode/decode.md): `utf-8`, `latin-1`,
`ascii`, `utf-16`/`utf-16be`, `utf-32`/`utf-32be`, com os apelidos
(`iso-8859-1`, `cp819`, `u8`, …). Não é uma lista à parte: a leitura **chama o
mesmo `.decode()`**, então o que funciona num funciona no outro.

Erros são os de lá: byte que não cabe no charset é `UnicodeDecodeError`, nome
que não existe é `LookupError: unknown encoding: …`.

Sem `encoding`, o arquivo é lido como UTF-8, como o `open().read()` do Python:
o que não é UTF-8 é `UnicodeDecodeError`, com a posição do byte. Antes saía um
`str` com o byte solto dentro, e o erro só aparecia longe dali.

```ps
import os
os.writeFile("legado.txt", "café", encoding="latin-1")
try {
    str t = os.readFile("legado.txt")
} catch (UnicodeDecodeError e) {
    post(e)
}
post(os.readFile("legado.txt", encoding="latin-1"))
```

```saida
'utf-8' codec can't decode byte 0xe9 in position 3: unexpected end of data (linha 4)
café
```

A leitura vai até o fim do que o arquivo entrega, sem medir o tamanho antes:
pipe, FIFO e `/dev/stdin` também são lidos inteiros (`printf 'a\nb' | jinga
p.pr` com `os.readFile("/dev/stdin")` dá `"a\nb"`; antes dava `""`).

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
