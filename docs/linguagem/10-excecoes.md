# Referência da Linguagem — 10. Exceptions (erros e tratamento)

Um erro em tempo de execução levanta uma **exception**. Sem tratamento, ela
sobe até o topo e encerra o programa com a mensagem. Com `try`/`catch` você
intercepta, e com `finally` garante uma limpeza. Você também levanta as suas com
`raise`.

Verificado na VM.

---

## 10.1. `raise` — levantar uma exception

```ps
raise ValorInvalido("idade não pode ser negativa")
```

- O **tipo** é um nome que **começa com maiúscula** e é **livre**: você inventa
  o nome que fizer sentido (`ValorInvalido`, `SaldoInsuficiente`, …) — não
  precisa declarar nada antes.
- A **mensagem** é opcional: `raise Erro()` também vale.

Sem um `try`/`catch` em volta, o `raise` **propaga** e o programa termina com
erro — dá pra usar `raise` sozinho, como uma parada com mensagem.

---

## 10.2. `try` / `catch` / `finally`

```ps
try {
    n = int(entrada)
} catch (ValueError e) {
    post("não é um número:", e)
} finally {
    post("terminei a tentativa")
}
```

- **`try:`** — o bloco que pode falhar.
- **`catch (...)`** — roda se o `try` levantar um erro que ele capture. Os
  parênteses são **obrigatórios**; dentro deles vão o tipo e/ou a variável (ver
  10.3). `catch:` sem parênteses é erro de sintaxe.
- **`finally:`** — opcional; roda **sempre**, tenha havido erro ou não (bom para
  fechar recursos). Roda tanto na saída normal quanto quando o erro vai propagar.

> **`try` + `finally` sem `catch` vale.** É a forma "faça isto aconteça o que
> acontecer, sem tratar o erro" — fechar arquivo, soltar trava, derrubar um
> servidor. O `finally` roda e a exceção, se houver, **propaga depois dele**:
>
> ```ps
> try {
>     abre_a_porteira()
>     pode_estourar()
> }
> finally {
>     fecha_a_porteira()          # roda mesmo se estourar
> }
> ```
>
> O que continua sendo erro é `try` **sozinho**, sem `catch` nem `finally` —
> aí ele não pediria nada:
> `SyntaxError: esperado 'catch' ou 'finally' apos bloco do try`.

Blocos `try` podem ser aninhados; um erro não capturado no `catch` interno sobe
para o `try` externo.

---

## 10.3. Formas do `catch`

O `catch` tem quatro formas, do mais específico ao mais geral:

| Forma | Captura | Liga |
|---|---|---|
| `catch (Tipo e)` | só erros daquele **tipo** | `e` = a mensagem/erro |
| `catch (Tipo)` | só erros daquele **tipo** | — (sem variável) |
| `catch (e)` | **qualquer** erro | `e` |
| `catch ()` | **qualquer** erro | — |

```ps
try {
    risco()
} catch (KeyError e) {        # só KeyError
    post("faltou uma chave:", e)
} catch (e) {                 # qualquer outro
    post("outro erro:", e)
}
```

- Um `catch` com **tipo** só pega erros **daquele tipo**; um erro de tipo
  diferente **não** é capturado ali e continua propagando (para o próximo
  `catch`, ou para fora).
- A variável do `catch` (`e`) **vale o texto do erro** — imprime como
  `mensagem (linha N)`. Ela é **local ao bloco do catch** (não existe depois;
  seção 4.6).

---

## 10.4. Tipos de erro embutidos

Além dos tipos livres que você levanta, a VM usa estes nomes ao reportar
erros — e você pode capturá-los por tipo:

A divisão entre eles é a do Python: **`TypeError`** quando o TIPO está errado,
**`ValueError`** quando o tipo está certo e o VALOR não serve.

| Tipo | Quando ocorre |
|---|---|
| `TypeError` | o **tipo** está errado: `"a" - 1`, `len(5)`, aridade errada de funct ou método |
| `ValueError` | o tipo está certo e o **valor** não serve: `int("abc")`, `max([])`, `chr(99999999)` |
| `NameError` | nome que não existe no escopo: `post(x)` |
| `AttributeError` | membro que o objeto não tem: `"abc".m`, `json.naoexiste` |
| `IndexError` | índice fora da faixa, lendo ou escrevendo |
| `KeyError` | chave ausente num dict (`d["x"]` / `d.x` / `d.pop("x")`) |
| `ZeroDivisionError` | divisão ou resto por zero: `1 / 0` |
| `OverflowError` | número que não cabe no destino: `int(flo("inf"))` |
| `AttributedValueError` | valor incompatível com o tipo declarado (`str x = 10`) |
| `ConversionError` | coerção de declaração tipada que não dá (`int z = "abc"`) |
| `ImportError` | módulo não encontrado no `import` |
| `MemoryError` | sem memória |
| `RuntimeError` | `raise "texto"`, e o que só existe aqui: `private` e `nonnull` |

```ps
try {
    x = 1 / 0
} catch (ZeroDivisionError e) {
    post("dividiu por zero:", e)
}
```

> Este exemplo dizia `catch (TypeError e)`, e o `catch` **nunca disparava** —
> divisão por zero é `ZeroDivisionError`. Quem copiasse escrevia um handler
> morto e o erro escapava assim mesmo. Nem `audita_doc.ps` nem
> `audita_exemplos_doc.ps` pegavam: o primeiro só confere que o nome do tipo
> existe, o segundo só roda `--check`, que é sintaxe.

Observações:

- **`SyntaxError`** acontece **antes** de rodar (na análise do código), então não
  é capturável por `try`/`catch` — é erro de escrita, não de execução.
- **`IndexError`** é levantado ao **ler** fora do intervalo (`l[99]`) em `list`,
  `tup`, `str` e `bytes`; a mensagem nomeia o tipo (`list index out of range`),
  menos em `bytes`, que sai como `index out of range`. Até 28/08 a leitura era
  um aviso não-fatal que devolvia `null` e não podia ser capturado.
- Na **escrita** por índice, `IndexError` só existe em `list`
  (`list assignment index out of range`): `tup`, `str` e `bytes` são imutáveis,
  e a escrita neles é `TypeError: '<tipo>' object does not support item
  assignment`, fora do intervalo ou não.
- **`for each` sobre tipo que não itera** é `TypeError`
  (`'int' object is not iterable`), não `RuntimeError` — um
  `catch (RuntimeError e)` não pega.
- As **bibliotecas** (banco, rede, e-mail, …) levantam os próprios tipos
  (ex.: `DatabaseError`, `NetworkError`), documentados na parte de bibliotecas.

---

## 10.5. Resumo

- **`raise Tipo("msg")`** — tipo livre (maiúsculo), mensagem opcional; sem
  `try` em volta, propaga e encerra o programa.
- **`try` / `catch (...)` / `finally`** — `catch` exige parênteses; `finally`
  roda sempre. Um dos dois tem que existir; os dois juntos também valem, e
  `try` + `finally` **sem** `catch` é legítimo (o erro propaga depois do
  `finally`).
- **Formas de `catch`**: `(Tipo e)`, `(Tipo)`, `(e)`, `()` — com tipo só pega
  aquele tipo (senão propaga); `e` é o texto do erro, local ao bloco.
- **Tipos embutidos**: `TypeError`, `ValueError`, `NameError`,
  `AttributeError`, `IndexError`, `KeyError`, `ZeroDivisionError`,
  `OverflowError`, `AttributedValueError`, `ConversionError`, `ImportError`,
  `MemoryError`, `RuntimeError`. `SyntaxError` não é capturável (é de
  compilação). Índice fora da faixa **levanta** `IndexError` — a linha que
  dizia "aviso não-fatal (→ `null`)" contradizia a seção 10.4 desta mesma
  página e era resto do comportamento anterior a 28/08.
