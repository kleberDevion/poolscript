# Exceptions — erros, `raise` e `catch`

Um erro na PoolScript tem um **tipo** (um nome) e uma **mensagem**. Você pode
deixá-lo **propagar** (para o programa com uma mensagem limpa — `catch` é
opcional) ou **capturar** com `try/catch`.

---

## `raise` — levantar um erro

```
raise "algo deu errado"           // tipo RuntimeError, com essa mensagem
raise NetworkError("a conexão caiu")   // tipo LIVRE: qualquer NomeAssim vira o tipo
raise MinhaFalha                  // só o tipo, sem mensagem
```

Regra: um nome que **começa com maiúscula** vira o **tipo** do erro — você
inventa o nome que quiser (não existe lista fixa). `raise "texto"` (ou uma
variável) manda só a mensagem, e o tipo é `RuntimeError`.

**Sem `catch`**, o erro propaga e para o programa com a mensagem — nunca é um
crash:

```
raise NetworkError("caiu")
// NetworkError: caiu
//   em app.ps, linha 1
```

---

## `catch` — capturar (opcional)

O **tipo** e a **variável do erro** são os dois opcionais:

```
try { ... } catch (NetworkError e) { post("caiu:", e) }   // tipo + variável
try { ... } catch (NetworkError)   { post("caiu") }        // só o tipo, sem var
try { ... } catch (e)              { post("qualquer:", e) } // captura tudo, com var
try { ... } catch ()               { post("qualquer") }     // captura tudo, sem var
```

Vários `catch` em cadeia + `finally` opcional:

```
try {
    conn = psodbc.connect(driver="postgres", ...)
} catch (DatabaseError e) {
    post("banco:", e)
} catch (NetworkError e) {
    post("rede:", e)
} finally {
    post("sempre roda")
}
```

Se nenhum `catch` casa o tipo, o erro **continua propagando** (um `try` de fora
ainda pode pegá-lo) — `catch (KeyError)` não vira um catch-tudo silencioso.

---

## Os tipos que o motor levanta

> **Mudou em 29/08.** Havia um tipo chamado `SomeValueUnexpected` que cobria
> **468** dos sítios de erro do motor, contra 3 de `TypeError`. Na prática
> `catch (TypeError e)` era inútil: operação entre tipos, valor inválido,
> divisão por zero e falha de conversão caíam todos no mesmo balde, e não dava
> pra tratar um sem tratar os outros.
>
> Ele não existe mais. A divisão agora é a do Python: **`TypeError`** quando o
> TIPO está errado, **`ValueError`** quando o tipo está certo e o VALOR não
> serve.

| tipo | quando acontece |
|---|---|
| `TypeError` | **o tipo está errado**: `"a" - 1`, `sum(["a"])`, `len(5)`, `[1,2]["x"]`, aridade errada de método. É o mais comum. |
| `ValueError` | **o tipo está certo e o valor não serve**: `int("abc")`, `"banana".index("zz")`, `max([])`, `chr(99999999)`. A divisão é a mesma do Python. |
| `ZeroDivisionError` | divisão ou resto por zero: `1 / 0`, `1 % 0`, `1.5 % 0.0` |
| `AttributedValueError` | `+` entre tipos que não somam: `"a" + 1`. A mensagem diz quais são: `'+' entre tipos incompativeis: str + int` |
| `IndexError` | índice inválido ao **escrever**: `l[99] = x`. Ler fora da faixa é outra coisa — ver a nota abaixo |
| `NotImplementedError` | construção que o motor reconhece e ainda não executa |
| `FileNotFoundError` | arquivo que não existe, quando o motor consegue distinguir de outra falha de I/O |
| `SyntaxError` | erro de sintaxe. Não é capturável em tempo de execução: acontece **antes** de o programa rodar, e é o que o `pool --check` relata |
| `KeyError` | chave inexistente num dict: `d["naoexiste"]` |
| `ImportError` | módulo não encontrado: `import naoexiste` |
| `ConversionError` | falha de conversão (`Parsing`) |
| `NetworkError` | falha de rede/conexão (`request`, http) |
| `DatabaseError` | erro de banco (`psodbc`) |
| `TimeoutError` | tempo esgotado (`request` com `timeout=`) |
| `IOError` / `OSError` | arquivo / sistema (`os`) |
| `OutputUnexpectedValues` | desempacotar com aridade errada: `a, b = [1, 2, 3]` |
| `MemoryError` | sem memória |
| `RuntimeError` | variável não definida; `raise "texto"`; erro genérico de runtime |
| *(o seu)* | qualquer nome que você levantar com `raise Nome("msg")` |

> **Nota — "não existe" tem UMA resposta só, desde 28/08.** Ler índice fora da
> faixa, escrever fora da faixa e pedir chave ausente levantam, e as mensagens
> dizem a mesma coisa do mesmo jeito:
>
> ```
> IndexError: indice 99 fora do tamanho de list (3 itens)
> IndexError: indice 9 fora do tamanho de str (3 caracteres)
> IndexError: indice 9 fora do tamanho de bytes (2 bytes)
> KeyError: chave 'z' nao existe no dict (1 chave)
> ```
>
> Antes eram TRÊS comportamentos: LER devolvia `null` com rc=0 e escrevia
> `IndexOutOfBoundsWarning` direto no stderr — que não era exceção, então
> `try { post(l[99]) } catch (e)` **não pegava nada** e o `null` seguia adiante
> no pipeline, com o erro aparecendo longe da causa. Escrever e chave ausente
> já levantavam. Era o ilogismo I4/I5 do catálogo.

---

## Exemplo completo

```
import request

action buscar(url=str) {
    try {
        r = request.get(url=url, timeout=5)
        return r.json()
    } catch (TimeoutError) {
        post("demorou demais")
        return null
    } catch (NetworkError e) {
        post("sem rede:", e)
        return null
    }
}
```
