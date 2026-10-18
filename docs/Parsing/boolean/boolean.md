# `Parsing.boolean(value, to_type="bool")`

Converte um valor pra **booleano** (`True`/`False`), entendendo os textos
comuns de sim e não.

```assinatura
Parsing.boolean(value, to_type="bool") -> bool
```

`to_type` existe por uniformidade com as outras funções do `Parsing` — aqui
**não muda nada**: o resultado é sempre `bool`.

---

## Uso

```ps
import Parsing

post(Parsing.boolean("sim"), Parsing.boolean("true"), Parsing.boolean("1"), Parsing.boolean("x"))
post(Parsing.boolean("não"), Parsing.boolean("NÃO"), Parsing.boolean(" off "), Parsing.boolean("0"), Parsing.boolean(""), Parsing.boolean("null"))
```

```saida
True True True True
False False False False False False
```

Num texto, são **falsos** `""`, `"0"`, `"false"`, `"no"`, `"off"`, `"não"`,
`"nao"`, `"null"` e `"none"`, sem olhar a caixa (`"NÃO"` e `"False"` também) e
sem os brancos das pontas. O resto é verdadeiro. Antes `"não"` dava `True`,
embora esta página dissesse `False`.

Valor que não é texto segue a regra da linguagem: `0`, `0.0`, `Null` e
coleção vazia são falsos.

---

## Relacionados

- [visão geral do Parsing](../Parsing.md)
