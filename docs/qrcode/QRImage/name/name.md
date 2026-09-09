# `QRImage.name` — o nome do arquivo (campo, sem `()`)

O nome de arquivo que a imagem carrega consigo. É **campo**, não método: lê-se
`img.name`, **nunca** `img.name()`.

```
img.name        # str   -> "qrcode.png"
img.name()      # TypeError: 'str' object is not callable
```

---

## Parâmetros

Nenhum. `name` não é chamável — é um campo de leitura, como
[`PoolFile.name`](../../../os/PoolFile/PoolFile.md). Pôr parêntese não
"chama o campo": o parêntese cai em cima da `str` que o campo já devolveu, e
daí o `TypeError`.

---

## Retorno

**`str`** — o nome do arquivo com extensão, sem pasta. Nunca `Null`: quando
ninguém informou um nome, o valor é `"qrcode.png"`.

```
import qrcode

qrcode.make("oi").name                      # "qrcode.png"   (default)
qrcode.make("oi", name="site.png").name     # "site.png"
```

De onde o valor vem, em cada caminho:

| como a imagem nasceu | quem define o `name` | default |
|---|---|---|
| [`qrcode.make(data, …)`](../../make/make.md) | 7º parâmetro, `name=` | `"qrcode.png"` |
| [`QRCode().make_image(…)`](../../../objetos-internos/PoolQRCode.md) | 3º parâmetro, `name=` | `"qrcode.png"` |

---

## Pra que ele serve: salvar dentro de uma pasta

[`.save()`](../save/save.md) olha o destino. Se o destino for uma **pasta** —
termina em `/`, é `.`, é `..`, ou é um diretório que já existe — o arquivo é
gravado em `<pasta>/<name>`. Se for um **caminho de arquivo**, o `.name` é
ignorado e vale o caminho.

```
import qrcode

img = qrcode.make("https://exemplo.com", name="site.png")

img.save("/tmp/qrs/")        # grava /tmp/qrs/site.png   <- usou o .name
img.save("/tmp/outro.png")   # grava /tmp/outro.png      <- ignorou o .name
```

O `name` também **viaja** pro arquivo em memória: o
[`QRPoolFile`](../to_file/to_file.md) que sai de `.to_file()` nasce com o mesmo
nome, e é dele que saem o `.name` e o `.ext` de lá.

```
f = img.to_file()
post(f.name)   # "site.png"
post(f.ext)    # ".png"
```

---

## Erros

- **TypeError: `'str' object is not callable`** — escreveu `img.name()`.
- **AttributeError: `'QRImage' object has no attribute 'name'`** — tentou
  **escrever**: `img.name = "outro.png"`. O campo é somente leitura; pra outro
  nome, gere outra imagem com `name=` ou passe o caminho completo pro
  [`.save()`](../save/save.md).
- **IOError: `nao consegui salvar '<caminho>'`** — vem do `.save()`, não do
  campo, quando a pasta de destino não existe. O motor **não cria** o
  diretório pai, e um `name` com `/` dentro (`"sub/dentro.png"`) cai nesse
  mesmo erro se `sub/` não existir.

---

## Bordas

- **`.save()` não altera o `.name`.** Depois de `img.save("/tmp/outro.png")`, o
  campo continua valendo `"site.png"`. Ele descreve a imagem, não o último
  lugar onde ela foi gravada.
- **A extensão do `name` não escolhe formato.** O conteúdo gerado é **sempre
  PNG** — medido: um arquivo salvo com nome `.jpg` começa com a assinatura PNG
  `89 50 4e 47 0d 0a 1a 0a`. A extensão só entra no nome do arquivo e no
  `.ext` do [`QRPoolFile`](../to_file/to_file.md).
- **Nome sem extensão é aceito.** `name="sem_extensao"` grava o arquivo com
  esse nome exato, ainda com bytes PNG dentro.

---

## Relacionados

- [`QRImage`](../QRImage.md) — visão geral do objeto
- [`QRImage.save()`](../save/save.md) — quem usa o `name` quando o destino é pasta
- [`QRImage.to_file()`](../to_file/to_file.md) — leva o `name` pro arquivo em memória

[← índice](../QRImage.md)
