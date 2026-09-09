# qrcode — Gerar QR Codes

Lib pra gerar imagens de **QR Code** a partir de um texto/URL.
**Zero dependências externas.**

```
import qrcode
# ou: import qrcode   (apelido)
```

| Membro | O que faz | Página |
|---|---|---|
| `qrcode.gen(data, ...)` | gera um QR Code (função de alto nível, mais opções) | [gen/gen.md](gen/gen.md) |
| `qrcode.make(data, ...)` | atalho — `make(data)` devolve a imagem | [make/make.md](make/make.md) |
| `QRImage` | a imagem gerada (`.save`/`.resize`/`.to_file`/`.name`) | [QRImage/QRImage.md](QRImage/QRImage.md) |

---

## Constantes de correção de erro

Quatro constantes (lidas **sem `()`**), cada uma uma `str` de 1 caractere. Elas
dizem quanto do código pode ser perdido e o leitor ainda decodificar — e, em
troca, quanto dado ainda cabe:

| Constante | Valor | Recupera até | Cabe no máximo | Página |
|---|---|---|---|---|
| `qrcode.ERROR_CORRECT_L` | `"L"` | ~7% | 2953 bytes | [ERROR_CORRECT_L/ERROR_CORRECT_L.md](ERROR_CORRECT_L/ERROR_CORRECT_L.md) |
| `qrcode.ERROR_CORRECT_M` | `"M"` | ~15% | 2331 bytes | [ERROR_CORRECT_M/ERROR_CORRECT_M.md](ERROR_CORRECT_M/ERROR_CORRECT_M.md) |
| `qrcode.ERROR_CORRECT_Q` | `"Q"` | ~25% | 1663 bytes | [ERROR_CORRECT_Q/ERROR_CORRECT_Q.md](ERROR_CORRECT_Q/ERROR_CORRECT_Q.md) |
| `qrcode.ERROR_CORRECT_H` | `"H"` | ~30% | 1273 bytes | [ERROR_CORRECT_H/ERROR_CORRECT_H.md](ERROR_CORRECT_H/ERROR_CORRECT_H.md) |

`L` é o default. Nível não reconhecido — `"Z"`, um `int`, `null` — cai em `L`
**sem erro nenhum**; é pra isso que as constantes existem.

```
import qrcode

qrcode.make("https://meusite.com", error_correction=qrcode.ERROR_CORRECT_H)
```

---

## Exemplo rápido

```
import qrcode

# gera e salva num arquivo
qrcode.gen("https://meusite.com", save="site.png")

# ou o atalho, e salva depois
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
