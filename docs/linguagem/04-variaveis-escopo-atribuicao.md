# Referência da Linguagem — 4. Variáveis, escopo e atribuição

Uma variável é um nome ligado a um valor. Esta seção especifica como criar esse
vínculo (declaração e atribuição), como alterá-lo (atribuição aumentada,
incremento, desempacotamento, atribuição a índice/membro) e — o ponto mais
sutil — **onde** cada nome existe e por quanto tempo (escopo).

Como no resto da linguagem, tudo aqui foi verificado rodando o mesmo fonte nos
dois motores (interpretador e VM em C) e comparando a saída.

---

## 4.1. Declaração e atribuição

Há duas formas de introduzir uma variável:

![exemplo 1](../assets/linguagem__04-variaveis-escopo-atribuicao_ex1.png)

<details><summary>código</summary>

```ps
nome = "ana"          // atribuição simples (dinâmica)
str nome = "ana"      // declaração com tipo (checada/coagida na criação)
```

</details>

- **Simples** (`nome = valor`): o nome recebe o valor e passa a existir; o tipo
  é o do valor.
- **Tipada** (`Tipo nome = valor`): o valor é checado e, quando seguro, coagido
  para o tipo declarado, seguindo a matriz de coerção da seção 2.6
  (`int x = "7"` vira `7`; `int x = 5.0` é erro). A checagem vale **só na
  criação** — ver 4.5.4.

A declaração tipada **exige** um valor: `int x` sozinho é erro de sintaxe
(`declaração de variável exige '='`).

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

![exemplo 2](../assets/linguagem__04-variaveis-escopo-atribuicao_ex2.png)

<details><summary>código</summary>

```ps
n = 10
n += 5           // 15
s = "a"
s += "b"         // "ab"  (+ concatena str; ver 3.2.5)
l = [1]
l += [2]         // [1, 2]
```

</details>

Funcionam também sobre índice, membro e chave de dict (4.3):
`l[0] += 1`, `self.x += 1`, `d.chave += 1`.

---

## 4.3. Incremento e decremento

`++` e `--` são **pós-fixados** e alteram a variável no lugar. Como expressão,
devolvem o valor **anterior** (pós-incremento):

![exemplo 3](../assets/linguagem__04-variaveis-escopo-atribuicao_ex3.png)

<details><summary>código</summary>

```ps
x = 5
x++              // agora x é 6
y = x++          // y = 6 (valor antes), x = 7
```

</details>

Não há forma **prefixa**: `++x` é erro de sintaxe.

---

## 4.4. Atribuição a alvos compostos

O lado esquerdo pode ser um elemento, um membro ou uma chave:

![exemplo 4](../assets/linguagem__04-variaveis-escopo-atribuicao_ex4.png)

<details><summary>código</summary>

```ps
l = [1, 2, 3]
l[0] = 99            // índice de lista

obj.campo = 10       // membro de Entity (self.x dentro dela)

d = { "nome": "ana" }
d["idade"] = 30      // chave de dict por colchete
d.cidade = "SP"      // chave de dict por atributo (equivalente; ver seção de dict)
```

</details>

Não há **atribuição por fatia**: `l[1:3] = [...]` é erro de sintaxe. Para
substituir um trecho, monte a lista nova.

---

## 4.5. Desempacotamento (unpacking)

Vários alvos de uma vez, no estilo Python. O lado direito é distribuído pelos
nomes à esquerda.

![exemplo 5](../assets/linguagem__04-variaveis-escopo-atribuicao_ex5.png)

<details><summary>código</summary>

```ps
a, b = 1, 2                 // a=1, b=2
a, b = b, a                 // troca (swap) sem variável temporária

primeiro, *resto = [10, 20, 30, 40]   // primeiro=10, resto=[20,30,40]

(x, y), z = (1, 2), 3       // aninhado: x=1, y=2, z=3
```

</details>

- Um único alvo pode ter `*` (recebe uma lista com o que sobrar); só um `*` por
  nível.
- A quantidade de nomes tem que casar com a de valores (fora o `*`).

---

## 4.6. Escopo

O escopo é **léxico** e por **bloco** — igual ao interpretador. Cada `if`,
`elif`, `else`, `while`, `for each`, `count each`, `try`/`catch`/`finally`,
`match`/`case` e `run_selfwith_` abre um escopo próprio.

### 4.6.1. Variável de bloco não vaza

Um nome **criado** dentro de um bloco só existe ali. Depois do bloco, ele não é
mais visível:

![exemplo 6](../assets/linguagem__04-variaveis-escopo-atribuicao_ex6.png)

<details><summary>código</summary>

```ps
if true:
    dentro = 5
post(dentro)        // ERRO — variável não definida: dentro
```

</details>

### 4.6.2. Reatribuir uma variável de fora (write-through)

Se o nome **já existe** num escopo mais externo, a atribuição no bloco **altera
essa variável** (não cria uma nova):

![exemplo 7](../assets/linguagem__04-variaveis-escopo-atribuicao_ex7.png)

<details><summary>código</summary>

```ps
x = 1
if true:
    x = 2           // mesma x de fora
post(x)             // 2

total = 0
for each i in [1, 2, 3]:
    total = total + i   // acumula na total externa
post(total)             // 6
```

</details>

Ou seja: **atribuir a um nome existente lá fora → atualiza; atribuir a um nome
novo → cria, preso ao bloco.**

### 4.6.3. Laço reinicia o corpo a cada volta

O corpo de um laço é um escopo **por iteração**: uma variável criada numa volta
não sobrevive para a próxima (a menos que exista fora do laço). A variável do
`for each` também não vaza depois do laço:

![exemplo 8](../assets/linguagem__04-variaveis-escopo-atribuicao_ex8.png)

<details><summary>código</summary>

```ps
for each i in [1, 2, 3]:
    if i > 1:
        post(prev)   // ERRO na 2ª volta: prev da volta anterior não existe
    prev = i

for each i in [1, 2, 3]:
    ultimo = i
post(i)              // ERRO — i não existe fora do for
```

</details>

`break` e `continue` respeitam isso: ao sair (ou reiniciar), o que nasceu no
laço é descartado.

### 4.6.4. A checagem de tipo é só na declaração

Depois de criada, a variável é **dinâmica**: uma atribuição simples posterior
pode trocar o tipo à vontade. O `Tipo` na frente vale só no momento da criação.

![exemplo 9](../assets/linguagem__04-variaveis-escopo-atribuicao_ex9.png)

<details><summary>código</summary>

```ps
str s = "oi"
s = 42              // ok — agora s é o int 42
post(type(s))       // int
```

</details>

### 4.6.5. Funções e o escopo de módulo

- Variáveis no **topo do corpo de uma função** vivem enquanto a função roda
  (não são de bloco; sobrevivem entre os blocos internos dela).
- Variáveis no **topo do arquivo** são do **escopo de módulo** (globais).
- Dentro de uma função, atribuir a um nome que **não existe fora** cria uma
  variável **local** à função — ela some quando a função retorna:

![exemplo 10](../assets/linguagem__04-variaveis-escopo-atribuicao_ex10.png)

<details><summary>código</summary>

```ps
action f():
    local = 5       // local à função
f()
post(local)         // ERRO — local não existe aqui
```

</details>

---

## 4.7. `global` — escrever no escopo de módulo

Dentro de uma função, `global nome` faz esse nome se referir à variável de
**módulo**, em vez de criar uma local. É como se cria ou altera uma global de
dentro de uma função:

![exemplo 11](../assets/linguagem__04-variaveis-escopo-atribuicao_ex11.png)

<details><summary>código</summary>

```ps
contador = 0

action bump():
    global contador
    contador = contador + 1

bump()
bump()
post(contador)      // 2
```

</details>

Sem o `global`, `contador = ...` dentro de `bump` criaria uma local e a de fora
ficaria em `0`. (Apenas **reatribuir** uma global que já existe funciona sem
`global` — o write-through de 4.6.2 —; o `global` é necessário para **criar** a
global de dentro, ou deixar a intenção explícita.)

---

## 4.8. `using` — fechamento automático de recurso

`using <expr> as <nome>:` avalia a expressão, liga ao nome e roda o bloco; ao
terminar (**por saída normal ou por erro**) ele **fecha o recurso**, se for um
dos tipos que a linguagem sabe fechar — **arquivo**, **conexão de banco**
(com `commit` antes de fechar) e **planilha** (salva ao sair). É o `with` do
Python: garante a liberação mesmo se o bloco estourar. Para um valor que não é
um desses recursos, o `using` só executa o bloco (não há o que fechar).

![exemplo 12](../assets/linguagem__04-variaveis-escopo-atribuicao_ex12.png)

<details><summary>código</summary>

```ps
using conexao as db:        // db é fechada/comitada ao sair, mesmo com erro
    // ... usa db aqui dentro ...
    resultado = 42
post(resultado)             // resultado ainda existe aqui (ver nota abaixo)
```

</details>

> **`using` é a exceção ao escopo de bloco.** Diferente dos outros blocos, a
> variável do `using` (`db`) e as variáveis atribuídas no corpo (`resultado`)
> **sobrevivem** depois dele — é deliberado, pra você abrir o recurso, extrair
> o dado e seguir usando-o já com o recurso fechado. O que o `using` garante é
> o fechamento do recurso, não o descarte das variáveis.

(As bibliotecas de arquivo e de banco — e o que exatamente cada `close`
faz — são detalhadas na parte de bibliotecas.)

---

## 4.9. Notas

- **Não há `pass`.** Um bloco precisa de pelo menos um statement; não existe
  um "não faça nada" — use um comentário se quiser marcar o lugar, ou
  reestruture para não ter bloco vazio.
- Nomes seguem as regras léxicas da seção 1.4. A caixa MAIÚSCULA é
  **convenção** para libs/classes/tipos de erro, mas não é reserva: um nome
  maiúsculo pode ser variável comum (`MAX = 100`, `str NOME = "ana"`).

---

## 4.10. Resumo

- `nome = v` cria/atualiza; `Tipo nome = v` checa e coage na criação (exige
  `=`); sem atribuição encadeada.
- Aumentadas (`+=` etc.) e `++`/`--` (só pós-fixado) herdam as regras da seção 3.
- Alvos compostos: `l[i]`, `obj.x`, `d.chave` (sem atribuição por fatia).
- Desempacotamento com `*` e aninhamento.
- **Escopo de bloco**: variável nova não vaza; reatribuir uma de fora atualiza;
  laço reinicia o corpo a cada volta. **`using` é exceção** (sobrevive).
- `global` cria/escreve no escopo de módulo de dentro de uma função.
