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

Valem `utf-8`, `latin-1` (e apelidos) e `ascii`. Caractere que não cabe no
charset é `UnicodeEncodeError`; nome desconhecido é `LookupError`; `utf-16`/
`utf-32` não são aplicados aqui, e a mensagem manda usar `.encode(...)` e
gravar os bytes.

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
