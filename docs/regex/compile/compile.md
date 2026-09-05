# `regex.compile(pattern, flags=0)`

Compila o padrão **uma vez** e devolve um objeto `Pattern` reusável, com os
mesmos métodos do módulo — sem repetir o padrão a cada chamada.

```
regex.compile(pattern: str, flags=0) -> Pattern
```

As funções soltas (`regex.match`, `regex.sub`, ...) compilam o padrão **em
toda chamada** e jogam fora. Num laço, isso é trabalho repetido: `compile`
paga o custo uma vez.

---

## Uso

```
import regex

p = regex.compile("\\d{3}-\\d{4}")

for each t in ["123-4567", "abc", "999-0000"] {
    if (p.match(t)) {
        post(t, "válido")
    }
}
```

---

## Métodos do `Pattern`

Os mesmos do módulo, **sem** o argumento do padrão:

| Método | Devolve | O que faz |
|---|---|---|
| `.match(string)` | `bool` | a string **inteira** casa? (igual ao [`regex.match`](../match/match.md)) |
| `.fullmatch(string)` | `bool` | idêntico a `.match` |
| `.search(string)` | `bool` | casa em **qualquer** posição |
| `.findall(string)` | `list` | **todas** as ocorrências |
| `.sub(repl, string, count=0)` | `str` | substitui as ocorrências |
| `.split(string, maxsplit=0)` | `list` | divide o texto onde o padrão bate |

E um campo (sem parênteses):

| Campo | O que é |
|---|---|
| `.pattern` | o texto do padrão, como você escreveu |

```
p = regex.compile("\\d+")
post(type(p))            # Pattern
post(p.pattern)          # \d+
post(p.findall("a1b22")) # ['1', '22']
post(p.sub("#", "a1b22"))# a#b#
post(p.split("a1b22c"))  # ['a', 'b', 'c']
```

---

## Quando compensa

- **Compensa**: o mesmo padrão usado várias vezes (laço, validação de muitos
  campos, parser de linhas de um arquivo).
- **Tanto faz**: uma chamada só — `regex.match(p, t)` é mais curto.

---

## Padrão inválido

Erra **na hora do `compile`** (e não em cada uso), o que é a vantagem de
compilar cedo:

```
p = regex.compile("[a-")    # TypeError: classe [ ] nao fechada
```

---

## Relacionados

- [`regex.match()`](../match/match.md) — versão solta
- [`regex.findall()`](../findall/findall.md) — versão solta
- [`regex.sub()`](../sub/sub.md) — versão solta
