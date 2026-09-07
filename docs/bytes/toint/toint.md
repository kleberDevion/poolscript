# `bytes.toint(b, byteorder="big")`

Desempacota bytes num inteiro. É o caminho de volta do
[`bytes.fromint()`](../fromint/fromint.md). Até 8 bytes sem o bit alto o
resultado é positivo; a nota de bordas explica o que acontece acima disso.

```
bytes.toint(b: bytes, byteorder: str = "big") -> int
```

- `byteorder` — `"big"` (padrão) ou `"little"`. Precisa ser o **mesmo** usado pra empacotar.

---

## Uso

```
import bytes

bytes.toint(bytes.fromint(258, 4))          # 258
bytes.toint(bytes.fromhex("deadbeef"))      # 3735928559
bytes.toint(bytes.fromint(258, 4, "little"), "little")   # 258
```

---

## Erros

- **TypeError** — o argumento não é bytes.
- **ValueError** — `byteorder` inválido (o tipo está certo, o valor não
  serve): `byteorder must be either 'little' or 'big'`.

> **O inteiro é de 64 bits COM SINAL, e isso vaza aqui.** Com 8 bytes e o bit
> alto ligado o resultado é **negativo**, e com 9 bytes ou mais o byte que
> sobra some, sem erro:
>
> ```ps
> post(bytes.toint(bytes.fromhex("ffffffffffffffff")))     # -1
> post(bytes.toint(bytes.fromhex("8000000000000000")))     # -9223372036854775808
> post(bytes.toint(bytes.fromhex("010000000000000000")))   # 0   — 9 bytes, o alto some
> ```
>
> Ou seja: o "(>= 0)" do começo desta página só vale enquanto o valor cabe em
> 2^63. Acima disso, confira o tamanho antes de desempacotar.

---

## Relacionados

- [`bytes.fromint()`](../fromint/fromint.md) — o caminho de ida (inteiro → bytes)
- [visão geral do bytes](../bytes.md)
