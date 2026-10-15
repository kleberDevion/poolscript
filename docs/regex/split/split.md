# `regex.split(pattern, string, maxsplit=0)`

Divide *string* onde *pattern* casa. Se o padrão tem parênteses de captura, o
texto de todos os grupos do padrão também entra na lista resultante. Se
*maxsplit* não é zero, acontecem no máximo *maxsplit* divisões, e o resto do
texto vem como o último item da lista.

```
regex.split(pattern: str, string: str, maxsplit=0) -> list
```

| Parâmetro | Padrão | O que é |
|---|---|---|
| `pattern` | — | o separador (como expressão regular) |
| `string` | — | o texto a dividir |
| `maxsplit` | `0` | máximo de divisões (`0` = todas; negativo = nenhuma) |

---

## Uso

```ps
import regex

post(regex.split(r"\W+", "Words, words, words."))
post(regex.split(r"(\W+)", "Words, words, words."))
post(regex.split(r"\W+", "Words, words, words.", maxsplit=1))
post(regex.split("(?i)[a-f]+", "0a3B9"))
```

```saida
['Words', 'words', 'words', '']
['Words', ', ', 'words', ', ', 'words', '.', '']
['Words', 'words, words.']
['0', '3', '9']
```

Se há grupos de captura no separador e ele casa no começo do texto, o
resultado começa com um texto vazio. O mesmo vale pro fim do texto:

```ps
import regex

post(regex.split(r"(\W+)", "...words, words..."))
```

```saida
['', '...', 'words', ', ', 'words', '...', '']
```

Assim, as partes do separador ficam sempre nas mesmas posições relativas da
lista. Um grupo do separador que não participou do casamento entra como
`Null` (no `re`, `None`).

Casamentos vazios vizinhos não existem, mas um casamento vazio pode vir logo
depois de um não vazio:

```ps
import regex

post(regex.split(r"\b", "Words, words, words."))
post(regex.split(r"\W*", "...words..."))
post(regex.split(r"(\W*)", "...words..."))
```

```saida
['', 'Words', ', ', 'words', ', ', 'words', '.']
['', '', 'w', 'o', 'r', 'd', 's', '', '']
['', '...', '', '', 'w', '', 'o', '', 'r', '', 'd', '', 's', '...', '', '', '']
```

Diferente do `split` de texto (que separa por um texto fixo), aqui o
separador é um **padrão**: bom quando ele varia (espaço, vírgula,
ponto e vírgula, tudo junto).

---

## Relacionados

- [regex](../regex.md) — a sintaxe do padrão; o exemplo da agenda usa o `split`
- [`regex.sub()`](../sub/sub.md) — trocar em vez de dividir
- [`regex.findall()`](../findall/findall.md) — os pedaços que casam (o oposto de dividir)
