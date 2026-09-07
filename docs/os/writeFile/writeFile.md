# `os.writeFile(path, content, encoding="utf-8")`

**Escreve** `content` (str ou bytes) num arquivo, **criando a pasta pai** se não
existir. Sobrescreve se já existir. Devolve o caminho escrito.

## Parâmetros

| nome | tipo | default | nota |
|---|---|---|---|
| `path` | str | — | onde gravar (pastas são criadas) |
| `content` | str \| bytes | — | o que gravar |
| `encoding` | str | `"utf-8"` | charset em que gravar; só vale quando `content` é `str` |

Com `content` em **bytes**, o `encoding` é ignorado de propósito: bytes já são a
sequência final. Com `str`, o texto (que a linguagem guarda em UTF-8) é
convertido para o charset pedido:

```ps
os.writeFile("legado.txt", "café", encoding="latin-1")
post(os.size("legado.txt"))     # 4 bytes — em utf-8 seriam 5
```

Vale **qualquer charset que a linguagem conhece** — os mesmos de
[`.encode()`](../../string/encode/encode.md): `utf-8`, `latin-1`, `ascii`,
`utf-16`/`utf-16be`, `utf-32`/`utf-32be`, com os apelidos. A gravação **chama o
mesmo `.encode()`**, então não há lista à parte nem codec que valha num e no
outro não.

Caractere que não cabe no charset é `UnicodeEncodeError`; nome que não existe é
`LookupError: unknown encoding: …`.

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
