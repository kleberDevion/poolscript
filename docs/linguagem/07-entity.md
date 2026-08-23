# Referência da Linguagem — 7. Entity (classes e objetos)

`Entity` é a construção de orientação a objetos da PoolScript: define um **tipo**
com campos e métodos, do qual se criam **instâncias**. Esta seção cobre a
declaração, os campos e o construtor (sintetizado ou próprio), os métodos e o
`self`, os métodos `@static`, a herança com `base`, e o encapsulamento
`private`/`public`.

Tudo verificado nos dois motores.

---

## 7.1. Declaração

![exemplo 1](../assets/linguagem__07-entity_ex1.png)

<details><summary>código</summary>

```ps
Entity Usuario():
    nome: str
    idade: int
```

</details>

- O nome da Entity começa com **maiúscula** (é `IDENT_UPPER`, seção 1.4).
- Os **parênteses são obrigatórios**: `Entity Usuario:` é erro; use
  `Entity Usuario()`. Entre eles vão as superclasses (7.6), ou nada.
- **`class` e `Class` são sinônimos de `Entity`** — mesma semântica.

![exemplo 2](../assets/linguagem__07-entity_ex2.png)

<details><summary>código</summary>

```ps
class Ponto():          // idêntico a Entity Ponto()
    x: int
    y: int
```

</details>

---

## 7.2. Campos e construtor sintetizado

Campos são declarados com **tipo** (`nome: tipo`). A partir deles a linguagem
**sintetiza um construtor** (`__init__`) que recebe um argumento por campo, na
ordem declarada:

![exemplo 3](../assets/linguagem__07-entity_ex3.png)

<details><summary>código</summary>

```ps
Entity Usuario():
    nome: str
    idade: int

u = Usuario("ana", 30)
post(u.nome, u.idade)      // ana 30
```

</details>

- **Valor padrão** num campo torna o argumento opcional:

  ![exemplo 4](../assets/linguagem__07-entity_ex4.png)

<details><summary>código</summary>

```ps
  Entity Config():
      host: str = "localhost"
      porta: int = 8080

  c = Config()              // usa os padrões
  post(c.host, c.porta)     // localhost 8080
  ```

</details>

- **Campos dinâmicos:** um método pode criar um campo não declarado com
  `self.x = ...` — ele passa a existir na instância:

  ![exemplo 5](../assets/linguagem__07-entity_ex5.png)

<details><summary>código</summary>

```ps
  Entity Bolsa():
      action guarda(self, item):
          self.conteudo = item
  ```

</details>

### 7.2.1. Construtor próprio — `action __init__`

Para um construtor com lógica própria (validação, campos derivados), defina
`action __init__(self, …)`. Isso **substitui** o construtor sintetizado:

![exemplo 6](../assets/linguagem__07-entity_ex6.png)

<details><summary>código</summary>

```ps
Entity Retangulo():
    action __init__(self, largura, altura):
        self.largura = largura
        self.altura  = altura
        self.area    = largura * altura

r = Retangulo(3, 4)
post(r.area)              // 12
```

</details>

---

## 7.3. Métodos e `self`

Um método é uma `action` cujo **primeiro parâmetro é `self`** (a instância).
Sem `self`, é erro (a não ser que seja `@static`, 7.4).

![exemplo 7](../assets/linguagem__07-entity_ex7.png)

<details><summary>código</summary>

```ps
Entity Contador():
    valor: int
    action inc(self):
        self.valor += 1
        return self.valor
    action zera(self):
        self.valor = 0

c = Contador(0)
post(c.inc(), c.inc())   // 1 2
```

</details>

Dentro de um método, `self.campo` acessa/atribui campos e `self.outro(...)`
chama outros métodos da instância.

---

## 7.4. Métodos `@static`

Prefixado com `@static`, o método **não recebe `self`** e é chamado **na
própria Entity** (não numa instância):

![exemplo 8](../assets/linguagem__07-entity_ex8.png)

<details><summary>código</summary>

```ps
Entity Mat():
    @static
    action soma(a, b):
        return a + b

post(Mat.soma(2, 3))     // 5
```

</details>

Chamar um `@static` por uma instância (`m.soma(...)`) é erro — ele pertence ao
tipo, não ao objeto. (Ver também a nota sobre `@static` na seção de
decoradores.)

---

## 7.5. Herança

Uma Entity pode herdar de uma ou mais outras, listadas entre os parênteses. Os
**métodos** do(s) pai(s) ficam disponíveis; um método redefinido no filho
**sobrescreve** o do pai.

![exemplo 9](../assets/linguagem__07-entity_ex9.png)

<details><summary>código</summary>

```ps
Entity Animal():
    nome: str
    action fala(self):
        return "..."

Entity Cao(Animal):
    action fala(self):
        return "au"      // sobrescreve

c = Cao("rex")
post(c.nome, c.fala())   // rex au

// herança múltipla:
Entity C(A, B):          // herda métodos de A e de B
    x: int
```

</details>

### 7.5.1. Construtor e herança

- Um filho que **não declara campos nem `__init__`** herda o construtor do pai
  (recebe os campos do pai).
- Um filho que **declara campos próprios** sintetiza o seu próprio construtor,
  só com os **campos dele** — os do pai não entram automaticamente. Para incluí-los,
  redeclare-os no filho ou escreva um `__init__` próprio.

### 7.5.2. `base(...)` — construtor do pai

Dentro de um `__init__` próprio, `base(args)` chama o **construtor da
superclasse**:

![exemplo 10](../assets/linguagem__07-entity_ex10.png)

<details><summary>código</summary>

```ps
Entity A():
    action __init__(self, x):
        self.x = x

Entity B(A):
    action __init__(self, x, y):
        base(x)          // roda o __init__ de A
        self.y = y

b = B(1, 2)
post(b.x, b.y)           // 1 2
```

</details>

`base` serve para o **construtor** do pai; não é a forma de chamar um método
qualquer da superclasse.

---

## 7.6. Encapsulamento — `private` e `public`

Um campo ou método marcado **`private`** só é acessível **de dentro da própria
Entity**. Acessá-lo de fora é erro (`acesso negado: 'x' é private de …`) — a
regra é **imposta pelos dois motores**, não é só convenção.

![exemplo 11](../assets/linguagem__07-entity_ex11.png)

<details><summary>código</summary>

```ps
Entity Conta():
    private saldo: int
    public action ver(self):
        return self.saldo
    private action log(self):
        return "..."

c = Conta(100)
post(c.ver())            // 100  (acessa saldo de dentro)
post(c.saldo)            // ERRO — saldo é private
```

</details>

`public` é o padrão (tudo é público se não disser nada); a palavra existe para
deixar a intenção explícita. Também há `private class`/`private Entity` (a
classe não é exportada).

---

## 7.7. Resumo

- `Entity Nome()` (parênteses obrigatórios); `class`/`Class` são sinônimos.
- Campos `nome: tipo` sintetizam o construtor (1 arg por campo, na ordem);
  padrão torna opcional; campos dinâmicos via `self.x = …`.
- Construtor próprio: `action __init__(self, …)`.
- Métodos têm `self` como 1º parâmetro; `@static` não tem `self` e é chamado na
  Entity.
- Herança (inclusive múltipla) compartilha métodos; filho com campos gera o
  próprio construtor; `base(...)` chama o construtor do pai.
- `private` é imposto pelos dois motores; `public` é o padrão.
