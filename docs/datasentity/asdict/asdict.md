# `asdict(instance)`

Converte uma instância de `@dataentity` num **dict**, com os campos como chaves.

```assinatura
asdict(instance) -> dict
```

---

## Uso

```ps
from datasentity import dataentity, asdict

@dataentity
Entity Pessoa {
    nome: str
    idade: int
}

p = Pessoa(nome="Ana", idade=30)
post(asdict(p))     # {"nome": "Ana", "idade": 30}
```

Útil pra devolver a instância como JSON numa API, ou salvar no banco.

A Entity de dentro vira `dict` também — num campo, ou em `list`, `tup` e
`dict` de dentro, em qualquer profundidade:

```ps
from datasentity import dataentity, asdict

@dataentity
Entity Endereco {
    rua: str
}

@dataentity
Entity Pessoa {
    nome: str
    end: Endereco
}

Pessoa p = Pessoa("Ana", Endereco("Rua 1"))
post(asdict(p))     # {'nome': 'Ana', 'end': {'rua': 'Rua 1'}}
```

Uma Entity que contém a si mesma (direto ou por uma lista) não tem fim:
`RecursionError: asdict(): Entity aninhada demais (ou que contem a si mesma)`.

---

## Relacionados

- [`asjson()`](../asjson/asjson.md) — direto pra string JSON
- [`astuple()`](../astuple/astuple.md) · [`aslist()`](../aslist/aslist.md)
