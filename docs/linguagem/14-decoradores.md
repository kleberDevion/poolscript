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
    funct soma(a, b) {
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
funct saudar(nome) {
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

O protocolo é um só, e vale para qualquer decorador que não seja `static`,
`nonnull` ou `dataentity` — inclusive os que **você** escreve em `.ps`:

1. A expressão do decorador é avaliada. **Com parênteses é uma chamada**
   (`@app.route("/x")`, `@log()`); **sem parênteses é o valor** (`@log` é a
   própria funct `log`).
2. A `funct` abaixo é definida.
3. O valor do passo 1 recebe a funct, de um de dois jeitos:
   - se ele tem um método **`.register(f)`** (o registrador do jinker, ou uma
     Entity sua com esse método), o motor chama `register(funct)` e o nome da
     funct continua sendo a própria funct;
   - senão, se ele é **chamável**, o motor chama `decorador(funct)` e o nome
     passa a valer **o que ele devolveu** — é assim que um decorador envolve
     a funct. Devolver `Null` mantém a funct original.
   - Qualquer outro valor é erro:
     `TypeError: decorador @app.route vale 'str', que nao registra (.register) nem envolve (chamavel) a funct`.

Uma lib sua, então, não precisa de `register`: basta o método devolver uma
funct que recebe a funct decorada:

```ps
Entity NET() {
    funct __init__(self) {
        self.rotas = {}
    }
    funct route(self, caminho) {
        funct registra(f) {
            self.rotas[caminho] = f
            return f
        }
        return registra
    }
}

app = NET()

@app.route("/x")
funct h() {
    return "ok"
}

post(h(), app.rotas["/x"](), len(app.rotas))    # ok ok 1
```

E um decorador que envolve:

```ps
funct log(f) {
    funct w() {
        post("antes")
        return f()
    }
    return w
}

@log
funct h() {
    return 1
}

post(h())     # antes
              # 1
```

Decoradores **empilham**: o de baixo aplica primeiro e o de cima recebe a
funct já envolvida (`@a` sobre `@b` sobre `h` dá `a(b(h))`).

Os detalhes de cada decorador de biblioteca (rotas, middleware, eventos de
socket…) ficam na documentação da biblioteca que os oferece (ex.: jinker).

A funct abaixo pode ter **qualquer** modificador, em qualquer ordem —
`int async funct`, `public async funct`, `bool funct`, `static funct`… (ver
[6.4.2](06-funcoes.md)). A cabeça da declaração é uma unidade só, e o decorador
a captura inteira.

### As três posições

O mesmo `@objeto.metodo(...)` vale em cima de uma **funct solta**, em cima de
uma **classe** e em cima de um **método dentro da classe** — e é o mesmo
protocolo nos três. Só a funct solta pode ser **envolvida** (o nome passa a
valer o que o decorador devolveu); em cima de classe e de método o decorador
**registra**: um chamável que devolva outra funct ali é erro
(`decorador @log de metodo so pode registrar (devolver Null ou o proprio metodo)`).
Uma Entity sem método (fora `__init__`) embaixo de decorador é `SyntaxError`
(`nao ha o que registrar`).

```ps
@r.rota("/funct")
funct f() { return 1 }             # registra f

@r.rota("/classe")
class H() {
    funct handler(self) { return 1 }   # registra o 1º método (fora o __init__), numa instância
}

class D() {
    @r.rota("/dentro")
    static funct h() { return 1 }      # registra D.h — a própria funct

    @r.rota("/inst")
    funct i(self) { return 1 }         # método comum: instancia D() e registra a instância.i
}
```

Dentro da classe, o decorador **roda quando a classe é declarada** — antes de
existir instância. Por isso o objeto que ele usa precisa ser um campo
**`static`** (ver [7.4.1](07-entity.md)):

```ps
class App() {
    public static object mapp = jinker.Jinker(__name__)   # static: existe já na declaração

    @mapp.post("/opa/<data>")
    public static string funct handler(data) { return "ok" }
}
```

Sem o `static`, o campo é de instância e o decorador não tem o que ler. Isso é
recusado na compilação, com a palavra que falta:

```
SyntaxError: 'mapp' e campo de instancia — o decorador no corpo da classe
roda antes de existir instancia; declare-o `static`: static mapp = ...
```

Antes, o decorador em cima de um método era **descartado** pelo compilador: o
método compilava sem registro nenhum e a rota nunca existia — sem erro.

---

## 14.5. Decorador desconhecido é erro

Nada é engolido. Um nome que não existe é `NameError` **na linha do `@`**; um
valor que não registra nem envolve é `TypeError`; um `@` sem nada embaixo é
`SyntaxError`:

```ps
@nao_existe
funct f() {
    return 1
}
# NameError: name 'nao_existe' is not defined
#   em app.ps, linha 1

x = 5
@x
funct g() {
    return 1
}
# TypeError: decorador @x vale 'int', que nao registra (.register) nem envolve (chamavel) a funct

@x
y = 1
# SyntaxError: decorador sem funct ou Entity embaixo
```

> Antes, `@qualquer` desconhecido descartava a `funct` inteira, calado — `f()`
> depois dava `NameError` sobre a **funct**, e esta página documentava isso
> como comportamento. Não é mais.

---

## 14.6. Resumo

- **`static`** (colado; `@static` é a grafia antiga) — método de Entity sem
  `self`, chamado na Entity.
- **`nonnull`** (colado; `@NonNull` é a grafia antiga) — recusa argumento
  `null` na funct.
- **`@dataentity`** — marca uma Entity de dados (construtor de campos tipados já
  é automático); conversões `asdict`/`astuple`/`aslist`/`asjson` vêm da lib
  `datasentity`.
- **Forma geral** (`@objeto.metodo(...)`, `@log`, `@log()`) — com parênteses é
  chamada, sem é o valor; o valor registra (`.register(f)`) ou envolve
  (chamável: o nome passa a valer o retorno; `Null` mantém). Empilha.
- **Decorador desconhecido** — erro: `NameError` (nome), `TypeError` (valor que
  não serve) ou `SyntaxError` (nada embaixo). Nunca engole a funct.
