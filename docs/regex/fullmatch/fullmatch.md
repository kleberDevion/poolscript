# `regex.fullmatch(pattern, string)`

Se o texto **inteiro** casa com a expressão regular *pattern*, devolve
`True`; se não casa, `False`. Repare que isso é diferente de um casamento de
largura zero.

```assinatura
regex.fullmatch(pattern: str, string: str) -> bool
```

É **o mesmo** que [`regex.match()`](../match/match.md). Existe porque o nome
diz a regra: o texto TODO casa. No `re` do Python, `fullmatch` é isto e o
`match` é outra coisa (casa só no começo); quem vem do Python e quer o texto
inteiro escreve `fullmatch` e acerta.

---

## Uso

```ps
import regex

post(regex.fullmatch(r"\d+", "123"))
post(regex.fullmatch(r"\d+", "123abc"))    # sobrou "abc"
post(regex.fullmatch(r"\d+", "a123"))
post(regex.fullmatch(r"\d{5}-?\d{3}", "01310-100"))
```

```saida
True
False
False
True
```

---

## Qual usar

| Quero | Função |
|---|---|
| o texto todo casa? | `fullmatch` (ou `match`, é o mesmo) |
| casa em algum lugar? | [`search`](../search/search.md) |
| todos os casamentos | [`findall`](../findall/findall.md) |

---

## Relacionados

- [regex](../regex.md) — a sintaxe do padrão
- [`regex.match()`](../match/match.md) — idêntico
- [`regex.search()`](../search/search.md) — em qualquer posição
- [`regex.compile()`](../compile/compile.md) — `.fullmatch()` no padrão compilado
