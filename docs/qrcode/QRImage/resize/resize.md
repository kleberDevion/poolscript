# `QRImage.resize(largura, altura)`

Redimensiona a imagem do QR Code pra um tamanho específico, em pixels.

```
img.resize(largura: int, altura: int) -> None
```

---

## Uso

```
import qrcode

img = qrcode.make("https://meusite.com")
img.resize(300, 300)          // 300x300 pixels
img.save("site.png")
```

Útil pra deixar o QR num tamanho exato pra impressão ou tela.

> No [`gen()`](../../gen/gen.md) dá pra redimensionar direto na geração com o
> parâmetro `qr32=(largura, altura)`.

---

## Relacionados

- [`QRImage.save()`](../save/save.md) — gravar depois de redimensionar
