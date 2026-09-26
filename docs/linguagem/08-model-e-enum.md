# Referência da Linguagem — 8. `model` e `enum`

Duas declarações pequenas e específicas: **`model`**, um *esquema* para validar
a forma de um dict, e **`enum`**, um conjunto de constantes inteiras nomeadas.
Nenhuma das duas cria instâncias como a `Entity` (seção 7) — são descritores.

Verificado na VM.

---

## 8.1. `model` — esquema de validação de dict

Um `model` descreve os campos que um dict deve ter e de que tipo. Depois, o
operador `==` valida um dict contra o model. `private model M() {` no topo
do arquivo é o mesmo model, só que não sai pelo import (seção
[9.2](09-imports.md)).

```ps
model Usuario() {
    nome: str
    idade: int
}

post({ "nome": "ana", "idade": 30 } == Usuario)     # True
post({ "nome": "ana", "idade": "x" } == Usuario)    # False (idade não é int)
post({ "nome": "ana" } == Usuario)                  # False (falta idade)
```

Sintaxe:

- **`model Nome() { … }`** — os parênteses (vazios) e as chaves são
  obrigatórios.
- Cada campo é `nome: tipo`, **um por linha** (sem vírgula entre eles).
- O tipo é **`str`**, **`int`**, **`flo`**, **`bool`**, **`list`**, **`dict`**
  ou o **nome de outro `model`** (estrutura aninhada, 8.1.3).
- Depois do tipo, entre parênteses, os **parâmetros** que validam o dado
  (8.1.1 e 8.1.2): `nome: str(length=20, regex="^[a-z]+$")`. Vírgula entre
  eles, qualquer ordem, valores sempre literais.

### 8.1.1. Limite de comprimento — `length`

Um campo `str`, `int` ou `list` pode fixar um **máximo** com `tipo(length=N)`:
caracteres na string, dígitos no inteiro, itens na lista.

```ps
model Documento() {
    cpf: str(length=11)
}

post({ "cpf": "12345678901" } == Documento)   # True  (11 caracteres)
post({ "cpf": "123" } == Documento)           # True  (menos que 11 — ok)
post({ "cpf": "123456789012" } == Documento)  # False (12 > 11)
```

`length` é um **máximo**, não um valor exato: strings mais curtas passam.

### 8.1.2. Restrições: o que cada parâmetro confere

O model valida o **dado**, não só a forma. Cada parâmetro vale em certos
tipos; usá-lo fora deles é erro antes de rodar, na linha do campo.

| parâmetro | vale em | confere |
|---|---|---|
| `length=N` | str, int, list | máximo de caracteres / dígitos / itens |
| `regex="…"` | str | o valor **inteiro** casa o padrão (o mesmo `regex.fullmatch`); padrão inválido é erro na compilação |
| `in=[…]` | str, int, flo, bool | o valor é **um dos** da lista |
| `not_in=[…]` | str, int, flo, bool | o valor **não é nenhum** da lista (valores proibidos) |
| `min=N`, `max=N` | int, flo | faixa, inclusiva |
| `optional=true` | qualquer | o campo pode **faltar** ou vir `null`; presente, o tipo e o resto valem |
| `of=T` | list | **cada item** é do tipo `T` (`str`, `int`, `flo`, `bool`, `dict` ou um model) |

```ps
model Usuario() {
    nome:    str(length=20, regex="^[A-Za-z ]+$")
    email:   str(regex="^[^@]+@[^@]+\\.[^@]+$")
    idade:   int(min=18, max=120)
    papel:   str(in=["admin", "user"])
    status:  str(not_in=["banido"])
    apelido: str(optional=true)
}

post({ "nome": "ana", "email": "a@b.co", "idade": 30, "papel": "user", "status": "ativo" } == Usuario)   # True
post({ "nome": "ana", "email": "a@b.co", "idade": 12, "papel": "user", "status": "ativo" } == Usuario)   # False (idade < 18)
post({ "nome": "ana", "email": "a@b.co", "idade": 30, "papel": "dono", "status": "ativo" } == Usuario)   # False (papel fora de in)
```

Os valores dos parâmetros são **literais** — número (com sinal), texto,
`true`/`false`, lista de literais do tipo do campo. `in=[x]` com uma variável,
`min=5, max=1`, `regex` num `int` ou `of` fora de `list` são erros de
compilação, com a frase dizendo o campo.

### 8.1.3. Estrutura aninhada e listas

O tipo de um campo pode ser **outro model**: o valor tem que ser um dict que
passa naquele model, recursivamente. Uma `list` com `of=` confere item a
item — `of` aceita um tipo ou um model. `dict` sem mais nada é um objeto
JSON livre.

```ps
model Endereco() {
    rua: str(length=80)
    cep: str(regex="^[0-9]{5}-[0-9]{3}$")
}

model Cadastro() {
    endereco: Endereco               # dict que passa em Endereco
    fones:    list(of=Endereco)      # lista de dicts, cada um um Endereco
    tags:     list(of=str, length=5) # até 5 strings
    extras:   dict                   # qualquer objeto
}

post({ "endereco": {"rua": "x", "cep": "12345-678"}, "fones": [], "tags": ["a"], "extras": {} } == Cadastro)   # True
post({ "endereco": {"rua": "x", "cep": "1"},         "fones": [], "tags": ["a"], "extras": {} } == Cadastro)   # False (cep de dentro)
```

O model usado como tipo pode estar declarado **depois** no arquivo, ou vir de
um `import`.

### 8.1.4. Regras da validação

`dict == Model` (nas duas ordens — `Model == dict` também) é `True` quando:

- **todo campo declarado** está presente no dict (a não ser que seja
  `optional=true`), e
- cada um tem o **tipo declarado** e passa nos parâmetros dele (8.1.2 e
  8.1.3).

Chaves **a mais** no dict são ignoradas (não invalidam):

```ps
model P() { x: int }
post({ "x": 1, "extra": 9 } == P)     # True — 'extra' é ignorado
```

Faltar um campo, um campo com o tipo errado, longo demais, fora do padrão,
fora da faixa ou com valor proibido — tudo torna a comparação `False`. O
`model` não levanta erro na validação — ele responde `True`/`False`, então
cabe direto num `if`. Quem diz **qual campo e por quê** é a rota do jinker
com `model=` ([route.md](../jinker/route/route.md)): o 422 traz `campo
'idade': no minimo 18, veio 12`, e no aninhado o caminho — `campo
'endereco.cep': …`, `campo 'tags[2]': …`.

---

## 8.2. `enum` — constantes inteiras nomeadas

Um `enum` agrupa constantes sob um nome. Cada membro é, no fundo, um **inteiro**.
`private enum E {` no topo do arquivo não sai pelo import (seção
[9.2](09-imports.md)).

```ps
enum Cor {
    RED
    GREEN
    BLUE
}

post(Cor.RED, Cor.GREEN, Cor.BLUE)    # 0 1 2
```

Sintaxe:

- **`enum Nome { … }`** — **sem** parênteses (ao contrário do `model`); as chaves
  são obrigatórias.
- Os membros são separados por **vírgula ou quebra de linha** (ambas valem).

### 8.2.1. Numeração

Sem valor explícito, os membros são numerados **a partir de 0**. Um valor
explícito (`= n`) fixa aquele membro; os seguintes **continuam a contar a partir
dele**:

```ps
enum Status {
    OK = 200,
    NOT_FOUND = 404
}
post(Status.OK, Status.NOT_FOUND)     # 200 404

enum E {
    A = 10,
    B,
    C
}
post(E.A, E.B, E.C)                   # 10 11 12   (B e C continuam de 10)
```

### 8.2.2. Uso

- Acesse um membro por `Nome.MEMBRO`.
- O valor **é um inteiro** de verdade: `type(Cor.RED)` é `"int"` e
  `Cor.RED == 0` é `True` — dá pra comparar e usar em contas normalmente.
- Acessar um membro inexistente é erro:
  `AttributeError: type object 'Cor' has no attribute 'AZUL'`.

---

## 8.3. Resumo

- **`model Nome() { campo: tipo(…) … }`** (parênteses + chaves; campos por
  linha; tipos `str`/`int`/`flo`/`bool`/`list`/`dict` ou outro model). Os
  parâmetros validam o dado: `length`, `regex`, `in`, `not_in`, `min`, `max`,
  `optional`, `of`. Valida um dict com `==` (True/False): campos declarados
  presentes (salvo `optional`), com o tipo certo e dentro das restrições,
  recursivo no aninhado; extras ignorados.
- **`enum Nome { A, B … }`** (sem parênteses; chaves; vírgula ou linha).
  Constantes **inteiras** a partir de `0` (ou explícitas, com auto-continuação);
  `Nome.MEMBRO` devolve o inteiro.
