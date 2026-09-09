# `QRImage.save(path)`

Grava a imagem do QR Code em disco.

## Parâmetros

| nome | tipo | default | nota |
|---|---|---|---|
| `path` | str | — | arquivo de destino, ou uma **pasta** |

O nome é este: `img.save(caminho="x.png")` é
`TypeError: 'caminho' is an invalid keyword argument`. O argumento é
obrigatório — `img.save()` é erro de aridade.

## Retorno

**`QRImage`** — o **próprio objeto**, não `Null`. Por isso `save` encadeia:

```ps
import qrcode

img = qrcode.make("texto")
post(type(img.save("/tmp/qr.png")))     # QRImage
```

## O formato é SEMPRE PNG

A extensão do caminho **não** escolhe formato nenhum. O motor gera PNG e grava
esses bytes no caminho que você deu, seja qual for o nome. Salvar como `.jpg`
produz um arquivo PNG com nome errado:

```ps
import qrcode
import os

img = qrcode.make("texto")
img.save("/tmp/codigo.jpg")
post(os.loadFile("/tmp/codigo.jpg").bytes()[0:4])   # b'\x89PNG' — nao e JPEG
```

Os quatro primeiros bytes são `89 50 4e 47`, a assinatura do PNG. Quem abrir o
arquivo pela extensão pode recusá-lo. Use `.png`.

## `path` pode ser uma pasta

Se o caminho é um diretório existente, o arquivo nasce dentro dele com o nome
do campo [`.name`](../name/name.md) (`"qrcode.png"` por padrão):

```ps
import qrcode

img = qrcode.make("texto")
post(img.name)              # qrcode.png
img.save("/tmp")            # grava /tmp/qrcode.png
```

## Erros

- **TypeError** — `path` que não é `str`:
  `save() argument 1 must be str, not int`.
- **IOError** — não conseguiu gravar (pasta inexistente, sem permissão):
  `nao consegui salvar '…'`.

## Bordas

- Arquivo existente é **sobrescrito**, sem aviso.
- `resize(w, h)` antes do `save` muda o tamanho gravado; ele também devolve o
  próprio objeto, então `img.resize(300, 300).save("/tmp/qr.png")` funciona.

## Relacionados

- [`QRImage`](../QRImage.md) — visão geral
- [`QRImage.name`](../name/name.md) — o nome usado quando `path` é pasta
- [`qrcode.gen()`](../../gen/gen.md) — salvar direto com `save=`, sem `.save()`

[← índice](../../qrcode.md)
