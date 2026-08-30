# qrcode — Gerar QR Codes

Lib pra gerar imagens de **QR Code** a partir de um texto/URL.
**Zero dependências externas.**

```
import qrcode
// ou: import qrcode   (apelido)
```

| Membro | O que faz | Página |
|---|---|---|
| `qrcode.gen(data, ...)` | gera um QR Code (função de alto nível, mais opções) | [gen/gen.md](gen/gen.md) |
| `qrcode.make(data, ...)` | atalho estilo Python — `make(data)` | [make/make.md](make/make.md) |
| `QRImage` | a imagem gerada (`.save`/`.resize`/`.to_file`) | [QRImage/QRImage.md](QRImage/QRImage.md) |

---

## Exemplo rápido

```
import qrcode

// gera e salva num arquivo
qrcode.gen("https://meusite.com", save="site.png")

// ou o atalho, e salva depois
img = qrcode.make("texto qualquer")
img.save("qr.png")
```

---

## `gen` vs `make`

- **[`make`](make/make.md)** — atalho simples: `qrcode.make(data)` devolve a
  imagem, você salva com `.save()`.
- **[`gen`](gen/gen.md)** — função de alto nível com mais controle (tamanho,
  cores, correção de erro) e opção de salvar direto.

Pra um QR rápido, `make`. Pra ajustar cor/tamanho, `gen`.
