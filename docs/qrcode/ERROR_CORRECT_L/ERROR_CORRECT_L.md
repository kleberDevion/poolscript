# `qrcode.ERROR_CORRECT_L` — correção de erro baixa (~7%)

Constante do nível **L** de correção de erro do QR Code. É o nível **padrão**:
quem não passa `error_correction` cai aqui.

```
qrcode.ERROR_CORRECT_L      # constante, lida sem ()
```

---

## Parâmetros

Nenhum — `ERROR_CORRECT_L` é **constante**, não função. Lê-se sem `()`;
`qrcode.ERROR_CORRECT_L()` é `TypeError: 'str' object is not callable`.

---

## Retorno

**`str`** de **1 caractere**: `"L"`.

Não é `int` nem um objeto de nível. O acesso já entrega o valor final, então
estas duas linhas geram exatamente o mesmo QR:

```
import qrcode

qrcode.make("oi", qrcode.ERROR_CORRECT_L)
qrcode.make("oi", "L")
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
| [`ERROR_CORRECT_L`](../ERROR_CORRECT_L/ERROR_CORRECT_L.md) | `"L"` | ~7% | **2953 bytes** |
| [`ERROR_CORRECT_M`](../ERROR_CORRECT_M/ERROR_CORRECT_M.md) | `"M"` | ~15% | 2331 bytes |
| [`ERROR_CORRECT_Q`](../ERROR_CORRECT_Q/ERROR_CORRECT_Q.md) | `"Q"` | ~25% | 1663 bytes |
| [`ERROR_CORRECT_H`](../ERROR_CORRECT_H/ERROR_CORRECT_H.md) | `"H"` | ~30% | 1273 bytes |

Os percentuais são os do padrão ISO/IEC 18004; a coluna de capacidade é medida
neste motor — `2954` bytes em L já levanta
`RuntimeError: Erro ao gerar QR Code: dados grandes demais pro QR (max ~2953 bytes)`.

**Quando usar L:** tela, PDF, link curto — situações em que o código chega
inteiro no leitor. É o mais barato: o maior payload e o menor número de módulos.

---

## Onde entra

Nos três pontos que aceitam `error_correction`:

```
import qrcode

qrcode.make("https://exemplo.com", error_correction=qrcode.ERROR_CORRECT_L)
qrcode.gen("https://exemplo.com", error_correction=qrcode.ERROR_CORRECT_L)
qrcode.QRCode(error_correction=qrcode.ERROR_CORRECT_L, box_size=10, border=4)
```

---

## Erros

- **TypeError: `'str' object is not callable`** — você escreveu
  `qrcode.ERROR_CORRECT_L()`. Constante não se chama.
- **RuntimeError: `Erro ao gerar QR Code: dados grandes demais pro QR
  (max ~2953 bytes)`** — não é da constante, é de quem gera: o payload passou
  do que L comporta.

---

## Bordas

- **Só a primeira letra conta, e sem diferenciar maiúscula de minúscula.**
  `"l"`, `"L"` e `"Lorem"` são todos o nível L.
- **Valor não reconhecido cai em L, calado.** `qrcode.make(x, "Z")`,
  `qrcode.make(x, 3)` e `qrcode.make(x, null)` geram um QR nível L sem
  reclamar — medido pela capacidade: os três aceitam 2953 bytes e recusam 2954.
  Digitar o nível à mão é justamente o que a constante evita.
- A constante é somente leitura no uso normal: ela vive no módulo, e o valor
  que sai é uma `str` comum — copiar pra uma variável não muda nada.

---

## Relacionados

- [`qrcode.make()`](../make/make.md) / [`qrcode.gen()`](../gen/gen.md) — quem recebe o nível
- [`QRImage`](../QRImage/QRImage.md) — o que sai de `make()`

[← índice](../qrcode.md)
