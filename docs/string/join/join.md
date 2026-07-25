# `join` — juntar uma lista numa string

Cola os itens de uma lista numa única string, colocando o **separador entre
eles**. É o inverso do [`split`](../split/split.md).

```
separador.join(lista)   -> str
```

---

## A parte que confunde todo mundo

O `join` é método do **separador**, não da lista. Você chama no texto que vai
**entre** os itens:

```
",".join(["a", "b", "c"])     // "a,b,c"
" - ".join(["ana", "bia"])    // "ana - bia"
"".join(["p", "o", "o", "l"]) // "pool"   (separador vazio = cola direto)
```

Leia como: *"junte esta lista usando `,` no meio"*.

---

## Todos os itens precisam ser string

`join` só cola texto. Se a lista tem números, converta antes:

```
nums = [1, 2, 3]
",".join(nums)                      // ERRO (int não é str)

partes = []
for each n in nums:
    addEnd(partes, str(n))
",".join(partes)                    // "1,2,3"
```

---

## Ida e volta com `split`

```
csv = "ana,bia,carol"
lista = csv.split(",")        // ["ana", "bia", "carol"]
de_volta = ",".join(lista)    // "ana,bia,carol"
```

---

## Uso comum: montar uma linha de saída

```
colunas = ["nome", "idade", "cidade"]
cabecalho = " | ".join(colunas)     // "nome | idade | cidade"
post(cabecalho)
```

---

## Relacionados

- [split](../split/split.md) — o inverso: string → lista
- [replace](../replace/replace.md) — trocar trechos
