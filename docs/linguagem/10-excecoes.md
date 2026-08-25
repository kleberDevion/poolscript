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
try:
    n = int(entrada)
catch (ConversionError e):
    post("não é um número:", e)
finally:
    post("terminei a tentativa")
```

- **`try:`** — o bloco que pode falhar.
- **`catch (...)`** — roda se o `try` levantar um erro que ele capture. Os
  parênteses são **obrigatórios**; dentro deles vão o tipo e/ou a variável (ver
  10.3). `catch:` sem parênteses é erro de sintaxe.
- **`finally:`** — opcional; roda **sempre**, tenha havido erro ou não (bom para
  fechar recursos). Roda tanto na saída normal quanto quando o erro vai propagar.

> **`catch` é obrigatório.** Não existe `try:` seguido direto de `finally:` —
> isso é `SyntaxError: esperado 'catch' apos bloco do try`. Para garantir
> limpeza **sem** capturar o erro, use `catch (e)` que relança:
>
> ```ps
> try:
>     arriscado()
> catch (e):
>     raise Erro(e)
> finally:
>     limpa()
> ```
>
> Para fechar arquivo/conexão, o caminho normal é o `using`, que fecha
> sozinho na saída do bloco, com erro ou sem.

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
try:
    risco()
catch (KeyError e):        // só KeyError
    post("faltou uma chave:", e)
catch (e):                 // qualquer outro
    post("outro erro:", e)
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

| Tipo | Quando ocorre |
|---|---|
| `RuntimeError` | erro genérico (ex.: variável não definida, aridade errada) |
| `KeyError` | chave ausente num dict (`d["x"]` / `d.x` sem a chave) |
| `AtributtedValueError` | valor incompatível com o tipo declarado (`int x = 5.0`) |
| `ConversionError` | conversão impossível (`int("abc")`, `int z = "abc"`) |
| `SomeValueUnexpected` | valor inválido numa operação (divisão por zero, `str` em conta, …) |
| `ImportError` | módulo não encontrado no `import` |
| `MemoryError` | sem memória |

```ps
try:
    x = 1 / 0
catch (SomeValueUnexpected e):
    post("erro de valor:", e)
```

Observações:

- **`SyntaxError`** acontece **antes** de rodar (na análise do código), então não
  é capturável por `try`/`catch` — é erro de escrita, não de execução.
- **`IndexOutOfBoundsWarning`** é um **aviso não-fatal**: acessar uma lista fora
  do intervalo (`l[99]`) **não** levanta erro — devolve `null` e segue (com um
  aviso). Portanto não é algo que você captura.
- As **bibliotecas** (banco, rede, e-mail, …) levantam os próprios tipos
  (ex.: `DatabaseError`, `NetworkError`), documentados na parte de bibliotecas.

---

## 10.5. Resumo

- **`raise Tipo("msg")`** — tipo livre (maiúsculo), mensagem opcional; sem
  `try` em volta, propaga e encerra o programa.
- **`try:` / `catch (...)` / `finally:`** — `catch` exige parênteses; `finally`
  roda sempre.
- **Formas de `catch`**: `(Tipo e)`, `(Tipo)`, `(e)`, `()` — com tipo só pega
  aquele tipo (senão propaga); `e` é o texto do erro, local ao bloco.
- **Tipos embutidos**: `RuntimeError`, `KeyError`, `AtributtedValueError`,
  `ConversionError`, `SomeValueUnexpected`, `ImportError`, `MemoryError`.
  `SyntaxError` não é capturável (é de compilação); índice fora do range é aviso
  não-fatal (→ `null`).
