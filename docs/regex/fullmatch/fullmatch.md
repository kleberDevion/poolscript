# `regex.fullmatch(pattern, string, flags=0)`

Testa se o texto **inteiro** casa com o padrão. Devolve **`bool`**.

É **o mesmo** que [`regex.match()`](../match/match.md) — existe porque este é o
nome que o `re` do Python usa pra essa semântica, e o `match` da PoolScript já
casa a string toda (ao contrário do `re.match`, que casa só o começo).

```
regex.fullmatch(pattern: str, string: str, flags=0) -> bool
```

---

## Uso

```
import regex

post(regex.fullmatch("\\d+", "123"))      // true
post(regex.fullmatch("\\d+", "123abc"))   // false — sobrou "abc"
post(regex.fullmatch("\\d+", "a123"))     // false
```

Validando um CEP:

```
if (regex.fullmatch("\\d{5}-?\\d{3}", cep)) {
    post("CEP válido")
} else {
    post("CEP inválido")
}
```

---

## Qual usar

| Quero | Função |
|---|---|
| a string toda casa? | `fullmatch` (ou `match`, é o mesmo) |
| casa em algum lugar? | [`search`](../search/search.md) |
| todas as ocorrências | [`findall`](../findall/findall.md) |

---

## Relacionados

- [`regex.match()`](../match/match.md) — idêntico
- [`regex.search()`](../search/search.md) — em qualquer posição
- [`regex.compile()`](../compile/compile.md) — `.fullmatch()` no padrão compilado
