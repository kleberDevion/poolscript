# `QRImage` — a imagem do QR Code

O objeto de imagem que [`qrcode.gen()`](../gen/gen.md) e
[`qrcode.make()`](../make/make.md) devolvem. Por ele você salva, redimensiona
ou pega os bytes.

---

## Métodos

| Método | O que faz | Página |
|---|---|---|
| `.save(caminho)` | grava a imagem num arquivo | [save/save.md](save/save.md) |
| `.resize(largura, altura)` | redimensiona a imagem | [resize/resize.md](resize/resize.md) |
| `.to_file()` | converte pra um arquivo (bytes) | [to_file/to_file.md](to_file/to_file.md) |

---

## Campos

| Campo | Devolve | O que é | Página |
|---|---|---|---|
| `.name` | `str` | o nome do arquivo (`"qrcode.png"` por padrão) — **campo, sem `()`** | [name/name.md](name/name.md) |

---

## Uso

```
import qrcode

img = qrcode.make("https://meusite.com")
img.resize(300, 300)          # ajusta o tamanho
img.save("site.png")          # grava
```

---

## Relacionados

- [`qrcode.gen()`](../gen/gen.md) / [`qrcode.make()`](../make/make.md) — o que devolvem um QRImage
