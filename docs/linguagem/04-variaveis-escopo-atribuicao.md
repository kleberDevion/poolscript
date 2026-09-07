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
  é o do valor, e pode mudar em outra atribuição.
- **Tipada** (`Tipo nome = valor`): o valor é checado e, quando seguro, coagido
  para o tipo declarado, seguindo a matriz de coerção da seção 2.6
  (`int x = "7"` vira `7`; `int x = 5.0` é erro). O tipo fica **na
  variável**: toda escrita seguinte é conferida igual — ver 4.6.4. Os tipos
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

obj.campo = 10       # membro de Entity (self.x dentro dela)

d = { "nome": "ana" }
d["idade"] = 30      # chave de dict por colchete
d.cidade = "SP"      # chave de dict por atributo (equivalente; ver seção de dict)
```

Não há **atribuição por fatia**: `l[1:3] = [...]` é erro de sintaxe. Para
substituir um trecho, monte a lista nova.

---

## 4.5. Desempacotamento (unpacking)

Vários alvos de uma vez, no estilo Python. O lado direito é distribuído pelos
alvos à esquerda.

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
lista[c], lista[c + 1] = lista[c + 1], lista[c]

d = {}
d["a"], d["b"] = 1, 2       # {'a': 1, 'b': 2}

o.x, o.y = 5, 6             # dois membros de uma Entity
```

- Um único alvo pode ter `*` (recebe uma lista com o que sobrar); só um `*` por
  nível.
- A quantidade de alvos tem que casar com a de valores (fora o `*`).
- A ordem é a do Python: o lado **direito inteiro** é avaliado primeiro, depois
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
if true {
    dentro = 5
}
post(dentro)        # NameError: name 'dentro' is not defined
```

O tropeço mais comum de quem vem do Python é criar a variável **nos dois ramos**
de um `if`/`else` e usá-la depois — na PoolScript isso é o mesmo erro. Declare
o nome **antes** do bloco (aí a atribuição dentro dele é write-through, 4.6.2):

```ps
novo = linha            # declarada FORA
if em_codigo {
    novo = troca(linha)
} else {
    novo = ajusta(linha)
}
post(novo)              # ok
```

### 4.6.2. Reatribuir uma variável de fora (write-through)

Se o nome **já existe** num escopo mais externo, a atribuição no bloco **altera
essa variável** (não cria uma nova):

```ps
x = 1
if true {
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
novo → cria, preso ao bloco.**

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

### 4.6.4. O tipo declarado é da variável — tipagem estática

`Tipo nome = valor` fixa o tipo da **variável**, não só do valor inicial: toda
escrita posterior nela é conferida pela mesma regra da criação (coerção onde a
matriz da seção 2.6 permite, erro onde não). Vale para reatribuição, `+=`,
`for each`, desempacotamento, escrita de dentro de uma funct (§4.7) e
closure.

```ps
str s = "oi"
s = 42              # AttributedValueError: variável s esperava str

int n = 1
n = "7"             # ok — coage: n vale 7
```

Uma variável criada **sem** tipo (`x = 1`) não tem essa restrição: `x = "a"`
depois dela vale. `Object` é o tipo de qualquer objeto — instância de classe,
servidor, conexão, arquivo — e é como se declara o que uma lib devolve:

```ps
Object app = Jinker(__name__)
object c = Conta("ana")      # `object` e `Object` são o mesmo tipo
Object s = "texto"           # AttributedValueError: variável s esperava Object
```

### 4.6.5. Funções e o escopo de módulo

- Variáveis no **topo do corpo de uma função** vivem enquanto a função roda
  (não são de bloco; sobrevivem entre os blocos internos dela).
- Variáveis no **topo do arquivo** são do **escopo de módulo** (globais).
- Dentro de uma função, atribuir a um nome que **não existe fora** cria uma
  variável **local** à função — ela some quando a função retorna:

```ps
funct f() {
    local = 5       # local à função
}
f()
post(local)         # NameError: name 'local' is not defined
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
(com `commit` antes de fechar) e **planilha** (salva ao sair). É o `with` do
Python: garante a liberação mesmo se o bloco estourar. Para um valor que não é
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
  e `if true { pass }` roda. Bloco **vazio** (`{ }`) também é aceito, em `if`,
  `while`, `for each` e no corpo de uma `funct`; a funct de corpo vazio devolve
  `Null`. O `pass` serve para deixar o lugar marcado à vista.
- Nomes seguem as regras léxicas da seção 1.4. A caixa MAIÚSCULA é
  **convenção** para libs/classes/tipos de erro, mas não é reserva: um nome
  maiúsculo pode ser variável comum (`MAX = 100`, `str NOME = "ana"`).

---

## 4.10. Resumo

- `nome = v` cria/atualiza; `Tipo nome = v` checa e coage em toda escrita (exige
  `=`); sem atribuição encadeada.
- Aumentadas (`+=` etc.) e `++`/`--` (só pós-fixado) herdam as regras da seção 3.
- Alvos compostos: `l[i]`, `obj.x`, `d.chave` (sem atribuição por fatia).
- Desempacotamento com `*` e aninhamento.
- **Escopo de bloco**: variável nova não vaza; reatribuir uma de fora atualiza;
  laço reinicia o corpo a cada volta. **`using` é exceção** (sobrevive).
- `global` cria/escreve no escopo de módulo de dentro de uma função.
