# `@dataentity`

Decorador aplicado a uma `Entity` que **gera o `__init__` automaticamente** a
partir dos campos tipados declarados no corpo.

```
from datasentity import dataentity

@dataentity
Entity Nome():
    campo: tipo
    campo: tipo = default
```

---

## Uso

```
from datasentity import dataentity

@dataentity
Entity Pessoa():
    nome: str
    idade: int = 18          // com default

p = Pessoa(nome="Ana", idade=30)   // construtor gerado, aceita kwargs
q = Pessoa(nome="Léo")              // idade usa o default 18
post(p.nome, p.idade)               // Ana 30
```

---

## Regras

- **Campos sem default** viram argumentos **obrigatórios** — não passá-los é erro.
- **Campos com `= default`** são opcionais.
- Se você declarar seu **próprio** `__init__` no corpo, ele tem prioridade — o
  automático só é gerado quando não há `__init__` escrito à mão.
- A ordem dos campos é a ordem usada por `astuple`/`aslist`.

---

## Depois: converter a instância

Com o `@dataentity`, a instância vira facilmente dict/tupla/lista/JSON:

```
from datasentity import dataentity, asdict, asjson

@dataentity
Entity Pessoa():
    nome: str
    idade: int

p = Pessoa(nome="Ana", idade=30)
post(asdict(p))     // {"nome": "Ana", "idade": 30}
post(asjson(p))     // '{"nome": "Ana", "idade": 30}'
```

---

## Relacionados

- [`asdict()`](../asdict/asdict.md) · [`astuple()`](../astuple/astuple.md) · [`aslist()`](../aslist/aslist.md) · [`asjson()`](../asjson/asjson.md)
- Entity/classes — ver `LANGUAGE.md`
