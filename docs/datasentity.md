# datasentity — Entity com Campos Tipados

Entity com campos tipados e `__init__` automático — sem escrever
`self.campo = campo` na mão.

Os exemplos desta página pressupõem:

```ps contexto
from datasentity import dataentity, asdict, astuple, aslist, asjson
```

---

## @dataentity

Decora uma `Entity` com sintaxe de campo (`nome: tipo`) e injeta o
`__init__` automaticamente. Campos sem valor padrão são obrigatórios;
com valor padrão são opcionais.

Os exemplos de `asdict`, `astuple`, `aslist` e `asjson` abaixo usam o `user`
deste exemplo:

```ps contexto
@dataentity
Entity Person {
    nome:  str
    idade: int
    email: str
}

user = Person(nome="Kleber", idade=17, email="k@mail.com")
post(user.nome)   # "Kleber"
```

---

## asdict(instance)

Converte a instância pra `dict` Jinga. A Entity de dentro (num campo, ou em
`list`, `tup` e `dict` de dentro, em qualquer profundidade) vira `dict`
também; uma que contém a si mesma é `RecursionError`.

```ps
asdict(user)
# {"nome": "Kleber", "idade": 17, "email": "k@mail.com"}
```

---

## astuple(instance)

Converte pra tupla, na ordem dos campos declarados.

```ps
astuple(user)
# ("Kleber", 17, "k@mail.com")
```

---

## aslist(instance)

Converte pra lista, na ordem dos campos declarados.

```ps
aslist(user)
# ["Kleber", 17, "k@mail.com"]
```

---

## asjson(instance)

Converte pra string JSON: é o `asdict` serializado, então a `DataEntity`
aninhada sai como objeto JSON, em qualquer profundidade.

```ps
asjson(user)
# {"nome": "Kleber", "idade": 17, "email": "k@mail.com"}
```

---

## Exemplo — resposta de API com jinker

```ps
from datasentity import dataentity, asdict
from jinker import Jinker, cors, jsonify

@dataentity
Entity Produto {
    nome:  str
    preco: flo
}

app = Jinker(__name__)

@app.route("/api/produto", auth=cors.permiser(), methods=cors.options(["GET"]))
funct produto() {
    p = Produto(nome="Caneca", preco=29.90)
    return jsonify(asdict(p)), 200
}
```
