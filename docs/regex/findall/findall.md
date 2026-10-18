# `regex.findall(pattern, string)`

Devolve todos os casamentos de *pattern* em *string* que não se sobrepõem,
numa lista de textos ou de tuplas. O texto é percorrido da esquerda pra
direita, e os casamentos voltam na ordem em que foram achados. Casamentos
vazios entram no resultado.

```assinatura
regex.findall(pattern: str, string: str) -> list
```

O resultado depende do número de grupos de captura do padrão. Sem grupo, uma
lista de textos que casaram o padrão inteiro. Com exatamente um grupo, uma
lista de textos que casaram esse grupo. Com vários grupos, uma lista de
tuplas de textos que casaram os grupos. Grupos não capturantes (`(?:...)`)
não mudam a forma do resultado. Um grupo que não participou do casamento vale
`""`.

---

## Uso

```ps
import regex

post(regex.findall(r"\bf[a-z]*", "which foot or hand fell fastest"))
post(regex.findall(r"(\w+)=(\d+)", "set width=20 and height=10"))
post(regex.findall(r"\d+", "tenho 3 gatos, 2 cães e 10 peixes"))
post(regex.findall(r"\w+", "olá, mundo!"))
```

```saida
['foot', 'fell', 'fastest']
[('width', '20'), ('height', '10')]
['3', '2', '10']
['olá', 'mundo']
```

Um casamento não vazio pode começar logo depois de um casamento vazio (a
regra do `re` desde o Python 3.7). Por isso o preguiçoso `a*?` acha o vazio e
o `'a'` em cada posição:

```ps
import regex

post(regex.findall(r"a*?", "aaa"))
post(regex.findall(r"x*", "ção"))
```

```saida
['', 'a', '', 'a', '', 'a', '']
['', '', '', '']
```

---

## `findall` x `search`

- **[`search`](../search/search.md)** diz se há um casamento (`bool`).
- **`findall`** devolve **todos**, numa lista.

O `re` do Python tem também o `finditer`, que dá a posição de cada
casamento; a Jinga não tem.

---

## Relacionados

- [regex](../regex.md) — a sintaxe do padrão
- [`regex.search()`](../search/search.md) — há um casamento?
- [`regex.sub()`](../sub/sub.md) — trocar os casamentos
- [`regex.split()`](../split/split.md) — dividir onde o padrão casa
