# datasentity — `@dataentity` e conversões de Entity

Lib que fornece o decorador **`@dataentity`** (gera um `__init__` automático a
partir de campos tipados) e funções pra converter uma instância em dict, tupla,
lista ou JSON.

```
from datasentity import dataentity, asdict, astuple, aslist, asjson
// (dataentity é o nome canônico; datasentity é o alias do módulo)
```

| Membro | O que faz | Página |
|---|---|---|
| `@dataentity` | gera `__init__` automático de campos tipados | [dataentity/dataentity.md](dataentity/dataentity.md) |
| `asdict(obj)` | instância → dict | [asdict/asdict.md](asdict/asdict.md) |
| `astuple(obj)` | instância → tupla | [astuple/astuple.md](astuple/astuple.md) |
| `aslist(obj)` | instância → lista | [aslist/aslist.md](aslist/aslist.md) |
| `asjson(obj)` | instância → string JSON | [asjson/asjson.md](asjson/asjson.md) |

---

## Exemplo completo

```
from datasentity import dataentity, asdict, astuple, aslist, asjson

@dataentity
Entity Pessoa():
    nome: str
    idade: int = 18          // default opcional

p = Pessoa(nome="Ana", idade=30)     // __init__ gerado — aceita kwargs
q = Pessoa(nome="Léo")                // idade cai no default 18

post(asdict(p))     // {"nome": "Ana", "idade": 30}
post(astuple(p))    // ("Ana", 30)
post(aslist(p))     // ["Ana", 30]
post(asjson(p))     // '{"nome": "Ana", "idade": 30}'
post(q.idade)       // 18
```

---

## O que `@dataentity` economiza

Sem ele, você escreveria o `__init__` na mão:

```
Entity Pessoa():
    action __init__(self, nome, idade):
        self.nome = nome
        self.idade = idade
```

Com `@dataentity`, você só declara os campos (`nome: str`) e o construtor é
gerado. Ver [Entity no LANGUAGE.md](../LANGUAGE.md) pra a sintaxe de classes.

---

## Relacionados

- [`@dataentity`](dataentity/dataentity.md) — o decorador em detalhe
- `Entity` (classes) — ver `../LANGUAGE.md`
