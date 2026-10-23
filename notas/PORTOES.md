# Os portões: a linguagem inteira conferida a cada commit

Decisão do Kleber (2026-10-17), depois do `forAll`: um defeito que a doc
ensina e o `--check` recusa, achado por ele e não pela suíte. A cobertura não
pode depender de agente, de limite de uso nem de alguém lembrar de olhar. Os
portões são mecânicos, rodam no `make check` e cobrem a superfície
documentada da linguagem inteira: sintaxe, semântica, checador, motor com e
sem JIT, libs e editor.

O modelo é o das linguagens consolidadas. O Rust testa todo bloco da doc
(`cargo test --doc`): bloco é código até dizer o contrário. O Python roda os
exemplos da doc pelo `doctest`. A conformidade é por cláusula da
especificação, como o Test262 do JavaScript. E o compilador é testado por
programas gerados da gramática, comparando duas formas de rodar (Csmith,
YARPGen).

## A regra

- Cada portão entra no `make check`.
- O que ele acha vira item de `notas/CORRECAO-BASE.md`, com o programa, a
  promessa da doc e a gravidade.
- O portão guarda as falhas conhecidas em `teste/portoes_conhecidas.txt`,
  cada uma apontando o item aberto (`scripts/portoes_catraca.pr`). A chave é a
  página e a impressão do bloco (os 12 primeiros dígitos do SHA-256 do texto
  dele): mudou o bloco, a falha volta a ser olhada. A falha que só acontece
  num sistema leva o nome dele (`darwin`, `linux`, `windows`). Falha nova
  deixa o portão vermelho. Item consertado que continua no arquivo também
  deixa vermelho, o que força a lista a andar. Linha mal escrita no arquivo
  reprova. Não há pulo calado.

## P1: a doc inteira é teste

- Toda cerca diz o que é: ```ps, ```ps contexto, ```ps sem-rodar: motivo,
  ```saida, ```erro, ```entrada, ```assinatura, ```texto, ```bash, ```json.
  Cerca sem marca reprova (`scripts/audita_exemplos_doc.pr`); o
  `scripts/marca_cercas.pr` propõe a marca e troca só a linha da cerca.
- Portão `check`: todo ```ps passa no `--check`, com os ```ps contexto da
  página na frente (`scripts/audita_exemplos_doc.pr`, que aceita uma página:
  menos de 1 s).
- Portão `roda`: todo ```ps roda duas vezes, com o JIT e sem
  (`scripts/confere_saida_doc.pr`). Com ```saida, o stdout tem que ser aquele;
  com ```erro, o stderr contém aquele trecho; sem ```erro, sai com 0 (o erro
  que o bloco anuncia em comentário, `# Tipo: frase`, vale como esperado);
  sem ```saida, as duas rodadas imprimem o mesmo, e a saída que muda sozinha
  (hora, sorteio) só não se compara.
- O contexto entra só no bloco que usa um nome dele: o bloco que passa no
  `--check` sozinho roda sozinho, como quem lê vai rodar, e o número das
  linhas nas mensagens não anda. Nesse caso o contexto roda antes, na mesma
  pasta, pra criar o que a página pressupõe (um arquivo, uma tabela).
- Cada rodada é isolada, porque o exemplo faz o que a doc ensina (apaga
  arquivo, abre porta, manda sinal, dispara processo; o exemplo do
  `docs/os/run/run.md` executa `rm -rf ~` de propósito, pra mostrar o perigo
  do `os.cmd`): pasta própria e vazia, que é o HOME dele; ambiente limpo; no
  macOS o `sandbox-exec` (escrita só na pasta, rede só no loopback e fora das
  portas de serviço, como a 8080, sinal só pros próprios filhos); no Linux o
  `bwrap` (disco só de leitura fora da pasta, rede e processos num espaço
  próprio); prazo por rodada. **Nenhum exemplo roda sem o isolamento**: no
  Linux sem o `bwrap` e no macOS sem o `sandbox-exec`, o portão para vermelho
  dizendo o que falta; no Windows, que não tem isolador, o portão `roda` não
  roda nenhum exemplo, diz isso e sai com 0 (decisão do Kleber, 2026-10-23).
  O portão `check` roda lá.
- Cada rodada roda num grupo de processos próprio (o `os.run` sem `capture`
  faz `setsid`), e o grupo inteiro morre no prazo e no fim: o servidor que o
  exemplo abre não fica órfão escutando na porta. O grupo morre pelo
  `kill -9 -- -<grupo>`, a forma do POSIX: o número negativo vem depois do
  `--`, e o sinal vai no 1º argumento (com `-s KILL`, o procps toma um grupo
  pequeno, como o 83, por sinal). Sem o `--`, o `kill` do procps (Ubuntu
  24.04) lê só o 1º dígito do número: em 2026-10-22, com o portão rodando
  como root na VPS, `kill -9 -1426999` virou `kill(-1, 9)` e matou todos os
  processos da máquina (caíram o Postgres, o e-mail e as APIs). Por isso,
  antes do 1º exemplo, o portão prova com o sinal 0 que o `kill` da máquina
  acerta o grupo: o grupo 1000000000, que não existe em sistema nenhum, tem
  que dar "No such process" com esse número, e um `sleep` de verdade tem que
  ser achado e morto. Se a prova falha, nenhum exemplo roda.
- Bloco que precisa de serviço de fora (servidor que fica escutando, banco,
  e-mail, rede) roda só no `--check` e leva a marca ```ps sem-rodar: motivo.
  O relatório lista cada um, com o motivo.
- Fragmento é o bloco com `...` no lugar de código, que o `--check` recusa:
  ele sai listado e não roda. O `...` num comentário ou num texto
  (`conn.execute(...)`) não faz fragmento; o bloco é conferido e roda como
  os outros.
- ```saida rc=N (ou ```erro rc=N) diz o código de saída que o exemplo
  devolve de propósito (o `sys.exit(3)`).
- O bloco que só para por importar um módulo de exemplo (`import mymod`,
  `from '../pacote/modulo.pr' import x`) sai numa lista, como no portão
  `check`.
- No portão `roda`, a falha de um bloco que já está na catraca do `check`
  (não compila) conta como conhecida.

Medido na entrada (2026-10-17): 1.773 blocos em 467 páginas; 836 sem marca,
marcados (659 ```ps, 140 ```assinatura, 28 ```texto, 5 ```bash, 2 ```json, 1
```erro); dos 1.223 ```ps conferidos, 358 recusados, em 145 páginas, quase
todos por contexto não escrito. Com o contexto da `channel.md`, o portão pegou
sozinho o `app.channel(forAll=msg)` recusado.

Medido em 2026-10-18, com o contexto de cada página escrito e as marcas
erradas da primeira passada consertadas (assinatura com `->`, árvore de
pastas, tabela de operadores marcadas como programa):

| portão | blocos | falhas | situação |
|---|---|---|---|
| `check` | 1.335 | 41 | todas itens abertos da lista; portão verde com a catraca |
| `roda` | 1.185 rodados, 298 com a saída conferida | 228 na primeira rodada | em classificação |

Fechado em 2026-10-22, medido no Mac:

| portão | blocos | de fora (com o motivo) | falhas conhecidas | situação |
|---|---|---|---|---|
| `check` | 1.368 conferidos | 24 fragmentos | 41, todas itens abertos | verde |
| `roda` | 1.116 rodados com JIT e sem, 306 com a saída conferida | 94 `sem-rodar`, 24 fragmentos, 17 de módulo de exemplo | 27, todas itens abertos (5 só fora do Linux) | verde |

Conferido no Linux em 2026-10-23, na VPS (`make check` dentro de
`unshare --pid`, com o `bwrap`): os mesmos blocos, os dois portões verdes. O
`roda` conta 63 falhas conhecidas contra 68 no Mac (a conta junta as do
`check`): as 5 que só falham fora do Linux não falham lá.

## P2: o checador e o motor concordam

`scripts/confere_checador.pr`, no `make check` depois do portão `roda`. Cada
membro que o motor publica no `--metadata` é chamado do mesmo jeito dos dois
lados:

- num programa com o receptor DECLARADO com o tipo dele (`Process p =
  os.run(...)`), uma chamada por linha: o `--check` diz quais recusa, e com
  que frase. Chamado direto na expressão, o checador não sabe o tipo do que
  uma função de união devolve (`os.run` dá `int|str|Process`);
- no mesmo programa com o membro guardado num `Object`, rodando, com o JIT e
  sem: o checador não confere a chamada, e quem decide é o motor.

Quem é chamado: os builtins, as funções e os valores de módulo (o
`jinker.cors` é valor e se chama), os métodos e as propriedades dos tipos (a
propriedade é lida, e chamada: o `app.channel` se chama). Os receptores saem
de `scripts/portoes_donos.pr`, a mesma tabela do `scripts/mede_aridades.pr`,
e dos que nascem de outro dono (`app.channel`, `app.route(...)`, o `psodbc`
no sqlite). Os tipos que só existem com a jinker no ar (`request` da rota,
`PoolFileUpload`, `Response`, `WsConnection`) rodam com uma jinker de verdade
no loopback da rodada isolada, numa porta nova a cada rodada. O Mongo fica de
fora (precisa de um `mongod`), e o relatório diz quantos membros e por quê.

As chamadas de cada membro: de 0 a um argumento a mais que os parâmetros, por
posição; o 1º argumento com cada tipo básico; cada parâmetro por nome (os do
`--metadata`, os da assinatura da doc e os que os exemplos da doc passam por
nome: o `forAll` só aparece num exemplo); um nome que não existe; a chamada
com os tipos da assinatura da doc; e o retorno guardado numa variável de um
tipo que não é o publicado (o checador tem que recusar a declaração, como o
motor recusa rodando).

E um membro que não existe (`p2_nao_existe()`, desde o 18.0.0), uma vez em
cada tipo de valor (os básicos com a amostra, menos o `Null`, que o `Object`
não guarda; os donos; os da jinker no ar) e em dois valores `funct` (o
`os.getenv` e o `len`): os dois lados têm que recusar, com a mesma frase.
Rodando, o receptor vai num `Object` e o membro é chamado nele (guardado
antes, o membro daria a frase da leitura, não a da chamada). Contra o
17.0.20, ela achou sozinha o motor-16 (`os.getenv.p2_nao_existe()` passava
no `--check`).

Reprova:

- o `--check` recusa e o motor aceita (a classe do `forAll`);
- o `--check` aceita e o motor recusa pelo que o checador promete pegar: a
  quantidade, o nome de parâmetro, o tipo do 1º argumento, o membro ou o nome
  que não existe, o tipo do retorno;
- os dois recusam com frases diferentes;
- rodar com o JIT e sem dá resultado diferente, ou o motor cai;
- o receptor não nasce, ou a jinker da rodada não fica de pé (nada se
  compararia: o portão para em vez de passar calado).

A chamada sobre um membro que vale `Null` rodando não se compara (o `Object`
não guarda `Null`) e sai contada no relatório. Tudo roda no isolamento dos
portões (`scripts/portoes_isola.pr`, o mesmo do P1): as chamadas mexem de
verdade.

O título de cada página de método também é conferido
(`scripts/audita_doc.pr`): o tipo sai do título, quando ele escreve o nome do
tipo, ou da pasta de cima do membro (`docs/str/upper/`), e os nomes e a ordem
dos parâmetros têm que ser os do motor (portão `titulo` na catraca). O método
sem página com a assinatura no título conta numa catraca de número, que só
pode descer (`TETO_SEM_PAGINA`, item doc-23).

"Todo programa da suíte" ficou pro P4: a regra "o `--check` aprova e quebra
rodando com o que ele promete pegar" só se decide sabendo o tipo de cada
chamada, e os casos da suíte usam `Object` de propósito pra o erro sair
rodando; quem sabe o tipo é o gerador do P4, que escreve o programa tipado. Os
blocos da doc o P1 já cobre.

Medido na entrada (2026-10-23, no Mac): 565 membros, 8.269 chamadas, cada uma
no `--check` e rodando com JIT e sem, em 3 minutos. 490 discordâncias, nenhuma
entre o JIT e sem ele, nenhuma queda:

| causa | chamadas | item |
|---|---|---|
| o `--check` recusa e o motor aceita: `app.channel(forAll=)`, `app.socket(path=, channel=)` | 5 | jinker-7 |
| o `--check` não confere a chamada dos membros de 11 tipos (os de fora da tabela medida) nem dos valores que se chamam (`jinker.cors`, `os.PoolFile`) | 395 | motor-11 |
| chamar o que não se chama: a propriedade como método (frases diferentes) e a constante de módulo (`sockets.AF_INET(0)` passa no `--check`) | 88 | motor-12 |
| o tipo do `app.route(...)` com dois nomes e o retorno do `register` | 1 | motor-13 |
| o `.type()`, que sai da linguagem (decisão do Kleber) | 1 | motor-14 |

No 18.0.0 (2026-10-23, no Mac) o `.type()` saiu (motor-14 consertado, e a
linha dele saiu da catraca) e entrou a chamada de membro que não existe: 564
membros, 8.292 chamadas, 36 delas de membro que não existe. As 491
conhecidas: as 490 de antes, menos a do motor-14, mais a do
`app.route(...).p2_nao_existe()` (motor-13, o `_RouteRegistrar` que o
checador não conhece) e a do `len.p2_nao_existe()` (motor-17, o `--check`
não sabe o tipo do nome do builtin). O membro de funct de módulo
(`os.getenv.p2_nao_existe()`, motor-16) foi consertado no mesmo lote.

Nas páginas de método: 206 conferidas, 192 batem; 14 dão aos parâmetros
outros nomes que o motor (doc-26); 73 métodos sem página com a assinatura no
título.

## P3: o editor e o motor concordam

`scripts/confere_editor.pr`, no `make check` depois do P2. Em cada bloco
```ps da doc, com os ```ps contexto anteriores da página na frente (os blocos
do portão `check`), o portão conversa com o servidor do editor
(`editor/vscode/server.js`, o mesmo do VS Code, do Neovim e do IntelliJ) como
o VS Code faz, e confere:

- **o diagnóstico**, em todo bloco: o que o servidor publica é o que o
  `--check --utf16` do mesmo arquivo diz. Cada erro tem a frase
  (`Tipo: mensagem`), o começo e o fim (`l2`/`c2`); cada aviso tem a frase e o
  começo. A posição é em UTF-16, a do protocolo. O servidor que não publica
  nada também reprova;
- **o hover**, no bloco que o `--check` aprova: em cada nome do bloco (os
  `IDENT` do `--tokens --utf16`), ele não diz "não definido" nem "rodando dá
  `NameError`", e não vem em branco (`docs/lsp.md`: "o hover nunca fica em
  branco");
- **o completion depois de `x.`**, no bloco que o `--check` aprova. Quando ele
  lista alguma coisa, a lista tem o membro que o próprio programa usa; a
  exceção é a chave de dict (`d.chave`), que vale qualquer nome. Quando `x` é
  declarado com um tipo do motor (`Jinker app = ...`, o parâmetro `str s`; o
  tipo sai do `--ast`), a lista é exatamente a desse tipo no `--metadata`.

As duas fontes do completion são independentes do editor: o programa que
passa no `--check` e a declaração escrita. Nenhum comando do motor diz o tipo
que o checador deduz de um nome.

Como roda:

- o cliente do protocolo é em Jinga. O portão escuta numa porta do loopback
  que o sistema escolhe, e o servidor (`node editor/vscode/server.js
  --socket=<porta>`) conecta nela. O enquadramento (`Content-Length`, que
  conta BYTES) está em `scripts/enquadramento.pr`, o mesmo do
  `teste/depurador.pr`;
- um servidor só pra doc inteira: um `didOpen` por bloco, os pedidos e um
  `didClose`; no fim, `shutdown` e `exit`. Cada leitura tem prazo (60 s). O
  servidor que não conecta ou não responde reprova, e o portão diz isso;
- o servidor usa o motor do portão (`sys.executable`) e a doc da árvore. O
  arquivo de cada bloco fica na pasta da rodada, e o `--check` do portão e o
  do servidor leem o mesmo caminho;
- nenhum exemplo roda (só o `--check`, o `--tokens`, o `--ast` e o servidor).
  Por isso não há isolamento, e o portão roda também no Windows;
- sem o `node`, ou sem o `npm install` em `editor/vscode`, o portão fica
  vermelho e diz o que falta: portão que não roda calado é falso verde. O CI
  do Linux instala antes do `make check` (teste-6);
- o fragmento (bloco com `...` que o `--check` recusa) fica de fora, como no
  portão `check`.

O relatório diz o papel de cada nome, pelos tokens em volta dele: nome de
argumento (`f(nome=valor)`), nome no import (a linha começa com `from` ou
`import`), nome que o motor liga (depois de `@`, e o `__name__`, o `_index`,
o `_match`, o `_count` e o `static`), membro na declaração (depois de
`funct`, ou antes de `:`) e nome ligado pelo programa (o resto). O papel ajuda
a ler o relatório; quem diz o item é a lista.

Na catraca, o portão é `p3`, a página é a da doc, e a impressão é a do lugar
conferido junto do bloco: `#p3 diag`, `#p3 hover nome 2:9` (linha e coluna
dentro do bloco), `#p3 membro x.nome 2:9` e `#p3 tipo x:T 2:9`. Mudou o bloco,
ou o lugar andou dentro dele, a falha volta a ser olhada.

Medido na entrada (2026-10-23, no Mac, 18.0.0): 1.368 blocos conferidos (24
fragmentos de fora), 10.121 hovers e 2.644 completions depois de `x.`, 233 com
o tipo declarado, em 4 minutos. O diagnóstico do editor bateu com o do
`--check` em todos os blocos, nenhum hover veio em branco, e a lista dos 233
receptores declarados é a do `--metadata`. As 1.207 discordâncias:

| causa | lugares | item |
|---|---|---|
| o hover do nome de argumento diz "não definido" (`room_id=`, o `length=` do campo do `model`) | 574 | editor-1 |
| o hover diz "não definido" em nomes que o motor liga: o módulo na linha do `from`, o `__name__`, o decorador, o membro na declaração, a variável do `catch`, da lambda, da compreensão, do `using`, do `case` | 549 | editor-3 |
| o `request` da rota leva o hover do módulo cliente ("falta `import request`") | 49 | editor-2 |
| o completion dá os nomes do escopo depois do ponto que começa a linha (a cadeia que continua) e do ponto de uma lista ou tupla escrita | 33 | editor-4 |
| o `--check` aprova o nome do laço lido fora da volta dele; o hover está certo | 1 | motor-18 |
| o `--tokens` lê o formato da f-string como código: o `x` de `{n:x}` vira nome | 1 | motor-19 |

A prova de que o portão pega: contra o 18.0.0, ele achou sozinho o editor-1,
o relato do `forAll`, pelo `room_id` de `docs/jinker/channel/channel.md:25`. O
bloco do próprio `forAll` o `--check` recusa (jinker-7), e o hover só se
confere em bloco que passa.

## P4: a gramática escrita e o gerador (a fazer)

A gramática em EBNF, tirada do parser e conferida contra ele
(`docs/linguagem/gramatica.md`), e um gerador de programas tipados a partir
dela, com os oráculos: JIT e sem JIT idênticos, checador e motor de acordo,
nada cai nem trava, e o Python onde a doc diz "como no Python".

## A sonda com agentes

Fica para o que só se acha rodando programa de verdade além do que a doc
mostra (`notas/SONDA-METODO.md`). Os portões vêm primeiro, porque não dependem
de limite de uso.
