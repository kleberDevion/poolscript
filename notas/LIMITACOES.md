# Limitações da linguagem — achadas, corrigidas, e as que faltam

Este arquivo é **só sobre defeitos**: `.ps` que deveria funcionar e não
funciona, ou que funciona errado em silêncio.

Quase sempre aparecem por acaso, escrevendo `.ps` de teste para outra coisa —
por isso ficam agrupadas aqui em vez de espalhadas. Cada uma tem regressão em
`teste/` e o arquivo serve de fila de trabalho.

## Como corrigir uma

Uma limitação atravessa a linguagem inteira, então o remendo em um só lugar
deixa o resto inconsistente. A ordem que funciona:

1. `vm/ps_ast.h` / `.c` — `N_<NOME>` e o nome legível, se for nó novo
2. `vm/ps_parser.c` — o reconhecimento
3. `vm/ps_compiler.c` — emissão de bytecode
4. `vm/poolscript_vm.c` — opcode novo, se precisar
5. `teste/casos_*.c` — a regressão (o comportamento CERTO, escrito)
6. `docs/` — a página do que mudou
7. `psl-poolscript-vsix/` — senão o editor acusa erro de sintaxe em código
   válido

Depois: `make check`.

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
não por lookahead: se o que saiu do parser de expressão é um `IndexAccess` e o
token seguinte é de atribuição, o nó vira `IndexAssignment`.

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

**O que não funcionava:** `[1,2] + [3]` (concatenar) e `[1,2] * 3` (repetir)
levantavam `'+' entre tipos incompatíveis`.

Achado rodando o stress de AddressSanitizer do lote de objetos — um caso do
próprio stress falhou, e o que parecia erro do script era defeito real.

A concatenação exige o **mesmo tipo**: `[1] + (2,)` é erro, igual ao Python.
Repetição com contagem ≤ 0 dá sequência vazia.

### Chave de dicionário só podia ser string ou identificador

**O que não funcionava:** `{1: "a"}`, `{True: 1}`, `{1.5: "x"}` — todos
`SyntaxError`. Mas `d[1] = "a"` sempre funcionou — as duas formas de escrever
a mesma coisa estavam em desacordo.

**Como foi resolvido:** a chave passou a ser uma expressão qualquer.
`{1+1: "x"}` e `{(1,2): "t"}` saem de graça.

### Ordem de inserção do dict na VM

**O que não funcionava:** `post({"b":1,"a":2})` saía
`{'a': 2, 'b': 1}` — a tabela hash de endereçamento aberto guardava as
entradas na ordem do hash, e a linguagem perdia a ordem de inserção.

Não é detalhe estético: é o que faz a saída de um `.ps` ser reproduzível. E
contaminava tudo que percorre dict — `post`, `str()`, `list(d)`, JSON.

**Como foi resolvido:** o `PSDict` virou **dict compacto** — um array denso
em ordem de inserção mais uma tabela `indices` que
resolve o hash para a posição no denso. Sobrescrever uma chave existente não
muda a posição dela na ordem.

### Erro de builtin escapava do `try`

**O que não funcionava:** `try { post(len(1)) } catch (e) { ... }`
não capturava — o caminho de chamada nativa saía por `return -1`, abortando a
execução em vez de desviar para o desenrolamento.

Só apareceu quando os builtins passaram a ser vários: com `post` e `len`
apenas, quase nada falhava.

**Como foi resolvido:** o erro de builtin agora vai para o mesmo
`goto erro_runtime` do resto da VM, e o builtin escolhe o tipo do erro
(`TypeError`, `SomeValueUnexpected`, `IndexError`…) que o
`catch (Tipo e)` compara.

### Float impresso com um dígito de lixo

**O que não funcionava:** `post(1/3)` dava `0.33333333333333331` em vez de
`0.3333333333333333` — `%.17g` sempre volta ao mesmo double, mas não é a MENOR
representação que volta.

**Como foi resolvido:** `float_para_texto()` tenta precisão 1 a 17 e para na
primeira que faz `strtod` devolver o valor original; depois cola `.0` se não
sobrou `.` nem expoente. É a regra do `repr` do Python, e substituiu duas
cópias da formatação antiga.

### `Null` aninhado imprimia `None`

**O que não funcionava:** `post(Null)` dava `null`, mas `post([Null])` dava
`[None]` — um nome que não existe na PoolScript vazava para o usuário.

**Como foi resolvido:** a impressão de lista, tupla e dict passou a ser
recursiva e própria da linguagem, em vez de delegar pra representação de outra
runtime — `Null` sai `null` em qualquer profundidade.

### Métodos de list e dict não existiam na VM

**O que não funcionava:** `d.keys()`, `l.append(x)`, `.type()` —
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

**O que não funcionava:** `filter([1,2], 5)` devolvia `[]` em
vez de erro — nenhum ramo do laço casava com um não-chamável, nenhum item era
adicionado, e o resultado saía vazio como
se a lista de entrada é que estivesse.

O modo mais fácil de cair nisso é `map(l, str)`: nome nu de tipo resolve para
a REFERÊNCIA de tipo (é o que faz `x is str` funcionar), não para a função.

**Como foi resolvido:** o `else` que faltava, levantando `TypeError`.

---

### `type(x)` e `x.type()` discordavam

**O que não funcionava:**

```
type({"a": 1})  ->  "json"     {"a": 1}.type()  ->  "dict"
type((1, 2))    ->  "tuple"    (1, 2).type()    ->  "tup"
```

Dois caminhos independentes que ninguém tinha comparado. Os outros tipos
batiam.

**Como foi resolvido:** padronizado em `dict` e `tup`. A palavra-chave
`json` continua existindo — `json d = {}` e `d is json` são sintaxe, não nome
de tipo devolvido.

### `jwt.gen` mentia no header do token

**O que não funcionava:** `jwt.gen(payload, chave, "RS256")` escrevia
`"alg":"RS256"` no header e assinava com **HS256** mesmo assim.

Quem verificasse confiando no `alg` tentaria verificação RS256 num token HMAC.
É a classe de confusão de algoritmo que já rendeu CVE em várias bibliotecas de
JWT.

**Como foi resolvido:** algoritmo que não seja HS256 é recusado, em vez de
aceito e assinado com outro.

### `PoolFile` imprimia a classe do Python

**O que não funcionava:** `post(PoolFile)` saía como um caminho de módulo
interno, e `type(PoolFile)` respondia `action` — quando o irmão dele, `str`,
imprime `str` e responde `type`.

Além de inconsistente, o texto expunha detalhe de implementação que não existe
na linguagem.

**Como foi resolvido:** referência de tipo sai pelo nome (`PoolFile`) e se
declara `type`. Ela é um `V_TIPO` como qualquer outro, o que faz
`x is PoolFile` funcionar sem `import os`.

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

### Nomes de erro que não eram os da linguagem

`post(1/0)` levantava `ZeroDivisionError` e `1 + "a"` levantava `TypeError` —
nomes que não estão na tabela da linguagem. Não é estética:
`catch (SomeValueUnexpected e)` **não pegava o
outro** — o mesmo script tratava o erro aqui e abortava lá.

**Como foi resolvido:** a VM passou a usar a tabela documentada no
`docs/LANGUAGE.md`. Nome de erro fora dessa tabela não existe na linguagem e
não volta a aparecer.

### Nome todo em maiúsculo não pode ser atribuído

`PI = 3.14` é `SyntaxError`, mas `pi = 3.14` funciona. A
atribuição simples só reconhece `IDENT` no parser, não `IDENT_UPPER` — que
existe para distinguir nome de Entity. Constante em caixa alta é convenção
comum, e hoje a linguagem a proíbe sem dizer por quê.

---

### Fatia e índice de string em BYTES na VM

**O que não funcionava:** `"padrão"[0:5]` dava `padrã` em vez de `padrão`;
`"padrão: str"[5]` devolvia meio caractere (byte quebrado);
`s[s.find("x"):len(s)]` perdia o fim da string sempre que havia acento antes.
`len`/`find` já contavam CARACTERES, só `[]`/`[a:b]` contavam bytes — e a
combinação (índice de `find` usado numa fatia) corrompia texto em silêncio.

Achado em 2026-08-24 escrevendo um script de apoio em PoolScript
(reescrita de páginas da doc): as linhas com acento saíam truncadas.

**Como foi resolvido:** `utf8_byte_de(s, len, cp)` (codepoint → byte) em
`vm/poolscript_vm.c`; `OP_SLICE` e `OP_INDEX_GET` normalizam contra
`utf8_conta` e copiam por codepoint (fatia com passo 1 vira uma faixa contígua
de bytes). Regressão em `teste/`.

### `find`/`rfind`/`index`/`rindex`/`count` sem `inicio`/`fim` na VM

**O que não funcionava:** a doc prometia `s.find(sub, inicio=0)` e a VM
recusava com `find() espera 1 argumento(s)`. Buraco que nenhum teste
cobria porque nenhum teste passava o 2º argumento.

**Como foi resolvido:** `faixa_busca()` converte `inicio`/`fim` (em caracteres,
negativo conta do fim, satura; `inicio` além do tamanho → -1 como no Python)
pra uma faixa de bytes; os cinco métodos aceitam de 1 a 3 argumentos. As
páginas de `docs/string/` e a seção 12 da referência foram atualizadas, com
exemplos de faixa. Regressão em `teste/`.

### Comentário (ou linha vazia) como 1ª linha de um bloco `:`

**O que não funcionava:**

```
action f(x) {
    // comentário
    return x
}
```

dava `SyntaxError: faltou indentação após ':'`. O lexer não mexe na pilha de
indentação em linha só-comentário (certo), mas já tinha emitido o `NEWLINE`
dela — o parser via `: NEWLINE NEWLINE INDENT` e exigia `INDENT` logo após o
primeiro `NEWLINE`.

**Como foi resolvido:** no parser (`ps_parser.c`, bloco
comum e corpo de Entity), depois do `NEWLINE` obrigatório pulam-se os
`NEWLINE` extras antes de exigir o `INDENT`. Regressão:
`teste/` (action, if, for each, Entity, `#` e `//`).

### HEAD não existia em lugar nenhum (cliente nem servidor)

**O que não funcionava:** o método HTTP HEAD, nos três pontos onde ele aparece:

- `request.head(url)` / `requests.head(url)` — **não existia** (a lib tinha
  get/post/put/patch/delete e parou aí), embora o cliente HTTP já soubesse que
  HEAD não tem corpo (`ps_http.c`: `sem_corpo`);
- **jinker servindo HEAD** — respondia **404**, porque o método não casava com
  a rota de GET no laço de roteamento.

**Como foi resolvido:** cliente — `mod_req_head` na tabela `MOD_REQUEST`
(remapeia pro `request_comum` com método "HEAD"); o alias `request`/`requests`
exporta os dois. Servidor — uma rota que aceita GET responde HEAD com os MESMOS
headers (Content-Length inclusive) e **sem corpo** (RFC 9110):
`PSJkConn.sem_corpo` (ligado ao ler a requisição, checado no `ps_jk_responde`)
mais uma segunda tentativa de casamento com "GET".

### `multipart/form-data` não tinha como ser enviado

**O que não funcionava:** mandar arquivo pra uma API (upload, transcrição de
áudio) era impossível: `body=` só fazia JSON/texto/bytes, e montar o multipart
na mão em PoolScript não dá (precisa de boundary + bytes crus do arquivo).

**Como foi resolvido:** `fields=` (campos do formulário) e `file=`
(`{campo: {"name": caminho}}`) em get/post/put/patch/delete — a lib lê o arquivo
(absoluto | pasta do script | cwd), monta as partes e põe o `Content-Type` com o
boundary gerado (um `Content-Type` manual é descartado, senão o boundary não
bateria). `body=` junto com `fields=`/`file=` é erro claro.

### `--check`

`pool --check arq.ps` analisa sem executar — é o que um editor consome.

**Como foi resolvido:** `cmd_check` no `vm/main.c` responde um JSON de uma
linha (`{"ok":true}` /
`{"ok":false,"tipo":...,"msg":...,"linha":N,"coluna":N}`), lendo de arquivo ou
da entrada padrão, e no `--help` dos dois. Regressão:
`teste/casos_linguagem.c` (inclusive "não executa o script").

### Tupla podia ser MUTADA no VM

**O que não funcionava:** tupla é imutável — e no VM não era. Ela caía na
tabela de métodos da LISTA inteira (o `EH_SEQ` do `acha_metodo_valor` casa
lista E tupla), então `(1,2,3).append(9)` devolvia `(1,2,3,9)`, e `sort`,
`clear`, `pop`, `remove`, `insert`, `extend`, `reverse` mexiam na tupla do
mesmo jeito; `.copy()` devolvia uma **lista**. Os nove tinham que ser
recusados com "membro inexistente".

Achado em 2026-08-24 ligando o completion de dict/list/tup no editor: pra
listar os métodos de `tup` eu fui ler a tabela do VM e ela era a da lista.

**Como foi resolvido:** `METODOS_TUPLA` própria, só com os cinco de LEITURA
(`index`, `count`, `contains`, `has`, `len`), e `EH_TUPLA` testado **antes** do
`EH_SEQ` no `acha_metodo_valor`. Regressão: os 14 métodos, mais a lista
intacta.

### `regex.compile` liberava o padrão que o objeto ainda usava

**O que não funcionava:** ao criar o `Pattern` de `regex.compile()`, a segunda
chamada de `.sub()`/`.findall()` no mesmo objeto dava **segfault**. Causa:
`rx_sub` e `rx_findall` faziam `ps_regex_free(r)` no fim — elas tomavam posse
do PSRegex, o que era invisível enquanto só as funções soltas do módulo (que
compilam e jogam fora) as chamavam. Com um padrão compilado guardado no
objeto, o primeiro uso liberava e o segundo lia ponteiro solto.

**Como foi resolvido:** posse única e explícita — **quem compila, libera**.
Os helpers `rx_sub`/`rx_findall`/`rx_split` não liberam mais nada; cada
`mod_regex_*` chama `ps_regex_free` depois de usar, e o `Pattern` mantém o
seu vivo até o GC (finalizer `fin_regex`). Regressão:
`teste/` (inclui `sub` e `split` do mesmo objeto na mesma
linha, que era o repro, e 2000 compiles pro finalizer).

### Leva de 2026-08-25: o que a caça com agentes achou e foi corrigido

77 achados confirmados. Esta seção registra o que já está FECHADO; o que
falta vive em `teste/casos_pendentes.c`, cada caso falhando de propósito com o
comportamento certo escrito.

**Derrubavam o processo** (`teste/casos_crash.c`):

- imprimir lista/dict que contém a si mesmo → SEGFAULT. A recursão da
  impressão descia até estourar a pilha do C. Agora detecta o CICLO e imprime
  `[...]`, como o Python (`ps_em_ciclo` + pilha de ponteiros em
  `escreve_valor`/`valor_para_texto`, zerada a cada topo).
- `==`/`contains` entre estruturas mutuamente recursivas → SEGFAULT. Teto de
  profundidade em `val_iguais`.
- `-9223372036854775808 % -1` → **SIGFPE** (core dumped): é UB no C. O resto
  correto é 0.
- `"a".zfill(9223372036854775807)` → a VM alocava em laço até o OOM killer
  derrubar a SESSÃO da máquina. Teto de 256 MB (`PS_STR_MAX`) em
  zfill/ljust/rjust/center.
- `import` de módulo `.ps` dentro de `async action` → SEGFAULT. O `import`
  realoca `vm->protos` e as OUTRAS fibras seguiam com o `p` pendurado. Agora
  toda chamada nativa e todo `await` reancoram o ponteiro (macro `REANCORA`).

**Rodavam errado, calado** (`teste/casos_inteiros.c`, `teste/casos_erros.c`):

- `1 << 63` virava NEGATIVO e `1 << 64` virava 1 (shift em int64 = UB). Agora
  `<<`, `>>`, `|`, `^`, `&` promovem a bignum como `+` e `*` já faziam.
- bignum era recusado por `|`, `>>`, `abs`, `sum`, `sorted`, `max`, `min`,
  `round`, `flo`, `int` com "exige int" — enquanto o `type()` do MESMO valor
  respondia `int`. Todos aceitam agora.
- `abs(-9223372036854775808)` devolvia número NEGATIVO.
- `int("<32 dígitos>")` dava a volta em silêncio; `json.parse` de inteiro
  grande SATURAVA em INT64_MAX. Os dois vão pro bignum.
- a VM aceitava LISTA e DICT como chave de dicionário (valor mutável!).
  `dict_set` recusa, com mensagem própria.

**Não funcionavam** (`teste/casos_linguagem.c`):

- `finally` não rodava com `return`, `break`, `continue` nem com `raise`
  dentro do `catch` — o bloco só era emitido inline nas duas saídas "normais".
  Agora o compilador mantém uma pilha de `finally` pendentes e emite antes de
  cada salto; um try interno cobre o `raise` de dentro do catch.
- `for each i` DESTRUÍA a variável (ou a action!) de mesmo nome de fora. Agora
  sombreia: salva o valor anterior e devolve na saída.
- f-string com UMA expressão pulava o `BUILD_STR` "por otimização" e devolvia o
  valor cru — `f"{lista}"` era a PRÓPRIA lista (mutar o resultado mutava o
  original) e `type()` dizia `list`.
- `bytes[1:3]` respondia "tipo nao fatiavel".
- `f.read(3)` lia em blocos de 4096 e descartava o excedente: o cursor ia pro
  fim e a leitura seguinte vinha vazia.
- `@NonNull` dentro de Entity era descartado junto com o nó do decorador (o
  corpo da Entity só compilava `N_ACTION_DECL`) — mesmo buraco do `@static`.
- `import pacote.modulo`, documentado em `docs/linguagem/09-imports.md`, era
  `NotImplementedError`. Liga o último segmento, como a doc diz.

### PostgreSQL: o `?` do parâmetro nunca virava `$1`

**O que não funcionava:** `cursor.execute("... WHERE id = ?", (1,))` no driver
`postgres` chegava CRU no servidor e dava `syntax error at end of input`. A doc
promete `?` como placeholder em todos os drivers e o próprio comentário do
código dizia "troca cada `?` por $1,$2" — mas o laço procurava `%s`. No MySQL
funcionava, então passava por "problema do Postgres".

Achado exercitando `psodbc` contra um PostgreSQL de verdade
(`teste/e2e/db.ps`); nenhum teste de unidade pegaria, porque o erro só existe
no servidor.

**Como foi resolvido:** a conversão passou a trocar `?` (e `%s`, que continua
aceito) por `$1,$2,...`, PULANDO o que está entre aspas simples — um `?` dentro
de texto (`WHERE t = 'e ai?'`) não é placeholder.

### `MailReader.search()` e `.body()` eram esqueleto que sempre levantava

**O que não funcionava:** os dois. O corpo do método era literalmente

```c
MERRO(vm, "RuntimeError", "search() exige conexao IMAP ativa");
```

com o comentário "a implementação completa fica pro dia do servidor de teste".
Conectar, logar e `select()` funcionavam — depois disso o leitor não fazia
nada. As duas páginas de doc descreviam o retorno em detalhe (lista de dicts
com `id`/`from`/`subject`/`date`, e `body` com `include_body=true`), então a
doc prometia uma coisa que não existia.

A camada C já estava pronta (`ps_imap_search`, `ps_imap_fetch`,
`ps_mime_header`, `ps_mime_decodifica_header`, `ps_mime_corpo`) — só o método
da linguagem nunca foi ligado nela.

**Como foi resolvido:** `search()` faz o SEARCH, corta pelo `limit` pegando os
MAIS RECENTES (o IMAP devolve em ordem crescente, então a lista é lida de trás
pra frente), e um FETCH por id montando o dict; `From`/`Subject` passam pelo
decodificador RFC 2047. `body(id)` faz o FETCH RFC822 e extrai o texto pelo
parser MIME. A I/O dos dois cede a fibra (`fib_offload`), como o `select` já
fazia.

### IMAP: linha longa virava "conexao IMAP caiu"

**O que não funcionava:** `search("ALL")` numa caixa real. O leitor de linha
tinha buffer fixo e devolvia -1 quando a linha não cabia; quem chamava
traduzia isso pra "conexao IMAP caiu". A conexão estava boa — a LINHA é que era
grande: o `* SEARCH` de uma caixa com milhares de mensagens vem em UMA linha
com todos os ids, dezenas de kB.

Mensagem de erro apontando pro lugar errado é pior que erro nenhum: manda
investigar rede quando o problema é buffer.

**Como foi resolvido:** `le_linha_din` — buffer que cresce e ESCOA o que já
chegou em vez de desistir. Linha de tamanho arbitrário.

### IMAP: termo de busca acentuado dava "BAD Could not parse command"

**O que não funcionava:** `search("SUBJECT", "relatório")`. O termo ia entre
aspas, e o quoted-string do IMAP é 7-bit por definição (RFC 3501) — byte
acima de 0x7F ali é sintaxe inválida. O `CHARSET UTF-8` no comando não
conserta isso.

**Como foi resolvido:** termo com byte não-ASCII vai como **literal**
(`SUBJECT {12}` → o servidor responde `+` → os bytes crus seguem). Acento e
emoji funcionam; o caminho com aspas continua para termo ASCII, que é o comum.

## Em aberto

### Escrita silenciosa no módulo — RESOLVIDA COM AVISO (2026-09-05)

**Não era defeito: era design, e 31 casos da suíte dependem dele.** O que
faltava era não ser silencioso. Fica registrado inteiro porque o caminho até
descobrir isso é a parte útil.

Escrever dentro de uma função num nome que existe no módulo agora emite

```
arquivo.ps:3: SyntaxWarning: 'i' existe no modulo: esta atribuicao ESCREVE
NELE, nao cria uma local. Use 'global i', ou outro nome
```

no terminal e no `--check` (sublinhado amarelo no editor). Semântica intacta;
`global` cala o aviso; parâmetro não dispara.

**Por que NÃO viramos a semântica pro Python.** A tentativa foi feita e
medida: trocar `OP_STORE_NAME` pra sempre criar local quebra 13 casos, e — pior
— produz um TERCEIRO comportamento que não é o de ninguém:

| `a = 1; action inc() { a = a + 1  return a }; post(inc(), inc())` | |
|---|---|
| PoolScript | `2 3` (contador em closure, 26 casos dependem) |
| só trocando a ESCRITA | `2 2` — não bate com ninguém |
| CPython | `UnboundLocalError` |

Pra ser CPython de verdade a LEITURA teria que mudar junto (nome atribuído em
qualquer ponto da função é local no corpo inteiro), e aí o idioma do contador
morre. Decisão dele: fica o design, entra o aviso.

O texto abaixo é o relato original de quando eu achava que era defeito.

---

### (histórico) Atribuição dentro de action SOBRESCREVE a variável de módulo

Achado em 2026-09-05, e é o mais caro desta lista: **não dá erro, dá resultado
errado.**

```ps
action f() {
    i = 99
    return i
}
i = 0
while i < 3 {
    post("volta", i, "| f() =", f())
    i = i + 1
}
post("i no fim:", i)
```

| | |
|---|---|
| PoolScript | `volta 0 \| f() = 99` · `i no fim: 100` — **o laço rodou UMA vez** |
| CPython | três voltas, `i no fim: 3` |

O `i = 99` dentro da action escreveu no `i` do módulo. O laço então fez
`100 = 99 + 1` e terminou.

**O que é e o que não é:**

- **parâmetro NÃO vaza** — `action g(i)` com `i = 5` no módulo deixa o de fora
  intacto. Só a atribuição a nome livre vaza.
- **`global` faz exatamente o mesmo** que a atribuição comum. A única coisa que
  ele acrescenta hoje é *criar* uma global que ainda não existe.

**A doc se contradiz no mesmo parágrafo (§4.7):** diz que sem `global`,
`contador = ...` dentro da função "criaria uma local e a de fora ficaria em 0",
e logo em seguida que "apenas reatribuir uma global que já existe funciona sem
`global` — o write-through de 4.6.2". As duas frases não podem valer juntas.

**Custo real:** travou o `scripts/conserta_barra_doc.ps` por meia hora, sem
erro nenhum. A action tinha um `i` local; quem chamava tinha um `i` de laço; o
laço nunca terminava. Qualquer nome comum (`i`, `n`, `x`, `linha`, `caminho`)
tem esse risco em qualquer arquivo com mais de uma função.

**DECIDIDO** (ver acima): fica o design, entra o aviso. As opções eram:
ou (a) função passa a criar local, como o CPython — e `global` volta a ter
razão de existir; ou (b) o write-through fica e a doc é corrigida pra dizer
isso sem se contradizer, e o `global` é documentado como redundante.

### Sem list comprehension (design, não defeito)

`[f(x) for x in xs]` é `SyntaxError: faltou ']' na lista` — coerente, a doc
nunca prometeu. Fica registrado porque é o tropeço imediato de quem vem do
Python (junto com "variável criada dentro do `if` não existe depois", que é
regra documentada na seção 4.6.1). Se um dia entrar, é feature da linguagem
(parser + compilador + VM + vsix), não conserto.

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

**Era:** `f = str` não funcionava (nome nu de tipo virava um `V_TIPO`
não-chamável), então `map(l, str)` e `filter(l, bool)` falhavam.

**Como foi resolvido:** o tipo, chamado, converte usando o MESMO conversor da
chamada direta `str(...)` — `OP_CALL`/`chama_valor` em `V_TIPO` despacham via
`tipo_conversor` pra `nativa_*`. `json`/`dict`/`tup` recusam com erro claro.
Regressão em `teste/`.

Junto saiu um bug do `is` entre tipos: `int is str` dava True e `int is int`
dava False. Agora tipo-vs-tipo é IDENTIDADE (`str is str`/`int is int` True,
cruzados False, `X is type` True, `json`==`dict`).

### `jinker` e `ws_connect` na VM são single-thread

O servidor `jinker` do binário roda num event loop `poll` de uma thread só (o
handler `.ps` reentra na VM sem thread nem GC concorrente). Ele **multiplexa**
o socket de escuta + todas as conexões WS + todas as conexões HTTP keep-alive,
então conexões ociosas NÃO seguram o loop (bug antigo: uma keep-alive parada
travava conexões novas por até 30s — corrigido, `test_jinker_keepalive_ocioso_
nao_bloqueia`). O que ainda vale: como é uma thread só, um **handler lento**
(query pesada, cálculo longo) bloqueia os outros ENQUANTO roda, e não usa
múltiplos núcleos. Pro alvo (API/app pequeno-médio) atende; multi-core exigiria
multi-processo (fork de workers) — trabalho futuro, não impedimento.

O **`ws_connect`** (cliente WebSocket) usa o mesmo framing do servidor, com a
máscara obrigatória do lado cliente. O limite é QUANDO o `on_message` dispara:
as mensagens pendentes são entregues nas operações da conexão (antes de cada
`send` e no `close`). Num script que envia, espera e fecha, a saída é a
esperada; um script que só dorme esperando mensagem sem nunca tocar a conexão
não recebe callback — limite do modelo sem thread, catalogado de propósito.

`yield` era o caso vizinho e **está pronto**: gerador não precisa de thread,
só de frame suspensível.

### Regex: repetição de grupo é recursiva (teto de ~6.000 caracteres)

O casador de `vm/ps_regex.c` é backtracking com continuação explícita, e a
recursão é o ciclo `m_seq → m_pos_grupo → m_rep_grupo → m_alt → m_seq`: **um
quadro de pilha C por caractere consumido**. Um padrão com repetição de grupo
— `(?:[^"\\]|\\.)*` é o caso típico — sobre alguns milhares de caracteres
esgota a pilha de 8 MB.

**O que já foi feito:** existia teto de PASSOS (`RX_MAX_PASSOS`, 2 milhões) e
ele não protegia disso, porque a pilha acaba muito antes de 2 milhões de
passos. O processo morria de SIGSEGV, sem mensagem nenhuma — achado escrevendo
o semeador do fuzzer em PoolScript, casando `"((?:[^"\\]|\\.)*)"` contra um
trecho de 29 mil caracteres de `teste/casos_diferencial.c`. Entrou
`RX_MAX_PROF` (6.000 quadros, folgado nos 8 MB) e agora o mesmo caso é
`RuntimeError: regex: backtracking demais`, com linha e coluna. Três casos de
regressão em `teste/casos_linguagem.c`.

**O que continua limitado:** o Python casa esse padrão sobre texto arbitrário;
aqui, acima de ~6.000 caracteres consumidos por uma repetição de grupo, o
resultado é erro em vez de resposta. Quem precisa varrer arquivo grande divide
por linha antes (é o que `teste/fuzz_semeia.ps` faz, e o comentário lá explica
por quê).

**Saída definitiva:** tornar a repetição de grupo ITERATIVA. O `*` de um átomo
simples já é um laço; é a repetição de GRUPO que recursa. Enquanto isso não
existir, o teto fica — e teto que avisa é melhor que morte calada.

---

## Não dá pra saber se um comando externo deu certo

**Onde bate:** `os.cmd()` e `os.run()` devolvem a **saída** (com
`capture=true`) ou `Null`. O **código de saída não é acessível** de lugar
nenhum da linguagem.

```
os.cmd("git push")            # devolve Null — deu certo? deu errado? não dá pra saber
os.run(["make", "check"])     # idem
```

Em Python é `subprocess.run(...).returncode`; em shell é `$?`; em Go é o
`error` do `cmd.Run()`. Aqui não existe equivalente.

**O contorno, que está espalhado pela suíte inteira:**

```
os.cmd("o comando aqui > /tmp/saida.txt 2>&1; echo $? > /tmp/rc.txt")
rc = int(open("/tmp/rc.txt").read().strip())
```

`teste/fuzz_replay.ps`, `teste/e2e/pkg.ps` e `teste/e2e_roda.ps` fazem
exatamente isso. É feio, escreve em `/tmp`, e obriga a passar por shell mesmo
quando o certo seria `os.run` (que existe justamente pra NÃO passar por shell).

**A armadilha que vem junto:** `Null == 0` é **True** neste motor — é
deliberado, o `val_iguais` documenta o porquê. Então quem escreve o teste
óbvio:

```
if os.cmd("comando") != 0 {      # NUNCA é verdade: Null != 0 é False
    post("falhou")
}
```

…escreve um teste que aprova qualquer coisa. Falso verde perfeito: parece
certo, passa na revisão, e não testa nada. Aconteceu aqui, no primeiro
rascunho do `teste/e2e/pkg.ps`.

**Saída:** é decisão de API dele. As formas usuais são um terceiro retorno
(`os.run(args, capture=true, check=false)` devolvendo `[rc, saida]`), um
`os.run(...).code`, ou um `check=true` que levanta exceção quando o comando
falha — que é o que o `subprocess.run(check=True)` faz e cobre a maioria dos
usos sem mudar assinatura.
