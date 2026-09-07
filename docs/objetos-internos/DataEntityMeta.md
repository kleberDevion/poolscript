# `DataEntityMeta` — não existe

Esta página descrevia um tipo `DataEntityMeta`, "marcador interno" de uma
Entity decorada com `@dataentity`. **Ele não existe na VM**: o nome não aparece
em `pool --metadata`, não está no fonte do motor, e `type()` nunca o devolve.

O que o `@dataentity` deixa é uma Entity comum:

```ps
@dataentity
Entity Pessoa() {
    nome: str
    idade: int = 18
}

p = Pessoa(nome="Ana")
post(type(p))         # Pessoa   — o nome da própria Entity
post(type(Pessoa))    # Entity
```

Não há marcador consultável dizendo "esta foi decorada". O que o decorador faz
— gerar o `__init__` a partir dos campos tipados — está em
[`14-decoradores`](../linguagem/14-decoradores.md) e em
[`datasentity`](../datasentity/datasentity.md).

[← índice](objetos-internos.md)
