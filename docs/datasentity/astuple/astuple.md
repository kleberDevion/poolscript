# `astuple(instance)`

Converte uma instância de `@dataentity` numa **tupla**, com os valores na ordem
de declaração dos campos.

```
astuple(instance) -> tup
```

---

## Uso

```
from datasentity import dataentity, astuple

@dataentity
Entity Pessoa() {
    nome: str
    idade: int
}

p = Pessoa(nome="Ana", idade=30)
post(astuple(p))     # ("Ana", 30)
```

Só os valores, na ordem dos campos — sem as chaves.

---

## Relacionados

- [`aslist()`](../aslist/aslist.md) — o mesmo, mas lista (mutável)
- [`asdict()`](../asdict/asdict.md) — com as chaves
