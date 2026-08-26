# datasentity — Entity com Campos Tipados

Equivalente ao `@dataclass` do Python — Entity com campos tipados,
`__init__` automático (sem escrever `self.campo = campo` na mão).

```
from datasentity import dataentity, asdict, astuple, aslist, asjson
```

---

## @dataentity

Decora uma `Entity` com sintaxe de campo (`nome: tipo`) e injeta o
`__init__` automaticamente. Campos sem valor padrão são obrigatórios;
com valor padrão são opcionais.

```
@dataentity
Entity Person {
    nome:  str
    idade: int
    email: str
}

user = Person(nome="Kleber", idade=17, email="k@mail.com")
post(user.nome)   // "Kleber"
```

---

## asdict(instance)

Converte a instância pra `dict` PoolScript.

```
asdict(user)
// {"nome": "Kleber", "idade": 17, "email": "k@mail.com"}
```

---

## astuple(instance)

Converte pra tupla, na ordem dos campos declarados.

```
astuple(user)
// ("Kleber", 17, "k@mail.com")
```

---

## aslist(instance)

Converte pra lista, na ordem dos campos declarados.

```
aslist(user)
// ["Kleber", 17, "k@mail.com"]
```

---

## asjson(instance)

Converte pra string JSON. Serializa `DataEntity` aninhada recursivamente.

```
asjson(user)
// {"nome": "Kleber", "idade": 17, "email": "k@mail.com"}
```

---

## Exemplo — resposta de API com jinker

```
from datasentity import dataentity, asdict
from jinker import Jinker, cors, jsonify

@dataentity
Entity Produto {
    nome:  str
    preco: flo
}

app = Jinker(__name__)

@app.route("/api/produto", auth=cors.permiser(), methods=cors.options(["GET"]))
action produto() {
    p = Produto(nome="Caneca", preco=29.90)
    return jsonify(asdict(p)), 200
}
```
