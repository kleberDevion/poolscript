# Parsing — Conversão de tipo tolerante

Conversões de tipo "seguras": não quebram com lixo no texto, e não mudam a
variável de entrada. A página de referência é [Parsing](Parsing/Parsing.md),
com uma página por função; aqui vai o resumo.

`Parsing` é uma **lib**: entra com `import Parsing` no topo do arquivo, como
`json` ou `os` (até a versão 16.2.14 ele nascia ligado, sem import).

```ps
import Parsing

post(Parsing.integer("R$ 1.234"), Parsing.floating("1.299,90"), Parsing.boolean("não"))
```

```saida
1234 1299.9 False
```

| Função | Faz | Página |
|---|---|---|
| `Parsing.string(value, to_type="str")` | o texto, com os brancos normalizados; `to int`/`to flo` passam pelo `integer`/`floating` | [string](Parsing/string/string.md) |
| `Parsing.integer(value, to_type="int")` | o número do texto, truncado pra inteiro (de qualquer tamanho); `flo` trunca; `bool` é 0/1 | [integer](Parsing/integer/integer.md) |
| `Parsing.floating(value, to_type="flo")` | o número do texto, como `flo` | [floating](Parsing/floating/floating.md) |
| `Parsing.boolean(value, to_type="bool")` | `""`, `"0"`, `"false"`, `"no"`, `"off"`, `"não"`, `"nao"`, `"null"` e `"none"` são falsos; o resto, verdadeiro | [boolean](Parsing/boolean/boolean.md) |
| `Parsing.TransientValue(value, to_type="str")` | `to int`/`to flo` como o `integer`/`floating`; qualquer outro alvo, o texto | [TransientValue](Parsing/TransientValue/TransientValue.md) |
| `Parsing.JSONformatt(value, to_type="json")` | dict/lista; JSON inválido, ou que não é objeto nem lista, dá `{}` | [JSONformatt](Parsing/JSONformatt/JSONformatt.md) |
| `Parsing.Arrayformatt(value, to_type="list")` | lista: tupla vira lista, texto a lista de caracteres, dict a lista das chaves, o resto `[value]` | [Arrayformatt](Parsing/Arrayformatt/Arrayformatt.md) |
| `Parsing.Tuplasformatt(value, to_type="tup")` | tupla, com a mesma regra | [Tuplasformatt](Parsing/Tuplasformatt/Tuplasformatt.md) |

O número dentro do texto segue a regra do formato brasileiro (ponto de milhar,
vírgula decimal, sinal colado antes, expoente colado depois), descrita em
[O número dentro do texto](Parsing/Parsing.md#o-número-dentro-do-texto).

As conversões devolvem o valor puro (`int`, `flo`, `str`...). O objeto
`TransientValue` que guardava o tipo de origem, de que esta página falava,
não existe mais:

```ps
import Parsing

int v = Parsing.integer("42")
post(v + 8, type(v), v == 42)
```

```saida
50 int True
```
