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
| `ZeroDivisionError` | divisão ou resto por zero. O texto separa quatro casos, como no Python: `1/0` → `division by zero`; `1.0/0` → `flo division by zero`; `1%0` → `integer modulo by zero`; `1.5%0.0` → `flo modulo` |
| `NameError` | nome que não existe no escopo: `post(x)` → `name 'x' is not defined` |
| `AttributeError` | membro que o objeto não tem: `"abc".m` → `'str' object has no attribute 'm'`; também `module 'json' has no attribute 'x'` |
| `OverflowError` | número que não cabe no destino: `int(flo("inf"))` → `cannot convert flo infinity to integer` |
| `RecursionError` | recursão ou expressão funda demais: `maximum recursion depth exceeded`. Antes era `RuntimeError`, e por isso só dava pra pegar junto com todo o resto |
| `MemoryError` | sem memória. **Único caso em que a mensagem não é a do CPython**, e de propósito: lá ela é vazia (`MemoryError:` e nada mais), aqui ela diz onde acabou — `sem memoria em sorted()`. Num processo que morreu de memória essa é a única pista que sobra |
| `AttributedValueError` | valor incompatível atribuído a variável **tipada**: `str x = 10`. **Não** cobre o `+` — somar tipos que não somam é `TypeError` |
| `IndexError` | índice fora da faixa, lendo **ou** escrevendo. O texto diz qual: `list index out of range`, `string index out of range`, `tup index out of range`, `index out of range` (bytes), e `list assignment index out of range` na escrita |
| `AssertionError` | `assert(...)` que não passou. Com um valor só, o texto é o valor recebido (`false nao e verdadeiro`); com dois, os dois lados (`veio 3, esperava 4`); com uma nota, ela vem na frente. É capturável como qualquer outra — um teste pode contar as falhas em vez de parar na primeira |
| `NotImplemented` | função de lib "stub": existe, e ainda não faz nada. O nome é esse mesmo, **sem** o `Error` no fim — `catch (NotImplementedError e)` não pega esta |
| `NotImplementedError` | módulo importado que **não compila** por motivo que não é sintaxe. É outro erro, apesar do nome parecido — não confunda com o de cima |
| `SyntaxError` | erro de sintaxe. Não é capturável em tempo de execução: acontece **antes** de o programa rodar, e é o que o `pool --check` relata |
| `KeyError` | chave inexistente num dict: `d["naoexiste"]`. A mensagem é a **chave**, e só ela: `KeyError: 'naoexiste'`. Vale igual em `d.chave` e `d.pop("chave")` |
| `ImportError` | módulo não encontrado: `import naoexiste` |
| `ConversionError` | coerção de **declaração tipada** que não dá: `int z = "abc"`, `char c = -1`. A lib `Parsing` **não** levanta — ela é best-effort e devolve `0`/`0.0`/`{}` |
| `NetworkError` | falha de rede/conexão (`request`, http) |
| `DatabaseError` | erro de banco (`psodbc`) |
| `TimeoutError` | tempo esgotado (`request` com `timeout=`) |
| `IOError` | falha de I/O que o motor não conseguiu classificar melhor |
| `OSError` | erro do sistema sem tipo próprio. A mensagem é a do sistema, no formato do Python: `[Errno 39] Directory not empty: '/tmp/x'` |
| `FileNotFoundError` | o caminho não existe: `os.readFile("sumiu.txt")` → `[Errno 2] No such file or directory: 'sumiu.txt'` |
| `FileExistsError` | já existe: `os.mkdir` de pasta que está lá → `[Errno 17] File exists: '…'` |
| `PermissionError` | sem permissão: `[Errno 13] Permission denied: '…'` |
| `IsADirectoryError` | esperava arquivo, veio pasta: `os.writeFile(pasta, "x")` → `[Errno 21] Is a directory: '…'` |
| `NotADirectoryError` | esperava pasta, veio arquivo: `[Errno 20] Not a directory: '…'` |
| `LookupError` | nome de codec ou de handler que não existe: `"a".encode("xyz")` → `unknown encoding: xyz` |
| `UnicodeEncodeError` | caractere que não cabe no encoding pedido: `"é".encode("ascii")` |
| `UnicodeDecodeError` | bytes que não formam texto válido no encoding pedido |
| `RuntimeError` | `raise "texto"`; e o que é só desta linguagem e não tem par no Python: `acesso negado: … private`, `@NonNull`, `for each` sobre tipo que não itera |
| *(o seu)* | qualquer nome que você levantar com `raise Nome("msg")` |

> **`catch` casa o NOME do tipo, não uma árvore.** Vindo do Python, a
> armadilha é escrever `catch (OSError e)` esperando que ele pegue
> `FileNotFoundError` — não pega, porque aqui `OSError` é só um nome, não um
> ancestral. As duas formas que funcionam:
>
> ```ps
> try {
>     conteudo = os.readFile(caminho)
> } catch (FileNotFoundError e) {
>     post("não achei:", caminho)
> } catch (PermissionError e) {
>     post("sem permissão:", caminho)
> }
> ```
>
> ...ou `catch (e)` sem tipo, que pega qualquer erro. O `catch (e)` é o mais
> curto e vale quando tanto faz o motivo; o encadeado é o que você quer quando
> cada motivo pede uma resposta diferente — ou quando não quer engolir junto um
> `NameError` de digitação sua.

> **Nota — "não existe" sempre LEVANTA, desde 28/08.** Ler índice fora da
> faixa, escrever fora da faixa e pedir chave ausente levantam, e cada um diz o
> nome do seu tipo:
>
> ```
> IndexError: list index out of range
> IndexError: string index out of range
> IndexError: tup index out of range
> IndexError: index out of range              (bytes — o Python também não põe o tipo aqui)
> IndexError: list assignment index out of range     (escrevendo: l[99] = x)
> KeyError: 'z'
> ```
>
> As frases são as do CPython, palavra por palavra. A única diferença é o nome
> do tipo, que é o que o `type()` desta linguagem devolve: `tup`, não `tuple`.
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
