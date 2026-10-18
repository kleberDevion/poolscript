# `Parsing.floating(value, to_type="flo")`

Converte um valor pra **número decimal** (`flo`), entendendo o formato
brasileiro (vírgula decimal, ponto de milhar).

```assinatura
Parsing.floating(value, to_type="flo") -> flo
```

`to_type` existe por uniformidade com as outras funções do `Parsing` — aqui
**não muda nada**: o resultado é sempre `flo`.

---

## Uso

```ps
import Parsing

post(Parsing.floating("R$ 19,90"), Parsing.floating("1.299,90"), Parsing.floating("3.14"), Parsing.floating("1.234"))
post(Parsing.floating("-19,90"), Parsing.floating("2,5e-3"), Parsing.floating(10), Parsing.floating("abc"))
```

```saida
19.9 1299.9 3.14 1234.0
-19.9 0.0025 10.0 0.0
```

A regra é a de [O número dentro do texto](../Parsing.md#o-número-dentro-do-texto):
o sinal colado antes do número (antes ele se perdia: `"-19,90"` dava 19.9) e
o expoente colado depois (antes `"1e5"` dava 15.0). `bool` é `1.0` ou `0.0`.

---

## Vírgula BR vs ponto

Diferente do `flo()` builtin (que espera ponto), `Parsing.floating` entende
`"19,90"` como `19.90` — útil pra valores digitados no formato brasileiro. Por
isso `"1.234"` é mil duzentos e trinta e quatro: texto no formato americano
com três casas decimais vai no `flo()`.

---

## Relacionados

- [`Parsing.integer()`](../integer/integer.md) — pra inteiros
- builtin `flo()` — conversão estrita (espera ponto)
