# `aslist(instance)`

Converte uma instância de `@dataentity` numa **lista**, com os valores na ordem
de declaração dos campos.

```
aslist(instance) -> list
```

---

## Uso

```
from datasentity import dataentity, aslist

@dataentity
Entity Pessoa() {
    nome: str
    idade: int
}

p = Pessoa(nome="Ana", idade=30)
post(aslist(p))     # ["Ana", 30]
```

Igual ao [`astuple`](../astuple/astuple.md), mas devolve uma lista (mutável).

---

## Relacionados

- [`astuple()`](../astuple/astuple.md) — versão imutável
- [`asdict()`](../asdict/asdict.md) — com as chaves
