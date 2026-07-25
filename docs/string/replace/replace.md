# `replace` — trocar trechos (com superpoder de lista)

Troca ocorrências de um trecho por outro. Devolve string nova, **encadeável**.
Em PoolScript o `replace` vai além do Python: aceita **lista** de alvos.

```
s.replace(alvo, novo?, count?)   -> str
```

- `novo` é **opcional** — sem ele, remove o alvo (troca por `""`).
- `count` é opcional — limita quantas trocas fazer.

---

## Básico

```
"Hello".replace("l", "L")        // "HeLLo"   (todas)
"a-b-c".replace("-", "")         // "abc"     (novo omitido = remove)
"aaa".replace("a", "b", 2)       // "bba"     (só as 2 primeiras)
```

---

## Superpoder 1 — lista de alvos, um só substituto

Passe uma **lista** no lugar do alvo: todos são trocados pelo mesmo `novo`.
Ótimo pra limpar vários caracteres de uma vez:

```
"(11) 99999-8888".replace(["(", ")", "-", " "], "")   // "11999998888"
```

Sem lista, você teria que encadear vários `.replace`. Com lista, é um só.

---

## Superpoder 2 — lista pra lista (par a par)

Se `novo` também for lista, troca **na ordem**: alvo[0]→novo[0], alvo[1]→novo[1]…

```
"a e i".replace(["a", "e", "i"], ["1", "2", "3"])     // "1 2 3"
```

Útil pra tabelas de substituição (acentos, códigos):

```
texto.replace(["á", "é", "ç"], ["a", "e", "c"])
```

---

## Encadeamento

Como devolve string, você emenda:

```
"  Título Longo  ".strip().lower().replace(" ", "_")   // "título_longo"
```

---

## Pra padrões (não texto literal) — use regex

`replace` troca **texto exato**. Pra padrões (dígitos, qualquer letra…), use
[`.sub`](../regex/regex.md):

```
"tel: 1234".sub("[0-9]", "*")     // "tel: ****"
```

---

## Relacionados

- [regex — `.sub`](../regex/regex.md) — troca por padrão
- [split](../split/split.md) — quebrar em vez de trocar
- [strip](../strip/strip.md) — limpar só as bordas
