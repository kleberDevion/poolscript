# Referência da Linguagem — 14. Decoradores

Um **decorador** é um marcador `@nome` escrito na linha **antes** de uma
`funct` (ou de uma `Entity`), que muda como aquela declaração é tratada. A
PoolScript tem o `@dataentity` embutido e uma forma geral `@objeto.metodo(...)`
usada por bibliotecas para registrar handlers.

Dois marcadores que eram decoradores viraram **modificadores colados** na
declaração: `static` e `nonnull`. As grafias `@static` e `@NonNull`, em linha
própria, continuam funcionando — as duas primeiras seções cobrem as duas
formas.

Verificado na VM.

---

## 14.1. `static` — método sem `self` (Entity), e o `@static` antigo

Dentro de uma `Entity`, marca um método que **não recebe `self`** e é chamado
**na própria Entity**, não numa instância (ver seção 7.4). O modificador vem
colado na declaração:

```ps
Entity Mat() {
    static funct soma(a, b) {
        return a + b
    }
}

post(Mat.soma(2, 3))     # 5
```

Sem `static`, um método precisa de `self`; com ele, é uma função ligada ao
tipo. Chamar um `static` pela instância é erro:

```
funct 'soma' e static: chame pela Entity (Tipo.soma(...)), nao pela instancia
```

A grafia antiga, em linha própria, faz exatamente o mesmo:

```ps
Entity Mat() {
    @static
    action soma(a, b) {
        return a + b
    }
}
```

O modificador vale só para a funct em que está escrito — uma funct declarada
**dentro** do corpo dela não herda a marca.

### Método `static` que também declara `self`

Você pode escrever `self` num método `static` — útil quando o **mesmo** método
é chamado dos dois jeitos: na Entity (`C.metodo(x)`) e numa instância
(`C().metodo(x)`). Na chamada **estática** não existe instância, então o `self`
é **dropado**: o argumento posicional cai no **primeiro parâmetro real**, não no
`self`.

```ps
Entity C() {
    static funct f(self, a, b=10) {
        return a + b
    }
}

post(C.f(5))        # 15  — o 5 vai pro `a`, NÃO pro self
post(C.f(a=7))      # 17
post(C().f(5))      # 15  — via instância, self = a instância
```

Passar argumento demais, ou faltar um obrigatório, dá erro claro
(`f() takes N positional arguments but M were given` / `f() missing 1 required positional argument: 'a'`) — nunca um erro
obscuro lá adentro.

---

## 14.2. `nonnull` — barra argumento nulo (e o `@NonNull` antigo)

Marca uma `funct` cujos **argumentos não podem ser `null`**. Passar `null` num
parâmetro levanta erro antes do corpo rodar:

```ps
nonnull funct saudar(nome) {
    return "olá, " + nome
}

post(saudar("ana"))      # olá, ana
saudar(null)             # RuntimeError: nonnull: parametro 'nome' em 'saudar' nao pode ser Null
```

A checagem é sobre os **parâmetros** (a entrada), não sobre o valor de retorno,
e um valor padrão que avalie pra `Null` também é barrado.

A grafia antiga continua valendo, com o mesmo efeito e a mesma mensagem:

```ps
@NonNull
action saudar(nome) {
    return "olá, " + nome
}
```

`static` e `nonnull` **não são palavras reservadas**: valem por posição, só na
cabeça da declaração. Fora dali são nome comum — `nonnull = 1` é uma variável.

---

## 14.3. `@dataentity` — marcador de Entity de dados

Marca uma `Entity` como "de dados". O
construtor automático a partir dos **campos tipados** (seção 7.2) já acontece
com ou sem o decorador — então `@dataentity` é sobretudo uma **declaração de
intenção**, deixando claro que aquela Entity é um registro de dados.

```ps
@dataentity
Entity Pessoa() {
    nome: str
    idade: int = 18
}

p = Pessoa(nome="Ana", idade=30)     # construtor aceita posicional e nomeado
q = Pessoa(nome="Léo")               # idade cai no default 18
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
post(asdict(p))     # {'nome': 'Ana', 'idade': 30}
post(astuple(p))    # ('Ana', 30)
post(aslist(p))     # ['Ana', 30]
post(asjson(p))     # {"nome": "Ana", "idade": 30}
```

(Detalhe de cada função na parte de bibliotecas.)

---

## 14.4. Forma geral — `@objeto.metodo(...)`

Um decorador também pode ser uma **chamada a um método de um objeto**. É o que as
bibliotecas usam para **registrar** a funct decorada como um handler — o caso
mais comum é registrar rotas de servidor com o **jinker**:

```ps
@app.route("/usuarios", methods=["GET"])
funct listar() {
    return { "ok": true }
}
```

O protocolo é: a expressão do decorador (`app.route(...)`) é avaliada, a `funct`
abaixo é definida, e o objeto a registra como handler. Os detalhes de cada
decorador desse tipo (rotas, middleware, eventos de socket…) ficam na
documentação da biblioteca que os oferece (ex.: jinker).

A funct abaixo pode ter **qualquer** modificador, em qualquer ordem —
`int async funct`, `public async funct`, `bool funct`, `static funct`… (ver
[6.4.2](06-funcoes.md)). A cabeça da declaração é uma unidade só, e o decorador
a captura inteira.

---

## 14.5. Decorador desconhecido

Se o decorador não é nenhum dos embutidos nem um registrador válido, a `funct`
decorada **não é registrada** — ela simplesmente não passa a existir:

```ps
@qualquer
funct f() {
    return 1
}

f()      # NameError: name 'f' is not defined  (o @qualquer engoliu a funct)
```

Um decorador que a linguagem não
entende descarta a declaração em vez de rodá-la sem o decorador.

---

## 14.6. Resumo

- **`static`** (colado; `@static` é a grafia antiga) — método de Entity sem
  `self`, chamado na Entity.
- **`nonnull`** (colado; `@NonNull` é a grafia antiga) — recusa argumento
  `null` na funct.
- **`@dataentity`** — marca uma Entity de dados (construtor de campos tipados já
  é automático); conversões `asdict`/`astuple`/`aslist`/`asjson` vêm da lib
  `datasentity`.
- **`@objeto.metodo(...)`** — forma geral: registra a funct como handler (rotas
  do jinker etc.); detalhes na doc da lib.
- **Decorador desconhecido** — descarta a funct (ela não é registrada).
