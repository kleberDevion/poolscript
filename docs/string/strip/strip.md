# Limpar bordas: `strip`, `lstrip`, `rstrip`

Removem caracteres das **pontas** da string — por padrão, espaços em branco
(incluindo `\n`, `\t`). Devolvem string nova, encadeável.

```
s.strip(chars?)    -> str   // dos dois lados
s.lstrip(chars?)   -> str   // só da esquerda (left)
s.rstrip(chars?)   -> str   // só da direita (right)
```

`chars` é **opcional**. Sem ele, tira espaços.

---

## Sem argumento — tira espaços

```
"   ana   ".strip()     // "ana"
"   ana   ".lstrip()    // "ana   "   (sobra à direita)
"   ana   ".rstrip()    // "   ana"   (sobra à esquerda)
```

Cuidado: só mexe nas **pontas**, nunca no meio:

```
"  a b  ".strip()       // "a b"   (o espaço do meio fica)
```

---

## Com argumento — tira caracteres específicos

O argumento é um **conjunto de caracteres** a remover, não uma palavra:

```
"###titulo###".strip("#")     // "titulo"
"xxyabcyxx".strip("xy")       // "abc"   (remove qualquer x OU y das bordas)
"arquivo.txt".rstrip(".txt")  // "arquivo"  — CUIDADO (veja abaixo)
```

> ⚠️ **Pegadinha**: `rstrip(".txt")` remove qualquer `.`, `t`, `x` do fim, não a
> palavra `.txt`. Em `"reporttxt".rstrip(".txt")` sobra `"repor"`. Pra tirar um
> **sufixo exato**, use [`removeEnd`-style com `replace`](../replace/replace.md)
> ou fatie a string.

---

## Uso comum: limpar entrada do usuário

```
nome = input("nome: ").strip()
if len(nome) == 0:
    post("nome vazio!")
```

Sem o `strip`, um `"   "` (só espaços) passaria como se tivesse conteúdo.

---

## Relacionados

- [replace](../replace/replace.md) — trocar/remover trechos no meio também
- [caso](../caso/caso.md) — maiúsc./minúsc.
