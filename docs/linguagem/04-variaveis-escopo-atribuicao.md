# Referência da Linguagem — 4. Variáveis, escopo e atribuição

Uma variável é um nome ligado a um valor. Esta seção especifica como criar esse
vínculo (declaração e atribuição), como alterá-lo (atribuição aumentada,
incremento, desempacotamento, atribuição a índice/membro) e — o ponto mais
sutil — **onde** cada nome existe e por quanto tempo (escopo).

Como no resto da linguagem, tudo aqui foi verificado rodando o mesmo fonte nos
VM e comparando a saída.

---

## 4.1. Declaração e atribuição

Há duas formas de introduzir uma variável:

```ps
nome = "ana"          # atribuição simples (sem tipo declarado)
str nome = "ana"      # declaração com tipo (checada/coagida em toda escrita)
```

- **Simples** (`nome = valor`): o nome recebe o valor e passa a existir; o tipo
  é o do valor, e fica **fixado**: outra atribuição com outro tipo é erro
  (4.6.4). `Null` não fixa.
- **Tipada** (`Tipo nome = valor`): o valor é **checado** contra o tipo
  declarado, e tem que já ser dele — nada é convertido (`int x = "7"` é erro,
  como `int x = 5.0`; converter é `int("7")`) — ver a seção 2.6. O tipo fica
  **na variável**: toda escrita seguinte é conferida igual — ver 4.6.4. Os tipos
  declaráveis: `str`, `int`, `flo`, `bool`, `char`, `list`, `dict`/`json`,
  `tup` e `Object` (qualquer objeto: instância, servidor, conexão, arquivo).
  `string`/`String`, `integer`/`Integer`, `tuple`/`Tuple` e
  `dictionary`/`Dictionary` são apelidos de `str`, `int`, `tup` e `dict`, com
  a mesma regra — e só valem na posição de tipo: não são palavras reservadas,
  `string` continua podendo ser variável ou nome de argumento.

A declaração tipada **exige** um valor: `int x` sozinho é erro de sintaxe
(`declaracao de variavel exige '='`).

Não existe **atribuição encadeada**: `a = b = 5` é erro de sintaxe. Atribua uma
por linha (ou use desempacotamento, 4.4).

### 4.1.1. `private` na variável de módulo

No **topo do arquivo**, as duas formas (e o desempacotamento, 4.5) aceitam
`private` na frente: a variável existe e funciona como qualquer outra dentro
do arquivo, mas **não sai pelo import** (`m.x`, `from m import x` e `*` — a
regra da seção [9.2](09-imports.md)). `public` é o default e é aceito.

```ps
private x = 1               # simples
private int n = 1           # tipada: o tipo é conferido como sem o modificador
private a, b = 1, 2         # os dois nomes
public y = 2                # igual a `y = 2`
```

O `private` é do **nome**: `x = 2` mais abaixo continua private. Fora do topo
(dentro de funct ou de bloco) ele não esconderia nada, e é erro (9.2).

---

## 4.2. Atribuição aumentada

As formas aumentadas leem o valor atual, aplicam o operador e regravam. Herdam
exatamente as regras de tipo do operador binário correspondente (seção 3).

| Forma | Equivale a |
|---|---|
| `x += e` | `x = x + e` |
| `x -= e` | `x = x - e` |
| `x *= e` | `x = x * e` |
| `x /= e` | `x = x / e` |
| `x %= e` | `x = x % e` |

```ps
n = 10
n += 5           # 15
s = "a"
s += "b"         # "ab"  (+ concatena str; ver 3.2.5)
l = [1]
l += [2]         # [1, 2]
```

Funcionam também sobre índice, membro e chave de dict (4.3):
`l[0] += 1`, `self.x += 1`, `d.chave += 1`.

---

## 4.3. Incremento e decremento

`++` e `--` são **pós-fixados** e alteram a variável no lugar. Como expressão,
devolvem o valor **anterior** (pós-incremento):

```ps
x = 5
x++              # agora x é 6
y = x++          # y = 6 (valor antes), x = 7
```

Não há forma **prefixa**: `++x` é erro de sintaxe.

---

## 4.4. Atribuição a alvos compostos

O lado esquerdo pode ser um elemento, um membro ou uma chave:

```ps
l = [1, 2, 3]
l[0] = 99            # índice de lista

Entity Ponto { campo: int }
obj = Ponto(0)
obj.campo = 10       # membro de Entity (self.x dentro dela)

d = { "nome": "ana" }
d["idade"] = 30      # chave de dict por colchete
d.cidade = "SP"      # chave de dict por atributo (equivalente; ver seção de dict)
```

Não há **atribuição por fatia**: `l[1:3] = [...]` é erro de sintaxe. Para
substituir um trecho, monte a lista nova.

---

## 4.5. Desempacotamento (unpacking)

Vários alvos de uma vez: o lado direito é distribuído pelos alvos à esquerda.

```ps
a, b = 1, 2                 # a=1, b=2
a, b = b, a                 # troca (swap) sem variável temporária

primeiro, *resto = [10, 20, 30, 40]   # primeiro=10, resto=[20,30,40]

(x, y), z = (1, 2), 3       # aninhado: x=1, y=2, z=3
```

**Alvo é o mesmo da seção 4.4**: nome, membro (`o.x`), índice (`l[i]`, `d[k]`)
e as cadeias deles (`o.d["k"]`). É o que faz a troca do bubble sort caber numa
linha:

```ps
lista = [3, 1, 2]
c = 0
lista[c], lista[c + 1] = lista[c + 1], lista[c]

d = {}
d["a"], d["b"] = 1, 2       # {'a': 1, 'b': 2}

Entity P {
    x: int
    y: int
}
o = P(0, 0)
o.x, o.y = 5, 6             # dois membros de uma Entity
```

- Um único alvo pode ter `*` (recebe uma lista com o que sobrar); só um `*` por
  nível.
- A quantidade de alvos tem que casar com a de valores (fora o `*`).
- A ordem: o lado **direito inteiro** é avaliado primeiro, depois
  a quantidade é conferida, e só então os alvos recebem — da esquerda pra
  direita, cada índice calculado na hora de escrever nele.
- Fatia continua fora (`l[0:2], x = ...`), pelo mesmo motivo de 4.4: `l[0:2] =`
  também não existe.

---

## 4.6. Escopo

O escopo é **léxico** e por **bloco**. Cada `if`,
`elif`, `else`, `while`, `for each`, `count each`, `try`/`catch`/`finally`,
`match`/`case` e o guard `if __name__ == "main"` abre um escopo próprio.

### 4.6.1. Variável de bloco não vaza

Um nome **criado** dentro de um bloco só existe ali. Depois do bloco, ele não é
mais visível:

```ps
if True {
    dentro = 5
}
post(dentro)        # NameError: name 'dentro' is not defined
```

O tropeço mais comum é criar a variável **nos dois ramos** de um `if`/`else` e
usá-la depois — ela não existe fora do bloco. Declare
o nome **antes** do bloco (aí a atribuição dentro dele é write-through, 4.6.2):

```ps
linha = "  texto  "
em_codigo = True
funct troca(s) { return s.upper() }
funct ajusta(s) { return s.strip() }

novo = linha            # declarada FORA
if em_codigo {
    novo = troca(linha)
} else {
    novo = ajusta(linha)
}
post(novo)              # ok
```

Vale também pra funct: a funct escrita **fora** do bloco não enxerga a
variável dele, nem enquanto o bloco roda — é erro antes de rodar, no nome. O
guard `if __name__ == "main"` é um bloco como os outros:

```ps
int funct mostra() {
    return porta
}
if __name__ == "main" {
    int porta = 8080
    post(mostra())
}
# NameError: name 'porta' is not defined: 'porta' só existe dentro de um bloco, e esta funct foi escrita fora dele (declare 'porta' antes do bloco)
```

A funct escrita **dentro** do bloco enxerga a variável dele e a leva junto,
mesmo depois que o bloco termina (4.6.3).

### 4.6.2. Reatribuir uma variável de fora (write-through)

Se o nome **já existe** num escopo mais externo, a atribuição no bloco **altera
essa variável** (não cria uma nova):

```ps
x = 1
if True {
    x = 2           # mesma x de fora
}
post(x)             # 2

total = 0
for each i in [1, 2, 3] {
    total = total + i   # acumula na total externa
}
post(total)             # 6
```

Ou seja: **atribuir a um nome existente lá fora → atualiza; atribuir a um nome
novo → cria, preso ao bloco.** Declarar de novo, com o mesmo tipo, um nome que
já existe (`int x = 2` com o `int x` de fora) também é a mesma variável.

De dentro de uma funct vale igual, com ou sem uma funct aninhada (ou lambda)
citando o nome — a escrita chega na de fora, e a de dentro lê a mesma:

```ps
int pontos = 0
list funct soma(list l) {
    pontos += 100
    return map(l, funct(int x) { return x + pontos })
}
post(soma([1, 2]), pontos)      # [101, 102] 100
```

### 4.6.3. Laço reinicia o corpo a cada volta

O corpo de um laço é um escopo **por iteração**: uma variável criada numa volta
não sobrevive para a próxima (a menos que exista fora do laço). A variável do
`for each` também não vaza depois do laço:

```ps
for each i in [1, 2, 3] {
    if i > 1 {
        post(prev)   # NameError na 2ª volta: name 'prev' is not defined
    }
    prev = i
}

for each i in [1, 2, 3] {
    ultimo = i
}
post(i)              # NameError: name 'i' is not defined (i não vaza do for)
```

`break` e `continue` respeitam isso: ao sair (ou reiniciar), o que nasceu no
laço é descartado.

A variável do `for each` (e a da compreensão) é **do laço**: uma de mesmo nome
que exista fora — no bloco, na funct, num parâmetro ou na funct que envolve
esta — continua com o valor dela depois do laço. Se a de fora foi declarada
com tipo (`int i = 0`), a do laço segue o tipo dela: `for each i in ["a"]` é
erro de tipo. O desempacotamento no cabeçalho (`for each a, b in pares`) é a
atribuição `a, b = ...` a cada volta: nome que já existe recebe os valores.

Por isso a funct (ou lambda) criada numa volta guarda a variável **daquela
volta** — a do laço e as que o corpo declara —, e cada volta tem a sua, como
o `let` do laço em JavaScript. Vale no `for each`, no `while`, na compreensão
e no topo do arquivo; o gerador, a tarefa (`async funct`) e o método de uma
classe declarados no laço também:

```ps
list fs = []
for each i in [1, 2, 3] {
    int dobro = i * 2
    fs.append(funct() { return i + dobro })
}
post(fs[0](), fs[1](), fs[2]())      # 3 6 9
```

A variável declarada **fora** do laço é uma só: as functs criadas nas voltas
veem o último valor dela.

### 4.6.4. O tipo é da variável — tipagem estática

`Tipo nome = valor` fixa o tipo da **variável**, não só do valor inicial: toda
escrita posterior nela é conferida pela mesma regra da criação (tipo exato, sem
conversão — seção 2.6). Vale para reatribuição, `+=`,
`for each`, desempacotamento, escrita de dentro de uma funct (§4.7) e
closure. A conferência é **antes de rodar** quando o tipo do valor é
conhecido, e rodando quando não é (`d["k"]`, `json.parse`) — o programa com
erro conhecido nem começa.

```ps
str s = "oi"
s = 42              # AttributedValueError: variável s esperava str, recebeu int

int n = 1
n = "7"             # AttributedValueError — "7" é str, não int; escreva int("7")
```

Uma variável criada **sem** tipo (`x = 1`) tem o tipo **fixado na primeira
atribuição**, como o `var` do Java: o tipo é o do primeiro valor, e outro tipo
depois é erro. `Null` não fixa nem conflita — a variável pode começar vazia
(`x = Null`), receber o valor depois, e voltar a `Null`.

```ps
x = 1
x = "a"             # AttributedValueError: variável x é int (tipo fixado na primeira atribuição), recebeu str
```

Declarar o mesmo nome de novo com outro tipo também é erro (`int y = 1` e,
depois, `long y = 2`). `Object` é o tipo de qualquer objeto — instância de
classe, servidor, conexão, arquivo — e é como se declara o que uma lib devolve:

```ps
from jinker import Jinker
Entity Conta { nome: str }

Object app = Jinker(__name__)
object c = Conta("ana")      # `object` e `Object` são o mesmo tipo
Object s = "texto"           # AttributedValueError: variável s esperava Object
```

### 4.6.5. Funções e o escopo de módulo

- Variáveis no **topo do corpo de uma função** vivem enquanto a função roda
  (não são de bloco; sobrevivem entre os blocos internos dela).
- Variáveis no **topo do arquivo** são do **escopo de módulo** (globais).
- Uma funct enxerga o topo do arquivo — o que está fora de bloco, declarado
  antes ou depois dela — e os blocos em que ela foi escrita. O nome que só
  nasce num bloco que não a envolve é do bloco (4.6.1).
- Dentro de uma função, atribuir a um nome que **não existe fora** cria uma
  variável **local** à função — ela some quando a função retorna:

```ps
funct f() {
    local = 5       # local à função
}
f()
post(local)         # NameError: name 'local' is not defined
```

- Uma classe (ou enum, ou model) **declarada dentro de uma funct** é um nome
  dela, como uma variável: a funct aninhada e a lambda escritas ali a
  enxergam. E os **métodos** dessa classe são código escrito dentro da funct:
  enxergam os nomes dela pelo mesmo escopo léxico, a própria classe inclusive
  (é por ela que o campo `static` solto chega no valor, 7.4.1).

```ps
int funct usa() {
    int k = 5
    class Local {
        static int n = 1
        static int funct le() {
            return k + n        # o `k` da funct e o static da classe
        }
    }
    return Local.le()
}
post(usa())                     # 6
```

---

## 4.7. `global` — escrever no escopo de módulo

Dentro de uma função, `global nome` faz esse nome se referir à variável de
**módulo**, em vez de criar uma local. É como se cria ou altera uma global de
dentro de uma função:

```ps
contador = 0

funct bump() {
    global contador
    contador = contador + 1
}

bump()
bump()
post(contador)      # 2
```

Neste exemplo o `global` é a **intenção explícita**, não o que faz funcionar:
reatribuir uma global que já existe funciona sem ele, pelo write-through de
4.6.2 — sem a linha `global contador`, o programa acima imprime `2` do mesmo
jeito. O `global` é obrigatório para **criar** uma global de dentro da funct:
sem ele, `nova = 1` dentro da funct e `post(nova)` fora dá
`NameError: name 'nova' is not defined`.

---

## 4.8. `using` — fechamento automático de recurso

`using <expr> as <nome> { … }` avalia a expressão, liga ao nome e roda o bloco; ao
terminar (**por saída normal ou por erro**) ele **fecha o recurso**, se for um
dos tipos que a linguagem sabe fechar — **arquivo**, **conexão de banco**
(com `commit` antes de fechar), **planilha** (salva ao sair), **socket** e
**processo** do `os.run(..., capture="live")` (fecha os três canais; o
processo segue vivo, e quem quer esperar por ele chama `wait()` dentro do
bloco). Garante a liberação mesmo se o bloco estourar. Para um valor que não é
um desses recursos, o `using` só executa o bloco (não há o que fechar).

```ps
using conexao as db {        # db é fechada/comitada ao sair, mesmo com erro
    # ... usa db aqui dentro ...
    resultado = 42
}
post(resultado)             # resultado ainda existe aqui (ver nota abaixo)
```

> **`using` é a exceção ao escopo de bloco.** Diferente dos outros blocos, a
> variável do `using` (`db`) e as variáveis atribuídas no corpo (`resultado`)
> **sobrevivem** depois dele — é deliberado, pra você abrir o recurso, extrair
> o dado e seguir usando-o já com o recurso fechado. O que o `using` garante é
> o fechamento do recurso, não o descarte das variáveis.

(As bibliotecas de arquivo e de banco — e o que exatamente cada `close`
faz — são detalhadas na parte de bibliotecas.)

---

## 4.9. Notas

- **`pass` existe** e é o "não faça nada" (seção 5.4.1) — é palavra reservada,
  e `if True { pass }` roda. Bloco **vazio** (`{ }`) também é aceito, em `if`,
  `while`, `for each` e no corpo de uma `funct`; a funct de corpo vazio devolve
  `Null`. O `pass` serve para deixar o lugar marcado à vista.
- Nome lido que não existe é `NameError` **antes de rodar**, e a frase sugere
  o nome parecido que existe no escopo, no arquivo ou entre os que a
  linguagem liga sozinha: `valor = 1` e depois `vlaor` dá
  `NameError: name 'vlaor' is not defined. Did you mean: 'valor'?`.
  Parecido é até uma letra trocada, posta ou tirada quando um dos dois nomes
  tem até 4 letras, até três quando os dois são maiores (duas letras vizinhas
  invertidas contam meia), e sempre menos da metade das letras do mais curto:
  `valor` não vira `False`, que troca três de cinco. Sem nada parecido, a
  frase fica sem dica.
- Dentro de um método, o nome solto que é membro de instância da classe
  (campo, método, `self.x` gravado, os dos pais) só se alcança por `self`, e
  a dica diz isso: `post(saldo)` dá
  `NameError: name 'saldo' is not defined. Did you mean: 'self.saldo'?`.
  Vale também na funct e na lambda escritas dentro do método. O membro
  `static` o nome solto alcança (seção 7).
- Nomes seguem as regras léxicas da seção 1.4. A caixa MAIÚSCULA é
  **convenção** para libs/classes/tipos de erro, mas não é reserva: um nome
  maiúsculo pode ser variável comum (`MAX = 100`, `str NOME = "ana"`).

---

## 4.10. Resumo

- `nome = v` cria/atualiza; `Tipo nome = v` checa o tipo — e **não converte** em toda escrita (exige
  `=`); sem atribuição encadeada.
- Aumentadas (`+=` etc.) e `++`/`--` (só pós-fixado) herdam as regras da seção 3.
- Alvos compostos: `l[i]`, `obj.x`, `d.chave` (sem atribuição por fatia).
- Desempacotamento com `*` e aninhamento.
- **Escopo de bloco**: variável nova não vaza; reatribuir uma de fora atualiza;
  laço reinicia o corpo a cada volta. **`using` é exceção** (sobrevive).
- `global` cria/escreve no escopo de módulo de dentro de uma função.
