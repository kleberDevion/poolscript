# `QRImage.save(path)`

Grava a imagem do QR Code num arquivo.

```
img.save(path: str) -> None
```

---

## Uso

```
import qrcode

img = qrcode.make("texto")
img.save("qr.png")            # grava como PNG
img.save("pasta/codigo.jpg") # o formato vem da extensão
```

O formato (PNG, JPG…) é decidido pela extensão do caminho.

---

## Relacionados

- [`QRImage`](../QRImage.md) — visão geral
- [`qrcode.gen()`](../../gen/gen.md) — salvar direto com `save=` (sem `.save()`)
