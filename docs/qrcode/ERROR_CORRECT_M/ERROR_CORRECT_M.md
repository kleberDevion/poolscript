# `qrcode.ERROR_CORRECT_M` — correção de erro média (~15%)

Constante do nível **M** de correção de erro do QR Code. Um degrau acima do
padrão: dobra a tolerância a dano e custa ~21% da capacidade de dados.

```
qrcode.ERROR_CORRECT_M      # constante, lida sem ()
```

---

## Parâmetros

Nenhum — `ERROR_CORRECT_M` é **constante**, não função. Lê-se sem `()`;
`qrcode.ERROR_CORRECT_M()` é `TypeError: 'str' object is not callable`.

---

## Retorno

**`str`** de **1 caractere**: `"M"`.

Não é `int` nem um objeto de nível. O acesso já entrega o valor final, então
estas duas linhas geram exatamente o mesmo QR:

```
import qrcode

qrcode.make("oi", qrcode.ERROR_CORRECT_M)
qrcode.make("oi", "M")
```

A constante existe pra você não digitar a letra errada — não pra esconder um
tipo.

---

## O que o nível significa na prática

O QR guarda, além dos dados, **códigos de recuperação**. O nível diz quanto do
símbolo pode ser perdido (sujeira, rasgo, logo por cima, reflexo) e o leitor
ainda decodificar. Quanto mais recuperação, **menos** dado cabe no mesmo
tamanho.

| constante | valor | recupera até | cabe no máximo |
|---|---|---|---|
| [`ERROR_CORRECT_L`](../ERROR_CORRECT_L/ERROR_CORRECT_L.md) | `"L"` | ~7% | 2953 bytes |
| [`ERROR_CORRECT_M`](../ERROR_CORRECT_M/ERROR_CORRECT_M.md) | `"M"` | ~15% | **2331 bytes** |
| [`ERROR_CORRECT_Q`](../ERROR_CORRECT_Q/ERROR_CORRECT_Q.md) | `"Q"` | ~25% | 1663 bytes |
| [`ERROR_CORRECT_H`](../ERROR_CORRECT_H/ERROR_CORRECT_H.md) | `"H"` | ~30% | 1273 bytes |

Os percentuais são os do padrão ISO/IEC 18004; a coluna de capacidade é medida
neste motor — `2332` bytes em M já levanta
`RuntimeError: Erro ao gerar QR Code: dados grandes demais pro QR (max 2331 bytes no nivel M)`.

**Quando usar M:** impressão comum — etiqueta, cartaz, folheto, embalagem. É o
meio-termo: aguenta sujeira leve sem sacrificar muito payload.

---

## Onde entra

Nos três pontos que aceitam `error_correction`:

```
import qrcode

qrcode.make("https://exemplo.com", error_correction=qrcode.ERROR_CORRECT_M)
qrcode.gen("https://exemplo.com", error_correction=qrcode.ERROR_CORRECT_M)
qrcode.QRCode(error_correction=qrcode.ERROR_CORRECT_M, box_size=10, border=4)
```

---

## Erros

- **TypeError: `'str' object is not callable`** — você escreveu
  `qrcode.ERROR_CORRECT_M()`. Constante não se chama.
- **RuntimeError: `Erro ao gerar QR Code: dados grandes demais pro QR
  (max 2331 bytes no nivel M)`** — não é da constante, é de quem gera: o
  payload passou do que M comporta.

---

## Bordas

- **Só a primeira letra conta, e sem diferenciar maiúscula de minúscula.**
  `"m"`, `"M"` e `"Medio"` são todos o nível M.
- **Valor não reconhecido cai em L, calado** — não em M. `qrcode.make(x, "Z")`
  gera nível L sem reclamar (medido: aceita 2953 bytes). Se você quer M, passe
  a constante.
- A constante é somente leitura no uso normal: ela vive no módulo, e o valor
  que sai é uma `str` comum — copiar pra uma variável não muda nada.

---

## Relacionados

- [`qrcode.make()`](../make/make.md) / [`qrcode.gen()`](../gen/gen.md) — quem recebe o nível
- [`QRImage`](../QRImage/QRImage.md) — o que sai de `make()`

[← índice](../qrcode.md)
