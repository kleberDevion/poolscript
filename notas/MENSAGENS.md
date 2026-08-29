# As mensagens que o usuário lê — 174 achadas, 74 mentem

Varredura de 29/08 com 15 agentes em cinco áreas (operadores, builtins, libs
em C, seções "## Erros" da doc, e prosa), cada uma com dois céticos: um
caçando o que o primeiro não viu, outro conferindo se a redação sugerida não
ficou **pior** que o original.

| gravidade | quantas | o que significa |
|---|---|---|
| **mentem** | 74 | levam pro caminho errado |
| não ajudam | 85 | corretas, e o usuário fica sem saber o que fazer |
| só mal escritas | 15 | entende-se, mas está feio |

**Como usar:** escreva na linha `→` a redação que você quer, ou risque o item
se discordar. Eu aplico o que estiver escrito e não toco no que ficar em
branco. Onde eu não escrevi sugestão, é porque a decisão é sua.

As regras de vocabulário que eu usei nas sugestões, e que você me corrigiu
hoje: o tipo se chama `str`, não "texto"; `int`/`flo`, não "número"; `list`,
não "lista"/"vetor"; `action`, não "função".

---

# MENTEM — levam pro caminho errado

O usuário lê, acredita, e vai consertar a coisa errada. São as que custam hora de depuração.

## doc: secoes "## Erros" — 27

### 1. `docs/string/get_json/get_json.md:11`

```
- **TypeError** — a `str` não contém um JSON válido
```

**Por que não serve:** `get_json()` NUNCA levanta erro — JSON inválido devolve `Null` calado, então o `catch` escrito a partir dessa linha nunca roda e o `Null` segue viagem sem ninguém perceber.

**Proposta:** Não levanta erro.

JSON inválido devolve `Null` — `"[1,2".get_json()` é `Null`, não é exceção. Compare o retorno com `Null` antes de usar; `try/catch` aqui não pega nada.

→ 

### 2. `docs/string/get/get.md:17`

```
- **TypeError** — a `str` não contém um JSON válido
```

**Por que não serve:** `get()` também nunca levanta erro: JSON quebrado E chave ausente devolvem `Null`, então quem seguir a doc escreve um `catch` morto.

**Proposta:** Não levanta erro.

Se a `str` não for um JSON de objeto, ou se a chave não existir, devolve `Null`. Teste o retorno em vez de usar `catch`.

→ 

### 3. `docs/builtins/removeEnd/removeEnd.md:17`

```
- **ValueError** — a lista está vazia, ou o argumento não é `list`
```

**Por que não serve:** As duas metades estão erradas: `removeEnd([])` não erra (devolve `Null`), e argumento que não é `list` levanta `TypeError`, não `ValueError`.

**Proposta:** - **TypeError** — o argumento não é `list` (nem `tup`: `removeEnd((1,2))` erra)

Lista vazia não é erro: `removeEnd([])` devolve `Null`.

→ 

### 4. `docs/builtins/removeStart/removeStart.md:17`

```
- **ValueError** — a lista está vazia, ou o argumento não é `list`
```

**Por que não serve:** Mesmo defeito do removeEnd: `removeStart([])` devolve `Null` sem erro, e tipo errado é `TypeError`, não `ValueError`.

**Proposta:** - **TypeError** — o argumento não é `list` (nem `tup`)

Lista vazia não é erro: `removeStart([])` devolve `Null`.

→ 

### 5. `docs/builtins/len/len.md:17`

```
- **TypeError** — `int`, `flo`, `bool` e `Null` não têm tamanho
```

**Por que não serve:** `len(Null)` devolve `0`, não erra — a linha manda o leitor proteger justamente o caso que funciona.

**Proposta:** - **TypeError** — `int`, `flo` e `bool` não têm tamanho

`len(Null)` é `0`, não é erro.

→ 

### 6. `docs/builtins/reversed/reversed.md:17`

```
- **TypeError** — o valor não é sequência (`str`, `list`, `tup` ou `bytes`)
```

**Por que não serve:** A lista de tipos erra nas duas pontas: `bytes` NÃO é aceito (levanta o erro) e `dict` é (devolve as chaves ao contrário).

**Proposta:** - **TypeError** — o valor não é `str`, `list`, `tup` nem `dict`

`bytes` não entra: passe `bytes.tolist(b)` antes. `dict` devolve as chaves na ordem inversa.

→ 

### 7. `docs/builtins/max/max.md:17`

```
- **ValueError** — a lista está vazia, ou os itens não se comparam entre si
```

**Por que não serve:** São dois erros de nomes diferentes espremidos numa linha só: vazia é `ValueError`, itens incomparáveis é `TypeError` — quem filtrar só `ValueError` deixa o segundo escapar.

**Proposta:** - **ValueError** — a lista está vazia; `max([])` não tem resposta
- **TypeError** — os itens não se comparam entre si (ex.: `int` com `str`)
- **TypeError** — o argumento não é `list` nem vários valores soltos (`max(5)` erra)

→ 

### 8. `docs/builtins/min/min.md:17`

```
- **ValueError** — a lista está vazia, ou os itens não se comparam entre si
```

**Por que não serve:** Mesmo caso do max: vazia é `ValueError`, incomparáveis é `TypeError`, e chamar sem nenhum argumento é um terceiro `TypeError` que a linha não cita.

**Proposta:** - **ValueError** — a lista está vazia; `min([])` não tem resposta
- **TypeError** — os itens não se comparam entre si (ex.: `int` com `str`)
- **TypeError** — `min()` sem argumento nenhum, ou `min(5)` com um valor que não é `list`

→ 

### 9. `docs/builtins/chr/chr.md:17`

```
- **TypeError** — o número está fora da faixa Unicode (0 a 0x10FFFF)
```

**Por que não serve:** Fora da faixa levanta `ValueError` (o tipo está certo, o valor é que não serve); o `TypeError` de verdade é `chr` receber algo que não é `int`.

**Proposta:** - **ValueError** — o código está fora da faixa Unicode (0 a 0x10FFFF); negativo também
- **TypeError** — `n` não é `int`

→ 

### 10. `docs/builtins/map/map.md:18`

```
- **TypeError** — fn não é chamável (ex.: `map(l, str)` — nome nu de tipo não é função)
```

**Por que não serve:** O exemplo não é mais verdade: `map(l, str)` funciona hoje e devolve os itens convertidos — a doc proíbe uma linha que roda.

**Proposta:** - **TypeError** — `fn` não é uma `action` (nem outra coisa que dá pra chamar): `map(l, 5)`
- **TypeError** — `lista` não é `list` nem `tup`

`map(l, str)` funciona: os nomes de tipo (`str`, `int`, `flo`) são chamáveis e convertem item a item.

→ 

### 11. `docs/builtins/range/range.md:19`

```
- **TypeError** — o argumento não é `int`
```

**Por que não serve:** `range(2.5)` funciona (trunca), então a linha proíbe o que o motor aceita — e some com o `ValueError` de `passo = 0`, que é o erro que as pessoas realmente batem.

**Proposta:** - **TypeError** — o argumento não é número: `str`, `list` e `dict` erram (`flo` passa, truncado)
- **ValueError** — `passo` é `0`; o range nunca terminaria

→ 

### 12. `docs/bytes/base64/base64.md:25`

```
- **AttributedValueError** — o argumento não é bytes.
```

**Por que não serve:** O motor levanta `TypeError` — um `catch (AttributedValueError e)` escrito a partir dessa linha nunca dispara e o erro vaza pro topo.

**Proposta:** - **TypeError** — o argumento não é bytes; a mensagem diz o tipo que chegou. Se você tem uma `str`, passe `bytes.new(s)`.

→ 

### 13. `docs/bytes/hex/hex.md:24`

```
- **AttributedValueError** — o argumento não é bytes.
```

**Por que não serve:** É `TypeError` que o motor levanta, não `AttributedValueError` — o `catch` da doc não pega nada.

**Proposta:** - **TypeError** — o argumento não é bytes. Converta antes: `bytes.hex(bytes.new(s))`.

→ 

### 14. `docs/bytes/tolist/tolist.md:25`

```
- **AttributedValueError** — o argumento não é bytes.
```

**Por que não serve:** O motor levanta `TypeError`; `AttributedValueError` aqui é um nome que nunca aparece.

**Proposta:** - **TypeError** — o argumento não é bytes; pra virar `list` de `int` a partir de uma `str`, faça `bytes.tolist(bytes.new(s))`.

→ 

### 15. `docs/bytes/toint/toint.md:28`

```
- **AttributedValueError** — o argumento não é bytes.
```

**Por que não serve:** O motor levanta `TypeError`, não `AttributedValueError`.

**Proposta:** - **TypeError** — o primeiro argumento não é bytes.

→ 

### 16. `docs/bytes/toint/toint.md:29`

```
- **TypeError** — `byteorder` inválido.
```

**Por que não serve:** `byteorder` errado levanta `ValueError` (o tipo está certo, o valor é que não serve) — e a linha não diz quais valores valem, que é a única informação útil ali.

**Proposta:** - **ValueError** — `byteorder` fora de `"big"` e `"little"`.

→ 

### 17. `docs/bytes/concat/concat.md:28`

```
- **AttributedValueError** — o argumento não é lista, ou algum item não é bytes.
```

**Por que não serve:** São dois casos diferentes numa linha só e nenhum dos dois é `AttributedValueError` — os dois saem como `TypeError`.

**Proposta:** - **TypeError** — o argumento não é `list`, ou um item dela não é bytes. A mensagem diz a posição e o tipo do item (ex.: `item 1 não é bytes (int)`).

→ 

### 18. `docs/bytes/frombase64/frombase64.md:25`

```
- **TypeError** — a `str` não é base64 válido.
```

**Por que não serve:** Base64 malformado levanta `ValueError`, não `TypeError` — o tipo está certo, o conteúdo é que não serve.

**Proposta:** - **ValueError** — a `str` não é base64 válido.

→ 

### 19. `docs/bytes/fromint/fromint.md:29`

```
- **AttributedValueError** — `n` não é inteiro, ou `length` não é inteiro.
```

**Por que não serve:** O motor levanta `TypeError`; além disso a linha repete "não é inteiro" duas vezes sem usar o vocabulário da linguagem (`int`).

**Proposta:** - **TypeError** — `n` não é `int`, ou `length` não é `int`.

→ 

### 20. `docs/bytes/get/get.md:29`

```
- **AttributedValueError** — `b` não é bytes, ou `i` não é inteiro.
```

**Por que não serve:** O motor levanta `TypeError`, e "inteiro" devia ser `int`.

**Proposta:** - **TypeError** — `b` não é bytes, ou `i` não é `int`.

→ 

### 21. `docs/bytes/new/new.md:29`

```
- **AttributedValueError** — tipo que não dá pra virar bytes (ex: `flo`), ou `bool` como tamanho.
```

**Por que não serve:** O motor levanta `TypeError`, e a linha não diz o que ENTRA — só o que não entra, deixando o leitor adivinhar.

**Proposta:** - **TypeError** — o valor não vira bytes. Aceita `str` (o conteúdo), `int` (o tamanho) e `list` de `int`; `flo` e `bool` não servem em nenhum dos dois papéis.

→ 

### 22. `docs/bytes/slice/slice.md:30`

```
- **AttributedValueError** — `b` não é bytes, ou `ini`/`fim` não são inteiros.
```

**Por que não serve:** O motor levanta `TypeError`; e falta a informação que evita um `try` desnecessário — índice grande demais não erra, a fatia sai cortada.

**Proposta:** - **TypeError** — `b` não é bytes, ou `ini`/`fim` não são `int`.

Índice além do tamanho não é erro: a fatia sai cortada no que existe, e negativo conta do fim.

→ 

### 23. `docs/bytes/xor/xor.md:33`

```
- **AttributedValueError** — `dados` ou `chave` não são bytes.
```

**Por que não serve:** O motor levanta `TypeError`, não `AttributedValueError`.

**Proposta:** - **TypeError** — `dados` ou `chave` não é bytes; a mensagem diz qual tipo chegou.

→ 

### 24. `docs/bytes/xor/xor.md:34`

```
- **TypeError** — a chave está vazia.
```

**Por que não serve:** Chave vazia levanta `ValueError` — o tipo está certo, o valor é que não serve.

**Proposta:** - **ValueError** — `chave` está vazia; o XOR precisa de pelo menos 1 byte pra repetir sobre os dados.

→ 

### 25. `docs/LANGUAGE.md:1123`

```
| `TypeError` | Tipo errado: operação entre tipos incompatíveis, aridade errada, RHS não-iterável em unpacking |
```

**Por que não serve:** Duas das três causas estão erradas: `"a" + 1` levanta `AttributedValueError`, e chamar uma `action` com o número errado de argumentos levanta `RuntimeError` — que nem aparece na tabela.

**Proposta:** | `TypeError` | Tipo errado onde a operação não aceita aquele tipo: `len(5)`, `list(Null)`, item não-`str` em `join`, lado direito não-iterável em unpacking |
| `RuntimeError` | Nome que não existe (`variável não definida`) e chamada de `action` com argumentos a mais ou a menos |

→ 

### 26. `docs/LANGUAGE.md:1122`

```
| `OutputUnexpectedValues` | Redeclaração no mesmo escopo; aridade errada em unpacking |
```

**Por que não serve:** Redeclarar no mesmo escopo NÃO erra (`int a = 1; int a = 2` roda e vale 2); esse código só sai quando a quantidade de valores do unpacking não bate.

**Proposta:** | `OutputUnexpectedValues` | Unpacking com quantidade errada de valores: `a, b = [1,2,3]` (valores demais) ou `a, b = [1]` (insuficientes) |

→ 

### 27. `docs/LANGUAGE.md:1121`

```
| `AttributedValueError` | Valor incompatível atribuído a variável tipada (ex.: `str x = 10`) |
```

**Por que não serve:** Falta o caso mais frequente de todos: operação entre tipos que não combinam (`"a" + 1`) também sai como `AttributedValueError`, e a linha de baixo manda o leitor procurar isso em `TypeError`.

**Proposta:** | `AttributedValueError` | Valor incompatível numa variável tipada (`str x = 10`) e operador entre tipos que não combinam (`"a" + 1`) |

→ 

---

## libs em C — 11

### 28. `vm/ps_db.c:416`

```
falha de conexão: connection to server at "localhost" (127.0.0.1), port 5432 failed: FATAL:  password authentication failed for user "u"\nconnection to server at "localhost" (127.0.0.1), port 5432 fai
```

**Por que não serve:** despeja o texto cru do libpq em ingles, com a mesma frase repetida duas vezes (o PQerrorMessage e multilinha) e cortada no meio de uma palavra.

**Proposta:** Pegar SO a primeira linha do PQerrorMessage, tirar o \n, e dar o contexto que o libpq nao da: snprintf(erro, ecap, "falha de conexão com o postgres em %s:%d (base '%s'): %.*s", host, porta, db, (int)prim_linha, msg) — ex.: falha de conexão com o postgres em localhost:5432 (base 'x'): usuario ou senha recusados

→ 

### 29. `vm/ps_db.c:452`

```
driver ainda nao suportado na VM
```

**Por que não serve:** "na VM" e vocabulario de quem escreve o motor; para quem programa em PoolScript so existe a linguagem, entao a frase soa como se houvesse outro lugar onde funciona.

**Proposta:** "driver reconhecido mas ainda sem implementacao — os que rodam hoje sao sqlite, postgres, mysql, mssql e mongo"

→ 

### 30. `vm/ps_jinker.c:126`

```
NetworkError: bind 127.0.0.1:1: Permission denied
```

**Por que não serve:** "bind" e o nome da chamada de sistema em C e "Permission denied" e o strerror em ingles — quem le nao descobre que o problema e porta abaixo de 1024.

**Proposta:** "nao consegui abrir a porta %d em %s: %s" com o motivo traduzido — EACCES: "porta abaixo de 1024 so abre como root; use uma porta acima de 1024"; EADDRINUSE: "ja tem outro programa escutando nessa porta"

→ 

### 31. `vm/ps_jinker.c:684`

```
servidor recusou o upgrade
```

**Por que não serve:** "upgrade" e o nome do cabecalho HTTP que negocia o WebSocket; quem chamou ws_connect nao sabe que existe um "upgrade".

**Proposta:** "o servidor respondeu HTTP normal e nao aceitou virar WebSocket — confira se o caminho e mesmo uma rota de socket"

→ 

### 32. `vm/poolscript_vm.c:13197`

```
Error: não conectado
```

**Por que não serve:** prefixo "Error:" em ingles, sai no stdout (nao e capturavel) e nao lembra que o ws_connect ja tinha falhado antes — quem le acha que o send e que quebrou.

**Proposta:** "o WebSocket nunca chegou a conectar em %s — a mensagem nao foi enviada" (com a url guardada em w->url)

→ 

### 33. `vm/ps_mail.c:55`

```
[Errno -2] Name or service not known
```

**Por que não serve:** e a saida crua do getaddrinfo em ingles, e pior: o -2 esta escrito na mao, entao QUALQUER falha de resolucao mostra esse numero mesmo quando o motivo foi outro.

**Proposta:** "nao consegui resolver o endereco do servidor de e-mail '%s' — confira o nome do host e a conexao"

→ 

### 34. `vm/ps_mail.c:330 (idem 340 e 349)`

```
MAIL FROM recusado / RCPT TO falhou / DATA recusado
```

**Por que não serve:** sao os nomes literais dos comandos do protocolo SMTP; o usuario pensa em remetente, destinatario e corpo, nao em MAIL FROM/RCPT TO/DATA.

**Proposta:** 330: "o servidor recusou o remetente '%s' — confira se o e-mail do from_address() e o mesmo da conta autenticada"; 340/342: "o servidor recusou o destinatario '%s'"; 349: "o servidor recusou receber o corpo da mensagem"

→ 

### 35. `vm/ps_regex.c:380`

```
regex: grupo especial nao suportado (?:...) (?P<n>...) (?ims:) (?= ?! ?<= ?<!) (posicao 1)
```

**Por que não serve:** a lista que vem depois de "nao suportado" e exatamente a lista do que E suportado — quem le entende o contrario do que a frase quer dizer.

**Proposta:** "regex: '(?' seguido de algo que nao reconheco — os grupos aceitos sao (?:...), (?P<nome>...), (?<nome>...), (?i)/(?m)/(?s), (?i:...), (?=...), (?!...), (?<=...) e (?<!...)"

→ 

### 36. `vm/ps_xlsx.c:313`

```
TypeError: xlsx sem xl/worksheets/sheet1.xml
```

**Por que não serve:** expoe o caminho interno do zip do formato OOXML; pro usuario o que aconteceu e simplesmente que o arquivo nao e uma planilha valida.

**Proposta:** "'%s' nao parece uma planilha xlsx valida (nao achei a primeira aba dentro dele)"

→ 

### 37. `vm/ps_guzer.c:278`

```
guzer: sem servidor X (defina DISPLAY / rode num desktop)
```

**Por que não serve:** "servidor X" e nomenclatura do X11 — e ainda sai por stderr com o script terminando em 0, entao parece que rodou bem.

**Proposta:** "guzer: nao achei uma sessao grafica pra abrir a janela — rode num desktop (ou defina DISPLAY, ex.: DISPLAY=:0)"

→ 

### 38. `vm/poolscript_vm.c:8791`

```
TypeError: base64 invalido
```

**Por que não serve:** nao diz o que esta invalido (caractere fora do alfabeto? tamanho?) e sai como TypeError, embora o tipo str esteja certo — o errado e o valor.

**Proposta:** BERRO(vm, "ValueError", "b64decode(): a str nao esta em base64 valido (caractere fora de A-Z a-z 0-9 + / = ou tamanho quebrado)")

→ 

---

## motor: builtins e metodos — 4

### 39. `vm/poolscript_vm.c:3759`

```
"operacao invalida: sum() espera lista"
```

**Por que não serve:** o teste e EH_SEQ — sum(tup([1,2])) FUNCIONA — entao a mensagem manda o usuario trocar um tup que estava certo; e "lista" nao e o nome do tipo, que e list.

**Proposta:** "sum() soma os itens de uma list ou tup, recebeu %s", nome_do_tipo_valor(args[0])

→ 

### 40. `vm/poolscript_vm.c:4774`

```
"join() espera uma lista"
```

**Por que não serve:** mesma armadilha: "-".join(tup(["a","b"])) funciona, mas a frase diz que tup nao serve; e escreve "lista" onde o tipo e list.

**Proposta:** "join() junta os itens de uma list ou tup, recebeu %s", nome_do_tipo_valor(args[0])

→ 

### 41. `vm/poolscript_vm.c:5336`

```
"extend() espera uma lista"
```

**Por que não serve:** aceita tup na pratica (EH_SEQ), mas a mensagem nega; e "lista" nao e o nome do tipo.

**Proposta:** "extend() espera list ou tup, recebeu %s", nome_do_tipo_valor(args[0])

→ 

### 42. `vm/poolscript_vm.c:4679`

```
"rsplit() espera str no separador"
```

**Por que não serve:** essa linha quase nunca e alcancada: com 1 argumento rsplit delega pro met_split, e "a".rsplit(1) imprime "split() espera str" — o motor culpa um metodo que o usuario nao chamou.

**Proposta:** passar o nome de quem chamou ao met_split (parametro quem) pra sair "rsplit() espera str no separador, recebeu int"; enquanto isso a frase certa e "rsplit() espera str no separador, recebeu %s"

→ 

---

## motor: operador e indice — 32

### 43. `vm/poolscript_vm.c:18337 (e 18400, 18413, 18464, 18478, 18537)`

```
TypeError: '-' entre tipos incompativeis: int - str   (para `true - "a"`)
```

**Por que não serve:** o bool virou int NAS LINHAS ACIMA da mensagem, então o erro acusa um `int` que o usuário nunca escreveu — ele procura um int no código e não acha.

**Proposta:** guardar os tipos ORIGINAIS antes das duas linhas `if (a.t == V_BOOL) {...}` e usar esses nomes na mensagem: `'-' entre tipos incompativeis: bool - str`. Vale igual em ADD/SUB/MUL/DIV/MOD e no macro CMP (e no INDEX_GET, que diz `indice 1` quando o usuário escreveu `l[true]`).

→ 

### 44. `vm/poolscript_vm.c:19535`

```
RuntimeError: tipo nao suporta atribuicao por indice
```

**Por que não serve:** não diz QUAL tipo, e espreme dois casos opostos numa frase: `t[0]=1` numa tup (é imutável, existe índice) e `x[0]=1` num int (não tem índice nenhum) dão a mesma frase — é o caso "lista vazia ou não-lista" de novo.

**Proposta:** separar. Para tup/str/bytes: `tup e imutavel: nao da pra trocar o item 0` (com TypeError). Para o resto: `nao da pra atribuir por indice em int: so list e dict aceitam l[i] = valor` (com TypeError, não RuntimeError).

→ 

### 45. `vm/poolscript_vm.c:19606`

```
RuntimeError: for each exige lista, tupla ou string
```

**Por que não serve:** usa nomes que não são os tipos da linguagem (`tupla`/`string` em vez de `tup`/`str`), não diz o que veio, e MENTE por omissão: o generator é aceito na linha 19595 e não está na lista.

**Proposta:** `for each nao percorre int: use list, tup, str ou um generator` — com o nome do tipo vindo de `nome_do_tipo_valor(cont)` e TypeError no lugar de RuntimeError.

→ 

### 46. `vm/poolscript_vm.c:18393`

```
AttributedValueError: '+' entre tipos incompativeis: dict + dict
```

**Por que não serve:** chama de "incompativeis" dois tipos IDÊNTICOS — a frase se contradiz e o usuário fica achando que existe um dict de outra espécie; o problema real é que dict não soma.

**Proposta:** quando os dois nomes forem iguais, trocar a frase: `'+' nao soma dict com dict` (ou `dict nao suporta '+'`). Mesmo tratamento em `'*' entre tipos incompativeis: str * str`.

→ 

### 47. `vm/poolscript_vm.c:19908`

```
AttributedValueError: variável c esperava char (um caractere), recebeu 2
```

**Por que não serve:** "recebeu 2" lê como se tivesse recebido o VALOR 2; é a contagem de caracteres, e a frase não diz isso.

**Proposta:** `variável c esperava char (um caractere), recebeu 2 caracteres` — ou, melhor, mostrando o texto: `recebeu "ab" (2 caracteres)`.

→ 

### 48. `vm/poolscript_vm.c:20616`

```
TypeError: desempacotamento espera lista ou tupla
```

**Por que não serve:** usa "tupla" onde o tipo é `tup`, não diz o que veio, e a lista está errada: str e generator também são desempacotados (linhas 19881 e 19902 do mesmo opcode).

**Proposta:** `nao da pra desempacotar int: so list, tup, str e generator`.

→ 

### 49. `docs/linguagem/02-tipos-e-valores.md:122`

```
- **Não se ordena.** Qualquer `<`, `>`, `<=`, `>=` com `Null` de um dos lados é `False` — inclusive `Null >= Null`. Isso é proposital: `if x > 0` com `x` ainda não preenchido simplesmente não entra, em vez de estourar.
```

**Por que não serve:** A doc escolhe o cenário exato do leitor (`if x > 0` com x vazio) e promete que ele é seguro, mas hoje `Null > 0` levanta `TypeError: '>' nao se aplica a Null: Null > int` e mata o programa.

**Proposta:** - **Não se ordena.** `<`, `>`, `<=`, `>=` com `Null` de um dos lados **levantam** `TypeError` (`'>' nao se aplica a Null: Null > int`) — inclusive `Null >= Null`. Comparar magnitude de um valor que ainda não chegou quase sempre é um bug, e a linguagem prefere apontá-lo a fingir `False`. Antes de comparar, teste a presença: `if x is not Null and x > 0`.

→ 

### 50. `docs/linguagem/02-tipos-e-valores.md:126`

```
- Em igualdade, `Null == Null` é `True`; `Null == 0` e `Null == 0.0` são `True` (compatibilidade numérica); com o resto é `False`.
```

**Por que não serve:** `Null == 0` e `Null == 0.0` devolvem `False` hoje — quem confia nesta linha escreve `if x == 0` esperando pegar o Null e o ramo nunca entra, sem erro nenhum pra avisar.

**Proposta:** - Em igualdade, `Null` só é igual a `Null`. `Null == 0`, `Null == 0.0`, `Null == ""` e `Null == []` são todos `False` — zero e vazio são valores, ausência de valor é outra coisa. Pra testar ausência, use `x is Null` (ou `x is not Null`).

→ 

### 51. `docs/linguagem/03-operadores-e-expressoes.md:197`

```
`null == null` é `True`. Na **igualdade**, `null` equivale a zero numérico (`null == 0` é `True`). Nas comparações de **ordem** (`<`, `>`, `<=`, `>=`), qualquer lado `null` resulta sempre `False` — `null` não tem magnitude.
```

**Por que não serve:** Mesma promessa vencida da seção 2.5, no arquivo que é a referência de operadores: nem a igualdade com zero nem o `False` na ordem valem mais.

**Proposta:** `Null == Null` é `True`; `Null` comparado com qualquer outro valor (inclusive `0`) é `False` — ausência de valor não é zero. Já `<`, `>`, `<=`, `>=` com `Null` de um dos lados **levantam** `TypeError`, em vez de responder `False` calado: se o valor pode não ter chegado, teste `x is not Null` antes de comparar.

→ 

### 52. `docs/linguagem/03-operadores-e-expressoes.md:202`

```
post(null == null)   // True
post(null == 0)      // True
post(null < 5)       // False
post(null >= 0)      // False
```

**Por que não serve:** Rodando o bloco: a 2ª linha imprime `False` (a doc diz `True`) e a 3ª aborta o programa com `TypeError` — as duas últimas linhas nem chegam a executar.

**Proposta:** post(null == null)   // True
post(null == 0)      // False — ausência de valor não é zero

// ordem com Null levanta:
// post(null < 5)    // TypeError: '<' nao se aplica a Null: Null < int
if (x is not Null and x < 5) { post("menor") }   // o jeito de comparar com segurança

→ 

### 53. `docs/LANGUAGE.md:196`

```
`Null == 0` é `True` (igualdade "nullish", só para `==`; comparações de magnitude `<`/`>` com `Null` são sempre `False`).
```

**Por que não serve:** As duas metades estão vencidas: `Null == 0` é `False` e `Null < 1` levanta `TypeError` — e "igualdade nullish" é jargão que não diz nada a quem só quer saber se a variável está vazia.

**Proposta:** `Null` é igual só a `Null`: `Null == 0` é `False`. E `<`, `>`, `<=`, `>=` com `Null` **levantam** `TypeError` em vez de devolver `False` — comparar magnitude de algo que ainda não chegou é um bug, e a linguagem avisa. Pra testar ausência: `x is Null` / `x is not Null`.

→ 

### 54. `docs/linguagem/10-excecoes.md:109`

```
| `TypeError` | valor inválido numa operação (divisão por zero, `str` em conta, …) |
```

**Por que não serve:** Divisão por zero levanta `ZeroDivisionError`, não `TypeError` — e o exemplo logo abaixo (linhas 145-149) faz exatamente `catch (TypeError e)` em `1 / 0`, então o programa da doc morre com rc=1.

**Proposta:** | `TypeError` | o **tipo** está errado pra operação: `"a" - 1`, `len(5)`, `[1,2]["x"]` |
| `ValueError` | o tipo está certo e o **valor** não serve: `int("abc")`, `max([])`, `"abc".index("z")` |
| `ZeroDivisionError` | divisão ou resto por zero: `1 / 0`, `1 % 0` |

(e trocar o exemplo abaixo da tabela para `catch (ZeroDivisionError e)`)

→ 

### 55. `docs/linguagem/10-excecoes.md:144`

```
`SyntaxError` não é capturável (é de compilação); índice fora do range é aviso não-fatal (→ `null`).
```

**Por que não serve:** O resumo desmente a própria seção 10.4 vinte linhas acima: ler fora da faixa levanta `IndexError` desde 28/08, então quem confia no resumo escreve `if l[99] == null` e leva uma exceção na cara.

**Proposta:** `SyntaxError` não é capturável (acontece antes de rodar); índice fora da faixa **levanta** `IndexError`, lendo ou escrevendo, com o índice e o tamanho na mensagem.

→ 

### 56. `docs/exceptions/exceptions.md:80`

```
| `IndexError` | índice inválido ao **escrever**: `l[99] = x`. Ler fora da faixa é outra coisa — ver a nota abaixo |
```

**Por que não serve:** A nota logo abaixo diz o contrário — que ler, escrever e chave ausente levantam do mesmo jeito desde 28/08 — então a linha manda o leitor procurar um tratamento separado que não existe mais.

**Proposta:** | `IndexError` | índice fora da faixa, **lendo ou escrevendo**: `l[99]`, `l[99] = x`. Vale pra `list`, `tup`, `str` e `bytes`; a mensagem diz o índice e o tamanho |

→ 

### 57. `docs/PoolScript.md:382`

```
| `IndexError` | índice inválido ao **escrever** (`l[99] = x`) |
```

**Por que não serve:** A nota logo abaixo da mesma tabela já diz "ler ou escrever índice fora da faixa levanta IndexError" — a linha faz o leitor achar que precisa de outro `catch` pra leitura.

**Proposta:** | `IndexError` | índice fora da faixa, lendo **ou** escrevendo (`l[99]`, `l[99] = x`) — em `list`, `tup`, `str` e `bytes` |

→ 

### 58. `docs/PoolScript.md:390`

```
| `NotImplementedError` | construção reconhecida e ainda não executada |
```

**Por que não serve:** O motor levanta `NotImplemented` (sem o "Error") — `catch (NotImplementedError e)` nunca dispara, que é exatamente o buraco do `PermissionError` que o aviso três linhas abaixo diz ter fechado.

**Proposta:** | `NotImplemented` | lib "stub" chamada — `sqlite`, `smtplib`, `mimetext`, `multipart`, `flask` existem no `import`, mas chamar levanta: "esta funcao ainda nao esta implementada nesta versao da PoolScript" |

→ 

### 59. `docs/exceptions/exceptions.md:81`

```
| `NotImplementedError` | construção que o motor reconhece e ainda não executa |
```

**Por que não serve:** Nenhum erro capturável usa esse nome — o único sítio no motor (`stub_chamada`) levanta `NotImplemented`, então o `catch` escrito a partir desta linha passa batido e o erro escapa.

**Proposta:** | `NotImplemented` | lib "stub" chamada: `sqlite`, `smtplib`, `mimetext`, `multipart` e `flask` importam sem reclamar, mas qualquer chamada levanta "esta funcao ainda nao esta implementada nesta versao da PoolScript" |

→ 

### 60. `docs/PoolScript.md:377`

```
| `TypeError` | o **tipo** está errado: `"a" - 1`, `len(5)`, aridade errada. O mais comum |
```

**Por que não serve:** Aridade errada levanta `RuntimeError` ("action 'f' faltando argumento: 'b'"), não `TypeError` — quem cerca a chamada com `catch (TypeError e)` confiando nesta linha vê o programa morrer mesmo assim.

**Proposta:** | `TypeError` | o **tipo** está errado: `"a" - 1`, `len(5)`, `[1,2]["x"]`. O mais comum |
| `RuntimeError` | variável não definida, `raise "texto"`, e **argumento faltando ou sobrando** numa `action` |

→ 

### 61. `docs/LANGUAGE.md:1123`

```
| `TypeError` | Tipo errado: operação entre tipos incompatíveis, aridade errada, RHS não-iterável em unpacking |
```

**Por que não serve:** Dois problemas: "aridade errada" numa chamada de `action` é `RuntimeError`, não `TypeError`; e "RHS não-iterável" é jargão de compilador que não diz ao leitor o que ele escreveu de errado.

**Proposta:** | `TypeError` | o tipo está errado pra operação (`"a" - 1`, `len(5)`), ou o lado direito de um desempacotamento não é `list`/`tup`/`str` (`a, b = 5`) |
| `RuntimeError` | variável não definida, `raise "texto"`, e argumento faltando ou sobrando numa `action` |

→ 

### 62. `docs/linguagem/10-excecoes.md:108`

```
| `ConversionError` | conversão impossível (`int("abc")`, `int z = "abc"`) |
```

**Por que não serve:** Os dois exemplos levantam erros diferentes: `int z = "abc"` é `ConversionError` mesmo, mas `int("abc")` é `ValueError` — a linha junta os dois e quem escreve o `catch` pelo primeiro exemplo não pega nada.

**Proposta:** | `ConversionError` | a **declaração tipada** não conseguiu converter o valor: `int z = "abc"` |
| `ValueError` | o tipo está certo e o valor não serve: o **builtin** `int("abc")`, `max([])`, `"abc".index("z")` |

→ 

### 63. `docs/linguagem/11-builtins.md:38`

```
| `int` | `int(x)` | → inteiro: string decimal, `flo` (**trunca** pra zero: `int(3.9)`→3), `bool`. `int()` sem argumento → `0`. String não-numérica → `TypeError`. |
```

**Por que não serve:** `int("abc")` levanta `ValueError` ("valor invalido: nao da pra converter 'abc' em int"), não `TypeError` — o `catch` escrito a partir desta linha não pega, e a mesma célula diz "string" onde o tipo se chama `str`.

**Proposta:** | `int` | `int(x)` | → inteiro: `str` numérica, `flo` (**trunca** pra zero: `int(3.9)`→3), `bool`. `int()` sem argumento → `0`. `str` que não é um número levanta `ValueError`: "nao da pra converter 'abc' em int". |

→ 

### 64. `docs/linguagem/10-excecoes.md:52`

```
> try:
>     arriscado()
> catch (e):
>     raise Erro(e)
> finally:
>     limpa()
```

**Por que não serve:** O bloco com `:` não existe mais na linguagem: colado num arquivo, esse exemplo nem chega a rodar — sai `SyntaxError: bloco com ':' nao existe mais — use '{ }'` na primeira linha.

**Proposta:** > try {
>     arriscado()
> } catch (e) {
>     raise Erro(e)
> } finally {
>     limpa()
> }

(e trocar as menções a **`try:`**, **`catch:`** e **`finally:`** no corpo e no resumo por **`try`**, **`catch (...)`** e **`finally`** — só `if __name__ == "main":` ainda abre bloco com `:`)

→ 

### 65. `docs/LANGUAGE.md:1141`

```
- **String bruta imediatamente seguida de `{` em estilo chave é ambígua**: `for each c in "abc" { ... }` tenta interpretar o `{` como início de interpolação (`"texto" {expr}`) em vez de abrir o bloco do loop. Contorne usando o estilo `:` (`for each c in "abc":`) ou uma variável em vez do literal direto.
```

**Por que não serve:** Erra duas vezes: `for each c in "abc" { post(c) }` roda certinho hoje, e o contorno receitado (`:`) é justamente o que o parser recusa com `SyntaxError: bloco com ':' nao existe mais`.

**Proposta:** (apagar o item — a ambiguidade não existe mais; `for each c in "abc" { ... }` itera os caracteres normalmente)

→ 

### 66. `docs/LANGUAGE.md:1146`

```
- **`elif`/`else` no estilo chave precisam ficar na mesma linha** do `}` anterior (`} elif (...) {`), não numa linha nova.
```

**Por que não serve:** O `elif` numa linha nova funciona — e a seção "Blocos" do mesmo arquivo (linha 91) já diz o contrário ("podem vir colados ao `}` ou numa linha nova — as quatro combinações valem"), então o leitor não sabe em qual acreditar.

**Proposta:** (apagar o item — está resolvido e a seção "Blocos: `{ }`" já documenta o comportamento certo)

→ 

### 67. `docs/exceptions/exceptions.md:120`

```
action buscar(url=str) {
```

**Por que não serve:** `url=str` parece anotação de tipo mas é valor padrão: chamar `buscar()` passa o próprio objeto-tipo (`type(url)` responde `"type"`) pra dentro do `request.get` — e a seção 6.2.3 diz explicitamente que parâmetro de `action` não tem tipo.

**Proposta:** action buscar(url) {

(ou, se a intenção era um padrão de verdade, `action buscar(url="")` — parâmetro de `action` não aceita tipo, ver 06-funcoes §6.2.3)

→ 

### 68. `docs/LANGUAGE.md:1006`

```
| `.casefold()` | hoje igual a `.lower()` |
```

**Por que não serve:** Não são iguais: `"ß".casefold()` devolve `"ss"` e `"ß".lower()` devolve `"ß"` — quem lê isto usa `.lower()` pra comparar sem caixa e perde justamente os casos que o `casefold` existe pra resolver.

**Proposta:** | `.casefold()` | minúscula agressiva, pra comparar sem caixa: vai além do `.lower()` onde a letra não tem minúscula 1-pra-1 (`"ß".casefold()` é `"ss"`, `"ß".lower()` é `"ß"`) |

→ 

### 69. `docs/LANGUAGE.md:273`

```
tupla = (1, 2, 3)            # imutável (implementada como tuple do Python)
```

**Por que não serve:** O motor é uma VM em C (o próprio cabeçalho do arquivo diz isso) — a nota vaza um detalhe interno que é falso e não ajuda em nada quem só quer saber o que a `tup` faz.

**Proposta:** tupla = (1, 2, 3)            # tup — imutável: não dá pra trocar item nem mudar o tamanho depois de criada

→ 

### 70. `docs/LANGUAGE.md:455`

```
`async action`/`async reaction` executam em uma thread separada (`ThreadPoolExecutor`) e devolvem imediatamente um **future** (`PoolFuture`) —
```

**Por que não serve:** `ThreadPoolExecutor` é classe do Python e não descreve o que roda aqui: a seção 6.8 de docs/linguagem diz que o async roda sobre fibras (green-threads) — o leitor sai daqui dimensionando pool de threads que não existe.

**Proposta:** `async action`/`async reaction` não rodam na chamada: devolvem na hora um **future** (a promessa do resultado) e o corpo segue numa **fibra** (green-thread) escalonada pela VM. O valor sai com `await` (um future) ou `gather` (vários).

→ 

### 71. `docs/LANGUAGE.md:6`

```
Substitui `docs/PoolScript.md` (que documentava a v1.0.8 e está bastante desatualizado — não cobre `async`/`await`, `Entity`, `match`/`case`, `count`, decorators nem unpacking).
```

**Por que não serve:** PoolScript.md hoje abre com "v8.3.84" e cobre `--check`, ternário, `enum`, `count each`, bitwise e `using` — enquanto este arquivo ainda se anuncia como "v8.2.18" na linha 1; o leitor é mandado ignorar a doc mais nova das duas.

**Proposta:** Companheiro de `docs/PoolScript.md`: lá está o tour rápido da linguagem, aqui a referência completa. Onde as duas divergirem, vale o que o motor faz — confira com `pool -e '...'`.

(e acertar a linha 1 para a versão que `pool --version` reporta)

→ 

### 72. `docs/linguagem/06-funcoes.md:144`

```
Os prefixos de uma action/reaction — tipo de retorno (`int`/`bool`/`str`/`flo`), `async` e visibilidade (`public`/`private`) — podem vir em **qualquer ordem**.
```

**Por que não serve:** `str action`/`flo action` até compilam, mas não ganham o contrato de "nunca propaga erro" que a seção inteira está descrevendo (`str action` com `1/0` dentro morre com `ZeroDivisionError`) — e a §2.1 e o resumo §6.9 dizem que só existem `int` e `bool`.

**Proposta:** Os prefixos de uma action/reaction — tipo de retorno (`int`/`bool`), `async` e visibilidade (`public`/`private`) — podem vir em **qualquer ordem**.

> Só `int action` e `bool action` mudam o tratamento de erro (500/False). `str action` e `flo action` são aceitos pelo parser mas não têm contrato nenhum: o erro dentro deles propaga igual a uma action sem prefixo.

→ 

### 73. `docs/list/list.md:16`

```
| [`index`](index/index.md) | `l.index(item)` | Posição da primeira ocorrência do item. |
```

**Por que não serve:** Não diz o que acontece quando o item não está lá — e como a tabela irmã de `str` ensina que `find` devolve `-1`, o leitor escreve `if l.index(x) == -1` e o programa morre com `ValueError: index() nao achou o item`.

**Proposta:** | [`index`](index/index.md) | `l.index(item, inicio=0, fim=len)` | Posição da primeira ocorrência do item. **Levanta `ValueError` se não achar** — pra só saber se está lá, use `l.contains(item)`. |

→ 

### 74. `docs/dict/dict.md:17`

```
| [`pop`](pop/pop.md) | `d.pop(chave)` | Remove a chave e DEVOLVE o valor dela (muta). |
```

**Por que não serve:** Some com o 2º parâmetro (`d.pop("zz", "padrao")` funciona) e não avisa que sem ele a chave ausente levanta `KeyError` — logo acima, o `get` é vendido como "NÃO erra", o que reforça a impressão errada de que o `pop` também não.

**Proposta:** | [`pop`](pop/pop.md) | `d.pop(chave, default)` | Remove a chave e DEVOLVE o valor dela (muta). Sem `default`, chave que não existe levanta `KeyError`; com `default`, devolve ele em vez de erro. |

→ 

---

# NÃO AJUDAM — corretas, mas o usuário fica sem saber o que fazer

Nada do que dizem é falso. O problema é que quem lê continua sem saber qual foi o erro dele.

## doc: secoes "## Erros" — 20

### 75. `docs/builtins/open/open.md:18`

```
- **IOError** — arquivo inexistente no modo de leitura
```

**Por que não serve:** Cobre um caso de quatro: também dá `IOError` quando o caminho é um diretório e quando a pasta de destino não existe no modo `"w"`, e existem `TypeError` e `ValueError` que a linha nem cita.

**Proposta:** - **IOError** — o caminho não existe (leitura), aponta pra um diretório, ou a pasta de destino não existe (escrita)
- **TypeError** — `caminho` não é `str`
- **ValueError** — `modo` não é um modo válido (`"r"`, `"w"`, `"a"`, `"rb"`, `"wb"`...)

→ 

### 76. `docs/dict/pop/pop.md:17`

```
- **KeyError** — a chave não existe
```

**Por que não serve:** Omite a única coisa que o leitor precisa saber ali: o erro só acontece quando você NÃO passa `default` — com ele, `pop` devolve o default em silêncio.

**Proposta:** - **KeyError** — a chave não existe e você não passou `default`. Passando, ele volta no lugar do erro: `d.pop("x", 0)`.

→ 

### 77. `docs/builtins/sum/sum.md:17`

```
- **TypeError** — algum item da lista não é `int` nem `flo`
```

**Por que não serve:** Falta o segundo `TypeError` (`sum(5)` — o argumento não é `list`) e falta dizer que `sum([])` é `0`, não erro.

**Proposta:** - **TypeError** — algum item não é `int` nem `flo`; `sum(["a","b"])` não concatena
- **TypeError** — o argumento não é `list` nem `tup`

`sum([])` é `0`, não é erro.

→ 

### 78. `docs/builtins/sorted/sorted.md:17`

```
- **TypeError** — os itens não se comparam entre si (ex.: `int` com `str`)
```

**Por que não serve:** Falta o caso mais comum — passar algo que não dá pra percorrer (`sorted(5)`), que levanta o mesmo `TypeError` com outra mensagem, então o leitor não reconhece o erro que recebeu.

**Proposta:** - **TypeError** — os itens não se comparam entre si (ex.: `int` com `str`)
- **TypeError** — o argumento não dá pra percorrer: `sorted(5)` erra; `list`, `tup`, `str` e `dict` passam (`dict` ordena as chaves)

→ 

### 79. `docs/builtins/round/round.md:18`

```
- **TypeError** — o argumento não é `int` nem `flo`
```

**Por que não serve:** `round` tem dois parâmetros e a linha diz "o argumento" — não dá pra saber se fala de `n` ou de `casas`, que tem erro próprio e mensagem própria.

**Proposta:** - **TypeError** — `n` não é `int`, `flo` nem `bool`
- **TypeError** — `casas` não é `int`

→ 

### 80. `docs/builtins/ord/ord.md:17`

```
- **TypeError** — a `str` não tem exatamente 1 caractere
```

**Por que não serve:** Não cobre `ord(5)` (argumento que nem é `str`), e "não tem exatamente 1" deixa em aberto se `""` conta.

**Proposta:** - **TypeError** — `c` não é uma `str` de 1 caractere: `""` e `"ab"` erram, e `ord(5)` também

→ 

### 81. `docs/builtins/int/int.md:17`

```
- **ValueError** — a `str` não contém um número válido: `int("abc")`
```

**Por que não serve:** Falta o outro erro, de nome diferente: `int([])` e `int({})` levantam `TypeError`, então filtrar só `ValueError` não cobre a página.

**Proposta:** - **ValueError** — a `str` não tem um número dentro: `int("abc")` (espaços em volta são tolerados: `int(" 12 ")` é 12)
- **TypeError** — o valor não é `str`, `flo`, `bool` nem `int`: `int([])`

→ 

### 82. `docs/builtins/flo/flo.md:17`

```
- **ValueError** — a `str` não contém um número válido: `flo("x")`
```

**Por que não serve:** Mesma lacuna do `int`: `flo([])` levanta `TypeError`, nome que a página não menciona.

**Proposta:** - **ValueError** — a `str` não tem um número dentro: `flo("x")` (notação científica passa: `flo("1e3")` é 1000.0)
- **TypeError** — o valor não é `str`, `int`, `bool` nem `flo`: `flo([])`

→ 

### 83. `docs/builtins/filter/filter.md:18`

```
- **TypeError** — fn não é chamável
```

**Por que não serve:** "chamável" é jargão que não existe no vocabulário de quem escreve PoolScript, `fn` aparece sem crase como se fosse prosa, e falta o erro de `lista` não ser `list`.

**Proposta:** - **TypeError** — `fn` não é uma `action` (nem outra coisa que dá pra chamar): `filter(l, 5)`
- **TypeError** — `lista` não é `list` nem `tup`

→ 

### 84. `docs/string/join/join.md:17`

```
- **TypeError** — algum item da lista não é `str`
```

**Por que não serve:** Diz o que aconteceu mas não o que fazer — e falta o segundo `TypeError`, de `join` receber algo que não é `list`.

**Proposta:** - **TypeError** — algum item não é `str`. Converta antes: `"-".join(map(nums, str))`.
- **TypeError** — o argumento não é `list` nem `tup`

→ 

### 85. `docs/string/format/format.md:17`

```
- **TypeError** — a chave ou o índice citado no formato não existe
```

**Por que não serve:** Cobre um dos três `TypeError` de `format`: faltam `{` sem fechar e letra de formato desconhecida, que dão o mesmo nome de erro com mensagem completamente diferente.

**Proposta:** - **TypeError** — o campo citado não tem valor: `"{1}".format("a")` ou `"{z}".format(1)`
- **TypeError** — o formato está quebrado: `{` sem fechar, ou letra de formato que não existe (`"{:z}"`)

→ 

### 86. `docs/string/format_map/format_map.md:17`

```
- **TypeError** — a chave citada no formato não existe
```

**Por que não serve:** "não existe" onde? Falta dizer que é no `dict` passado, e falta o erro de passar algo que não é `dict`.

**Proposta:** - **TypeError** — o formato cita uma chave que não está no `dict` passado
- **TypeError** — o argumento não é `dict`

→ 

### 87. `docs/string/maketrans/maketrans.md:18`

```
- **TypeError** — os dois argumentos têm tamanhos diferentes
```

**Por que não serve:** Não diz que o tamanho é contado em caracteres nem por que precisa bater, e não cobre argumento que não é `str`.

**Proposta:** - **TypeError** — os dois argumentos não têm o mesmo número de caracteres; a troca é letra a letra, então cada um do primeiro precisa de um par no segundo
- **TypeError** — algum argumento não é `str`

→ 

### 88. `docs/string/index/index.md:19`

```
- **ValueError** — a subcadeia não aparece no trecho pedido
```

**Por que não serve:** "subcadeia" não é palavra de quem programa em PoolScript, e a linha não aponta a saída sem erro — `find`, que devolve `-1`.

**Proposta:** - **ValueError** — `sub` não aparece entre `inicio` e `fim`. Se você não quer erro, use `find`, que devolve `-1`.
- **TypeError** — `sub` não é `str`

→ 

### 89. `docs/string/rindex/rindex.md:19`

```
- **ValueError** — a subcadeia não aparece no trecho pedido
```

**Por que não serve:** Mesmo jargão do `index`, e some com o `rfind`, que resolve o caso sem exceção.

**Proposta:** - **ValueError** — `sub` não aparece entre `inicio` e `fim`. Se você não quer erro, use `rfind`, que devolve `-1`.
- **TypeError** — `sub` não é `str`

→ 

### 90. `docs/list/index/index.md:17`

```
- **ValueError** — o item não está na lista
```

**Por que não serve:** A assinatura da própria página aceita `inicio`/`fim`, então o item pode estar na `list` e ainda assim dar erro por estar fora do trecho — a linha nega isso.

**Proposta:** - **ValueError** — o item não aparece na `list`, ou não aparece no trecho `inicio`..`fim` quando você passa os dois

→ 

### 91. `docs/string/match/match.md:17`

```
- **TypeError** — o padrão não é uma expressão regular válida
```

**Por que não serve:** Não cobre padrão que nem é `str`, e esconde do leitor que a mensagem do motor aponta o defeito e a posição — informação que faz a diferença entre corrigir e adivinhar.

**Proposta:** - **TypeError** — o padrão não é uma regex válida; a mensagem diz o defeito e a posição (ex.: `classe nao fechada (posicao 1)`)
- **TypeError** — o padrão não é `str`

→ 

### 92. `docs/string/findall/findall.md:17`

```
- **TypeError** — o padrão não é uma expressão regular válida
```

**Por que não serve:** Mesma lacuna do `match`: falta o caso de padrão que não é `str` e a dica de que o motor aponta a posição do defeito.

**Proposta:** - **TypeError** — o padrão não é uma regex válida; a mensagem diz o defeito e a posição (ex.: `classe nao fechada (posicao 1)`)
- **TypeError** — o padrão não é `str`

→ 

### 93. `docs/string/sub/sub.md:18`

```
- **TypeError** — o padrão não é uma expressão regular válida
```

**Por que não serve:** `sub` tem dois argumentos `str` e a linha só fala do padrão — passar uma troca que não é `str` levanta o mesmo erro e a doc não avisa.

**Proposta:** - **TypeError** — o padrão não é uma regex válida; a mensagem diz o defeito e a posição (ex.: `classe nao fechada (posicao 1)`)
- **TypeError** — o padrão ou o texto de troca não é `str`

→ 

### 94. `docs/bytes/get/get.md:30`

```
- **ValueError** — o índice está fora da faixa.
```

**Por que não serve:** Não diz qual é a faixa nem que índice negativo é válido, então quem lê acha que `-1` erra — e `bytes.get(b, -1)` funciona.

**Proposta:** - **ValueError** — `i` não existe em `b`: vale de `0` a `len(b) - 1`, ou negativo contando do fim (`-1` é o último byte). A mensagem mostra o range.

→ 

---

## libs em C — 22

### 95. `vm/ps_db.c:427`

```
falha de conexão: %s   (%s = mysql_error(c->my))
```

**Por que não serve:** repassa o texto do MySQL em ingles sem dizer sequer a qual servidor/base a tentativa se referia.

**Proposta:** snprintf(erro, ecap, "falha de conexão com o mysql em %s:%d (base '%s'): %s", host, porta, db, mysql_error(c->my))

→ 

### 96. `vm/ps_db.c:96 (idem 107, 140)`

```
erro de banco de dados: near "SELET": syntax error
```

**Por que não serve:** "erro de banco de dados" ja e o nome do tipo (DatabaseError) — repete e nao acrescenta nada, e o que sobra e ingles cru do SQLite sem dizer que quem recusou foi o banco.

**Proposta:** snprintf(erro, ecap, "o banco recusou o comando SQL: %s", sqlite3_errmsg(c->sq)) — vira "DatabaseError: o banco recusou o comando SQL: near \"SELET\": syntax error"

→ 

### 97. `vm/poolscript_vm.c:10631`

```
erro de banco de dados: Cannot operate on a closed database.
```

**Por que não serve:** frase inteira em ingles, escrita a mao no nosso codigo (nem vem de lib), e nao diz o que fazer depois.

**Proposta:** "a conexao com o banco ja foi fechada por .close() — abra outra com sqlite3.connect()"

→ 

### 98. `vm/poolscript_vm.c:14589 (idem 14690, 14700, 14962, 15074)`

```
TypeError: conexao fechada
```

**Por que não serve:** nao diz qual conexao, quem fechou, nem como seguir — e sai como TypeError, que sugere erro de tipo de argumento.

**Proposta:** "a conexao com o banco ja foi fechada por .close() — abra outra com db.connect()"

→ 

### 99. `vm/poolscript_vm.c:14783`

```
TypeError: driver desconhecido: oracle
```

**Por que não serve:** diz o que nao serve mas nao diz o que serve, e o usuario nao tem onde descobrir a lista sem abrir o fonte.

**Proposta:** "driver desconhecido: '%s' — os aceitos sao sqlite, postgres, mysql, mssql e mongo"

→ 

### 100. `vm/ps_db.c:319`

```
erro de banco de dados
```

**Por que não serve:** quando o SQLGetDiagRec nao devolve diagnostico, a mensagem fica sem nenhuma informacao: repete o nome do tipo e para por ai.

**Proposta:** "o driver ODBC falhou e nao devolveu motivo — confira o SQL Server e o 'ODBC Driver 18 for SQL Server' instalado"

→ 

### 101. `vm/ps_jinker.c:111 e 130`

```
socket: %s / listen: %s
```

**Por que não serve:** nome de syscall + strerror em ingles: nao diz que quem falhou foi o servidor subindo, nem o que o usuario deveria fazer.

**Proposta:** 111: "nao consegui criar o socket do servidor: %s"; 130: "nao consegui colocar o servidor pra escutar na porta %d: %s"

→ 

### 102. `vm/ps_jinker.c:701`

```
Sec-WebSocket-Accept invalido
```

**Por que não serve:** e o nome literal de um cabecalho do protocolo; nao existe nada que o usuario possa fazer com essa frase.

**Proposta:** "o servidor respondeu um handshake de WebSocket que nao confere — provavelmente nao e um servidor WebSocket"

→ 

### 103. `vm/ps_jinker.c:929 (idem 950, 953)`

```
SSL_CTX_new falhou / geracao de chave RSA falhou / X509_new falhou
```

**Por que não serve:** sao nomes de funcoes internas do OpenSSL vazando pro usuario final que so pediu ssl=True no servidor.

**Proposta:** 929: "nao consegui preparar o TLS (certificado '%s' / chave '%s') — confira se os dois arquivos existem e sao legiveis"; 950/953: "nao consegui gerar o certificado self-signed do jinker"

→ 

### 104. `vm/ps_mail.c:68`

```
[Errno %d] %s   (errno + strerror)
```

**Por que não serve:** numero de errno e texto em ingles do sistema, sem dizer a qual host/porta a conexao se referia.

**Proposta:** "nao consegui conectar em %s:%d: %s" com o motivo traduzido (recusada / expirou / rede inacessivel)

→ 

### 105. `vm/ps_mail.c:243 (idem 254)`

```
EHLO recusado / EHLO recusado depois do TLS
```

**Por que não serve:** EHLO e um comando do protocolo SMTP; quem so quer mandar e-mail nao tem como saber o que recusou nem o que fazer.

**Proposta:** "o servidor SMTP %s:%d nao aceitou a apresentacao inicial — confira o host e a porta (587 com STARTTLS, 465 com TLS direto)"

→ 

### 106. `vm/ps_mail.c:238`

```
servidor nao respondeu 220 no greeting
```

**Por que não serve:** mistura codigo numerico do SMTP com a palavra inglesa "greeting"; nada disso significa algo pra quem chamou .conn().

**Proposta:** "quem atendeu em %s:%d nao parece ser um servidor SMTP (nao mandou a saudacao inicial) — confira a porta"

→ 

### 107. `vm/poolscript_vm.c:11249`

```
TypeError: tipo não suportado para anexo
```

**Por que não serve:** nao diz qual tipo foi passado nem quais o attach() aceita — o usuario fica sem saber o que trocar.

**Proposta:** "attach() espera str com o caminho do arquivo ou um PoolFile — veio %s" (usando o nome do tipo real: int, list, dict, …)

→ 

### 108. `vm/ps_regex.c:319`

```
regex: classe nao fechada (posicao 3)
```

**Por que não serve:** "classe" e o termo tecnico do gramatica de regex; o usuario ve um colchete aberto, nao uma "classe".

**Proposta:** "regex: faltou fechar o ']' aberto na posicao %d"

→ 

### 109. `vm/ps_regex.c:590`

```
regex: caractere inesperado (posicao 1)
```

**Por que não serve:** nao diz QUAL caractere sobrou — em padrao longo o usuario tem que contar as posicoes na mao pra descobrir.

**Proposta:** "regex: o caractere '%c' na posicao %d nao encaixa aqui (')' sem '(' correspondente?)"

→ 

### 110. `vm/ps_regex.c:444`

```
regex: quantificador sem alvo (posicao 0)
```

**Por que não serve:** "quantificador" e "alvo" sao jargao de quem escreve o parser; quem escreveu '*a' nao sabe o que e um quantificador.

**Proposta:** "regex: '%c' na posicao %d precisa vir DEPOIS de algo pra repetir (ex.: 'a*', nao '*a')"

→ 

### 111. `vm/ps_regex.c:497`

```
regex: {n,m} com m < n (posicao 6)
```

**Por que não serve:** repete a notacao da gramatica em vez dos numeros que o usuario escreveu — ele digitou {3,1} e le "m < n".

**Proposta:** "regex: em {%d,%d} o maximo e menor que o minimo — inverta pra {%d,%d}"

→ 

### 112. `vm/ps_regex.c:439`

```
regex: escape desconhecido (posicao N)
```

**Por que não serve:** nao diz qual barra invertida deu problema nem quais existem; hoje '\q' nem chega a imprimir posicao util.

**Proposta:** "regex: '\\%c' na posicao %d nao e um escape conhecido — validos: \\d \\w \\s \\D \\W \\S \\b \\B \\A \\Z \\n \\t \\r e \\ antes de pontuacao"

→ 

### 113. `vm/ps_xlsx.c:330`

```
xlsx corrompido
```

**Por que não serve:** nao diz qual arquivo, e "corrompido" nao distingue arquivo truncado de arquivo que nunca foi xlsx (um .csv renomeado, p.ex.).

**Proposta:** "nao consegui ler '%s' como xlsx — o arquivo esta incompleto ou nao e um xlsx"

→ 

### 114. `vm/ps_pkg.c:334`

```
Erro: registry respondeu HTTP 404
```

**Por que não serve:** joga o codigo HTTP cru: quem instala pacote nao tem que saber que 404 significa URL errada e 403 significa acesso negado.

**Proposta:** "Erro: o registry %s respondeu HTTP %ld — %s" com o motivo em portugues (404: "a URL do registry esta errada, confira com `psl registry show`"; 401/403: "acesso negado"; 5xx: "o servidor do registry esta fora do ar")

→ 

### 115. `vm/ps_mongo.c:60 (idem 139, 152, 154)`

```
erro de banco de dados: query invalida: %s   (%s = be.message do libbson)
```

**Por que não serve:** prefixo redundante com o tipo DatabaseError e, depois dele, a mensagem do libbson em ingles falando de BSON, que nao e o que o usuario escreveu (ele escreveu um dict/JSON).

**Proposta:** "o filtro da consulta nao e um JSON valido: %s" (e nas outras: "o documento nao e um JSON valido", "o set= nao e um JSON valido")

→ 

### 116. `vm/ps_http.c:318`

```
NetworkError: falha de conexão: sem resposta
```

**Por que não serve:** "sem resposta" nao separa servidor que derrubou a conexao de servidor que nunca falou nada, e nao diz a URL — em script com varias chamadas nao da pra achar qual quebrou.

**Proposta:** "falha de conexão: o servidor fechou a conexão antes de responder (url=%.200s)"

→ 

---

## motor: builtins e metodos — 20

### 117. `vm/poolscript_vm.c:3039`

```
"%s() espera %d argumento(s)", nome, quant
```

**Por que não serve:** o "(s)" e enfeite de maquina e a frase nunca diz quantos chegaram — o usuario ve "abs() espera 1 argumento(s)" e nao sabe se mandou 0 ou 3.

**Proposta:** "%s() espera %d argumento%s, recebeu %d", nome, quant, quant == 1 ? "" : "s", n  — mesma redacao que a linha 18136 ja usa ("aceita ate N argumentos, recebeu M"); vale pra todo builtin que passa por EXIGE_ARGS

→ 

### 118. `vm/poolscript_vm.c:4243`

```
"%s() espera %d argumento(s)", nome, quant
```

**Por que não serve:** mesmo "(s)" e mesma omissao do recebido no ARGS_MET, que cobre count/insert/get/contains e dezenas de metodos — e fica ao lado de "upper() nao aceita argumento, recebeu 2", que ja acerta.

**Proposta:** "%s() espera %d argumento%s, recebeu %d", nome, quant, quant == 1 ? "" : "s", n

→ 

### 119. `vm/poolscript_vm.c:3013`

```
"len() espera 1 argumento"
```

**Por que não serve:** len() e len(1,2) dao a MESMA frase, e nenhuma das duas conta o que chegou.

**Proposta:** "len() espera 1 argumento, recebeu %d", n

→ 

### 120. `vm/poolscript_vm.c:3025`

```
"len() nao se aplica a este tipo"
```

**Por que não serve:** "este tipo" e invisivel: nao diz o que o usuario passou nem o que len() aceita, entao nao ha o que corrigir a partir da frase.

**Proposta:** "len() nao mede %s; mede str, list, tup, dict e bytes", nome_do_tipo_valor(args[0])

→ 

### 121. `vm/poolscript_vm.c:3725 (mesma frase em 3656, 3699, 3830, 3861, 3878 e 3902)`

```
"operacao invalida: list() nao itera este tipo"
```

**Por que não serve:** e o jargao "tipo iteravel" que ele apontou, em sete lugares (list, tup, dict, sorted, reversed, enumerate, zip): nao nomeia o tipo recebido nem os que serviriam — e o "operacao invalida:" so repete o "TypeError:" que ja sai antes.

**Proposta:** "%s() nao percorre %s; passe str, list, tup, dict, bytes ou range()", nome, nome_do_tipo_valor(args[0])

→ 

### 122. `vm/poolscript_vm.c:5395`

```
"valor invalido: remove() nao achou o item"
```

**Por que não serve:** nao diz QUAL item nao foi achado, e o "valor invalido:" so duplica o "ValueError:" impresso na frente — o dict ao lado (linha 5568) ja faz certo, com valor_para_texto.

**Proposta:** "remove() nao achou %s na list" com o item renderizado por valor_para_texto(&kb, &args[0], 1), igual ao KeyError do dict

→ 

### 123. `vm/poolscript_vm.c:5420`

```
"valor invalido: index() nao achou o item"
```

**Por que não serve:** mesmo caso, agravado: index() aceita inicio/fim, entao o item pode existir na list e estar so fora da faixa — a frase nao permite distinguir.

**Proposta:** "index() nao achou %s na list" (valor_para_texto) e, quando vieram inicio/fim, "...na faixa %lld..%lld"

→ 

### 124. `vm/poolscript_vm.c:4506`

```
"valor invalido: subcadeia nao encontrada"
```

**Por que não serve:** "subcadeia" e jargao de compilador; a frase nao diz qual metodo levantou (o quem ja esta na mao, e index/rindex compartilham o codigo com find/rfind), nem o que se procurava.

**Proposta:** "%s() nao achou '%.60s' na str; find() devolve -1 em vez de levantar", quem, sub->chars

→ 

### 125. `vm/poolscript_vm.c:5376`

```
"indice fora do intervalo em pop()"
```

**Por que não serve:** o operador de indexacao ja diz "indice 7 fora do tamanho de list (3 itens)"; o pop() ficou pra tras e nao mostra nem o indice pedido nem o tamanho.

**Proposta:** "pop(): indice %lld fora do tamanho de list (%d itens)", args[0].as.i, l->len

→ 

### 126. `vm/poolscript_vm.c:3123 (mesma frase em 3130; a gemea de flo em 3169)`

```
"valor invalido: nao da pra converter '%s' em int", s->chars
```

**Por que não serve:** e exatamente o caso "string que nao e numero": a str CONTEM ou nao digitos, e a frase nao diz o que faltou — int("1.5") e int("0x10") caem aqui e o usuario nao descobre por que.

**Proposta:** "int() nao converte '%s': a str so aceita digitos e sinal; pra decimal passe por flo()" — e a de flo: "flo() nao converte '%s': a str precisa ser um numero decimal, como '1.5' ou '1e3'"

→ 

### 127. `vm/poolscript_vm.c:3134 (gemea em 3173 pra flo)`

```
"operacao invalida: int() nao aceita este tipo"
```

**Por que não serve:** "este tipo" nao diz nada: nem o que veio, nem de quais tipos int() sabe converter; e o prefixo repete o TypeError.

**Proposta:** "int() nao converte %s; converte str, flo e bool", nome_do_tipo_valor(v) — e "flo() nao converte %s; converte str, int e bool"

→ 

### 128. `vm/poolscript_vm.c:3294 (gemea em 3307 pra round)`

```
"operacao invalida: abs() so aceita numero"
```

**Por que não serve:** "numero" nao e um tipo da linguagem — os tipos sao int e flo — e a frase nao diz o que chegou.

**Proposta:** "abs() so aceita int ou flo, recebeu %s", nome_do_tipo_valor(args[0]) — e "round() so aceita int ou flo, recebeu %s"

→ 

### 129. `vm/poolscript_vm.c:3799`

```
"operacao invalida: %s() espera iteravel ou varios valores", nome
```

**Por que não serve:** "iteravel" e o jargao que ele reclamou, e "varios valores" nao mostra a forma de chamar — o usuario le e continua sem saber a sintaxe certa.

**Proposta:** "%s() precisa de uma list ou tup pra percorrer, ou dos valores soltos: %s(1, 2, 3)", nome, nome

→ 

### 130. `vm/poolscript_vm.c:3800`

```
"valor invalido: %s() de sequencia vazia", nome
```

**Por que não serve:** "sequencia" e jargao (o tipo e list ou tup) e a frase e um rotulo, nao um relato: nao diz que nao havia nada pra comparar.

**Proposta:** "%s() nao tem o que comparar: a %s esta vazia", nome, nome_do_tipo_valor(args[0])

→ 

### 131. `vm/poolscript_vm.c:3807 (mesma frase em 3816)`

```
"operacao invalida: %s() entre tipos incompativeis", nome
```

**Por que não serve:** o I14 ja fez o operador dizer QUAIS tipos ("'+' entre tipos incompativeis: int + str"), mas min/max ficaram na versao antiga — max([1,"a"]) nao revela que o choque e int com str.

**Proposta:** "%s() nao compara %s com %s", nome, nome_do_tipo_valor(melhor), nome_do_tipo_valor(item)

→ 

### 132. `vm/poolscript_vm.c:5465`

```
"operacao invalida: sort() entre tipos incompativeis"
```

**Por que não serve:** mesmo buraco do I14 no sort(): numa list grande e misturada, sem os tipos o usuario nao tem por onde comecar a procurar o item errado.

**Proposta:** "sort() nao compara %s com %s", nome_do_tipo_valor(a), nome_do_tipo_valor(b)

→ 

### 133. `vm/poolscript_vm.c:4778`

```
"join() so junta str"
```

**Por que não serve:** nao aponta o item culpado nem o tipo dele; o bytes.concat da linha 9047 ja faz certo ("item %d nao e bytes (%s)") — o join ficou atras do vizinho.

**Proposta:** "join() so junta str: o item %d e %s", i, nome_do_tipo_valor(l->itens[i])

→ 

### 134. `vm/poolscript_vm.c:4261`

```
"%s() espera str", quem
```

**Por que não serve:** e o helper exige_str, que atende find/index/startswith/endswith/split/replace — a frase diz o que era pra vir mas nunca o que veio, e e o erro de tipo mais encontrado dos metodos de str.

**Proposta:** "%s() espera str, recebeu %s", quem, nome_do_tipo_valor(v)

→ 

### 135. `vm/poolscript_vm.c:3581`

```
"operacao invalida: range() nao aceita esse texto"
```

**Por que não serve:** "esse texto" nao mostra o texto nem diz o que estava errado nele — e o mesmo defeito do "string que nao e numero".

**Proposta:** "range() so aceita int; '%.60s' nao e um int escrito em str", s

→ 

### 136. `vm/poolscript_vm.c:3561`

```
"operacao invalida: range() so aceita numero"
```

**Por que não serve:** "numero" nao e tipo da linguagem e a frase omite o que chegou; range() aceita int, bool, flo e str numerica, nada disso aparece.

**Proposta:** "range() so aceita int, recebeu %s", nome_do_tipo_valor(args[i])

→ 

---

## motor: operador e indice — 23

### 137. `vm/poolscript_vm.c:18579 e 18584`

```
TypeError: comparacao entre tipos incompativeis
```

**Por que não serve:** não diz o operador, não diz os tipos e não diz os valores — e o `+`/`-` logo acima JÁ dizem, então a mesma falha tem duas qualidades de mensagem no mesmo arquivo.

**Proposta:** seguir a forma que o '+' já usa: `'<' entre tipos incompativeis: str < int`. O macro CMP já tem `#C_OP` e `nome_do_tipo_valor` à mão — é a mesma linha do ERRO_TF do ADD.

→ 

### 138. `vm/poolscript_vm.c:20624 e 20626`

```
OutputUnexpectedValues: valores demais para desempacotar
```

**Por que não serve:** não diz quantos vieram nem quantos cabiam — que é exatamente a única informação que resolve o erro; `l->len` e `n_alvos` estão os dois na mão, a duas linhas dali.

**Proposta:** `valores demais para desempacotar: 3 valores para 2 nomes` e `valores insuficientes para desempacotar: 1 valor para 2 nomes` (concordando singular/plural como as mensagens de indice já fazem).

→ 

### 139. `vm/poolscript_vm.c:3134 e 3173`

```
TypeError: operacao invalida: int() nao aceita este tipo
```

**Por que não serve:** três camadas de rótulo (`TypeError:` + `operacao invalida:` + `este tipo`) antes de não dizer nada: não nomeia o tipo que veio nem o que o int() aceita.

**Proposta:** `int() nao converte list: aceita str, int, flo e bool`. Idem em flo(). (`nome_do_tipo_valor` está definido em 3185, depois destas duas funções — precisa de um protótipo acima.)

→ 

### 140. `vm/poolscript_vm.c:3656, 3699, 3725`

```
TypeError: operacao invalida: list() nao itera este tipo
```

**Por que não serve:** "nao itera este tipo" é jargão de implementação (é o `iteravel_tam` falando, não a linguagem), não nomeia o tipo e não diz o que serviria.

**Proposta:** `list() nao converte int: aceita list, tup, str, dict ou generator`. Mesma forma em tup() e dict().

→ 

### 141. `vm/poolscript_vm.c:3294, 3307, 3561, 3584`

```
TypeError: operacao invalida: abs() so aceita numero
```

**Por que não serve:** "numero" não é um tipo da linguagem — os tipos são `int` e `flo`; quem lê não sabe se bool ou uma str numérica contam.

**Proposta:** `abs() so aceita int ou flo, veio str`. Idem round() (3307) e range() (3561/3584, onde a lista real é `int`, `flo`, `bool` e str numérica).

→ 

### 142. `vm/poolscript_vm.c:9097`

```
ValueError: valor inválido: bytes.get: índice 9 fora do range (0..2)
```

**Por que não serve:** mistura português com inglês ("fora do range"), repete o rótulo duas vezes, e dá ValueError com um texto diferente do `b[9]`, que para o MESMO erro dá `IndexError: indice 9 fora do tamanho de bytes (3 bytes)`.

**Proposta:** usar o texto que o indexador já usa: `bytes.get: indice 9 fora do tamanho de bytes (3 bytes)`, levantando IndexError.

→ 

### 143. `vm/poolscript_vm.c:19949`

```
AttributedValueError: variável x esperava int
```

**Por que não serve:** diz o que era esperado mas não o que chegou — em `int x = 5.9` o usuário não vê que o problema é o `.9`, e a mensagem não avisa que `int x` não trunca.

**Proposta:** `variável x foi declarada int e recebeu flo (5.9)` — o valor cabe via valor_para_texto, que o INDEX_GET já usa pra imprimir a chave do dict.

→ 

### 144. `vm/poolscript_vm.c:19671`

```
RuntimeError: tipo nao fatiavel
```

**Por que não serve:** nomeia o problema com um adjetivo inventado ("fatiavel") e não diz o tipo nem o que aceita fatia — as linhas vizinhas do mesmo opcode já sabem dizer o tipo.

**Proposta:** `nao da pra fatiar int: [ini:fim] so vale em str, list, tup e bytes` (com TypeError, como o erro de passo logo abaixo).

→ 

### 145. `vm/poolscript_vm.c:19329 e 19356`

```
RuntimeError: deslocamento negativo
```

**Por que não serve:** substantivo solto: não diz o operador, não diz o valor, não diz o que fazer — e o mesmo erro no caminho bignum (linha 8487) sai como TypeError.

**Proposta:** `'<<' nao aceita deslocamento negativo: -1` (e igualar o tipo de exceção nos dois caminhos).

→ 

### 146. `vm/poolscript_vm.c:20762`

```
TypeError: 'in' nao se aplica a este tipo
```

**Por que não serve:** "este tipo" não nomeia nada, e a frase não diz onde o `in` funciona.

**Proposta:** `'in' nao se aplica a int: procura em list, tup, dict ou str`.

→ 

### 147. `vm/poolscript_vm.c:18530`

```
TypeError: '-' unario em tipo invalido
```

**Por que não serve:** "tipo invalido" não diz qual, e "unario" é vocabulário de compilador — quem escreveu `-x` não pensa em "menos unário".

**Proposta:** `nao da pra trocar o sinal de str: '-' so vale em int, flo e bool`.

→ 

### 148. `vm/poolscript_vm.c:19273, 19289, 19305, 19321, 19348, 19365`

```
TypeError: '|' exige int
```

**Por que não serve:** não diz o que chegou nem de que lado — com dois operandos, "exige int" deixa o usuário adivinhando qual dos dois está errado; o '+' na mesma tela já diz.

**Proposta:** `'|' so funciona entre int: int | str`, no mesmo formato do ADD. Vale para |, ^, &, <<, >> e ~ (este com um operando só: `'~' so funciona em int, veio str`).

→ 

### 149. `vm/poolscript_vm.c:5376`

```
IndexError: indice fora do intervalo em pop()
```

**Por que não serve:** não diz o índice pedido nem o tamanho da lista, enquanto `l[9]` no mesmo motor diz `indice 9 fora do tamanho de list (2 itens)` — duas mensagens para o mesmo erro, e a pior é a do método.

**Proposta:** `indice 9 fora do tamanho de list (2 itens) em pop()`, reaproveitando a redação do INDEX_GET.

→ 

### 150. `vm/poolscript_vm.c:3123, 3130, 3169`

```
ValueError: valor invalido: nao da pra converter 'abc' em int
```

**Por que não serve:** o rótulo `valor invalido:` repete o que o `ValueError:` já disse, e a frase não conta POR QUE não deu — `int("0x1f")` e `int("1.5")` falham pelo mesmo texto sem dizer que só entram dígitos decimais.

**Proposta:** `int() nao converte 'abc': so digitos decimais, com '+' ou '-' opcional` (e o par para flo em 3169).

→ 

### 151. `docs/builtins/builtins.md:27`

```
| [`max`](max/max.md) | `max(lista) | max(a, b, ...)` | Maior valor de uma lista ou dos argumentos. |
```

**Por que não serve:** O `|` dentro da crase não está escapado, então a linha vira 4 células numa tabela de 3 colunas e a coluna "o que faz" cai fora do render — some da página; o mesmo acontece com `min` (linha 28) e `range` (linha 31).

**Proposta:** | [`max`](max/max.md) | `max(lista)` ou `max(a, b, ...)` | Maior valor de uma `list`, ou dos argumentos soltos. |

(mesma troca de `|` por "ou" em `min` e `range`; se quiser manter o pipe, escapar como `\|`)

→ 

### 152. `docs/string/string.md:41`

```
| [`lstrip`](lstrip/lstrip.md) | `s.lstrip(chars=Null)` | Remove da ponta ESQUERDA. |
```

**Por que não serve:** Remove o quê da ponta esquerda? A frase não tem objeto — e a linha do `strip`, doze linhas abaixo, mostra que dava pra dizer ("Remove espaços (ou os chars dados) das duas pontas"); o `rstrip` na linha 53 tem o mesmo buraco.

**Proposta:** | [`lstrip`](lstrip/lstrip.md) | `s.lstrip(chars=Null)` | Remove espaços em branco da ponta ESQUERDA; com `chars`, remove qualquer caractere daquele conjunto, em qualquer ordem. |

(e o espelho para `rstrip`, na ponta DIREITA)

→ 

### 153. `docs/string/string.md:28`

```
| [`isdecimal`](isdecimal/isdecimal.md) | `s.isdecimal()` | True se só tem dígitos decimais. |
| [`isdigit`](isdigit/isdigit.md) | `s.isdigit()` | True se só tem dígitos. |
| [`isnumeric`](isnumeric/isnumeric.md) | `s.isnumeric()` | True se só tem caracteres numéricos. |
```

**Por que não serve:** As três descrições são sinônimas na leitura, então o índice não deixa escolher entre os três — enquanto 12-metodos §12.1 avisa que elas "não são sinônimos" e mostra que `"²".isdigit()` é True e `"²".isdecimal()` é False.

**Proposta:** | [`isdecimal`](isdecimal/isdecimal.md) | `s.isdecimal()` | True se só tem dígito decimal, de qualquer escrita (`0-9`, `٣`, `३`, `３`). O mais restrito dos três. |
| [`isdigit`](isdigit/isdigit.md) | `s.isdigit()` | True para os decimais **mais** sobrescrito/subscrito (`²`, `⁷`, `₄`). |
| [`isnumeric`](isnumeric/isnumeric.md) | `s.isnumeric()` | True para os anteriores **mais** fração e numeral romano (`½`, `Ⅷ`). O mais amplo. |

→ 

### 154. `docs/linguagem/12-metodos-string-list-dict.md:104`

```
| `maketrans(de, para)` / `translate(tab)` | tabela de tradução caractere-a-caractere e sua aplicação |
```

**Por que não serve:** A coluna "Faz" só recombina as palavras dos dois nomes — não diz que `maketrans` é chamado numa `str` qualquer, que devolve a tabela, nem que é o `translate` que recebe essa tabela; e não há exemplo em lugar nenhum da página.

**Proposta:** | `maketrans(de, para)` | Monta a tabela que troca cada caractere de `de` pelo da mesma posição em `para`. Pode ser chamado em qualquer `str` — o alvo não importa, só os dois argumentos. |
| `translate(tab)` | Aplica a tabela do `maketrans` e devolve a `str` nova. |

```ps
t = "".maketrans("áéí", "aei")
post("café".translate(t))    // cafe
```

→ 

### 155. `docs/PoolScript.md:693`

```
| `os` | `import os` | Sistema operacional |
| `hash` | `import hash` | Hash de senhas |
| `jwt` | `import jwt` | Tokens JWT |
| `manpu` | `import manpu as mp` | Manipulação de arquivos |
```

**Por que não serve:** A coluna "Descrição" repete o nome da lib traduzido ou nomeia a categoria — depois de ler a tabela inteira o usuário ainda não sabe qual lib abrir pra ler um CSV, nem o que `hash` faz que `jwt` não faz.

**Proposta:** | `os` | `import os` | Arquivos e pastas, variáveis de ambiente, rodar comando do terminal |
| `hash` | `import hash` | Guardar senha com segurança: `crypt()` gera o hash, `check()` confere |
| `jwt` | `import jwt` | Assinar e verificar token de login: `gen()` e `check()` |
| `manpu` | `import manpu as mp` | Ler e gravar CSV, XLSX e texto estruturado |

(mesma passada nas outras linhas — dizer o que a lib resolve, não o assunto dela)

→ 

### 156. `docs/PoolScript.md:620`

```
| `post.flush(texto, delay=N)` | Efeito de digitação |
```

**Por que não serve:** "Efeito de digitação" descreve a impressão que dá, não o que a chamada faz — o leitor não descobre que ela imprime caractere por caractere nem em que unidade está o `delay`.

**Proposta:** | `post.flush(texto, delay=N)` | Imprime o texto um caractere por vez, esperando `delay` segundos entre eles (aceita fração: `0.05`) |

→ 

### 157. `docs/PoolScript.md:626`

```
| `load()` | Carrega o .env |
```

**Por que não serve:** Não diz para onde carrega, o que devolve, nem o que acontece se a variável já existir — três coisas que a página de `load` documenta e que decidem se o leitor usa `os.getenv` depois.

**Proposta:** | `load(caminho=Null)` | Lê um `.env` e põe as variáveis no ambiente (leia depois com `os.getenv`). Devolve um `dict` com o que leu; variável já definida no ambiente **não** é sobrescrita; sem `.env`, devolve `{}` |

→ 

### 158. `docs/LANGUAGE.md:1139`

```
Encontradas e documentadas durante uma auditoria profunda desta versão (testes + correções, ver `CHANGELOG.md`):
```

**Por que não serve:** Não existe `CHANGELOG.md` no repositório — quem for atrás do histórico que a frase promete não acha nada e fica sem saber onde olhar.

**Proposta:** Encontradas e documentadas durante uma auditoria profunda desta versão (o histórico de cada correção está no `git log`):

→ 

### 159. `docs/linguagem/12-metodos-string-list-dict.md:14`

```
## 12.1. Métodos de `str` (54)
```

**Por que não serve:** `docs/string/string.md` lista 55 e diz que "a contagem sai da fonte, não da memória de ninguém" — com dois números diferentes, o leitor não sabe se a página que está lendo deixou algum método de fora (o resumo na linha 259 repete o 54).

**Proposta:** ## 12.1. Métodos de `str` (55)

(e o mesmo acerto no resumo §12.4, linha 259 — conferir contra `docs/string/string.md`, que hoje lista as 55 linhas)

→ 

---

# SÓ MAL ESCRITAS

Entende-se. Mas está feio.

## doc: secoes "## Erros" — 3

### 160. `docs/builtins/abs/abs.md:17`

```
- **TypeError** — o argumento não é `int` nem `flo`
```

**Por que não serve:** A tabela de parâmetros da mesma página aceita `bool`, e `abs(true)` devolve `1` — a linha de erro contradiz a linha de cima.

**Proposta:** - **TypeError** — `n` não é `int`, `flo` nem `bool`

→ 

### 161. `docs/builtins/addEnd/addEnd.md:18`

```
- **TypeError** — o primeiro argumento não é `list`
```

**Por que não serve:** Diz "o primeiro argumento" quando a tabela logo acima já deu nome a ele (`lista`), e não avisa que `tup` também é recusada — que é a confusão real, já que `tup` é imutável.

**Proposta:** - **TypeError** — `lista` não é `list`; `tup` não serve, ela não pode ser mutada

→ 

### 162. `docs/builtins/addStart/addStart.md:18`

```
- **TypeError** — o primeiro argumento não é `list`
```

**Por que não serve:** Mesmo caso do addEnd: ignora o nome do parâmetro e não explica por que `tup` cai aqui.

**Proposta:** - **TypeError** — `lista` não é `list`; `tup` não serve, ela não pode ser mutada

→ 

---

## libs em C — 5

### 163. `vm/poolscript_vm.c:14810 (e 14622)`

```
snprintf(vm->erro, sizeof(vm->erro), "%.200s", erro);
```

**Por que não serve:** corta a mensagem do driver em 200 bytes sem olhar onde, e foi o que produziu o "…port 5432 fai" acima — o usuario le uma frase truncada no meio.

**Proposta:** Copiar ate sizeof(vm->erro) (o buffer ja e maior que 200) e, se ainda assim nao couber, cortar no ultimo espaço e fechar com " …" em vez de partir palavra.

→ 

### 164. `vm/poolscript_vm.c:11322`

```
%s: timed out
```

**Por que não serve:** frase em ingles no meio de mensagens em portugues, e nao diz de quanto era o prazo que estourou.

**Proposta:** "%s: a operacao expirou (prazo de %.0fs definido em settimeout)"

→ 

### 165. `vm/poolscript_vm.c:11235`

```
IOError: arquivo não encontrado: arquivo não encontrado: /tmp/x.pdf
```

**Por que não serve:** a frase esta escrita duas vezes na mesma mensagem — parece defeito do motor, e ainda por cima o fopen pode ter falhado por permissao, nao por ausencia.

**Proposta:** "nao consegui abrir o anexo '%s': %s" (com strerror traduzido), ou no minimo tirar a duplicata: "arquivo não encontrado: %s"

→ 

### 166. `vm/poolscript_vm.c:13625 (idem 13398, 13405, 13581)`

```
RuntimeError: Erro ao gerar QR Code: dados grandes demais pro QR (max ~2953 bytes)
```

**Por que não serve:** "Erro ao gerar QR Code" gagueja com o RuntimeError que ja vem na frente, e a parte util nao diz quantos bytes o usuario mandou.

**Proposta:** Tirar o prefixo e passar o tamanho real: BERRO(vm, "RuntimeError", "%.200s", erro) com ps_qr.c:360 virando "os dados nao cabem num QR: %d bytes, o limite e 2953"

→ 

### 167. `vm/ps_pkg.c:328`

```
Erro: nao consegui buscar o registry em https://x/i.json:
```

**Por que não serve:** quando ps_http_request falha mas r.status nao e -1, o %s recebe "" e a mensagem termina em dois-pontos e nada.

**Proposta:** So imprimir o sufixo quando houver texto: "Erro: nao consegui buscar o registry em %s%s%s" com " — " + r.erro apenas se r.erro[0], senao fechar a frase em "(sem resposta do servidor)"

→ 

---

## motor: builtins e metodos — 2

### 168. `vm/poolscript_vm.c:5370`

```
"pop() de lista vazia"
```

**Por que não serve:** escreve "lista" onde o tipo e list, e a frase e um rotulo: nao diz o que aconteceu (nao havia item pra tirar).

**Proposta:** "pop() nao tem o que tirar: a list esta vazia"

→ 

### 169. `vm/poolscript_vm.c:3379 e 3384`

```
"ord() espera um caractere"  /  "operacao invalida: ord() espera UM caractere"
```

**Por que não serve:** duas grafias pra mesma ideia (uma com UM maiusculo), e nenhuma separa os dois casos reais: ord("") caiu na primeira, ord("abc") na segunda, e o usuario le a mesma coisa nas duas.

**Proposta:** vazio: "ord() espera uma str de 1 caractere, recebeu str vazia"; longo demais: "ord() espera uma str de 1 caractere, recebeu %d", conta_cp(s)

→ 

---

## motor: operador e indice — 5

### 170. `vm/poolscript_vm.c:9094 (macro BY_ERRO_TIPO em 8817)`

```
TypeError: operação inválida entre os tipos: bytes.get: índice deve ser inteiro
```

**Por que não serve:** "entre os tipos" é falso — só existe um tipo envolvido; e "inteiro" não é o nome do tipo (`int`), além do rótulo duplicado.

**Proposta:** `bytes.get: o indice precisa ser int, veio str` — e tirar o "entre os tipos" do macro BY_ERRO_TIPO, que o usa em dezenas de casos de um argumento só.

→ 

### 171. `vm/poolscript_vm.c:19685 e 19682`

```
RuntimeError: passo do slice nao pode ser zero  /  TypeError: passo do slice precisa ser int, veio str
```

**Por que não serve:** "slice" é inglês no meio de uma frase em português, e a linguagem escreve isso como `[ini:fim:passo]`; ainda por cima os dois erros irmãos saem com tipos de exceção diferentes.

**Proposta:** `o passo da fatia nao pode ser 0` e `o passo da fatia precisa ser int, veio str`, ambos como ValueError/TypeError coerentes entre si.

→ 

### 172. `vm/poolscript_vm.c:20758`

```
TypeError: 'in' em str espera str
```

**Por que não serve:** repete "str" duas vezes numa frase telegráfica e não diz o que veio; parece saída de máquina.

**Proposta:** `'in' em str so procura str, veio int`.

→ 

### 173. `vm/poolscript_vm.c:19502`

```
TypeError: tipo nao indexavel: int
```

**Por que não serve:** diz o tipo (bom), mas "indexavel" é jargão e a frase não diz o que fazer nem onde `[ ]` vale.

**Proposta:** `int nao aceita [ ]: so str, list, tup, dict e bytes tem indice`.

→ 

### 174. `vm/poolscript_vm.c:3039 (macro EXIGE_ARGS, usado por str/int/flo/bool)`

```
TypeError: str() espera 1 argumento(s)
```

**Por que não serve:** o "(s)" é saída de máquina crua — é a marca registrada do que ele está reclamando; e não diz quantos vieram.

**Proposta:** escolher a forma: `str() espera 1 argumento, veio 2` / `... espera 2 argumentos, veio 1`, com o plural resolvido por `quant == 1 ? "argumento" : "argumentos"` (o mesmo truque de `item`/`itens` que o IndexError já usa).

→ 

---

# O que os céticos acharam a mais

Cada área foi varrida duas vezes, a segunda por outro caminho. Isto é o que
a primeira passada não viu:

- vm/poolscript_vm.c:3784 — "operacao invalida: sum() so soma numero" (`sum(["1","2"])`). Usa exatamente a palavra que o dono baniu ("numero" onde é `int`/`flo`) e fica UMA LINHA de distância da 3759 que ele reescreveu: ele consertou o "que tipo você passou pro sum()" e deixou o "que tipo tem o item DENTRO da list", que é justamente o que o usuário esbarra somando uma list de str. Deveria virar algo como "sum() so soma int e flo: o item %d e %s", i, nome_do_tipo_valor(item) — a mesma forma que ele propôs pro join() em 4778.

- `map()` e `filter()` inteiros (linhas 17914 e 17939): "map() espera uma lista como primeiro argumento". Escreve "lista" onde o tipo e `list`, nao diz o que chegou, e — pior — a causa numero um desse erro e o usuario escrever na ordem do Python (`map(f, l)`), e a mensagem nao ajuda em nada: confirmei que `map(f, [1,2])` erra e `map([1,2], f)` funciona. Devia ser algo como "map() recebe a list primeiro e a action depois: map(l, f); recebeu action no 1o argumento".

Outras que ele deixou passar na mesma area:
- 3782 "sum() so soma numero": nao diz QUAL item nem qual tipo. `sum([1,null])` so diz isso. E o caso comum do sum(), muito mais que `sum(3)` que ele reescreveu — e ele mesmo criou o molde

- `post(len(5))` → "TypeError: len() nao se aplica a este tipo" (vm/poolscript_vm.c:3025). É o builtin mais usado da linguagem, está no MESMO bloco que ele varreu (3134 int(), 3173 flo(), 3294 abs()), e tem o mesmíssimo "este tipo" que ele condenou em todos os outros. Devia ser `len() nao se aplica a int: vale em str, list, tup, dict e bytes`.

E o mesmo furo se repete em bloco, tudo confirmado rodando:
- 3830 sorted(), 3861 reversed(), 3878 enumerate(), 3902 zip() → "sorted() nao itera este tipo". Ele corrigiu list()/tup()/dict() (3656/3699/3725) e deixou os QUATRO irmãos com a frase idêntica. Consertar três de sete é justamente a feature meio-feita.
- min()/max() (`post(min(5))`) → "min() es

- by_nome() (vm/poolscript_vm.c:8821), a função que alimenta os textos do módulo bytes, chama dict de "json" e tup de "tuple" — os dois nomes errados de tipo, no mesmo trecho do macro BY_ERRO_TIPO (8817) que ele mandou mexer. Provado: `bytes.new({})` -> "bytes.new: não sei criar bytes de json"; `bytes.xor((1,), "a")` -> "bytes.xor: esperava bytes, recebeu tuple"; enquanto `type({})` diz `dict` e `type((1,))` diz `tup`. São 11 sítios contaminados. Se ele reescrever bytes.get sem tocar em by_nome, o `veio %s` novo vai sair dizendo "json". Faltou também: (1) a família "nao itera este tipo" tem 7 sítios, ele listou 3 — ficaram de fora sorted() 3830, reversed() 3861, enumerate() 3878 e zip() 3902, 

- docs/builtins/abs/abs.md:17 — "- **TypeError** — o argumento não é `int` nem `flo`", junto com bin.md:17, hex.md:17 e oct.md:17 ("o argumento não é `int`"). É mentira: `bool` passa nos quatro. `abs(true)` → 1, `bin(true)` → 0b1, `hex(true)` → 0x1, `oct(true)` → 0o1. Ele consertou EXATAMENTE essa forma em round.md ("`n` não é `int`, `flo` nem `bool`") e não varreu os quatro vizinhos idênticos — a categoria ficou pela metade. E a mensagem do motor por trás é a pior de todas: `TypeError: operacao invalida: abs() so aceita numero` (idem `round() so aceita numero`, `sleep() espera numero`, `range() so aceita numero`, `hora() espera numero`, `format: 'f' espera numero`) — "numero" é justamente a p

- `open` com modo invalido responde EM INGLES, e ele mexeu exatamente nesse arquivo (docs/builtins/open/open.md:18) sem ver. `./pool -e 'post(open("/tmp/zz.txt","q"))'` da:

  ValueError: valor inválido: invalid mode: 'q'

Fonte: vm/poolscript_vm.c:5995 — e o comentario da linha 5989 admite que so esta assim pra "casar com o interp". Numa lang em portugues isso e a pior mensagem da area toda: metade traduzida, metade nao, e nao diz quais modos existem. Devia ser algo como `modo 'q' nao existe; use "r", "w", "a", "rb", "wb" ou "ab"`.

Dois vizinhos da mesma varredura que ele tambem deixou passar:

1) A familia "nao itera este tipo" — e a reclamacao "tipo nao-iteravel" dele redita em portugues, 

- vm/ps_http.c:101 — `REDE(r, "NetworkError", "falha de conexão: %s", strerror(errno))`. Rodando `request.get("http://127.0.0.1:1/")` o motor cospe `NetworkError: falha de conexão: Connection refused`: strerror em INGLÊS no meio de uma frase em português, sem dizer em que host nem em que porta a conexão foi recusada — e `host` e `porta` estão em escopo ali (a função é `conecta_cru(host, porta, ...)`, e o ramo de timeout logo abaixo, na 316, já imprime a url). É o buraco mais grave da área: `request` é a lib que todo mundo usa, e ele listou de ps_http.c só a linha 318 ("sem resposta"), que é o caso raro. Pior: ele traduziu errno à mão em dois outros arquivos — ps_jinker.c:126 (EACCES/EADDRINUSE

- A família `este tipo` / `tipo nao …`: 15 mensagens do motor que dizem que o tipo está errado e NÃO dizem qual. É o molde exato de que ele reclamou ("tipo não-iterável"). Todas rodadas agora, vivas:

  ./pool -e 'post(len(5))'      -> TypeError: len() nao se aplica a este tipo        (vm/poolscript_vm.c:3025)
  ./pool -e 'post(5 in 3)'      -> TypeError: 'in' nao se aplica a este tipo         (:20762)
  ./pool -e 'post(list(5))'     -> TypeError: operacao invalida: list() nao itera este tipo
      (idem tup/dict/sorted/reversed/enumerate/zip — :3656,3699,3725,3830,3861,3878,3902)
  ./pool -e 'post(int([1,2]))'  -> TypeError: operacao invalida: int() nao aceita este tipo   (:3134, flo em :3173

- O prefixo que engole as QUATRO reescritas de e-mail dele. Todo erro de ps_mail.c sai pela macro MAIL_ERRO_REDE (vm/poolscript_vm.c:11058) como `OSError: erro de sistema/arquivo: <msg>`. Rodei:

    import mail
    s = mail.MailServer()
    s.conn("127.0.0.1", 9)
    -> OSError: erro de sistema/arquivo: [Errno 111] Connection refused

Quatro defeitos numa linha: "OSError" e "erro de sistema/arquivo" dizem a mesma coisa duas vezes; "sistema/arquivo" está errado — não tem arquivo nenhum, é rede; "[Errno 111]" é entulho de Python; e "Connection refused" é inglês cru sem host nem porta. Ele reescreve ps_mail.c:55/68/238/243, mas o prefixo sobrevive a todas — o resultado da lista dele seria `OSErr

- `TypeError: len() nao se aplica a este tipo` (`len(5)`, `len(3.2)`, `len(True)`). É literalmente o padrão que o dono reclamou ("tipo não-iterável"): não diz QUE tipo veio (`int`) nem o que `len()` aceita (`str`/`list`/`tup`/`dict`/`bytes`). Ele usou `len(5)` como exemplo em 10-excecoes.md:109, PoolScript.md:377 e LANGUAGE.md:1123 e nunca rodou. Pior: `len(null)` devolve `0` calado, então a regra que ele está documentando tem buraco. Três irmãs na mesma área, todas fora da lista dele:
- `t=(1,2); t[0]=5` → `RuntimeError: tipo nao suporta atribuicao por indice`. Não nomeia o tipo, não diz o motivo real (`tup` é imutável), e vem como `RuntimeError` num caso claramente de tipo. Mesma frase para 

---

# Sugestões que os céticos DERRUBARAM

Redação proposta que ficou pior que o original, ou que usou vocabulário
errado, ou que inventou comportamento que o motor não tem. Estão aqui pra
você ver o que eu quase escrevi:

- vm/poolscript_vm.c:3799 — trocar "%s() espera iteravel ou varios valores" por "%s() precisa de uma list ou tup pra percorrer, ou dos valores soltos: %s(1, 2, 3)". Isso é MENTIRA e o comentário duas linhas acima no próprio código já diz: "um argumento: itera. Vale string e dict, não só lista." Conferido no motor: `max("abc")` → `c`, `min("abc")` → `a`, `max({"a":1,"b":2})` → `b`. A frase de hoje é vaga mas verdadeira; a dele é específica e falsa — manda o usuário parar de fazer o que funciona. A frase certa seria "%s() percorre str, list, tup ou dict, ou compara os valores soltos: %s(1, 2, 3)".

- vm/poolscript_vm.c:3725 (e as gemeas 3656, 3699, 3830, 3861, 3878, 3902) — "%s() nao percorre %s; passe str, list, tup, dict, bytes ou range()".

`bytes` NAO entra nessa lista. `iteravel_tam` (3465) so devolve tamanho pra list/tup, str e dict; pra bytes devolve -1. Conferido no motor:

    ./pool -e 'post(list("ab".encode()))'
    TypeError: operacao invalida: list() nao itera este tipo

Ou seja: a mensagem manda o usuario passar bytes, ele passa bytes, e leva o mesmo erro de volta. Mensagem que anda em circulo e pior que a original, que pelo menos nao promete nada.

Ele tirou a lista de tipos do len() (linha 3025, onde bytes VALE mesmo) e colou na do list(), sem conferir que sao dois conjun

- O parêntese dele no primeiro item: "e no INDEX_GET, que diz `indice 1` quando o usuário escreveu `l[true]`". Isso está errado nos dois sentidos.

Primeiro, `l[true]` não é erro: com `l = [1,2]`, `post(l[true])` imprime 2. A coerção em 19411 é uma decisão deliberada e comentada no código ("`l[true]` é `l[1]`: bool é 0/1 na linguagem inteira, então recusar só aqui era incoerência"). Preservar o tipo original ali só teria efeito de fazer o TypeError disparar num caso que o motor aceita de propósito.

Segundo, a única mensagem que ele consegue atingir é o IndexError. Com `l = [1]`, `post(l[true])` hoje sai `indice 1 fora do tamanho de list (1 item)` — e esse "1" é o dado útil: é o que se compara

- A parte do INDEX_GET no primeiro item deixa a mensagem PIOR. Ele quer que `l[true]` pare de dizer `indice 1` e passe a mostrar o tipo/valor original (`true`). Mas hoje sai `IndexError: indice 1 fora do tamanho de list (1 item)` — a conta fecha sozinha na cabeça do leitor: 1 não cabe em 1 item. Trocando para "indice true fora do tamanho de list (1 item)" some justamente a informação que explica o erro (que `true` vale 1) e a frase deixa de fazer sentido. Nos operadores aritméticos a queixa dele é certa (`true - "a"` dizendo `int - str` esconde o bool que o usuário escreveu), mas aqui é o contrário: o certo seria `indice true (=1) fora do tamanho de list (1 item)`, não trocar 1 por true. Duas 

- docs/builtins/range/range.md:19 — ele troca "o argumento não é `int`" por "- **TypeError** — o argumento não é número: `str`, `list` e `dict` erram (`flo` passa, truncado)". Os fatos estão certos (conferi: `range(3.5)` → [0,1,2]; `range("a")`, `range([1])` e `range({"a":1})` erram), mas a redação REGRIDE: o original usava o nome do tipo, `int`, e ele o substituiu por "número" — a palavra exata que a regra de vocabulário proíbe onde o tipo é `int`/`flo`. Nesse ponto a doc passa a ecoar o defeito do motor (`range() so aceita numero`) em vez de corrigi-lo. Deveria ser "o argumento não é `int` nem `flo`". Dois primos menores: em chr.md ele escreve "- **TypeError** — `n` não é `int`", mas `chr(tr

- docs/builtins/range/range.md:19 — ele PIOROU o vocabulario.

  hoje: - **TypeError** — o argumento não é `int`
  dele: - **TypeError** — o argumento não é número: `str`, `list` e `dict` erram (`flo` passa, truncado)

O conteudo novo esta certo (confirmei: `range(1.5)` da [0], `range(3.9)` da [0,1,2], `range("a")`/`range([1,2])`/`range({})` erram), mas ele trocou o nome do tipo pela palavra proibida. A regra e explicita: nao escrever "numero" onde e `int`/`flo`. E o original ja usava a grafia certa — entao a reescrita e regressao justamente no eixo que o dono cobra. Devia ser "o argumento nao e `int` nem `flo`".

Piora que ele foi inconsistente consigo mesmo: em docs/builtins/round/round.md:1

- vm/ps_regex.c:319 — trocar "classe nao fechada" por "regex: faltou fechar o ']' aberto na posicao %d". Fica PIOR que o original, porque troca uma frase vaga-mas-verdadeira por uma frase precisa-e-falsa, em três camadas. (a) Semântica: quem se abre é o '[', não o ']' — "o ']' aberto" não existe. (b) A posição está errada: `le_classe()` faz `l->i++` pra passar o '[' logo na entrada (ps_regex.c:266) e NUNCA guarda o índice de abertura; quando bate o `rerro` da 319, `l->i` já é o fim do padrão. Confirmado rodando: `regex.compile("[abc")` responde `posicao 4` — o '[' está na posição 0. A mensagem dele apontaria o dedo pro fim da string dizendo que o colchete abriu ali. Pra cumprir o que ele prome

- docs/PoolScript.md:620 — `post.flush`. Ele pegou a linha vaga ("Efeito de digitação") e escreveu uma descrição rica e confiante: "Imprime o texto um caractere por vez, esperando `delay` segundos entre eles (aceita fração: `0.05`)". **`post.flush` não existe.**

  $ ./pool -e 'post.flush("oi", delay=0.02)'
  RuntimeError: membro inexistente: flush (em action)
  $ ./pool -e 'flush("oi", 0.01)'
  RuntimeError: variável não definida: flush
  $ ./pool -e 'post("oi", flush=0.01)'
  TypeError: esta funcao nao aceita argumento nomeado

O único `flush` do motor é `out.flush()` / `err.flush()`, ZERO argumentos, em vm/poolscript_vm.c:9445-9455 — é `fflush`, não digitação. `post.flush` é fantasma do int

- poolscript_vm.c:11322 — `"%s: a operacao expirou (prazo de %.0fs definido em settimeout)"`. Inventa comportamento que o motor não tem, e eu confirmei rodando:

    import sockets
    s = sockets.socket()
    s.settimeout(0)
    s.connect(("10.255.255.1", 80))
    -> connect: timed out          (volta NA HORA, sem esperar nada)

A macro SK_ERRNO (11320) manda EAGAIN, EWOULDBLOCK, **EINPROGRESS** e ETIMEDOUT todos pro mesmo "timed out". `settimeout(0)` é NÃO-BLOQUEANTE (o próprio comentário em 11272 diz isso), então o connect volta EINPROGRESS na hora — não expirou prazo nenhum. Com a frase dele o usuário leria "a operacao expirou (prazo de 0s definido em settimeout)" e sairia caçando um prazo

- docs/exceptions/exceptions.md:80 (e o gêmeo docs/PoolScript.md:382): "`IndexError` | índice fora da faixa, **lendo ou escrevendo** ... Vale pra `list`, `tup`, `str` e `bytes`". Inventa comportamento. Escrever em `tup`, `str` ou `bytes` NUNCA dá `IndexError` — dá `RuntimeError: tipo nao suporta atribuicao por indice`, e dá isso mesmo com índice dentro da faixa (`t=(1,2); t[0]=5` levanta, sendo 0 um índice válido). São imutáveis: não existe escrita pra estar "fora da faixa". Só `list` aceita atribuição por índice. A linha original ("índice inválido ao **escrever**: `l[99] = x`") era incompleta, mas não mentia; a dele mente em três dos quatro tipos que lista. O certo é separar: na LEITURA, `Ind

