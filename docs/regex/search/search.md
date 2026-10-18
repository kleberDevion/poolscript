# `regex.search(pattern, string)`

Percorre o texto procurando o primeiro lugar onde a expressão regular
*pattern* casa, e devolve `True`. Devolve `False` se nenhuma posição do texto
casa com o padrão; repare que isso é diferente de achar um casamento de
largura zero em algum ponto do texto.

```assinatura
regex.search(pattern: str, string: str) -> bool
```

No `re` do Python, `search` devolve um objeto Match (ou `None`); na Jinga
devolve `bool`. Pra pegar **o que** casou, use
[`findall`](../findall/findall.md).

---

## Uso

```ps
import regex

post(regex.search("d", "dog"))
post(regex.search("erro", "log: um erro aconteceu"))
post(regex.search(r"\d", "abc"))
```

```saida
True
True
False
```

---

## `search` x `match`

- **`search`** procura um casamento em **qualquer lugar** do texto (é o que o
  Perl faz por padrão).
- **[`match`](../match/match.md)** confere se o texto **inteiro** casa (é o
  mesmo que o `fullmatch`).

Um padrão que começa com `'^'` prende o `search` no começo do texto: é o
"começa com". No modo MULTILINE, o `'^'` casa no começo de cada linha:

```ps
import regex

post(regex.search("^c", "abcdef"), regex.search("^a", "abcdef"))
post(regex.search("^X", "A\nB\nX"), regex.search("(?m)^X", "A\nB\nX"))
```

```saida
False True
False True
```

---

## Relacionados

- [regex](../regex.md) — a sintaxe do padrão
- [`regex.match()`](../match/match.md) — o texto inteiro
- [`regex.findall()`](../findall/findall.md) — todos os casamentos, não só se há um
