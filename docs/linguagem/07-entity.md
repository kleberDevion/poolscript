# Referência da Linguagem — 7. Entity (classes e objetos)

`Entity` é a construção de orientação a objetos da Jinga: define um **tipo**
com campos e métodos, do qual se criam **instâncias**. Esta seção cobre a
declaração, os campos e o construtor (sintetizado ou próprio), os métodos e o
`self`, os métodos `static`, a herança com `base`, e o encapsulamento
`private`/`public`.

Tudo verificado na VM.

---

## 7.1. Declaração

```ps
Entity Usuario {
    nome: str
    idade: int
}
```

- O nome da Entity começa com **maiúscula** por convenção (é `IDENT_UPPER`,
  seção 1.4). O motor não exige: `Entity usuario` compila e roda. A maiúscula
  é o que separa, à leitura, o tipo do valor.
- **Sem parênteses** no cabeçalho: o `()` fica só na instanciação
  (`Usuario("ana", 3)`). Herança vai entre parênteses depois do nome:
  `Entity Cao(Animal) {` (7.5). `Entity Usuario() {` com `()` vazio é erro
  (`o '()' fica so na instanciacao: escreva Entity Usuario {`), e
  `Entity Usuario:` também (`bloco com ':' nao existe mais`).
- **`class` e `Class` são sinônimos de `Entity`** — mesma semântica.

```ps
class Ponto {          # idêntico a Entity Ponto {
    x: int
    y: int
}
```

---

## 7.2. Campos e construtor sintetizado

Campos são declarados como uma variável (seção 4.1): com **tipo**, em qualquer
das duas ordens — `nome: tipo` ou `tipo nome` — ou **sem tipo**, `nome = valor`,
e aí o campo aceita qualquer valor, como um `x = 1`. A partir deles a linguagem
**sintetiza um construtor** (`__init__`) que recebe um argumento por campo, na
ordem declarada:

```ps
Entity Usuario {
    nome: str
    idade: int
}

u = Usuario("ana", 30)
post(u.nome, u.idade)      # ana 30
```

```ps
Entity Usuario {         # idêntico ao de cima
    str nome
    int idade
}
```

- **Valor padrão** num campo torna o argumento opcional:

  ```ps
  Entity Config {
      host: str = "localhost"
      porta: int = 8080
  }

  c = Config()              # usa os padrões
  post(c.host, c.porta)     # localhost 8080
  ```

- **Campos criados no método:** um método pode criar um campo não declarado
  com `self.x = ...` — ele passa a existir na instância:

  ```ps
  Entity Bolsa {
      funct guarda(self, item) {
          self.conteudo = item
      }
  }
  ```

- **O campo é da instância, não da Entity.** Ele só existe depois de
  `Livro(...)`; lê-lo pelo nome do tipo é erro, acusado **antes de rodar**:

  ```ps
  Entity Livro {
      public string autor = "admin"
  }

  post(Livro.autor)     # RuntimeError: Entity 'Livro' não tem campo estático 'autor' — instancie primeiro
  ```

  As duas saídas, conforme o que se quer:

  ```ps
  Entity Livro {
      public string autor = "admin"
  }
  Entity Livro2 {
      public static string autor = "admin"
  }

  l = Livro()
  post(l.autor)         # admin   — um valor por instância
  post(Livro2.autor)    # admin   — um valor só, da classe (7.4.1)
  ```

  É a mesma regra do método: sem `static`, o membro pertence ao objeto, e o
  nome do tipo não chega nele.

### 7.2.1. Construtor próprio — `funct __init__`

Para um construtor com lógica própria (validação, campos derivados), defina
`funct __init__(self, …)`. Isso **substitui** o construtor sintetizado:

```ps
Entity Retangulo {
    funct __init__(self, largura, altura) {
        self.largura = largura
        self.altura  = altura
        self.area    = largura * altura
    }
}

r = Retangulo(3, 4)
post(r.area)              # 12
```

`Retangulo(3, 4)` **vale a instância**, sempre — não o que o `__init__`
retornar, e não o que ficou no primeiro parâmetro. A instância entra como o
**primeiro posicional**: num `__init__` sem `self` que só tem `*args`, ela é o
primeiro item da tup:

```ps
Entity E {
    funct __init__(*args) {
        post(len(args), type(args[0]))   # 3 E
    }
}

post(type(E(1, 2)))                      # E
```

A Entity também vale como função que se passa adiante: `map([1, 2], Ponto)`
cria uma instância por item. O `__init__` não pode ser `async` nem gerador
(`TypeError: __init__() should return None, not 'future'`).

---

## 7.3. Métodos e `self`

Um método é uma `funct` cujo **primeiro parâmetro é `self`** (a instância).
Sem `self`, é erro (a não ser que seja `static`, 7.4).

O erro é da **declaração** e sai antes de rodar, na linha do `funct` — não
no primeiro `self` do corpo, que seria só o sintoma. É um erro só: o motor
não repete o mesmo defeito em cada `self` e em cada chamada.

```ps
Entity Conta {
    saldo: int
    funct extrato() {        # TypeError: método extrato() sem self: o primeiro parâmetro de um método é self (ou marque static)
        return self.saldo
    }
}
```

Primeiro parâmetro com outro nome (`funct extrato(x)`) é o mesmo erro, e a
mensagem diz qual nome encontrou. A exceção é `*args` na frente: a instância
entra na tup (7.2).

```ps
Entity Contador {
    valor: int
    funct inc(self) {
        self.valor += 1
        return self.valor
    }
    funct zera(self) {
        self.valor = 0
    }
}

c = Contador(0)
post(c.inc(), c.inc())   # 1 2
```

Dentro de um método, `self.campo` acessa/atribui campos e `self.outro(...)`
chama outros métodos da instância.

---

## 7.4. Métodos `static`

Prefixado com `static`, o método **não recebe `self`** e é chamado **na
própria Entity** (não numa instância):

```ps
Entity Mat {
    static funct soma(a, b) {
        return a + b
    }
}

post(Mat.soma(2, 3))     # 5
```

Chamar um `static` por uma instância (`m.soma(...)`) é erro — ele pertence ao
tipo, não ao objeto. (Ver também a nota sobre `static` na seção de
decoradores.)

### 7.4.1. Campos `static` — o atributo da classe

Um campo comum pertence à **instância**: nasce no `__init__`, um por objeto.
Prefixado com `static`, o campo pertence à **classe**: é avaliado **uma vez**,
quando a classe é declarada, e existe antes de qualquer instância.

```ps
import jinker

class App {
    public static object mapp = jinker.Jinker(__name__)

    @mapp.post("/opa/<data>")
    public static string funct handler(data) {
        return "ok"
    }

    public static funct run() {
        mapp(debug=true, port=3003, host="127.0.0.1")
    }
}
App.run()
```

Os modificadores vêm em qualquer ordem, antes **ou depois** do tipo — a mesma
regra da cabeça de funct:

```ps
class Config {
    static int a = 1
    private static int b = 2
    private int static c = 3         # o `static` depois do tipo também vale
    int static private d = 4
}
post(Config.a, Config.d)             # 1 4
```

Sem inicializador o campo nasce `Null`.

Quem enxerga o campo, e como:

| De onde | Como se escreve |
|---|---|
| de fora | `App.mapp` |
| dentro da classe — corpo (decoradores), método `static` ou método comum | `mapp`, o nome solto (também `self.mapp` e `App.mapp` num método comum) |
| numa classe filha | `mapp`, o nome solto, igual — o `static` do pai é da filha (também `self.mapp` e `Pai.mapp`) |

A recíproca também vale: **campo sem `static` não é alcançado pelo nome do
tipo**. `App.nao_static` é `RuntimeError: Entity 'App' não tem campo estático
'nao_static' — instancie primeiro` (7.2), no `--check` e rodando.

A regra é uma só: **o que `App.x` alcança de fora, `x` alcança de dentro** —
e vale pro **método `static`** do mesmo jeito. `s()` solto dentro de qualquer
método chama o `static funct s()` da classe (ou de um pai), com a aridade
conferida antes de rodar como em `App.s()`. Método comum (com `self`) não:
`m()` solto é `NameError` — ele é da instância, `self.m()`.

```ps
class Pai {
    public static int total = 7
    static funct dobro(n) { return n * 2 }
}
class Filha(Pai) {
    funct f(self) { return dobro(total) }    # 14 — campo e método static do pai, soltos
}
```

Um parâmetro ou variável local com o mesmo nome **ganha** do campo: `funct
m(self, x)` devolve o `x` recebido, não o `C.x`. O editor oferece os mesmos
nomes: dentro da classe, o completion lista os `static` dela (e dos pais)
soltos, e o hover/definição num nome solto vai no campo ou método.

É **um só** valor, compartilhado: `K.n = K.n + 1` num método muda o que toda
instância lê em `self.n`. E ele **não entra** no construtor sintetizado —
`class P { static int total = 0  str nome }` continua sendo `P("ana")`.

Por que existe: sem ele, `App.mapp` não existia, um método `static` não tinha
como enxergar o campo, e o decorador no corpo da classe rodava antes de haver
instância — o programa acima passava no `--check` e não fazia nada.

---

## 7.5. Herança

Uma Entity pode herdar de uma ou mais outras, listadas entre parênteses depois
do nome (`Entity Cao(Animal) {`; sem herança não há parênteses). Os
**métodos** do(s) pai(s) ficam disponíveis; um método redefinido no filho
**sobrescreve** o do pai.

```ps
Entity Animal {
    nome: str
    funct fala(self) {
        return "..."
    }
}

Entity Cao(Animal) {
    funct fala(self) {
        return "au"      # sobrescreve
    }
}

c = Cao("rex")
post(c.nome, c.fala())   # rex au

# herança múltipla:
Entity C(A, B) {          # herda métodos de A e de B
    x: int
}
```

### 7.5.1. Construtor e herança

- Um filho que **não declara campos nem `__init__`** herda o construtor do pai
  (recebe os campos do pai).
- Um filho que **declara campos próprios** sintetiza o seu próprio construtor,
  só com os **campos dele** — os do pai não entram automaticamente. Para incluí-los,
  redeclare-os no filho ou escreva um `__init__` próprio.

### 7.5.2. `base()` / `base(Pai)` — o pai, como o `super`

`base()` designa o pai, e `.membro` em cima dele alcança **tudo que o pai
tem** com o `self` atual: o construtor (`base().__init__(args)`), qualquer
método (`base().fala()`, mesmo que o filho o sobrescreva) e os campos
(`base().x`). Com um pai só, `base()`; com mais de um, ou pra mirar um avô,
o nome vai dentro: `base(Pai)`.

```ps
Entity A {
    static int total = 7
    funct __init__(self, x) {
        self.x = x
    }
    str funct fala(self) {
        return "A diz"
    }
}

Entity B(A) {
    funct __init__(self, x, y) {
        base().__init__(x)          # roda o __init__ de A com este self
        self.y = y
    }
    str funct fala(self) {
        return base().fala() + " e B tambem"   # a versao de A, sobrescrita aqui
    }
    int funct soma(self) {
        return base().x + base().total         # campo da instancia e static do pai
    }
}

b = B(1, 2)
post(b.x, b.y, b.fala(), b.soma())   # 1 2 A diz e B tambem 8
```

Regras:

- **`.__init__(args)`** aceita posicional, nomeado (`base().__init__(x=1)`)
  e espalhado (`base().__init__(*a, **kw)`); o `self` é sempre o do método
  atual. Aridade e tipos são conferidos antes de rodar, com as mesmas frases
  de `A(1, 2)`.
- **`.metodo(args)`** chama a versão do pai (a dele própria, ou a que ele
  herdou), ligada ao `self` atual. Vale em qualquer método com `self`, não
  só no `__init__`.
- **`.campo`** lê: campo `static` do pai, ou o campo da instância (é o mesmo
  objeto que `self.campo`). Escrever é pelo `self`: `base().x = 1` é erro
  (`atribua pelo self: self.x = ...`).
- **`private` do pai** continua invisível pro filho, também por `base()`:
  `acesso negado: 'x' e private de A`.
- **Mais de um pai:** `base()` sem nome é erro que lista as opções
  (`base() com mais de um pai: diga qual, base(A) ou base(B)`);
  `base(Pai)` mira o pai escrito. `base(X)` com X que não é ancestral:
  `'X' nao e pai de 'Filha' (pais: A, B)`.

```ps
Entity Motor {
    funct __init__(self, cavalos) { self.cavalos = cavalos }
}
Entity Roda {
    funct __init__(self, qtd) { self.qtd = qtd }
}
Entity Carro(Motor, Roda) {
    funct __init__(self) {
        base(Motor).__init__(300)
        base(Roda).__init__(4)
    }
}
c = Carro()
post(c.cavalos, c.qtd)   # 300 4
```

`base(...)` só vale dentro de um método que declara `self` como primeiro
parâmetro (o `self` é o objeto que o pai recebe). Sem `self`, fora de Entity,
ou numa Entity sem herança, é erro na declaração:

```
SyntaxError: base() precisa do self: declare `funct __init__(self, ...)` (o self e o objeto que o pai inicializa)
SyntaxError: base() fora de Entity com heranca
SyntaxError: base() numa Entity sem heranca: nao ha pai pra inicializar
```

`base(Pai)` sem `.membro` depois, e a forma antiga `base(args)`, acusam a
forma certa: `base(Pai) precisa de um membro: base(Pai).__init__(...) ou
base(Pai).metodo(...)`. Membro que o pai não tem: `AttributeError: 'A' object
has no attribute 'z'`.

O pai precisa **ter** construtor pra `base().__init__`: um `__init__` próprio
ou campos declarados (que sintetizam um). Se não tem nenhum dos dois:
`TypeError: base(): a Entity pai 'A' nao tem __init__` — posicional ou
nomeado, a resposta é a mesma, e sai antes de rodar.

---

## 7.6. Encapsulamento — `private` e `public`

Um campo ou método marcado **`private`** só é acessível **de dentro da própria
Entity**. Acessá-lo de fora é erro (`acesso negado: 'x' e private de … (so acessivel de dentro da classe)`) — a
regra é **imposta pela VM**, não é só convenção.

```ps
Entity Conta {
    private saldo: int
    public funct ver(self) {
        return self.saldo
    }
    private funct log(self) {
        return "..."
    }
}

c = Conta(100)
post(c.ver())            # 100  (acessa saldo de dentro)
post(c.saldo)            # ERRO — saldo é private
```

`public` é o padrão (tudo é público se não disser nada); a palavra existe para
deixar a intenção explícita. Também há `private class`/`private Entity` (a
classe não é exportada).

### 7.6.1. Declarar o campo dentro do construtor

O campo pode nascer no `__init__`, com tipo e visibilidade, na forma
`<visibilidade> <tipo> <nome> = <valor>`:

```ps
private Class Pagamento {
    public funct __init__(self, nome, doc) {
        private str name = nome
        private int cpf  = doc
    }
    public funct mostra(self) {
        return self.name + "/" + str(self.cpf)
    }
}

p = Pagamento("ana", 123)
post(p.mostra())         # ana/123
post(p.name)             # ERRO — name é private
```

- É **campo do objeto**, não variável local: escreve em `self.<nome>`, e o
  `private` é registrado na classe, então a VM barra o acesso de fora
  exatamente como no campo declarado no corpo.
- O **tipo é conferido igual ao da variável** (seção 4.1): `private int n = 5.9`
  é `AttributedValueError`. Vale para os escalares (`str`, `int`, `flo`,
  `bool`, `char`) e também para `list` e `json` — `private list l = 5` é
  `AttributedValueError: variável l esperava list`. O que guarda sem reclamar é
  o campo cujo tipo é uma **Entity**: ali não há conferência.
- Só vale **dentro de uma funct de Entity que recebe `self`** — em `static`
  ou numa funct solta não há objeto para o campo pertencer, e é erro de
  sintaxe, não silêncio.
- A ordem `nome: tipo` é a do **corpo da classe**; dentro da funct use
  `tipo nome`. Escrever `private name: str = nome` dentro da funct é erro, e a
  mensagem diz o conserto.

---

## 7.7. Resumo

- `Entity Nome {` (sem parênteses; herança: `Entity Nome(Pai) {`);
  `class`/`Class` são sinônimos.
- Campos `nome: tipo` (ou `tipo nome`) sintetizam o construtor (1 arg por campo,
  na ordem); padrão torna opcional; campo criado no método via `self.x = …`.
- Campo declarado no construtor: `private <tipo> <nome> = <valor>` (7.6.1).
- Construtor próprio: `funct __init__(self, …)`.
- Métodos têm `self` como 1º parâmetro; `static` não tem `self` e é chamado na
  Entity.
- Herança (inclusive múltipla) compartilha métodos; filho com campos gera o
  próprio construtor; `base().__init__(...)` chama o construtor do pai, e
  `base(Pai).metodo()` / `base(Pai).campo` alcançam o resto do pai (7.5.2).
- `private` é imposto pela VM; `public` é o padrão.
