# Referência da Linguagem — 8. `model` e `enum`

Duas declarações pequenas e específicas: **`model`**, um *esquema* para validar
a forma de um dict, e **`enum`**, um conjunto de constantes inteiras nomeadas.
Nenhuma das duas cria instâncias como a `Entity` (seção 7) — são descritores.

Verificado nos dois motores.

---

## 8.1. `model` — esquema de validação de dict

Um `model` descreve os campos que um dict deve ter e de que tipo. Depois, o
operador `==` valida um dict contra o model.

![exemplo 1](../assets/linguagem__08-model-e-enum_ex1.png)

<details><summary>código</summary>

```ps
model Usuario() {
    nome: str
    idade: int
}

post({ "nome": "ana", "idade": 30 } == Usuario)     // True
post({ "nome": "ana", "idade": "x" } == Usuario)    // False (idade não é int)
post({ "nome": "ana" } == Usuario)                  // False (falta idade)
```

</details>

Sintaxe:

- **`model Nome() { … }`** — os parênteses (vazios) e as chaves são
  obrigatórios.
- Cada campo é `nome: tipo`, **um por linha** (sem vírgula entre eles).
- O tipo é um de **`str`**, **`int`**, **`flo`**, **`bool`** (só esses quatro).

### 8.1.1. Limite de comprimento — `length`

Um campo `str` (ou numérico) pode fixar um **comprimento máximo** com
`tipo(length=N)`:

![exemplo 2](../assets/linguagem__08-model-e-enum_ex2.png)

<details><summary>código</summary>

```ps
model Documento() {
    cpf: str(length=11)
}

post({ "cpf": "12345678901" } == Documento)   // True  (11 caracteres)
post({ "cpf": "123" } == Documento)           // True  (menos que 11 — ok)
post({ "cpf": "123456789012" } == Documento)  // False (12 > 11)
```

</details>

`length` é um **máximo**, não um valor exato: strings mais curtas passam.

### 8.1.2. Regras da validação

`dict == Model` (nas duas ordens — `Model == dict` também) é `True` quando:

- **todo campo declarado** está presente no dict, e
- cada um tem o **tipo declarado** (e respeita o `length`, se houver).

Chaves **a mais** no dict são ignoradas (não invalidam):

![exemplo 3](../assets/linguagem__08-model-e-enum_ex3.png)

<details><summary>código</summary>

```ps
model P() { x: int }
post({ "x": 1, "extra": 9 } == P)     // True — 'extra' é ignorado
```

</details>

Faltar um campo, ou um campo com o tipo errado / longo demais, torna a
comparação `False`. O `model` não levanta erro na validação — ele responde
`True`/`False`, então cabe direto num `if`.

---

## 8.2. `enum` — constantes inteiras nomeadas

Um `enum` agrupa constantes sob um nome. Cada membro é, no fundo, um **inteiro**.

![exemplo 4](../assets/linguagem__08-model-e-enum_ex4.png)

<details><summary>código</summary>

```ps
enum Cor {
    RED
    GREEN
    BLUE
}

post(Cor.RED, Cor.GREEN, Cor.BLUE)    // 0 1 2
```

</details>

Sintaxe:

- **`enum Nome { … }`** — **sem** parênteses (ao contrário do `model`); as chaves
  são obrigatórias.
- Os membros são separados por **vírgula ou quebra de linha** (ambas valem).

### 8.2.1. Numeração

Sem valor explícito, os membros são numerados **a partir de 0**. Um valor
explícito (`= n`) fixa aquele membro; os seguintes **continuam a contar a partir
dele**:

![exemplo 5](../assets/linguagem__08-model-e-enum_ex5.png)

<details><summary>código</summary>

```ps
enum Status {
    OK = 200,
    NOT_FOUND = 404
}
post(Status.OK, Status.NOT_FOUND)     // 200 404

enum E {
    A = 10,
    B,
    C
}
post(E.A, E.B, E.C)                   // 10 11 12   (B e C continuam de 10)
```

</details>

### 8.2.2. Uso

- Acesse um membro por `Nome.MEMBRO`.
- O valor **é um inteiro** de verdade: `type(Cor.RED)` é `"int"` e
  `Cor.RED == 0` é `True` — dá pra comparar e usar em contas normalmente.
- Acessar um membro inexistente é erro: `enum 'Cor' não tem membro 'AZUL'`.

---

## 8.3. Resumo

- **`model Nome() { campo: tipo … }`** (parênteses + chaves; campos por linha;
  tipos `str`/`int`/`flo`/`bool`; `tipo(length=N)` = máximo). Valida um dict com
  `==` (True/False): campos declarados presentes e com o tipo certo; extras
  ignorados.
- **`enum Nome { A, B … }`** (sem parênteses; chaves; vírgula ou linha).
  Constantes **inteiras** a partir de `0` (ou explícitas, com auto-continuação);
  `Nome.MEMBRO` devolve o inteiro.
