# Referência da Linguagem — 7. Entity (classes e objetos)

`Entity` é a construção de orientação a objetos da PoolScript: define um **tipo**
com campos e métodos, do qual se criam **instâncias**. Esta seção cobre a
declaração, os campos e o construtor (sintetizado ou próprio), os métodos e o
`self`, os métodos `static`, a herança com `base`, e o encapsulamento
`private`/`public`.

Tudo verificado na VM.

---

## 7.1. Declaração

```ps
Entity Usuario() {
    nome: str
    idade: int
}
```

- O nome da Entity começa com **maiúscula** por convenção (é `IDENT_UPPER`,
  seção 1.4). O motor não exige: `Entity usuario()` compila e roda. A maiúscula
  é o que separa, à leitura, o tipo do valor.
- Os **parênteses são obrigatórios**: `Entity Usuario:` é erro; use
  `Entity Usuario()`. Entre eles vão as superclasses (7.6), ou nada.
- **`class` e `Class` são sinônimos de `Entity`** — mesma semântica.

```ps
class Ponto() {          # idêntico a Entity Ponto()
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
Entity Usuario() {
    nome: str
    idade: int
}

u = Usuario("ana", 30)
post(u.nome, u.idade)      # ana 30
```

```ps
Entity Usuario() {         # idêntico ao de cima
    str nome
    int idade
}
```

- **Valor padrão** num campo torna o argumento opcional:

  ```ps
  Entity Config() {
      host: str = "localhost"
      porta: int = 8080
  }

  c = Config()              # usa os padrões
  post(c.host, c.porta)     # localhost 8080
  ```

- **Campos criados no método:** um método pode criar um campo não declarado
  com `self.x = ...` — ele passa a existir na instância:

  ```ps
  Entity Bolsa() {
      funct guarda(self, item) {
          self.conteudo = item
      }
  }
  ```

### 7.2.1. Construtor próprio — `funct __init__`

Para um construtor com lógica própria (validação, campos derivados), defina
`funct __init__(self, …)`. Isso **substitui** o construtor sintetizado:

```ps
Entity Retangulo() {
    funct __init__(self, largura, altura) {
        self.largura = largura
        self.altura  = altura
        self.area    = largura * altura
    }
}

r = Retangulo(3, 4)
post(r.area)              # 12
```

---

## 7.3. Métodos e `self`

Um método é uma `funct` cujo **primeiro parâmetro é `self`** (a instância).
Sem `self`, é erro (a não ser que seja `static`, 7.4).

```ps
Entity Contador() {
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
Entity Mat() {
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
class App() {
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
class Config() {
    static object a = 1
    private static object b = 2
    private object static c = 3      # o `static` depois do tipo também vale
    object static private d = 4
}
post(Config.a, Config.d)             # 1 4
```

Sem inicializador o campo nasce `Null`.

Quem enxerga o campo, e como:

| De onde | Como se escreve |
|---|---|
| de fora | `App.mapp` |
| no corpo da classe (decoradores) e nos métodos `static` da **própria** classe | `mapp`, o nome solto |
| num método comum | `self.mapp` ou `App.mapp` |
| numa classe filha | `self.mapp` ou `Pai.mapp` — o nome solto cobre só os `static` da própria classe |

Um parâmetro ou variável local com o mesmo nome **ganha** do campo: `funct
m(self, x)` devolve o `x` recebido, não o `C.x`.

É **um só** valor, compartilhado: `K.n = K.n + 1` num método muda o que toda
instância lê em `self.n`. E ele **não entra** no construtor sintetizado —
`class P() { static int total = 0  str nome }` continua sendo `P("ana")`.

Por que existe: sem ele, `App.mapp` não existia, um método `static` não tinha
como enxergar o campo, e o decorador no corpo da classe rodava antes de haver
instância — o programa acima passava no `--check` e não fazia nada.

---

## 7.5. Herança

Uma Entity pode herdar de uma ou mais outras, listadas entre os parênteses. Os
**métodos** do(s) pai(s) ficam disponíveis; um método redefinido no filho
**sobrescreve** o do pai.

```ps
Entity Animal() {
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

### 7.5.2. `base(...)` — construtor do pai

Dentro de um `__init__` próprio, `base(args)` chama o **construtor da
superclasse**:

```ps
Entity A() {
    funct __init__(self, x) {
        self.x = x
    }
}

Entity B(A) {
    funct __init__(self, x, y) {
        base(x)          # roda o __init__ de A
        self.y = y
    }
}

b = B(1, 2)
post(b.x, b.y)           # 1 2
```

`base` serve para o **construtor** do pai; não é a forma de chamar um método
qualquer da superclasse.

---

## 7.6. Encapsulamento — `private` e `public`

Um campo ou método marcado **`private`** só é acessível **de dentro da própria
Entity**. Acessá-lo de fora é erro (`acesso negado: 'x' e private de … (so acessivel de dentro da classe)`) — a
regra é **imposta pela VM**, não é só convenção.

```ps
Entity Conta() {
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
private Class Pagamento() {
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

- `Entity Nome()` (parênteses obrigatórios); `class`/`Class` são sinônimos.
- Campos `nome: tipo` (ou `tipo nome`) sintetizam o construtor (1 arg por campo,
  na ordem); padrão torna opcional; campo criado no método via `self.x = …`.
- Campo declarado no construtor: `private <tipo> <nome> = <valor>` (7.6.1).
- Construtor próprio: `funct __init__(self, …)`.
- Métodos têm `self` como 1º parâmetro; `static` não tem `self` e é chamado na
  Entity.
- Herança (inclusive múltipla) compartilha métodos; filho com campos gera o
  próprio construtor; `base(...)` chama o construtor do pai.
- `private` é imposto pela VM; `public` é o padrão.
