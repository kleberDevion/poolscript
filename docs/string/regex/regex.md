# Regex integrado: `match`, `findall`, `sub`

Três métodos de string falam **regex** direto, sem precisar `import regex`. Úteis
pra validar formato, extrair pedaços e trocar por padrão.

```
s.match(padrao)          -> bool    // a string INTEIRA casa?
s.findall(padrao)        -> list    // todas as ocorrências
s.sub(padrao, novo)      -> str     // troca por padrão (encadeável)
```

Para regex mais avançado (grupos nomeados, flags, compilar padrão), veja a lib
completa [`regex`](../../regex/regex.md).

---

## `match` — valida a string inteira

Devolve `true` só se **toda** a string casar com o padrão (é um *fullmatch*, não
"contém"):

```
"12345".match("[0-9]+")          // true
"123a".match("[0-9]+")           // false  (o 'a' não casa)
"ana@x.com".match(".+@.+\..+")   // true   (formato de email simples)
```

Por ser fullmatch, é perfeito pra **validação de formato**:

```
cep = input("cep: ").strip()
if cep.match("[0-9]{5}-?[0-9]{3}"):
    post("cep válido")
else:
    post("formato errado")
```

---

## `findall` — extrai todas as ocorrências

Devolve uma **lista** com tudo que casou:

```
"tel 1199, 3040".findall("[0-9]+")   // ["1199", "3040"]
"a1b2c3".findall("[a-z]")            // ["a", "b", "c"]
```

Lista vazia `[]` se não achar nada.

---

## `sub` — troca por padrão

Como o [`replace`](../replace/replace.md), mas o alvo é um **padrão**, não texto
literal. Devolve string nova, **encadeável**:

```
"tel: 1234-5678".sub("[0-9]", "*")      // "tel: ****-****"
"a  b   c".sub("\s+", " ")              // "a b c"   (colapsa espaços)
```

Diferença central:

| | troca o quê |
|---|---|
| [`replace("ab", "x")`](../replace/replace.md) | o texto **literal** `"ab"` |
| `sub("[ab]", "x")` | qualquer **`a` ou `b`** (padrão) |

---

## Uso comum: limpar pra deixar só números

```
"(11) 99999-8888".sub("[^0-9]", "")     // "11999998888"
```

`[^0-9]` = "tudo que **não** é dígito" → some com pontuação e espaços.

---

## Relacionados

- [regex (lib completa)](../../regex/regex.md) — grupos, flags, `compile`
- [replace](../replace/replace.md) — troca de texto literal
- [verificação](../verificacao/verificacao.md) — checagens simples sem regex
