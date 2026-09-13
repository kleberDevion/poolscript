# Exceptions — erros, `raise` e `catch`

Um erro na PoolScript tem um **tipo** (um nome) e uma **mensagem**. Você pode
deixá-lo **propagar** (para o programa com uma mensagem limpa — `catch` é
opcional) ou **capturar** com `try/catch`.

---

## `raise` — levantar um erro

```
raise "algo deu errado"           # tipo RuntimeError, com essa mensagem
raise NetworkError("a conexão caiu")   # tipo LIVRE: qualquer NomeAssim vira o tipo
raise MinhaFalha                  # só o tipo, sem mensagem
```

Regra: um nome que **começa com maiúscula** vira o **tipo** do erro — você
inventa o nome que quiser (não existe lista fixa). `raise "texto"` (ou uma
variável) manda só a mensagem, e o tipo é `RuntimeError`.

**Sem `catch`**, o erro propaga e para o programa com a mensagem — nunca é um
crash:

```
raise NetworkError("caiu")
# NetworkError: caiu
#   em app.ps, linha 1
```

---

## `catch` — capturar (opcional)

O **tipo** e a **variável do erro** são os dois opcionais:

```
try { ... } catch (NetworkError e) { post("caiu:", e) }   # tipo + variável
try { ... } catch (NetworkError)   { post("caiu") }        # só o tipo, sem var
try { ... } catch (e)              { post("qualquer:", e) } # captura tudo, com var
try { ... } catch ()               { post("qualquer") }     # captura tudo, sem var
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
> Ele não existe mais. A divisão agora é: **`TypeError`** quando o TIPO está
> errado, **`ValueError`** quando o tipo está certo e o VALOR não serve.

| tipo | quando acontece |
|---|---|
| `TypeError` | **o tipo está errado**: `"a" - 1`, `sum(["a"])`, `len(5)`, `[1,2]["x"]`, aridade errada de método. É o mais comum. |
| `ValueError` | **o tipo está certo e o valor não serve**: `int("abc")`, `"banana".index("zz")`, `max([])`, `chr(99999999)`. |
| `ZeroDivisionError` | divisão ou resto por zero. O texto separa quatro casos: `1/0` → `division by zero`; `1.0/0` → `flo division by zero`; `1%0` → `integer modulo by zero`; `1.5%0.0` → `flo modulo` |
| `NameError` | nome que não existe no escopo: `post(x)` → `name 'x' is not defined` |
| `AttributeError` | membro que o objeto não tem: `"abc".m` → `'str' object has no attribute 'm'`; também `module 'json' has no attribute 'x'` |
| `OverflowError` | número que não cabe no destino: `int(flo("inf"))` → `cannot convert flo infinity to integer` |
| `RecursionError` | recursão ou expressão funda demais: `maximum recursion depth exceeded`. Antes era `RuntimeError`, e por isso só dava pra pegar junto com todo o resto |
| `MemoryError` | sem memória. A mensagem diz ONDE acabou — `sem memoria em sorted()` — em vez de vir vazia: num processo que morreu de memória essa é a única pista que sobra |
| `AttributedValueError` | valor incompatível atribuído a variável **tipada**: `str x = 10`. **Não** cobre o `+` — somar tipos que não somam é `TypeError` |
| `IndexError` | índice fora da faixa, lendo **ou** escrevendo. O texto diz qual: `list index out of range`, `string index out of range`, `tup index out of range`, `index out of range` (bytes), e `list assignment index out of range` na escrita |
| `AssertionError` | `assert(...)` que não passou. Com um valor só, o texto é o valor recebido (`false nao e verdadeiro`); com dois, os dois lados (`veio 3, esperava 4`); com uma nota, ela vem na frente. É capturável como qualquer outra — um teste pode contar as falhas em vez de parar na primeira |
| `NotImplemented` | função de lib "stub": existe, e ainda não faz nada. O nome é esse mesmo, **sem** o `Error` no fim — `catch (NotImplementedError e)` não pega esta |
| `NotImplementedError` | módulo importado que **não compila** por motivo que não é sintaxe. É outro erro, apesar do nome parecido — não confunda com o de cima |
| `SyntaxError` | erro de sintaxe. Não é capturável em tempo de execução: acontece **antes** de o programa rodar, e é o que o `pool --check` relata |
| `KeyError` | chave inexistente num dict: `d["naoexiste"]`. A mensagem é a **chave**, e só ela: `KeyError: 'naoexiste'`. Vale igual em `d.chave` e `d.pop("chave")` |
| `ImportError` | módulo não encontrado: `import naoexiste` |
| `ConversionError` | `char c = -1` — inteiro que não é um codepoint válido. Só isso: a declaração tipada **não converte** (`int z = "abc"` é `AttributedValueError`, porque `"abc"` é `str`). A lib `Parsing` **não** levanta — ela é best-effort e devolve `0`/`0.0`/`{}` |
| `NetworkError` | falha de rede/conexão (`request`, http) |
| `DatabaseError` | erro de banco (`psodbc`) |
| `TimeoutError` | tempo esgotado (`request` com `timeout=`) |
| `IOError` | falha de I/O que o motor não conseguiu classificar melhor |
| `OSError` | erro do sistema sem tipo próprio. A mensagem é a do sistema, com o número do erro na frente: `[Errno 39] Directory not empty: '/tmp/x'` |
| `FileNotFoundError` | o caminho não existe: `os.readFile("sumiu.txt")` → `[Errno 2] No such file or directory: 'sumiu.txt'` |
| `FileExistsError` | já existe: `os.mkdir` de pasta que está lá → `[Errno 17] File exists: '…'` |
| `PermissionError` | sem permissão: `[Errno 13] Permission denied: '…'` |
| `IsADirectoryError` | esperava arquivo, veio pasta: `os.writeFile(pasta, "x")` → `[Errno 21] Is a directory: '…'` |
| `NotADirectoryError` | esperava pasta, veio arquivo: `[Errno 20] Not a directory: '…'` |
| `LookupError` | nome de codec ou de handler que não existe: `"a".encode("xyz")` → `unknown encoding: xyz` |
| `UnicodeEncodeError` | caractere que não cabe no encoding pedido: `"é".encode("ascii")` |
| `UnicodeDecodeError` | bytes que não formam texto válido no encoding pedido |
| `RuntimeError` | `raise "texto"`; e o que é só desta linguagem: `acesso negado: … private`, `nonnull`, `funct … e static` |
| *(o seu)* | qualquer nome que você levantar com `raise Nome("msg")` |

## A árvore de exceções

`catch (Tipo e)` pega o tipo pedido **e todos os descendentes dele**. `Exception`
é a raiz: `catch (Exception e)` pega qualquer erro.

```
Exception
├── OSError              FileExistsError · FileNotFoundError · IsADirectoryError
│                        NotADirectoryError · PermissionError · NetworkError
├── IOError
├── LookupError          IndexError · KeyError
├── ArithmeticError      ZeroDivisionError · OverflowError
├── ValueError           UnicodeError (UnicodeDecodeError · UnicodeEncodeError)
│                        AttributedValueError · ConversionError
├── RuntimeError         RecursionError
├── TypeError      ├── AttributeError   ├── NameError      ├── ImportError
└── MemoryError    └── AssertionError   └── SyntaxError    └── DatabaseError
```

Pegar por família, em vez de listar filho por filho:

```ps
try {
    conteudo = os.readFile(caminho)
} catch (OSError e) {
    post("problema de arquivo:", e)
}
```

...e ainda dá pra separar os motivos que pedem resposta diferente, pondo o
**mais específico primeiro** — o `catch` tenta na ordem em que você escreveu:

```ps
try {
    conteudo = os.readFile(caminho)
} catch (FileNotFoundError e) {
    post("não achei:", caminho)
} catch (OSError e) {
    post("outro problema de arquivo:", e)
}
```

`catch (e)` sem tipo continua pegando tudo, e é o mais curto quando tanto faz o
motivo.

> **`IOError` é IRMÃO de `OSError`, não pai nem filho.** `catch (IOError e)`
> não pega `FileNotFoundError`, e nunca pegou. Um tipo que você mesmo levantar
> com `raise Nome("msg")` fica fora da árvore: só o `catch` do próprio nome e o
> `catch (e)` o pegam.

> **Isto mudou na versão 15.90.14.** Antes o `catch` comparava os dois nomes
> letra a letra, então `catch (Exception e)` não pegava **nada** e
> `catch (OSError e)` não pegava `FileNotFoundError` — mesmo o motor gerando
> esses nomes de subclasse de propósito. Quem escreveu a cadeia filho a filho
> não precisa mudar nada: a árvore só acrescenta capturas, nunca tira.

> **Nota — "não existe" sempre LEVANTA, desde 28/08.** Ler índice fora da
> faixa, escrever fora da faixa e pedir chave ausente levantam, e cada um diz o
> nome do seu tipo:
>
> ```
> IndexError: list index out of range
> IndexError: string index out of range
> IndexError: tup index out of range
> IndexError: index out of range              (bytes — aqui o tipo não entra)
> IndexError: list assignment index out of range     (escrevendo: l[99] = x)
> KeyError: 'z'
> ```
>
> O nome do tipo na frase é o que o `type()` devolve: `tup`, não `tuple`.
>
> Antes eram TRÊS comportamentos: LER devolvia `null` com rc=0 e escrevia
> `IndexOutOfBoundsWarning` direto no stderr — que não era exceção, então
> `try { post(l[99]) } catch (e)` **não pegava nada** e o `null` seguia adiante
> no pipeline, com o erro aparecendo longe da causa. Escrever e chave ausente
> já levantavam. Era o ilogismo I4/I5 do catálogo.

---

## Os nomes são valores

Cada nome da árvore acima existe como **valor** da linguagem, sem importar
nada — dá pra guardar numa variável, passar pra funct, comparar e imprimir:

```ps
erro = FileNotFoundError
post(erro)                        # FileNotFoundError
post(type(erro))                  # type
post(erro is OSError)             # True  — `is` segue a árvore
post(ValueError is Exception)     # True
post(ValueError is TypeError)     # False
post(ValueError == "ValueError")  # True  — como os outros tipos (`str == "str"`)
```

`raise` aceita a variável — levanta o tipo guardado, sem mensagem — e o
`catch` da família pega:

```ps
erro = FileNotFoundError
try {
    raise erro
} catch (OSError e) {
    post("família OSError")
}
```

Regras que valem junto:

- **`raise Nome` com inicial maiúscula é tipo LITERAL**, não a variável:
  `Boom = ValueError` seguido de `raise Boom` levanta `Boom`, não `ValueError`.
  Pra levantar pelo valor, use nome minúsculo (`raise erro`).
- **O `e` do `catch` continua sendo o TEXTO do erro** (`type(e)` é `str`), então
  `e is ValueError` é sempre `False`. Pra testar o tipo, use o próprio `catch`:
  `catch (ValueError e)`.
- **`ValueError("msg")` fora do `raise` é erro**:
  `TypeError: ValueError("msg") só vale depois de raise`.
- Os nomes são globais como `PoolFile`: dá pra sombrear (`ValueError = 3`), e
  nome fora da árvore (`MeuErro`) continua `NameError` até você levantá-lo com
  `raise MeuErro(...)`.
- O editor conhece os nomes (completion, hover e realce) pelo `pool --metadata`
  (chave `excecoes`), que sai da mesma tabela do motor.

---

## Exemplo completo

```
import request

funct buscar(url=str) {
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
