# `qrcode.make(data, error_correction="L", box_size=10, border=4, fill_color="black", back_color="white", name="qrcode.png")`

O atalho pra gerar um QR Code: `qrcode.make(data)` devolve um
[`QRImage`](../QRImage/QRImage.md), que você salva com `.save()`. Mais simples
que [`gen`](../gen/gen.md), pros casos comuns.

```
qrcode.make(data, error_correction="L", box_size=10, border=4,
            fill_color="black", back_color="white", name="qrcode.png")
```

---

## Uso

```
import qrcode

img = qrcode.make("https://meusite.com")
img.save("site.png")
```

---

## `make` vs `gen`

- **`make`** — atalho: devolve a imagem, você chama `.save()`. Nomes de
  parâmetro no estilo da lib Python (`fill_color`, `back_color`, `box_size`).
- **[`gen`](../gen/gen.md)** — função de alto nível da PoolScript, com opção de
  salvar direto (`save=`), redimensionar (`qr32=`) e nomes mais curtos
  (`color`, `bg`, `size`).

Os dois fazem QR Code — escolha por preferência de estilo. Pra salvar numa
linha só, `gen(..., save=...)` é mais direto.

---

## Relacionados

- [`qrcode.gen()`](../gen/gen.md) — versão com salvar direto e mais opções
- [`QRImage`](../QRImage/QRImage.md) — o objeto devolvido (`.save`)
