# `asdict(instance)`

Converte uma instância de `@dataentity` num **dict**, com os campos como chaves.

```
asdict(instance) -> dict
```

---

## Uso

```
from datasentity import dataentity, asdict

@dataentity
Entity Pessoa() {
    nome: str
    idade: int
}

p = Pessoa(nome="Ana", idade=30)
post(asdict(p))     // {"nome": "Ana", "idade": 30}
```

Útil pra devolver a instância como JSON numa API, ou salvar no banco.

---

## Relacionados

- [`asjson()`](../asjson/asjson.md) — direto pra string JSON
- [`astuple()`](../astuple/astuple.md) · [`aslist()`](../aslist/aslist.md)
