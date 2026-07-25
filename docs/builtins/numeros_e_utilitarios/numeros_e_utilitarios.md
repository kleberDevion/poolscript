# Números e utilitários: `abs`, `round`, `sum`, `min`, `max`, `id`, `hex`, `bin`, `oct`, `ord`, `chr`

Funções builtin pequenas de número e conversão. Todas sempre disponíveis, sem
import.

---

## Matemática

```
abs(-5)            // 5      (valor absoluto)
round(3.7)         // 4      (arredonda)
sum([1, 2, 3])     // 6      (soma da lista)
min([3, 1, 2])     // 1      (menor)
max([3, 1, 2])     // 3      (maior)
min(5, 2, 8)       // 2      (também aceita argumentos soltos)
```

---

## Bases numéricas (int → texto)

```
hex(255)           // "0xff"    (hexadecimal)
bin(10)            // "0b1010"  (binário)
oct(15)            // "0o17"    (octal)
```

---

## Caracteres (código ↔ letra)

```
ord("A")           // 65    (letra → código)
chr(65)            // "A"    (código → letra)
```

Útil pra trabalhar com códigos de caractere (ex: gerar o alfabeto):

```
for each c in range(65, 91) {
    post(chr(c))   // A, B, C, ... Z
}
```

---

## `id(x)` — endereço na memória

```
id(x)              // um número que identifica o objeto na memória
```

Usado raramente — pra saber se duas variáveis apontam pro **mesmo** objeto.

---

## Relacionados

- [conversores](../conversores/conversores.md) — `str`/`int`/`flo`/`bool`
- [`sorted()`](../sorted/sorted.md) / [`reversed()`](../reversed/reversed.md) — ordenar/inverter listas
