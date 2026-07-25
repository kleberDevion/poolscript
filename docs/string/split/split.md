# `split` — quebrar texto em lista

Quebra a string numa **lista** de pedaços, cortando em cada separador.

```
s.split(sep?, maxsplit?)   -> list
```

- `sep` **opcional** — sem ele, quebra em **qualquer espaço em branco** (e ignora
  espaços repetidos).
- `maxsplit` opcional — número máximo de cortes.

---

## Com separador

```
"a,b,c".split(",")        // ["a", "b", "c"]
"2024-07-25".split("-")   // ["2024", "07", "25"]
"a,,b".split(",")         // ["a", "", "b"]   (vazio entre vírgulas conta)
```

## Sem separador — por espaços

```
"  ana   maria  souza ".split()    // ["ana", "maria", "souza"]
```

Repare: sem argumento ele **colapsa** espaços repetidos e ignora as bordas. Com
`" "` explícito, **não** colapsa:

```
"a  b".split(" ")     // ["a", "", "b"]   (o espaço duplo gera vazio)
"a  b".split()        // ["a", "b"]
```

## Limitando com `maxsplit`

```
"a=b=c".split("=", 1)     // ["a", "b=c"]   (corta só 1 vez)
```

Ótimo pra `chave=valor` onde o valor pode ter o separador dentro.

---

## Uso comum: percorrer os pedaços

```
linha = "ana;30;sp"
partes = linha.split(";")
nome = partes[0]
idade = int(partes[1])
```

E o caminho de volta é [`join`](../join/join.md):

```
partes.join("-")    // errado! join é do separador — veja a página
```

> A junção é `separador.join(lista)`, não `lista.join(...)`. Veja
> [join](../join/join.md).

---

## Relacionados

- [join](../join/join.md) — o inverso: lista → string
- [replace](../replace/replace.md) — trocar em vez de quebrar
- [`len()`](../../builtins/len/len.md) — quantos pedaços saíram
