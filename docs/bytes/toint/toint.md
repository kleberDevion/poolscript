# `bytes.toint(b, byteorder="big")`

Desempacota bytes num inteiro (>= 0). É o caminho de volta do
[`bytes.fromint()`](../fromint/fromint.md). O valor é exato com qualquer
quantidade de bytes: acima de 64 bits sai o inteiro grande.

```assinatura
bytes.toint(b: bytes, byteorder: str = "big") -> int
```

- `byteorder` — `"big"` (padrão) ou `"little"`. Precisa ser o **mesmo** usado pra empacotar.

---

## Uso

```ps
import bytes

bytes.toint(bytes.fromint(258, 4))          # 258
bytes.toint(bytes.fromhex("deadbeef"))      # 3735928559
bytes.toint(bytes.fromint(258, 4, "little"), "little")   # 258
```

```ps
import bytes
post(bytes.toint(bytes.fromhex("ffffffffffffffff")))
post(bytes.toint(bytes.fromhex("010000000000000000")))
post(bytes.toint(bytes.fromint(2 ** 70)) == 2 ** 70)
```

```saida
18446744073709551615
18446744073709551616
True
```

---

## Erros

- **TypeError** — o argumento não é bytes.
- **ValueError** — `byteorder` inválido (o tipo está certo, o valor não
  serve): `byteorder must be either 'little' or 'big'`.

---

## Relacionados

- [`bytes.fromint()`](../fromint/fromint.md) — o caminho de ida (inteiro → bytes)
- [visão geral do bytes](../bytes.md)
