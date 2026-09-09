# `hash.b64decode(texto)` — base64 de volta pro conteúdo

Decodifica uma string **base64** e devolve o conteúdo. É o caminho de volta do
[`hash.b64encode()`](../b64encode/b64encode.md), e também o que abre as partes
de um token que chegou por HTTP.

```
hash.b64decode(texto: str) -> str
```

---

## Parâmetros

| nome | tipo | default | nota |
|---|---|---|---|
| `texto` | `str` | — | posicional; o motor **não aceita por nome** |

**Só `str`.** Qualquer outro tipo — inclusive `bytes` — é recusado na hora,
sem conversão: `b64decode() argument 1 must be str, not bytes`.

O que a entrada aceita, medido:

| na entrada | vale? |
|---|---|
| alfabeto padrão `+` e `/` | sim |
| alfabeto URL-safe `-` e `_` | sim — os dois na mesma string, se quiser |
| padding `=` no fim, na quantidade exata | sim |
| **sem** padding nenhum (`"YQ"` no lugar de `"YQ=="`) | sim |
| espaço, `\t`, `\r` e `\n` em qualquer posição | sim, são ignorados |
| string vazia | sim, devolve `""` |

`hash.b64decode("--__")` e `hash.b64decode("++//")` devolvem exatamente o mesmo
resultado.

---

## Retorno

**`str`** — os bytes decodificados, um byte por posição, dentro de uma `str`.
**Não é `bytes`.**

O `len()` conta bytes, e não caracteres visíveis: `len(hash.b64decode("--__"))`
é `3`. Quando o conteúdo original não é texto UTF-8 válido, ele **volta assim
mesmo** — a `str` carrega os bytes crus e é `post()` que mostra lixo na tela.
Nada é recusado nem substituído na saída.

```
import hash

hash.b64decode("YWJj")               # "abc"
hash.b64decode("UG9vbFNjcmlwdA==")   # "PoolScript"
hash.b64decode("YQ==")               # "a"
hash.b64decode("YQ")                 # "a"    (mesmo sem padding)
hash.b64decode("Y WJ\nj")            # "abc"  (espaço e quebra ignorados)
hash.b64decode("")                   # ""
```

Se o que você quer é `bytes`, use
[`bytes.frombase64()`](../../bytes/frombase64/frombase64.md) — mesma
decodificação, tipo de saída diferente.

---

## Erros

- **TypeError** — o argumento não é `str`:
  `b64decode() argument 1 must be str, not int` (o nome do tipo entra na
  mensagem).
- **ValueError: `Only base64 data is allowed`** — apareceu caractere fora do
  alfabeto: `hash.b64decode("Y*Bj")`.
- **ValueError: `Invalid base64-encoded string: number of data characters (1)
  cannot be 1 more than a multiple of 4`** — sobra de 6 bits, que não forma
  byte nenhum: `hash.b64decode("Y")`.
- **ValueError: `Incorrect padding`** — tem `=`, mas não na quantidade que
  fecha múltiplo de 4: `hash.b64decode("Zg=")`, `hash.b64decode("Zg===")`.
- **ValueError: `Excess data after padding`** — veio dado **depois** do `=`,
  que é fim e não separador: `hash.b64decode("=Zm9v")`,
  `hash.b64decode("Zm==9v")`.

Nenhum desses devolve resultado parcial: ou sai o conteúdo inteiro, ou levanta.

---

## Bordas

- **Padding ausente passa; padding errado não.** `"YQ"` é aceito, `"YQ="` é
  `Incorrect padding`. A regra é: sem `=`, tudo bem; com `=`, tem que estar no
  fim e completar o múltiplo de 4.
- **Ida e volta preserva o texto** —
  `hash.b64decode(hash.b64encode("Ola, mundo!"))` é `"Ola, mundo!"`.
- Ida e volta **não** preserva um `bytes`: quem foi pro
  [`b64encode`](../b64encode/b64encode.md) como `bytes` virou a representação
  `b'...'` na ida, e é ela que volta.

---

## Uso: abrir a carga útil de um token

```
import hash
import json

partes = token.split(".")
carga = hash.b64decode(partes[1])   # base64 URL-safe, normalmente sem padding
dados = json.parse(carga)
post(dados["sub"])
```

Os dois motivos pra isso funcionar estão acima: o alfabeto URL-safe é aceito e
o padding é opcional.

---

## Relacionados

- [`hash.b64encode()`](../b64encode/b64encode.md) — o caminho de ida
- [`bytes.frombase64()`](../../bytes/frombase64/frombase64.md) — a mesma decodificação devolvendo `bytes`

[← índice](../hash.md)
