# Caso das letras: `upper`, `lower`, `title`, `capitalize`

Mudam **maiúsculas/minúsculas** do texto. Todos devolvem uma **string nova** (a
original não muda) e são **encadeáveis**.

```
s.upper()       -> str
s.lower()       -> str
s.title()       -> str
s.capitalize()  -> str
```

---

## Cada um

```
"poolScript".upper()        // "POOLSCRIPT"   (tudo maiúsculo)
"POOLScript".lower()        // "poolscript"   (tudo minúsculo)
"ana maria souza".title()   // "Ana Maria Souza"  (1ª letra de CADA palavra)
"ana maria".capitalize()    // "Ana maria"    (só a 1ª letra da frase)
```

A diferença entre `title` e `capitalize` é o que mais confunde:

| Método | `"joão da silva"` vira |
|---|---|
| `title()` | `"João Da Silva"` (toda palavra) |
| `capitalize()` | `"João da silva"` (só o começo) |

---

## Por que "devolve nova" importa

A string original **não muda** — o método entrega outra. Se você não guardar o
resultado, ele se perde:

```
nome = "ana"
nome.upper()          // gera "ANA" e joga fora
post(nome)            // "ana"   (intacta!)

nome = nome.upper()   // agora sim
post(nome)            // "ANA"
```

---

## Uso comum: comparar sem ligar pro caso

```
if entrada.lower() == "sim":
    post("confirmado")
```

Assim `"Sim"`, `"SIM"` e `"sim"` passam igual.

---

## Relacionados

- [strip](../strip/strip.md) — limpar espaços das bordas
- [verificação](../verificacao/verificacao.md) — `isupper()`, `islower()`
