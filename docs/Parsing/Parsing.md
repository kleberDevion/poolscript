# Parsing — Conversões de tipo seguras

`Parsing` converte valores entre tipos (texto → número, etc.) de forma
**tolerante**: limpa caracteres estranhos, entende separadores, e não quebra
com entrada suja como um `int()` cru quebraria.

`Parsing` é uma **lib**: entra com `import Parsing` no topo do arquivo, como
`json` ou `os`. Sem o import, o nome não existe, e o erro sai antes de rodar:
`NameError: name 'Parsing' is not defined. Did you forget to import 'Parsing'?`.
Até a versão 16.2.14 ele nascia ligado, sem import.

```ps
import Parsing

post(Parsing.integer("R$ 1.234"))     # 1234
```

`import Parsing as P` e `from Parsing import integer` também valem.

---

## Métodos

| Método | Converte pra | Página |
|---|---|---|
| `Parsing.string(v)` | texto (limpo) | [string/string.md](string/string.md) |
| `Parsing.integer(v)` | inteiro | [integer/integer.md](integer/integer.md) |
| `Parsing.floating(v)` | número decimal | [floating/floating.md](floating/floating.md) |
| `Parsing.boolean(v)` | booleano | [boolean/boolean.md](boolean/boolean.md) |
| `Parsing.JSONformatt(v)` | dict/lista (JSON) | [JSONformatt/JSONformatt.md](JSONformatt/JSONformatt.md) |
| `Parsing.Arrayformatt(v)` | lista | [Arrayformatt/Arrayformatt.md](Arrayformatt/Arrayformatt.md) |
| `Parsing.Tuplasformatt(v)` | tupla | [Tuplasformatt/Tuplasformatt.md](Tuplasformatt/Tuplasformatt.md) |
| `Parsing.TransientValue(v)` | conversão preservando o original | [TransientValue/TransientValue.md](TransientValue/TransientValue.md) |

---

## O número dentro do texto

`integer` e `floating` acham o número num texto sujo pela MESMA regra, a do
formato brasileiro:

- O número é o primeiro trecho de dígitos do texto, com `.` e `,` no meio. O
  `-` colado antes dele (só brancos no meio) é o sinal: `"R$ -19,90"` é
  -19,9. O `e` colado depois, com dígitos, é o expoente: `"1e5"`, `"2,5e-3"`.
- Os separadores:
  - vírgula **e** ponto: o ponto é milhar, e a vírgula, o decimal
    (`"1.299,90"` é 1299,9);
  - só vírgula: uma é o decimal (`"19,90"`); mais de uma é milhar
    (`"1,234,567"`);
  - só ponto: um ponto seguido de exatamente 3 dígitos é milhar (`"1.234"` é
    1234), e mais de um ponto também (`"1.234.567"`); senão é o decimal
    (`"12.7"`, `"3.14"`).
- Sem dígito nenhum, o resultado é `0` (ou `0.0`): o `Parsing` não quebra.

A regra tem um custo, e ele é de propósito: `"3.141"` é lido como milhar
(3141). Texto com ponto decimal de três casas é do formato americano, e vai no
`flo()` (`flo("3.141")` é 3.141).

```ps
import Parsing

post(Parsing.integer("R$ 1.234"), Parsing.integer("R$ 19,90"), Parsing.integer("12.7"), Parsing.integer("-5"))
post(Parsing.floating("1.299,90"), Parsing.floating("3.14"), Parsing.floating("1.234"), Parsing.floating("2,5e-3"))
```

```saida
1234 19 12 -5
1299.9 3.14 1234.0 0.0025
```

O texto tem qualquer tamanho (era cortado em 256 caracteres, e o `string` em
1.024).

---

## `Parsing` vs `int()`/`flo()` builtins

- **`int("abc")`** — quebra com erro se o texto não for um número limpo.
- **`Parsing.integer("R$ 1.234")`** — extrai o que dá (`1234`), tolerante.

Use os conversores builtin (`int`, `flo`) quando você **espera** um valor limpo
e quer que erro apareça se não for. Use `Parsing` quando a entrada é **suja/
incerta** (texto de usuário, planilha) e você quer o melhor esforço.

---

## Exemplo

```ps
import Parsing

# entrada suja de um formulário
flo preco  = Parsing.floating("R$ 19,90")     # 19.9 (entende vírgula BR)
int qtd    = Parsing.integer("12 unidades")   # 12
bool ativo = Parsing.boolean("sim")           # True
```
