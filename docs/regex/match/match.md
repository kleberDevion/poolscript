# `regex.match(pattern, string)`

Se o texto **inteiro** casa com a expressão regular *pattern*, devolve
`True`; se não casa, `False`. Repare que isso é diferente de um casamento de
largura zero.

```
regex.match(pattern: str, string: str) -> bool
```

> **Diferente do `re`:** no Python, `re.match` casa só no **começo** do texto
> (desde o 3.15 com o nome `prefixmatch`), e o texto inteiro é o
> `re.fullmatch`. Na Jinga, `match` e [`fullmatch`](../fullmatch/fullmatch.md)
> são a mesma função: o texto inteiro. Pra "casa em algum lugar", use
> [`search`](../search/search.md); pra "começa com", `search` com `^` no
> padrão.

---

## Uso

```ps
import regex

post(regex.match("o[gh]", "dog"))      # o "o" não está no começo
post(regex.match("o[gh]", "ogre"))     # sobrou "re": não é o texto inteiro
post(regex.match("o[gh]", "oh"))
post(regex.match(r"\d{3}-\d{4}", "123-4567"))
```

```saida
False
False
True
True
```

Pra validar um formato (CPF, e-mail, data), `match` é o certo: garante que
não sobrou nada nas pontas.

---

## `match` x `search`

| | `match` | `search` |
|---|---|---|
| onde casa | o texto **inteiro** | **qualquer** posição |
| devolve | `bool` | `bool` |
| serve pra | **validar** um formato | **achar** algo dentro |

```ps
import regex

post(regex.match("c", "abcdef"), regex.search("c", "abcdef"))
post(regex.match("p.*n", "python"), regex.match("r.*n", "python"))
```

```saida
False True
True False
```

---

## Padrão reusado: compile

Num laço, [`regex.compile()`](../compile/compile.md) compila uma vez só:

```ps
import regex

Object p = regex.compile(r"\d{3}-\d{4}")
for each t in ["123-4567", "abc", "999-0000"] {
    if (p.match(t)) {
        post(t, "valido")
    }
}
```

```saida
123-4567 valido
999-0000 valido
```

---

## Relacionados

- [regex](../regex.md) — a sintaxe do padrão
- [`regex.fullmatch()`](../fullmatch/fullmatch.md) — o mesmo, com o nome explícito
- [`regex.search()`](../search/search.md) — casa em qualquer posição
- [`regex.findall()`](../findall/findall.md) — todos os casamentos
- [`regex.compile()`](../compile/compile.md) — padrão compilado e reusável
