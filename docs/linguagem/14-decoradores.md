# Referência da Linguagem — 14. Decoradores

Um **decorador** é um marcador `@nome` escrito na linha **antes** de uma
`action` (ou de uma `Entity`), que muda como aquela declaração é tratada. A
PoolScript tem alguns decoradores embutidos (`@static`, `@NonNull`,
`@dataentity`) e uma forma geral `@objeto.metodo(...)` usada por bibliotecas
para registrar handlers.

Verificado na VM.

---

## 14.1. `@static` — método sem `self` (Entity)

Dentro de uma `Entity`, marca um método que **não recebe `self`** e é chamado
**na própria Entity**, não numa instância (ver seção 7.4):

```ps
Entity Mat() {
    @static
    action soma(a, b) {
        return a + b
    }
}

post(Mat.soma(2, 3))     // 5
```

Sem `@static`, um método precisa de `self`; com ele, é uma função ligada ao
tipo.

### Método `@static` que também declara `self`

Você pode escrever `self` num método `@static` — útil quando o **mesmo** método
é chamado dos dois jeitos: na Entity (`C.metodo(x)`) e numa instância
(`C().metodo(x)`). Na chamada **estática** não existe instância, então o `self`
é **dropado**: o argumento posicional cai no **primeiro parâmetro real**, não no
`self`.

```ps
Entity C() {
    @static
    reaction f(self, a, b=10) {
        return a + b
    }
}

post(C.f(5))        // 15  — o 5 vai pro `a`, NÃO pro self
post(C.f(a=7))      // 17
post(C().f(5))      // 15  — via instância, self = a instância
```

Passar argumento demais, ou faltar um obrigatório, dá erro claro
(`esperava até N argumentos` / `faltando argumento: 'a'`) — nunca um erro
obscuro lá adentro.

---

## 14.2. `@NonNull` — barra argumento nulo

Marca uma `action` cujos **argumentos não podem ser `null`**. Passar `null` num
parâmetro levanta erro antes do corpo rodar:

```ps
@NonNull
action saudar(nome) {
    return "olá, " + nome
}

post(saudar("ana"))      // olá, ana
saudar(null)             // RuntimeError: @NonNull: parametro 'nome' em 'saudar' nao pode ser Null
```

A checagem é sobre os **parâmetros** (a entrada), não sobre o valor de retorno.

---

## 14.3. `@dataentity` — marcador de Entity de dados

Marca uma `Entity` como "de dados" (no espírito do `@dataclass` do Python). O
construtor automático a partir dos **campos tipados** (seção 7.2) já acontece
com ou sem o decorador — então `@dataentity` é sobretudo uma **declaração de
intenção**, deixando claro que aquela Entity é um registro de dados.

```ps
@dataentity
Entity Pessoa() {
    nome: str
    idade: int = 18
}

p = Pessoa(nome="Ana", idade=30)     // construtor aceita posicional e nomeado
q = Pessoa(nome="Léo")               // idade cai no default 18
```

### Conversões — a lib `datasentity`

O companheiro do `@dataentity` são as funções de conversão da lib
**`datasentity`**, que transformam uma instância em dict, tupla, lista ou JSON.
São **funções** (recebem a instância), não métodos:

```ps
from datasentity import dataentity, asdict, astuple, aslist, asjson

@dataentity
Entity Pessoa() {
    nome: str
    idade: int
}

p = Pessoa("Ana", 30)
post(asdict(p))     // {'nome': 'Ana', 'idade': 30}
post(astuple(p))    // ('Ana', 30)
post(aslist(p))     // ['Ana', 30]
post(asjson(p))     // {"nome": "Ana", "idade": 30}
```

(Detalhe de cada função na parte de bibliotecas.)

---

## 14.4. Forma geral — `@objeto.metodo(...)`

Um decorador também pode ser uma **chamada a um método de um objeto**. É o que as
bibliotecas usam para **registrar** a action decorada como um handler — o caso
mais comum é registrar rotas de servidor com o **jinker**:

```ps
@app.route("/usuarios", methods=["GET"])
action listar() {
    return { "ok": true }
}
```

O protocolo é: a expressão do decorador (`app.route(...)`) é avaliada, a `action`
abaixo é definida, e o objeto a registra como handler. Os detalhes de cada
decorador desse tipo (rotas, middleware, eventos de socket…) ficam na
documentação da biblioteca que os oferece (ex.: jinker).

---

## 14.5. Decorador desconhecido

Se o decorador não é nenhum dos embutidos nem um registrador válido, a `action`
decorada **não é registrada** — ela simplesmente não passa a existir:

```ps
@qualquer
action f() {
    return 1
}

f()      // NameError: name 'f' is not defined  (o @qualquer engoliu a action)
```

Um decorador que a linguagem não
entende descarta a declaração em vez de rodá-la sem o decorador.

---

## 14.6. Resumo

- **`@static`** — método de Entity sem `self`, chamado na Entity.
- **`@NonNull`** — recusa argumento `null` na action.
- **`@dataentity`** — marca uma Entity de dados (construtor de campos tipados já
  é automático); conversões `asdict`/`astuple`/`aslist`/`asjson` vêm da lib
  `datasentity`.
- **`@objeto.metodo(...)`** — forma geral: registra a action como handler (rotas
  do jinker etc.); detalhes na doc da lib.
- **Decorador desconhecido** — descarta a action (ela não é registrada).
