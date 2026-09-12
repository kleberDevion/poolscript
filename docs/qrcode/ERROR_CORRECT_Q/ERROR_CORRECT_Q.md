# `qrcode.ERROR_CORRECT_Q` — correção de erro alta (~25%)

Constante do nível **Q** (*quartile*) de correção de erro do QR Code. Um quarto
do símbolo pode ser perdido e o leitor ainda decodifica — o preço é quase
metade da capacidade de L.

```
qrcode.ERROR_CORRECT_Q      # constante, lida sem ()
```

---

## Parâmetros

Nenhum — `ERROR_CORRECT_Q` é **constante**, não função. Lê-se sem `()`;
`qrcode.ERROR_CORRECT_Q()` é `TypeError: 'str' object is not callable`.

---

## Retorno

**`str`** de **1 caractere**: `"Q"`.

Não é `int` nem um objeto de nível. O acesso já entrega o valor final, então
estas duas linhas geram exatamente o mesmo QR:

```
import qrcode

qrcode.make("oi", qrcode.ERROR_CORRECT_Q)
qrcode.make("oi", "Q")
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
| [`ERROR_CORRECT_M`](../ERROR_CORRECT_M/ERROR_CORRECT_M.md) | `"M"` | ~15% | 2331 bytes |
| [`ERROR_CORRECT_Q`](../ERROR_CORRECT_Q/ERROR_CORRECT_Q.md) | `"Q"` | ~25% | **1663 bytes** |
| [`ERROR_CORRECT_H`](../ERROR_CORRECT_H/ERROR_CORRECT_H.md) | `"H"` | ~30% | 1273 bytes |

Os percentuais são os do padrão ISO/IEC 18004; a coluna de capacidade é medida
neste motor — `1664` bytes em Q já levanta
`RuntimeError: Erro ao gerar QR Code: dados grandes demais pro QR (max 1663 bytes no nivel Q)`.

**Quando usar Q:** superfície que se suja ou se dobra — chão de fábrica, caixa
de entrega, adesivo em equipamento. Também quando você vai colar um logo
pequeno no meio do código.

---

## Onde entra

Nos três pontos que aceitam `error_correction`:

```
import qrcode

qrcode.make("https://exemplo.com", error_correction=qrcode.ERROR_CORRECT_Q)
qrcode.gen("https://exemplo.com", error_correction=qrcode.ERROR_CORRECT_Q)
qrcode.QRCode(error_correction=qrcode.ERROR_CORRECT_Q, box_size=10, border=4)
```

---

## Erros

- **TypeError: `'str' object is not callable`** — você escreveu
  `qrcode.ERROR_CORRECT_Q()`. Constante não se chama.
- **RuntimeError: `Erro ao gerar QR Code: dados grandes demais pro QR
  (max 1663 bytes no nivel Q)`** — não é da constante, é de quem gera: o
  payload passou do que Q comporta.

---

## Bordas

- **Só a primeira letra conta, e sem diferenciar maiúscula de minúscula.**
  `"q"`, `"Q"` e `"Quartile"` são todos o nível Q.
- **Valor não reconhecido cai em L, calado** — não em Q. `qrcode.make(x, "Z")`
  gera nível L sem reclamar (medido: aceita 2953 bytes). Se você quer Q, passe
  a constante.
- A constante é somente leitura no uso normal: ela vive no módulo, e o valor
  que sai é uma `str` comum — copiar pra uma variável não muda nada.

---

## Relacionados

- [`qrcode.make()`](../make/make.md) / [`qrcode.gen()`](../gen/gen.md) — quem recebe o nível
- [`QRImage`](../QRImage/QRImage.md) — o que sai de `make()`

[← índice](../qrcode.md)
