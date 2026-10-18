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
- **Um nome, uma declaração por escopo:** declarar de novo uma classe, model
  ou enum com um nome que outra declaração ainda liga no mesmo escopo (outra
  classe, model, enum ou funct) é erro antes de rodar, no nome da segunda:
  `SyntaxError: 'C' ja foi declarado na linha 1 (class): um nome, uma
  declaracao no mesmo escopo`. Blocos irmãos são escopos diferentes: o `if` e
  o `else` podem declarar cada um a sua `class C`.

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

- **O valor padrão nasce sempre**, antes do construtor (o sintetizado ou o
  próprio, 7.2.1), uma vez por objeto, na ordem declarada — os padrões dos
  pais antes dos da filha (7.5.1). O construtor sintetizado só grava o
  argumento que recebeu: a expressão do padrão não roda de novo.

  ```ps
  int cont = 0
  int funct proximo() {
      cont += 1
      return cont
  }
  class Pedido {
      str item
      int id = proximo()
  }
  Pedido a = Pedido("x")
  Pedido b = Pedido("y", 50)
  post(a.id, b.id, cont)    # 1 50 2
  ```

  Numa classe declarada dentro de uma funct, o padrão enxerga a funct, como
  os métodos (`int v = base0 * 2` lê o parâmetro `base0` dela).

- **Membro repetido** no corpo (dois campos, dois métodos, ou um campo e um
  método com o mesmo nome) é erro antes de rodar, no segundo:
  `SyntaxError: membro 'x' repetido na classe C (o primeiro esta na linha 2)`.

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
`funct __init__(self, …)`. Isso **substitui** o construtor sintetizado (os
campos com valor padrão nascem antes dele mesmo assim, 7.2):

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
(`TypeError: __init__() should return Null, not 'future'`).

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
        mapp(debug=True, port=3003, host="127.0.0.1")
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
    static int funct soma() {
        return a + b + c + d         # de dentro da classe o private vale
    }
}
post(Config.a, Config.soma())        # 1 10
```

Cada modificador vale uma vez (`static static int a` e `public private int
b` são erro), e dois membros na mesma linha precisam de `;` entre eles, como
dois comandos: `class C { int x = 1; int y = 2 }`.

Sem inicializador o campo nasce `Null`.

Quem enxerga o campo, e como:

| De onde | Como se escreve |
|---|---|
| de fora | `App.mapp` (o `private static` não: de fora é `acesso negado`, 7.6) |
| dentro da classe — corpo (decoradores), método `static` ou método comum | `mapp`, o nome solto (também `self.mapp` e `App.mapp` num método comum) |
| numa classe filha | `mapp`, o nome solto, igual — o `static` público do pai é da filha (também `self.mapp` e `Pai.mapp`); o `private` do pai, não |

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

É **um só** valor, compartilhado, e tem **um dono**: a classe que o declara.
Toda grafia que o lê grava nele — `K.n = 5`, `self.n += 1` num método,
`obj.n = 5` de fora, `Filha.n = 5` pela filha —, e toda instância (da classe
e das filhas) lê o mesmo valor:

```ps
class Conta {
    static int total = 0
    str nome
    funct __init__(self, str nome) {
        self.nome = nome
        self.total += 1              # conta na classe, não numa cópia por objeto
    }
}
Conta a = Conta("a")
Conta b = Conta("b")
post(Conta.total, a.total, b.total)  # 2 2 2
```

A filha que **redeclara** o campo (`static int total = 0` também nela) tem o
dela, e a escrita pela filha vai nele.

O **tipo** escrito no campo `static` é conferido em toda escrita, como no campo
de instância: `K.n = "texto"` com `static int n` é `AttributedValueError: campo
n de K esperava int, recebeu str`, antes de rodar quando o tipo do valor se
sabe, rodando quando não (`K.n = l[0]`); o inicializador também. E `K.n` tem o
tipo do campo pra quem lê: `str s = K.n` é recusado antes de rodar. Só o
nascimento sem inicializador vale `Null`.

O campo `static` **não entra** no construtor sintetizado —
`class P { static int total = 0; str nome }` continua sendo `P("ana")`.

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

**A sobrescrita mantém o tipo de retorno.** Quem declara `p` como o pai
recebe de `p.m()` o tipo que o pai promete, então o método do filho que
sobrescreve um método com retorno declarado tem que declarar um retorno que
caiba nele (`Cao funct bicho` sobre `Animal funct bicho` vale). Trocar o tipo,
ou não declarar, é erro antes de rodar, no nome do método do filho:

```texto
TypeError: m() de Filha devolve str e sobrescreve m() de Pai, que devolve int
TypeError: m() de Filha nao declara o retorno e sobrescreve m() de Pai, que devolve str: declare str funct m
```

O método `private` do pai não é sobrescrito (o filho não o enxerga: o dele é
outro método), e o que um decorador geral pode trocar fica de fora.

### 7.5.1. Construtor e herança

Uma regra só decide o construtor que `Classe(...)` roda, a mesma da busca de
membro, e o `--check` confere a chamada contra ele antes de rodar:

- Um filho com **`__init__` próprio** usa o dele.
- Um filho que **declara campos próprios** (sem `__init__`) sintetiza o seu,
  só com os **campos dele** — os do pai não entram como argumento.
- Um filho **sem campos nem `__init__`** usa o construtor do primeiro pai que
  tem um (o próprio ou o sintetizado), na ordem dos pais; sem nenhum, a
  classe não recebe argumento (`Zero() takes no arguments (3 given)`).

Os **campos com valor** do pai nascem no filho sempre (7.2), com o
construtor que for. Os **sem valor** só o construtor sintetizado da classe
que os declara grava: com o do filho, ninguém os grava (a não ser o próprio
código: `self.x = ...`, `base(Pai).__init__(...)`), e lê-los é
`AttributeError` com a causa — antes de rodar, quando nada no arquivo pode
gravá-los:

```ps
class Pai {
    int x
}
class Filha(Pai) {
    str y = "oi"
}
Filha f = Filha()
post(f.x)
# AttributeError: 'Filha' object has no attribute 'x': o campo x de Pai foi
# declarado sem valor e nada o gravou (de um valor na declaracao ou grave self.x no __init__)
```

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
  só no `__init__`. Método `static` do pai sai sem ligar: `base().dobro(4)` é
  `Pai.dobro(4)`.
- **`.campo`** lê: campo `static` do pai, ou o campo da instância (é o mesmo
  objeto que `self.campo`). Escrever é pelo `self`: `base().x = 1` é erro
  (`atribua pelo self: self.x = ...`).
- **`private` do pai** continua invisível pro filho, também por `base()`:
  `acesso negado: 'x' e private de A`. Com `private funct __init__` no pai,
  `base().__init__()` do filho também é negado.
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

```texto
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
post(c.saldo)            # RuntimeError: acesso negado: 'saldo' e private de Conta
```

`public` é o padrão (tudo é público se não disser nada); a palavra existe para
deixar a intenção explícita. Também há `private class`/`private Entity` (a
classe não é exportada).

**De dentro** é o código **escrito no corpo da classe**: os métodos, e a lambda
e a funct aninhada escritas dentro deles (uma `map(self.itens, int funct(int
v) { return self.dobra(v) })` alcança o `private funct dobra`), o
inicializador de campo `static` e a expressão de decorador do corpo; também o
código de uma classe declarada dentro de um método dela. O wrapper de um
decorador declarado fora da classe é de fora (seção 14).

O `private` vale por **qualquer caminho** até o membro: pela instância
(`c.saldo`), pelo nome da classe (`Conta.taxa`, `Conta.calcula()` num
`private static`), pelo nome solto num método da filha e pelo `base()`.

Com herança, decide a **declaração que o nome alcança**: a primeira classe, na
ordem de busca de método (a própria, depois os pais da esquerda pra direita),
que declara o nome. Por isso:

- a filha pode declarar um membro **público** com o nome de um `private` do
  pai — ele é dela, e `f.x` lê o dela;
- o método do pai continua lendo o `private` dele numa instância da filha;
- a frase cita a classe que **declara** o private, não a do objeto:
  `acesso negado: 'segredo' e private de Pai`.

```ps
class Pai {
    private int segredo = 42
}
class Filha(Pai) {
    int funct le(self) {
        return self.segredo      # RuntimeError: acesso negado: 'segredo' e private de Pai
    }
}
```

`private funct __init__` impede **construir** de fora (o construtor private do
Java: fábrica, singleton). Dentro da classe, `C(...)` vale:

```ps
class C {
    int v = 0
    private funct __init__(self, int v) {
        self.v = v
    }
    static C funct cria(int v) {
        return C(v)              # de dentro: vale
    }
}
post(C.cria(4).v)                # 4
C c = C(1)                       # RuntimeError: acesso negado: '__init__' e private de C
```

O `--check` acusa o acesso negado **antes de rodar** quando o resultado é
certo: pelo nome da classe, e pela instância quando nenhuma classe do arquivo
que o valor pode ser libera o nome (uma filha que redeclara o nome como
público o libera rodando). O resto a VM barra rodando.

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
post(p.name)             # RuntimeError: acesso negado: 'name' e private de Pagamento
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
  `class`/`Class` são sinônimos; um nome, uma declaração por escopo.
- Campos `nome: tipo` (ou `tipo nome`) sintetizam o construtor (1 arg por campo,
  na ordem); padrão torna opcional e nasce sempre, antes do construtor, uma
  vez por objeto; campo criado no método via `self.x = …`; membro repetido é
  erro.
- Campo declarado no construtor: `private <tipo> <nome> = <valor>` (7.6.1).
- Construtor próprio: `funct __init__(self, …)`.
- Métodos têm `self` como 1º parâmetro; `static` não tem `self` e é chamado na
  Entity.
- Herança (inclusive múltipla) compartilha métodos; a sobrescrita mantém o tipo
  de retorno; filho com campos gera o próprio construtor (o campo sem valor do
  pai não nasce nele); `base().__init__(...)` chama o construtor do pai, e
  `base(Pai).metodo()` / `base(Pai).campo` alcançam o resto do pai (7.5.2).
- `obj is Classe` sobe pela herança; classe com classe é identidade (3.5).
- `private` é imposto pela VM (e pelo `--check` quando é certo) por qualquer
  caminho; "de dentro" é o código escrito no corpo da classe; `public` é o
  padrão (7.6).
- Campo `static` tem um dono, a classe que o declara: toda escrita
  (`K.n`, `self.n`, `obj.n`, `Filha.n`) grava nele, com o tipo conferido
  (7.4.1).
