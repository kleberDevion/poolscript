# `qrcode.ERROR_CORRECT_H` — correção de erro máxima (~30%)

Constante do nível **H** de correção de erro do QR Code. É o teto: quase um
terço do símbolo pode sumir e o leitor ainda decodifica. Em troca, cabe menos
da metade do que cabe em L.

```
qrcode.ERROR_CORRECT_H      # constante, lida sem ()
```

---

## Parâmetros

Nenhum — `ERROR_CORRECT_H` é **constante**, não função. Lê-se sem `()`;
`qrcode.ERROR_CORRECT_H()` é `TypeError: 'str' object is not callable`.

---

## Retorno

**`str`** de **1 caractere**: `"H"`.

Não é `int` nem um objeto de nível. O acesso já entrega o valor final, então
estas duas linhas geram exatamente o mesmo QR:

```
import qrcode

qrcode.make("oi", qrcode.ERROR_CORRECT_H)
qrcode.make("oi", "H")
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
| [`ERROR_CORRECT_Q`](../ERROR_CORRECT_Q/ERROR_CORRECT_Q.md) | `"Q"` | ~25% | 1663 bytes |
| [`ERROR_CORRECT_H`](../ERROR_CORRECT_H/ERROR_CORRECT_H.md) | `"H"` | ~30% | **1273 bytes** |

Os percentuais são os do padrão ISO/IEC 18004; a coluna de capacidade é medida
neste motor — `1274` bytes em H já levanta
`RuntimeError: Erro ao gerar QR Code: dados grandes demais pro QR (max ~2953 bytes)`
(a mensagem cita sempre o teto absoluto, o de L — em H o corte real é bem antes).

**Quando usar H:** código com logo no meio, gravação a laser, superfície que
arranha, leitura em movimento. Também quando o QR vai impresso pequeno e você
não controla a qualidade da impressão.

---

## Onde entra

Nos três pontos que aceitam `error_correction`:

```
import qrcode

qrcode.make("https://exemplo.com", error_correction=qrcode.ERROR_CORRECT_H)
qrcode.gen("https://exemplo.com", error_correction=qrcode.ERROR_CORRECT_H)
qrcode.QRCode(error_correction=qrcode.ERROR_CORRECT_H, box_size=10, border=4)
```

---

## Erros

- **TypeError: `'str' object is not callable`** — você escreveu
  `qrcode.ERROR_CORRECT_H()`. Constante não se chama.
- **RuntimeError: `Erro ao gerar QR Code: dados grandes demais pro QR
  (max ~2953 bytes)`** — não é da constante, é de quem gera: o payload passou
  do que H comporta (1273 bytes). O texto cita 2953 porque esse é o teto de L;
  em H o limite é menos da metade disso.

---

## Bordas

- **Só a primeira letra conta, e sem diferenciar maiúscula de minúscula.**
  `"h"`, `"H"` e `"Hxyz"` são todos o nível H — medido: os três recusam 1274
  bytes e aceitam 1273.
- **Valor não reconhecido cai em L, calado** — não em H. `qrcode.make(x, "Z")`
  gera nível L sem reclamar (medido: aceita 2953 bytes). Um erro de digitação
  no nível não levanta erro nenhum: ele te devolve, em silêncio, o código
  **menos** resistente que existe. É pra isso que a constante serve.
- A constante é somente leitura no uso normal: ela vive no módulo, e o valor
  que sai é uma `str` comum — copiar pra uma variável não muda nada.

---

## Relacionados

- [`qrcode.make()`](../make/make.md) / [`qrcode.gen()`](../gen/gen.md) — quem recebe o nível
- [`QRImage`](../QRImage/QRImage.md) — o que sai de `make()`

[← índice](../qrcode.md)
