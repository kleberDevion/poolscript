# Limitações da linguagem — achadas, corrigidas, e as que faltam

Este arquivo é **só sobre defeitos**, não sobre a migração para C. A diferença
importa: um nó que a VM ainda não compila é trabalho planejado e mora em
[`MIGRACAO_C.md`](MIGRACAO_C.md). Aqui entram duas coisas que o plano não
prevê:

- **buraco na linguagem** — `.ps` que deveria funcionar e não funciona em
  lugar nenhum, nem no interpretador;
- **divergência VM ↔ interpretador** que nenhum teste cobria, então o
  diferencial passava sem ver.

As duas aparecem quase sempre por acaso, escrevendo `.ps` de teste para outra
coisa — por isso ficam agrupadas aqui em vez de espalhadas. Cada uma tem
regressão em [`tests/test_limitacoes.py`](tests/test_limitacoes.py) ou
[`tests/test_builtins_c.py`](tests/test_builtins_c.py), e o arquivo serve de
fila de trabalho.

## Como corrigir uma

Uma limitação atravessa a linguagem inteira, então o remendo em um só lugar
deixa os dois lados divergentes. A ordem que funciona:

1. `src/poolscript/parser.py` — nó novo no AST e o reconhecimento
2. `src/poolscript/interpreter.py` — a semântica (**é a autoridade**)
3. `vm/ps_ast.h` / `.c` — `N_<NOME>` e o nome legível
4. `vm/ps_parser.c` — o mesmo reconhecimento, em C
5. `vm/ps_compiler.c` — emissão de bytecode
6. `vm/poolscript_vm.c` — opcode novo, se precisar
7. `vm/ps_parser_bind.c` — serialização, senão o diferencial
   de AST compara texto incompleto e passa sem ver a diferença
8. `tests/ast_sexp.py` — o lado Python da mesma serialização
9. `tests/test_limitacoes.py` — a regressão
10. `python psl-poolscript-vsix/bridge/sync_parser.py` — senão o editor
    acusa erro de sintaxe em código válido

Depois: `./rebuild_vm.sh` e a suíte inteira.

---

## Corrigidas

### Atribuição por índice: `l[0] = 9`

**O que não funcionava:** nada. `l[0] = 9`, `d["a"] = 1`, `l[i] += 1`,
`l[0][1] = x` — todos davam `SyntaxError: expressão inválida`. A única forma
de montar uma lista era reconstruindo (`l = l + [x]`).

Passou despercebido porque o opcode `INDEX_SET` já existia na VM desde o
começo, implementado e testado no papel — **mas nenhum caminho do compilador
o emitia**, porque o parser nunca produzia o nó. Opcode morto.

**Como foi resolvido:** o alvo é reconhecido *depois* de montar a expressão,
não por lookahead:

```python
expr = self.parse_expression()
if isinstance(expr, IndexAccess) and self.current().value in ASSIGN_OPS:
    ...  # vira IndexAssignment
```

Lookahead exigiria varrer colchetes balanceados à frente para achar o `=` —
refazer o trabalho que o parser de expressão já faz. Como consequência,
`l[0][1] = x` e `obj.d["k"] = v` funcionam de graça: qualquer expressão serve
de container.

A forma aumentada (`l[i] += 1`) precisou do opcode **`DUP2`**. Re-avaliar
container e índice para ler o valor atual chamaria `f()` duas vezes em
`l[f()] += 1`; `DUP2` duplica o par que já está na pilha.

Erro fora de faixa em `l[9] += 1` é **erro**, não warning. Leitura simples
(`post(l[9])`) avisa e devolve `Null`, que é o que a spec pede — mas numa
atribuição aumentada não haveria onde escrever depois, e avisar viraria
silêncio.

### `lista + lista` e `lista * int` na VM

**O que não funcionava:** só na VM. O interpretador concatena (`[1,2] + [3]`)
e repete (`[1,2] * 3`); a VM levantava `'+' entre tipos incompatíveis`.

Achado rodando o stress de AddressSanitizer do lote de objetos — um caso do
próprio stress falhou, e o que parecia erro do script era divergência real.

A concatenação exige o **mesmo tipo**: `[1] + (2,)` é erro nos dois lados,
igual ao Python. Repetição com contagem ≤ 0 dá sequência vazia.

### Chave de dicionário só podia ser string ou identificador

**O que não funcionava:** `{1: "a"}`, `{True: 1}`, `{1.5: "x"}` — todos
`SyntaxError`, nos dois motores. Mas `d[1] = "a"` sempre funcionou. As duas
formas de escrever a mesma coisa estavam em desacordo.

**Como foi resolvido:** a chave passou a ser uma expressão qualquer, nos dois
parsers. `{1+1: "x"}` e `{(1,2): "t"}` saem de graça.

### Ordem de inserção do dict na VM

**O que não funcionava:** só na VM. `post({"b":1,"a":2})` saía
`{'a': 2, 'b': 1}` — a tabela hash de endereçamento aberto guardava as
entradas na ordem do hash, e a linguagem perdia a ordem de inserção.

Não é detalhe estético: é o que faz a saída de um `.ps` ser reproduzível. E
contaminava tudo que percorre dict — `post`, `str()`, `list(d)`, JSON.

**Como foi resolvido:** o `PSDict` virou **dict compacto**, no formato do
CPython — um array denso em ordem de inserção mais uma tabela `indices` que
resolve o hash para a posição no denso. Sobrescrever uma chave existente não
muda a posição dela na ordem.

### Erro de builtin escapava do `try`

**O que não funcionava:** só na VM. `try { post(len(1)) } catch (e) { ... }`
não capturava — o caminho de chamada nativa saía por `return -1`, abortando a
execução em vez de desviar para o desenrolamento.

Só apareceu quando os builtins passaram a ser vários: com `post` e `len`
apenas, quase nada falhava.

**Como foi resolvido:** o erro de builtin agora vai para o mesmo
`goto erro_runtime` do resto da VM, e o builtin escolhe o tipo do erro
(`TypeError`, `SomeValueUnexpected`, `IndexError`…) que o
`catch (Tipo e)` compara.

### Float impresso com um dígito de lixo

**O que não funcionava:** só na VM. `post(1/3)` dava `0.33333333333333331`
contra `0.3333333333333333` do interpretador — `%.17g` sempre volta ao mesmo
double, mas não é a MENOR representação que volta.

**Como foi resolvido:** `float_para_texto()` tenta precisão 1 a 17 e para na
primeira que faz `strtod` devolver o valor original; depois cola `.0` se não
sobrou `.` nem expoente. É a regra do `repr` do Python, e substituiu duas
cópias da formatação antiga.

### `Null` aninhado imprimia `None`

**O que não funcionava:** o interpretador. `post(Null)` dava `null`, mas
`post([Null])` dava `[None]` — ele delegava ao `repr` da list do Python, e o
`None`, que não existe na PoolScript, vazava para o usuário.

**Como foi resolvido:** do lado Python, que era o errado. `stringify()` passou
a renderizar lista, tupla e dict sozinho, recursivamente, e `str()` deixou de
ser o `str` do Python (senão `str(Null)` continuaria devolvendo `"None"`).
A VM não copiou o vazamento.

### Métodos de list e dict não existiam na VM

**O que não funcionava:** só na VM. `d.keys()`, `l.append(x)`, `.type()` —
nada disso rodava, porque `GET_MEMBER` só resolvia em instância de Entity.
Passou despercebido porque a contagem de progresso olhava builtins e métodos
de string, e métodos de coleção não estavam em nenhuma das duas listas.

**Como foi resolvido:** o `OBJ_METODO_NAT` (feito para string) ganhou um campo
`tabela`, e entraram `METODOS_LIST` (12), `METODOS_DICT` (11) e
`METODOS_UNIV` (o `.type()`, que vale em qualquer valor). O `pop`/`clear` de
dict exigiram uma remoção de chave que o dict compacto ainda não tinha —
lápide no índice e buraco no denso, pra ordem de inserção das outras chaves
não mudar.

### `map`/`filter` devolviam `[]` em silêncio

**O que não funcionava:** o interpretador. `filter([1,2], 5)` devolvia `[]` em
vez de erro — os dois ramos do laço (`UserFunction` e `callable`) não casavam
com um não-chamável, nenhum item era adicionado, e o resultado saía vazio como
se a lista de entrada é que estivesse.

O modo mais fácil de cair nisso é `map(l, str)`: nome nu de tipo resolve para
`PoolTypeRef.STR` (é o que faz `x is str` funcionar), não para a função.

**Como foi resolvido:** o `else` que faltava, levantando `TypeError`.

---

### `type(x)` e `x.type()` discordavam

**O que não funcionava:** os dois motores, do mesmo jeito.

```
type({"a": 1})  ->  "json"     {"a": 1}.type()  ->  "dict"
type((1, 2))    ->  "tuple"    (1, 2).type()    ->  "tup"
```

Duas implementações independentes no interpretador que ninguém tinha
comparado. Os outros tipos batiam.

**Como foi resolvido:** padronizado em `dict` e `tup` nos dois. A palavra-chave
`json` continua existindo — `json d = {}` e `d is json` são sintaxe, não nome
de tipo devolvido.

### `jwt.gen` mentia no header do token

**O que não funcionava:** o interpretador. `jwt.gen(payload, chave, "RS256")`
escrevia `"alg":"RS256"` no header e assinava com **HS256** mesmo assim.

Quem verificasse confiando no `alg` tentaria verificação RS256 num token HMAC.
É a classe de confusão de algoritmo que já rendeu CVE em várias bibliotecas de
JWT — e achado só porque o lado C recusou e o diferencial acusou.

**Como foi resolvido:** os dois recusam algoritmo que não seja HS256, em vez de
aceitar e assinar com outro.

### `PoolFile` imprimia a classe do Python

**O que não funcionava:** o interpretador. `post(PoolFile)` saía como
`<class 'poolscript.stdlib.os_lib.PoolFile'>`, e `type(PoolFile)` respondia
`action` — quando o irmão dele, `str`, imprime `str` e responde `type`.

Além de inconsistente, o texto expõe o caminho do módulo Python que
implementa a linguagem. Nada disso existe na PoolScript, e num binário sem
CPython a frase é simplesmente mentira.

**Como foi resolvido:** referência de tipo sai pelo nome (`PoolFile`) e se
declara `type` nos dois motores. Na VM ela é um `V_TIPO` como qualquer outro,
o que faz `x is PoolFile` funcionar sem `import os`, igual ao interpretador.

### `catch` com tipo engolia o erro que não casava

**O que não funcionava:** a VM. `try { 1/0 } catch (KeyError e) { ... }` —
o erro não é KeyError, nenhum catch casa… e a execução **seguia em frente**
como se nada tivesse acontecido. O `try` de fora nunca via o erro; qualquer
`catch` tipado virava um catch-tudo silencioso.

**Como foi resolvido:** opcode `RERAISE`. Quando a cadeia de catches termina
sem casar, a mensagem E o tipo voltam a subir — o `finally` roda antes, e o
`try` externo (ou o topo do programa) recebe o erro original com o tipo
intacto.

### `return` dentro de `try` deixava o handler armado

**O que não funcionava:** a VM. Uma action que retornasse de dentro de um
`try` deixava o handler registrado depois do frame morrer. O próximo erro do
programa — em qualquer lugar — caía no `catch` de uma função que já tinha
retornado, executando código fora de contexto.

**Como foi resolvido:** o `RETURN` desarma todo handler registrado no frame
que está morrendo (e nos de cima, no caso de retorno através de frames).

### Os dois motores davam nomes diferentes ao mesmo erro

`post(1/0)` era `SomeValueUnexpected` no interpretador e `ZeroDivisionError`
na VM; `1 + "a"` era `AtributtedValueError` num e `TypeError` no outro. Não é
estética: `catch (SomeValueUnexpected e)` **funcionava num motor e não no
outro** — o mesmo script tratava o erro aqui e abortava lá.

**Como foi resolvido:** a VM adotou a tabela do interpretador, que é a
documentada no LANGUAGE.md. Os nomes internos do Python (`TypeError`,
`NameError`…) não existem na linguagem e não voltam a aparecer.

### Nome todo em maiúsculo não pode ser atribuído

`PI = 3.14` é `SyntaxError` nos dois motores, mas `pi = 3.14` funciona. A
atribuição simples só reconhece `IDENT` no parser, não `IDENT_UPPER` — que
existe para distinguir nome de Entity. Constante em caixa alta é convenção
comum, e hoje a linguagem a proíbe sem dizer por quê.

---

## Em aberto

### O motor de regex — CORRIGIDO (praticamente completo vs `re` do Python)

Fechado nesta leva (motor C bate com o `re` — `test_regex_recursos_avancados`):

```
(a)\1  \1..\9    retrovisor — A_BACKREF casa o span já capturado
(?P<nome>a)      grupo nomeado — capturado como numerado (findall/sub batem)
(?i)(?m)(?s)     flags inline GLOBAIS
(?i:...)         flags com ESCOPO (por-átomo, carimbadas no parse)
\b \B \A \Z      âncoras (fronteira de palavra, início/fim de string)
(?=) (?!)        lookahead / lookahead negativo
(?<=) (?<!)      lookbehind / negativo (largura fixa em codepoints, como o re)
IGNORECASE       dobra ASCII + Latin-1 (café/CAFÉ, ção, ñ)
```

**Ainda em aberto** (recusados na compilação, nunca casam errado em silêncio):

```
(?>...)          grupo atômico
(?(id)a|b)       condicional
(?#...)          comentário inline
[a\D]            \D/\W/\S negado DENTRO de []
IGNORECASE       fora de ASCII+Latin-1 (grego/cirílico não dobram)
```

O resto (classes, quantificadores gulosos/preguiçosos, alternância, `\d\w\s`)
bate com o Python, verificado caso a caso.

### `str`/`int` como valor de primeira classe — CORRIGIDO

**Era:** `f = str` não funcionava (nome nu de tipo virava `PoolTypeRef`/`V_TIPO`
não-chamável), então `map(l, str)` e `filter(l, bool)` falhavam.

**Como foi resolvido:** o tipo, chamado, converte usando o MESMO conversor da
chamada direta `str(...)`. Interp: `PoolTypeRef.__call__` delega a
str/int/float/bool/list e `ps_type`. VM: `OP_CALL`/`chama_valor` em `V_TIPO`
despacham via `tipo_conversor` pra `nativa_*`. `json`/`dict`/`tup` recusam com
erro claro. Regressão diferencial em `tests/test_binario_c.py`.

Junto saiu um bug do `is` entre tipos: como `PoolTypeRef` é subclasse de `str`,
`int is str` dava True e `int is int` dava False. Agora tipo-vs-tipo é
IDENTIDADE nos dois motores (`str is str`/`int is int` True, cruzados False,
`X is type` True, `json`==`dict`).

### `jinker` e `ws_connect` na VM são single-thread

O servidor `jinker` do binário roda num event loop `poll` de uma thread só (o
handler `.ps` reentra na VM sem thread nem GC concorrente). Isso atende HTTP e
WebSocket concorrentes de verdade — salas, broadcast e `emit`/`exclude_self`
batem com o interpretador —, mas uma requisição HTTP com `keep-alive` segura o
loop enquanto está sendo servida. Pro alvo da linguagem (API/app pequeno) é
aceitável; um servidor de altíssima concorrência não é o caso de uso.

O **`ws_connect`** (cliente WebSocket) está implementado no binário — mesmo
framing do servidor, com a máscara obrigatória do lado cliente. A diferença
pro interpretador é QUANDO o `on_message` dispara: lá uma thread entrega em
background; aqui as mensagens pendentes são entregues nas operações da
conexão (antes de cada `send` e no `close`). Num script que envia, espera e
fecha, a saída é idêntica; um script que só dorme esperando mensagem sem
nunca tocar a conexão não recebe callback — esse é o limite do modelo sem
thread, catalogado de propósito.

### `async`/`await` — fora de escopo, não pendente

Continua no interpretador, que roda `async action` em thread de verdade. Na
VM não entra: exigiria um modelo de concorrência convivendo com o GC de
marcação, e essa escolha custa mais que a ausência. Não está na fila de
trabalho — está fora dela.

`yield` era o caso vizinho e **está pronto**: gerador não precisa de thread,
só de frame suspensível.
