# `regex.compile(pattern)`

Compila o padrão de expressão regular num objeto `Pattern`, que casa com os
métodos dele (`match`, `search` e os outros, abaixo).

```
regex.compile(pattern: str) -> Pattern
```

A sequência

```ps
import regex

Object prog = regex.compile(r"\d+")
post(prog.search("abc 42"))
```

equivale a

```ps
import regex

post(regex.search(r"\d+", "abc 42"))
```

mas usar `regex.compile()` e guardar o objeto pra reusar é mais eficiente
quando a expressão é usada várias vezes no mesmo programa: as funções do
módulo compilam o padrão a cada chamada e jogam fora. (O `re` do Python
guarda os padrões recentes num cache; a Jinga não guarda, então num laço o
`compile` faz diferença.)

As flags vão no próprio padrão (`(?i)`, `(?m)`...): o `regex.compile` não
tem o parâmetro `flags` do `re`. Ver [Flags](../regex.md#flags).

---

## O objeto `Pattern`

Os métodos são os das funções do módulo, sem o argumento do padrão:

| Método | Devolve | Igual a |
|---|---|---|
| `.match(string)` | `bool` | [`regex.match()`](../match/match.md): o texto **inteiro** casa? |
| `.fullmatch(string)` | `bool` | [`regex.fullmatch()`](../fullmatch/fullmatch.md), o mesmo que `.match` |
| `.search(string)` | `bool` | [`regex.search()`](../search/search.md): casa em algum lugar? |
| `.findall(string)` | `list` | [`regex.findall()`](../findall/findall.md) |
| `.sub(repl, string, count=0)` | `str` | [`regex.sub()`](../sub/sub.md) |
| `.split(string, maxsplit=0)` | `list` | [`regex.split()`](../split/split.md) |

E um campo (sem parênteses):

| Campo | O que é |
|---|---|
| `.pattern` | o texto do padrão de que o objeto foi compilado |

```ps
import regex

Object p = regex.compile(r"\d+")
post(type(p))
post(p.pattern)
post(p.findall("a1b22"))
post(p.sub("#", "a1b22"))
post(p.split("a1b22c"))
```

```saida
Pattern
\d+
['1', '22']
a#b#
['a', 'b', 'c']
```

Os métodos do `re` não têm aqui os parâmetros `pos` e `endpos` (o pedaço do
texto onde procurar), e o objeto não tem `.flags`, `.groups` nem
`.groupindex`.

---

## Padrão inválido

O erro sai **na hora do `compile`**, e não em cada uso, o que é a vantagem de
compilar cedo. É `ValueError`, com a frase do `re` e a posição, contada em
caractere do padrão:

```ps
import regex

try {
    regex.compile("[a-")
} catch (ValueError e) {
    post(e)
}
```

```saida
unterminated character set at position 0 (linha 4)
```

As frases são as do `re` (ver [Erros](../regex.md#erros)).

---

## Relacionados

- [regex](../regex.md) — a sintaxe do padrão
- [`regex.match()`](../match/match.md) — versão solta
- [`regex.findall()`](../findall/findall.md) — versão solta
- [`regex.sub()`](../sub/sub.md) — versão solta
