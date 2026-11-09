# Correção da base (17.0.19): a lista

Única lista de defeitos da base. A sonda (`notas/SONDA-METODO.md`) escreve; o
conserto risca o item (`~~`), cita o commit e o caso de teste. Nada novo entra
na Jinga até esta lista zerar e duas sondas seguidas não acharem nada
(`CLAUDE.md`).

Cada item: programa mínimo, o que a doc promete, o que saiu (com JIT e sem),
classificação (`motor`, `doc`, `os dois`).

## jinker

Medidos em 2026-10-16 no código do 17.0.19, app de prova com
`Jinker(__name__, static_folder="front")`, pasta `front/` com `index.html`,
`docs/index.html`, `static/sub/index.html`, `a.wasm` e um vídeo. Os cinco
saíram juntos no **17.0.20** (2026-10-17), pelo que o `FileServer` do Go, o
`http.server` do Python e a RFC 9110 fazem. Os testes estão em
`teste/jinker_roda.pr`, na seção "estático: pasta, tipo, pedaço e cache" e no
bloco "arquivo grande sai em blocos".

- ~~**jinker-1** `GET /docs/` devolve o `index.html` da RAIZ do `static_folder`,
  não o de `docs/`; `GET /static/sub/` dá 404. `motor`.~~ Corrigido no 17.0.20:
  a pasta serve o `index.html` dela, e a pasta pedida sem a barra recebe `301`
  relativo pra URL com a barra. Testes: "pasta com barra serve o index.html
  DELA", "pasta sem barra e 301 relativo pra pasta com barra", "o 301 da pasta
  leva a query junto", "pasta sem index.html nao e listada" e os três da
  `/static/`; vetores novos de travessia `/estatico/link_pasta/` e
  `/estatico/link_pasta`.
- ~~**jinker-2** `a.wasm` sai com `Content-Type: application/octet-stream`; o
  navegador recusa `WebAssembly.instantiateStreaming`. `motor`.~~ Corrigido no
  17.0.20: a tabela de tipos é a padrão do `mimetypes` do Python 3.15 (213
  extensões; era uma à mão, com 36). Testes: "o .wasm sai application/wasm",
  "o .avif sai image/avif", "extensao desconhecida sai
  application/octet-stream", "o .txt continua text/plain".
- ~~**jinker-3** Pedido com `Range:` recebe 200 com o corpo inteiro (vídeo não
  toca no Safari); nenhuma resposta traz `Last-Modified`/`ETag`, e
  `If-Modified-Since` nunca dá 304. `motor`.~~ Corrigido no 17.0.20: 206 e 416
  com `Content-Range`, `Accept-Ranges`, `ETag`, `Last-Modified`, 304 com
  `If-None-Match` e `If-Modified-Since`, e `If-Range`; a data do
  `If-Modified-Since` é lida nos três formatos da RFC 9110 (5.6.7). Testes: os
  dez de `Range`, 304, `If-Range` e HEAD em `/estatico/cem.txt`, mais
  "If-Modified-Since no formato RFC 850 tambem da 304", "...no formato asctime
  tambem da 304", "If-Modified-Since que nao e data e ignorado (200)" e
  "If-Modified-Since antes da data do arquivo da 200".
- ~~**jinker-4** `jinker.md` diz que `static_url` é ignorado; o motor e
  `static_url.md` usam como prefixo da URL. `doc` (o motor está certo).~~
  Corrigido no 17.0.20, junto com o que a mesma página dizia errado: a
  `/static/` é a pasta ao lado do `.pr` (a página dizia "o diretório onde você
  rodou o servidor") e o fluxo ganhou o passo do 405.
- ~~**jinker-5** (achado lendo o código do jinker-1) Os dois servidores de
  arquivo faziam `malloc` do tamanho do arquivo e liam tudo antes de responder.
  Medido servindo 20 MB: pico de memória do servidor 23,6 MB. `motor`.~~
  Corrigido no 17.0.20: o arquivo sai em blocos de 64 KB; o mesmo teste dá
  3,6 MB. Testes: "arquivo de 20 MB sai inteiro e igual (em blocos)" e "pedaco
  de 1 MB do arquivo de 20 MB sai certo".

Achados em 2026-10-17, no conserto acima e no relato dele; consertados ou
fechados no **19.0.0** (2026-10-24), com os achados no caminho (jinker-12 a
16). No mesmo commit saiu o servidor de WebSocket da jinker (decisão dele,
2026-10-24: "REMOVA ISSO DE SOCKET DA JINGA, NÃO FAZ SENTIDO POIS TEM A LIB
SOCKETS"): `@app.socket`, o emissor (`app.socket()`, `.emit`,
`.status_send`), o `app.channel`, os tipos `ChannelManager`,
`ChannelStatus`, `SocketNamespace` e `SocketEmitter` e o servidor da porta
HTTP+1. O cliente (`request.ws_connect`) ficou, testado contra um par de eco
em Jinga (`teste/ws_par.pr`, sobre a lib `sockets`). Os objetos da jinker que
se chamam (o `app` e o `cors`) leem uma tabela só (`JK_CHAMAVEIS`,
`vm/jinga_vm.c`): o nome da chamada, os parâmetros com o padrão e o tipo de
cada um. Quem lê: as três chamadas (com nome, sem nome e a indireta, do
`map`) e o `--metadata` do valor de módulo que se chama (`jinker.cors`).

- ~~**jinker-6** Rota que devolve um `dict` que não vira JSON responde 200 com o
  corpo vazio, calada (nem com `debug=True` sai erro). Programa: rota com
  `return {"status": app.channel.status}`; o `ChannelStatus` não vira JSON
  (`json.stringify` dá "Object of type ChannelStatus is not JSON
  serializable"). Saída no 17.0.19 e no 17.0.20: `HTTP/1.1 200 OK`,
  `Content-Length: 0`. Promessa: `docs/jinker/route/route.md` ("dict/lista →
  JSON 200"); o que não vira JSON tem que dar erro visível (500 com a frase),
  como no Flask. `motor`.~~ Corrigido no 19.0.0: o retorno que não vira
  resposta (o dict, a lista, a tupla `(corpo, status)` e a tupla do
  middleware) segue o caminho de uma exceção no handler, `500` e
  `[jinker] erro no handler /rota: Object of type Jinker is not JSON
  serializable` no terminal (o `ChannelStatus` saiu com o WebSocket; as rotas
  do teste devolvem o próprio `app`, que também não vira JSON); a tupla é
  decidida por uma função só (`jk_resposta_da_tupla`), que a rota e o
  middleware usam. Medido no 18.0.1: a tupla com 201 também dava 200 com o
  corpo vazio, e a do middleware barrava com 200 vazio. Testes
  (`teste/jinker_roda.pr`): "a rota /nao_json... dá 500" (cinco rotas), "a
  tupla do middleware que não vira resposta dá 500" e "a saída do servidor
  diz: ..." (as frases).
- ~~**jinker-7** (relato dele) `app.channel(forAll=msg)`, a forma que
  `channel.md`, `jinker.md` e `ChannelManager.md` ensinam, é recusada antes de
  rodar com `TypeError: Jinker.channel() takes no keyword arguments` quando o
  `app` tem tipo (`Jinker app = Jinker(__name__)`) ou é atribuído direto; o
  `app.socket(path="/x")` igual (`Jinker.socket()`). Sem o checador, roda. A
  causa: o `--metadata` publica a propriedade (`channel`, `socket`) com
  `params: []`, sem os parâmetros da chamada do valor dela (`forAll`; `path`,
  `channel`), e o medidor de aridades gravou "não aceita nome". `motor`
  (checador). O portão P2 achou sozinho, em 2026-10-23. 5 chamadas na catraca
  do P2.~~ Fechado no 19.0.0: saiu com o WebSocket da jinker (decisão dele).
  `app.channel` e `app.socket` são membros que não existem, com o erro de
  qualquer membro, no `--check` e rodando, na mesma linha
  (`AttributeError: 'Jinker' object has no method 'channel'`). Testes: os
  quatro "WebSocket da jinker saiu: ..." de `teste/casos_libs.c`. A tabela
  única dos objetos que se chamam, feita pra este item, ficou pro `app` e o
  `cors`, e o `--metadata` publica os parâmetros do `jinker.cors`.
- ~~**jinker-8** Argumento a mais passa calado: `app.channel("a", "b")` e
  `app.socket("/x", True, 3)` rodam sem erro. Promessa: `channel.md` (um
  parâmetro, `forAll`) e `socket.md` (`path`, `channel`). `motor`.~~ Fechado
  no 19.0.0: saiu com o WebSocket da jinker (decisão dele). O mesmo defeito no
  `app(...)` e no `cors(...)` é o jinker-14, consertado.
- ~~**jinker-12** (achado no jinker-6) O status da resposta não seguia a regra do
  `.status()` (inteiro de 100 a 999) na tupla `(corpo, status)` da rota e do
  middleware, nem no `json(data, status)` e no `send(texto, status)`. Medido
  no 18.0.1: `return {"a": 1}, "201"` respondia 200; `return "x", 99999` e
  `JinkerResponse().json({"a": 1}, 99999)` saíam em `HTTP/1.1 99999 Server
  Error`, linha de status que o `curl` recusa; no `json`/`send` o texto era
  ignorado calado. `motor`.~~ Corrigido no 19.0.0: uma função só
  (`jk_status_de`) pros cinco lugares, com a frase do `.status()`; na tupla, o
  erro é 500 com a frase no terminal. Junto: o `jsonify` e o `.json()` davam
  `TypeError` pro `nan` (o `json.stringify` dá `ValueError`); a jinker passa a
  usar a mesma função do `json.stringify` (`json_escreve_ou_erro`). Testes:
  "jinker-12: o status do json() e do send() pela regra do status()" e as
  rotas `/status_texto` e `/status_fora` do `teste/jinker_roda.pr`.
- ~~**jinker-13** (achado no jinker-6) No WebSocket o erro sumia: o do handler só
  saía no terminal com `debug=True` (o da rota HTTP sai sempre); o retorno do
  handler que não vira JSON era descartado; e o `emit` (`app.channel(forAll=x)`,
  `.emit(x)`) com o que não vira JSON devolvia `Error` sem exceção, como se a
  rede tivesse falhado. `motor`.~~ Fechado no 19.0.0: saiu com o WebSocket da
  jinker (decisão dele).
- ~~**jinker-14** (achado na tabela do jinker-7) Os argumentos do `app(...)` e
  do `cors(...)` passavam calados. Medido no 18.0.1: `app(port=70000)`
  escutava na porta 4464 e o terminal dizia 70000; `app(port="18740")` subia
  na 2000; o `host` que não é texto virava `127.0.0.1`, o `workers` fora de
  1..256 virava 1 ou 256, e `debug`/`reload` valiam pela verdade do valor;
  `cors(app, origins="https://x.com")` (texto no lugar da lista) era
  ignorado, e o servidor ficava aberto a toda origem; o item que não é texto
  saía da lista calado. Na doc, o `Jinker.md` dizia que o host padrão é
  `0.0.0.0` (o motor usa `127.0.0.1`, como o Flask) e que o `static_url` não
  era usado (sobra do jinker-4). `os dois` (a doc estava errada no host e no
  `static_url`; o motor, no resto).~~ Corrigido no 19.0.0: o tipo de cada
  argumento pela tabela, a porta de 0 a 65535 (`OverflowError: app(): port
  must be 0-65535`, a frase do `sockets`), o `workers` de 1 a 256, os itens
  de `options`/`origins` texto, e o `ps_jk_listen` recusa a porta fora da
  faixa. Testes: "jinker-14: os argumentos do app(...) fora do tipo ou da
  faixa" e "jinker-14: os argumentos do cors(...) e os itens das listas".
- ~~**jinker-15** (achado escrevendo a doc do jinker-6) Antes do primeiro
  envio, `app.channel.status` e `app.socket().status_send()` diziam `Success`,
  e a regra do `ChannelStatus` é "chegou a pelo menos uma conexão". `os dois`
  (o motor estava errado no primeiro; a doc, no segundo).~~ Fechado no
  19.0.0: saiu com o WebSocket da jinker (decisão dele).
- ~~**jinker-16** (achado tirando o WebSocket da jinker, 2026-10-24) O close do
  cliente (`request.ws_connect`) saía SEM máscara, e o frame mascarado vindo
  do servidor era aceito. A RFC 6455 §5.1 manda o cliente mascarar todo
  frame, os de controle também (o close e o pong), e o servidor fechar a
  conexão com o cliente que não mascara; o cliente tem que recusar frame
  mascarado vindo do servidor. Medido no 18.0.1 contra o par de eco em Jinga,
  que anota cada frame: `frame 1 mascarado` (o texto) e `frame 8 SEM
  MASCARA` (o close); o frame mascarado do servidor, lido no código. `motor`.~~
  Corrigido no 19.0.0: um escritor só (`ws_manda_frame`, `vm/ps_jinker.c`)
  pra todo frame do cliente, que mascara com 4 bytes do `ps_random_bytes`, e
  o `ps_jk_ws_le_frame` recusa o frame mascarado do servidor. Medido no 19.0.0
  com o mesmo programa: `frame 8 mascarado`. Testes (`teste/jinker_roda.pr`,
  contra o `teste/ws_par.pr`): "o cliente WebSocket manda texto e JSON e
  recebe o eco", "o cliente WebSocket manda todo frame mascarado" e "o close
  do cliente WebSocket sai mascarado".

Medidos em 2026-10-22 (relato dele: "o `.get()` do `jinker.request.get()` e o
do `request` não diz se obtém string também"), num servidor da jinker no
loopback, com JIT e sem (`notas/` não guarda o programa; ele está descrito
em cada item):

- ~~**jinker-9** O `request.get(chave)` da rota devolve LISTA quando a chave se
  repete na query string: `?a=1&a=2` dá `['1', '2']`, e `?a=1` dá `'1'`. O
  tipo muda conforme o que o cliente manda, e a rota escrita com tipo
  (`str nome = request.get("nome")`) cai com 500 quando alguém repete o
  parâmetro. A página (`docs/jinker/request/get/get.md`) não diz nada sobre
  chave repetida. No Flask, `request.args.get` devolve o primeiro valor, e
  `getlist` devolve todos. `duvida`: a sua decisão é o primeiro valor, como
  no Flask, com um jeito de pegar todos, ou a lista de hoje escrita na doc.~~
  Decisão: mantém-se a lista atual. Uma ocorrência retorna `str`; várias
  retornam `list[str]`, na ordem original. A documentação e o roteiro
  `teste/jinker_roda.pr` cobrem os dois casos e o erro 500 da atribuição
  repetida a `str`.
- **jinker-10** O que o `request.get(chave)` lê, medido:
  - da query string, sempre texto: `?a=42` dá `'42'`, `?a=` dá `''`, e `%20`
    e `+` saem decodificados;
  - do corpo JSON, o tipo do JSON: `42` dá `int` (inteiro grande também),
    `4.5` dá `flo`, `1e400` dá `inf`, `true` dá `bool`, e lista e objeto dão
    `list` e `dict`;
  - o `null` do JSON e a chave que não existe dão os dois `Null`; o corpo
    JSON inválido e o corpo que é lista também dão `Null`, sem erro;
  - o corpo é lido como JSON mesmo sem `Content-Type: application/json`;
  - a chave diferencia maiúscula de minúscula (`?A=1` não acha `a`);
  - campo de formulário (urlencoded ou multipart) NÃO é lido: dá `Null`, e
    nenhum membro do `request` lê esses campos (só o `request.text()` cru).
  A página só diz `-> valor | Null` e "query string, depois o corpo JSON". A
  `docs/jinker/request/request.md` diz que o `request` representa "tudo que o
  cliente mandou: corpo, campos, ..."; os campos de formulário não chegam.
  `os dois`: a doc fica com tudo acima; o formulário é decisão sua (ler os
  campos, ou a doc dizer que não lê e como fazer com o `text()`).
- **jinker-11** O nome do tipo do `request` da rota: a doc diz que ele "é uma
  instância de `JinkerRequest`" (`docs/jinker/request/request.md`), e no
  `--metadata` o `JinkerRequest` não tem membro nenhum; os membros estão num
  tipo `RequestProxy`, que a doc não cita. O editor e o `type()` mostram um
  nome que a doc não tem. `os dois`.

## editor

- ~~**editor-1** (relato dele, 2026-10-17) O hover sobre o NOME de um argumento
  (`forAll` em `app.channel(forAll=msg)`) diz "nome não definido neste arquivo
  nem por import (rodando dá `NameError`)": o servidor do editor trata o nome
  do argumento como variável. Medido no `server.js` do 17.0.20, com um pedido
  de hover na coluna do `forAll`. `motor` (editor).

  O portão P3 (2026-10-23, no 18.0.0) mediu a doc inteira: 574 lugares, em
  blocos que o `--check` aprova. Ele achou sozinho o `room_id` de
  `app.channel.emit(msg, room_id="geral")` (`docs/jinker/channel/channel.md:25`).
  Entram também o parâmetro de validação do campo do `model` (`length` em
  `nome: str(length=60)`, `docs/LANGUAGE.md:737`) e o argumento nomeado que
  fica numa linha só dele, dentro de uma chamada de várias linhas (`border=2,`
  em `docs/qrcode.md:29`). A causa está no `editor/vscode/server.js`. O
  `chamadaEm`, que acha a chamada e os argumentos nomeados em volta do
  cursor, é usado só pelo completion e pela ajuda de assinatura; o `onHover`
  não pergunta a ele.~~ Corrigido no 19.0.0 (extensão 8.6.15): o hover acha
  o argumento pela árvore (o `CallArg` com o nome, que começa no token; no
  decorador, o `DecoratorCall`), antes da palavra-chave (o `count=2` não é o
  `count each`), e mostra o parâmetro do chamado (`origins = Null`,
  "parâmetro de `cors`", com a assinatura); o nome que não é parâmetro diz
  isso e lista os parâmetros; o que um `**opcoes` recebe diz onde cai; o
  parâmetro de validação do campo de `model` mostra o que confere, pelos
  tokens (vale em várias linhas). Os parâmetros do builtin (`open(...,
  mode=)`) entram na busca, e com eles a ajuda de assinatura do builtin. O
  `app.channel` do relato saiu com o WebSocket da jinker, no mesmo commit; os
  testes usam o `cors(...)` e o decorador `@app.route(..., methods=...)`.
  Testes (`editor/vscode/teste_servidor.js`): os "hover no ..." e
  "signatureHelp em `cors(` mostra `origins = Null`", e as linhas do editor-1
  saíram da catraca do P3.
- **editor-2** (relato dele, 2026-10-21: "a lib sockets não tem as constantes
  em sua doc") Medido no servidor do editor instalado (17.0.20), com pedidos
  de hover:
  - sobre `sockets.SO_LINGER` sai `sockets.SO_LINGER() -> int`, como se a
    constante fosse função, sem o valor e sem a linha que a tabela de
    constantes do `docs/sockets/sockets.md` já tem;
  - sobre `sockets.socket` e sobre `r.get` (o `Response` do `request`) sai
    só a assinatura, sem nenhuma frase da página, e o `r.get` sem o tipo do
    retorno;
  - dentro de uma rota da jinker, o `request.get` não é reconhecido como o
    `request` da rota: sai "receptor de tipo desconhecido · 5 tipos têm
    `get`" com a frase do `str.get`, que fala de outra coisa.
  `motor` (editor).

  O portão P3 (2026-10-23) achou o terceiro caso em 49 lugares da doc. O
  `request` que a jinker põe à disposição dentro da rota e do middleware tem
  o hover "módulo `request` — falta `import request` neste arquivo (rodando
  dá `NameError`)": o editor toma o nome pelo módulo cliente `request`
  (`docs/jinker/middleware/middleware.md:29`, `docs/jinker.md:53`).
- **editor-3** (achado pelo portão P3, 2026-10-23) O hover diz "nome não
  definido neste arquivo nem por import (rodando dá `NameError`)" em nomes que
  o motor liga, em blocos que o `--check` aprova. Na doc inteira são 550
  lugares. Pelo papel que o portão dá ao nome:
  - **o módulo na linha do import** (96): o `X` de `from X import ...`
    (`from jinker import Jinker`, `from os import getenv`) e o de
    `import X as Y` (`import request as web`). O `import X` sozinho responde
    certo. No `from json import parse`, o `json` sai como o apelido de tipo
    (`json = dict`);
  - **o nome que o motor põe à disposição** (102): `__name__`, os `_index`,
    `_match` e `_count` do `count each`, o nome do decorador (`@dataentity`,
    `@NonNull`) e o modificador `static`;
  - **o membro na própria declaração** (123): o campo escrito `nome: str`
    (no `model` e na Entity, `Entity P { nome: str }`) e o método
    (`funct falar(self)`, o `__init__`);
  - **o nome que o programa liga sem ser por atribuição** (228): a variável do
    `catch`, o parâmetro de lambda (com tipo ou sem), a variável da
    compreensão e do gerador (`[n * 2 for each n in nums]`), os nomes depois
    do primeiro no `for each nome, idade in ...`, o `as` do `using`, o nome
    do `case` (`case v if v < 50`, `case [x, y]`, `case _`), o `global`, o
    membro de `enum`, o campo da Entity escrito `str nome`, a variável de topo
    com modificador (`private int n = 1`, que o `--ast` dá como `FieldDecl`) e
    a exceção inventada no `raise` (`raise MinhaFalha`);
  - **o nome lido antes da linha que o liga** (1): o `prev` de
    `docs/linguagem/04-variaveis-escopo-atribuicao.md:271`, que o arquivo liga
    duas linhas abaixo. Rodando, ele dá `NameError` na segunda volta, como a
    doc promete (4.6.3), e o hover diz "não definido neste arquivo". Foi o
    motor-18 até o conserto do mesmo dia: o checador só promete o nome que o
    arquivo nunca liga (10.x), e o motor faz o que a doc diz.

  O contrário também acontece, pela mesma causa. Depois do laço
  `for each i in [1, 2, 3] { ultimo = i }`, o hover diz que `i` e `ultimo`
  existem ("declarada na linha 1", "na linha 2"), e rodando os dois dão
  `NameError`, que é o que a doc promete em 4.6.3.
  As causas estão em dois lugares do editor:
  - no `editor/vscode/analise.js`, o `ligacoesDe` só liga a atribuição, a
    declaração (`VarDecl`), a variável do `for each`, as declarações de topo
    e o desempacotamento. E o escopo só existe pro módulo, pra funct e pra
    Entity: bloco (laço, `if`, `try`) não tem;
  - no `editor/vscode/server.js`, o `onHover` não trata o nome do módulo na
    linha do `from` nem na do `import ... as`, nem os nomes que o motor liga.
    O `__name__`, por exemplo, o completion oferece e o hover não conhece.

  `motor` (editor).
- **editor-4** (achado pelo portão P3, 2026-10-23) O completion depois de um
  ponto devolve todos os nomes do escopo (de 132 a 141 nos blocos da doc), e
  não o membro que o programa usa, em dois casos:
  - **a cadeia que continua na linha de baixo** (26 lugares): `.send(...)`
    debaixo de `return jinker.JinkerResponse()` (`docs/jinker/cookie/cookie.md:43`),
    `.get("dados")` debaixo de `request.get(url=u)` e `.json()`
    (`docs/linguagem/01-estrutura-lexica.md:168`);
  - **a lista ou a tupla escrita antes do ponto** (7 lugares):
    `[10, 20, 30].index(20)` (`docs/list/index/index.md:24`),
    `(10, 20).contains(10.0)`.

  O `docs/lsp.md` promete que o literal responde pelo tipo dele (`[…]`, `{…}`
  e `"abc".` direto) e que tipo desconhecido "não sugere nada". A causa está
  no `cadeiaAntes` do `editor/vscode/analise.js`. Ele lê só a linha do
  cursor, e o grupo `[...]` ou `(...)` sem nome antes dele termina a cadeia
  vazia. Aí o completion responde como se não houvesse receptor. `motor`
  (editor).

## motor

- **motor-1** A trava de aninhamento fundo compara a folga da pilha com uma
  margem que cresce com o maior quadro visto na execução (`vm/ps_pilha.c`,
  `apertada_com`). Perto do limite, o MESMO programa ora dá o erro limpo, ora
  roda até o fim. A folga segue o `ulimit -s`, e o make 3.81 da Apple sobe o
  limite até o teto (64 MB) para os programas que ele roda (o comentário do
  caso dos 100 mil parênteses em `teste/casos_crash.c` já registra isso). Então
  o MESMO programa dá o erro limpo com 8 MB e roda até o fim com 64 MB. Medido
  no Mac em 2026-10-17: "regex.compile com 20 mil grupos aninhados e
  ValueError", "8000 blocos aninhados e 20000 decoradores sao erro limpo no
  --check", "regex profundo demais e erro" e os dif #536, #1260 e #1266 falham
  dentro do `make check` e com `ulimit -s 65532`, no binário do `main`
  (17.0.19) e no 17.0.20; com 8 MB os seis passam. O conserto que o parser já
  recebeu (um teto contado, `PS_PARSE_PROF_MAX`, que dá o mesmo resultado em
  qualquer pilha) falta no regex e nos blocos e decoradores. `motor`.
  Medido de novo em 2026-10-23, na rodada do P1: `ulimit -s` dá 8.192 KB no
  shell e 65.532 KB dentro do `make` 3.81; os mesmos seis falham no `make
  check` e passam com o teto em 8 MB (9.993 de 9.993). Nesse dia virou por
  engano um item novo, o teste-3 do `ec7f562`, tirado no commit seguinte.
- **motor-2** (achado pelo portão `check`, `docs/regex.md:17`) A chamada
  `match(r"\d+", "123")` sozinha na linha, a forma que `docs/regex.md` ensina
  (`from regex import match`), é lida como o comando `match`, e o erro sai na
  linha de BAIXO: num arquivo com essa linha seguida de `post("depois")`, o
  `--check` e o programa rodando (com JIT e sem) dão `SyntaxError: esperado ':'
  ou '{' apos expressao do match` na linha do `post`, rc 2. Dentro de uma
  expressão a chamada funciona (`bool r = match(...)`). Dois defeitos: o erro
  longe da causa (`motor`), e a chamada solta, que o Python aceita porque o
  `match` dele é palavra reservada só no começo do comando seguido de `:`
  (`duvida`: o parser aceita como o Python, ou a página mostra a chamada
  dentro de uma expressão).
- **motor-3** (achado pelo portão `check`, `docs/mail/MailServer/send/send.md:16`)
  O `to` usado como valor (`s.send(to, subject="x")`) dá `SyntaxError: esperado
  tipo apos 'to': int, float, str...`, que fala do operador de conversão. A
  declaração `str to = "x"` dá a frase certa: `'to' e palavra reservada da
  linguagem e nao pode ser usada como nome de variavel`. A frase do primeiro
  caso não aponta a causa. `motor`.
- ~~**motor-4** (GRAVE, perda de dado; achado lendo o `psodbc.query` em
  2026-10-18, por causa do doc-19) O `psodbc.query` copia o SQL num buffer
  fixo de 4.096 bytes com `snprintf` (`mod_db_query`, `vm/jinga_vm.c:21408`) e
  executa o pedaço que coube, calado. Programa: tabela `t` com 3 linhas e
  `psodbc.query(base="corte.db", cmd="DELETE FROM t" + " " * 4090 + "WHERE id = 1")`
  (4.115 bytes): sobram 0 linhas, não 2, rc 0, com JIT e sem. O `WHERE` caiu
  no corte e o `DELETE` apagou tudo. No mesmo assunto, o caminho do MySQL
  (`mysql_exec`, `vm/ps_db.c:472`) recusa com "sql grande demais" o SQL acima
  de 16 KB: não corta, mas recusa uso válido (um `INSERT` em lote). O mesmo
  no postgres com parâmetro (acima de 8 KB) e no ODBC (16 KB). `motor`.~~
  Corrigido no 18.0.1: o `query` monta o SQL em memória do tamanho dele, e os
  três drivers que reescrevem os marcadores escrevem num texto que cresce.
  Testes: `psodbc query: o SQL de mais de 4096 bytes vai inteiro`, o `INSERT`
  de mais de 20 KB e o SQL de 100 KB (`teste/casos_libs.c`); no
  `teste/e2e/db.pr`, o SQL de mais de 20 KB com parâmetro no postgres, no
  MySQL e no SQL Server.
- ~~**motor-5** (achado pelo portão `roda`, `docs/manpu/read/read.md:27`) O
  `manpu.read` de um arquivo que não existe não levanta erro: devolve o texto
  `Error: arquivo não encontrado: nao_existe.csv` como se fosse o conteúdo,
  rc 0. O exemplo da página quebra depois, no índice
  (`TypeError: string indices must be integers, not 'str'`), longe da causa.
  O `os.readFile` e o `open` da Jinga levantam `FileNotFoundError` no mesmo
  caso, como o `open` do Python. `motor`.~~ Fechado no 19.0.0: a lib `manpu`
  saiu inteira (decisão dele, 2026-10-24: "muito limitada e sem uso real").
- **motor-6** (achado pelo portão `roda`, 2026-10-18) O `request` da jinker
  fora de uma rota. A doc diz que ele "está disponível automaticamente dentro
  de qualquer rota" (`docs/jinker/request/request.md`), e a VM só cria o nome
  quando o servidor sobe (`jk_slot_global(vm, "request")`). Mas o checador
  aceita `request` e `channel` em qualquer lugar de um arquivo que usa a
  jinker (`tp_nome_inexistente`, `vm/ps_compiler.c:3302`). Programa:
  `from jinker import Jinker` e, no topo, `id = request.path_param("id")`: o
  `--check` dá `{"ok":true}` e rodando sai `NameError: name 'request' is not
  defined`, rc 1, com JIT e sem. Com `from jinker import request` o nome
  existe, e fora da rota `request.text()`, `request.method` e
  `request.get("x")` devolvem `Null` calados; o Flask, no mesmo caso, levanta
  `RuntimeError: Working outside of request context`. `motor`.

- **motor-7** (achado em 2026-10-22, medindo o jinker-10) O `--check` aprova
  método que o `request` da rota não tem: `v = request.get_query("a")` dentro
  de uma rota dá `{"ok":true}`, e rodando a rota cai com 500
  (`'RequestProxy' object has no method 'get_query'`). É da classe do
  `forAll` e do motor-6: o checador não confere os membros do `request`. A
  P2 dos portões pega a classe toda. `motor`.
- **motor-8** (achado pelo portão `roda`, 2026-10-22,
  `docs/os/loadFile/loadFile.md:98` e `:109`) O `os.loadFile` confere o
  `mode` DEPOIS de abrir o arquivo: `os.loadFile("nao_existe.txt",
  mode="xyz")` dá `FileNotFoundError`, e só com o arquivo existente sai o
  `ValueError: loadFile: 'xyz' nao e um mode valido`. A página mostra o
  `ValueError` sem criar arquivo nenhum, e o `open` do Python confere o modo
  antes de tocar no disco (`ValueError: invalid mode`). Com JIT e sem.
  `motor`.
- ~~**motor-9** (achado pelo portão `roda`, 2026-10-22, `docs/manpu.md:58`) O
  `mp.write(...)` devolve um `ManpuResult` que se imprime `Success` ou
  `Error` e que é `== True` quando deu certo. As páginas (`docs/manpu.md:58`
  e `docs/manpu/write/write.md:36`) leem o motivo do erro em `nm.status`, e o
  `ManpuResult` não tem membro nenhum (`--metadata`): rodando, `nm.status` dá
  `AttributeError: 'ManpuResult' object has no attribute 'status'`. E quando
  a escrita falha (destino numa pasta que não existe) volta `Error` sem
  motivo nenhum e sem exceção, com JIT e sem: o erro some. O `open` e o
  `os.writeFile` levantam a exceção com o caminho. `os dois`: o motor dá o
  motivo (o `.status` que a doc promete, ou a exceção), e a doc diz qual.~~
  Fechado no 19.0.0: a lib `manpu` saiu inteira (decisão dele, 2026-10-24).
- **motor-10** (achado no `make check` do Mac, 2026-10-23, pelo caso "import
  de caminho com extensao velha ensina a nova" do `teste/cli_roda.pr`) O
  `ImportError` do `import './x.ps'` manda rodar o migrador e corta o caminho
  dele em 100 bytes, calado (`jinga %.100s <pasta> --aplica`,
  `vm/jinga_vm.c:33343`). Com o repositório numa pasta comprida, sai
  `(jinga /Users/.../06928479-1e0f-496c-807 <pasta> --aplica)`: um comando
  que aponta pra um arquivo que não existe. O nome escrito no `import`
  também é cortado (`%.40s`, `%.120s`, `%.200s`), e a mensagem inteira cabe
  em 256 bytes (`char erro[256]` da VM). O caso só passa com o caminho curto
  (a VPS, o checkout dele). É a classe do Plano 10: caminho cortado calado
  por buffer fixo. `motor`.
- **motor-11** (achado pelo portão P2, 2026-10-23) O `--check` não confere a
  chamada dos membros de 11 tipos nem dos valores de módulo que se chamam:
  aceita qualquer quantidade, qualquer nome de parâmetro e qualquer tipo, e o
  motor recusa rodando. Os tipos são os que a tabela medida
  (`vm/aridades_medidas.inc`, do `scripts/mede_aridades.pr`) deixa de fora:
  - o da jinker que nasce sem servidor: `RouteRegistrar` (os do WebSocket da
    jinker, `ChannelManager`, `SocketNamespace` e `SocketEmitter`, saíram no
    19.0.0, e o `ManpuFile` saiu com a manpu);
  - os do `psodbc`, `DbConnection` e `DbCursor`;
  - os que só existem com a jinker no ar: `RequestProxy` (o `request` da
    rota), `PoolFileUpload`, `Response` e `WsConnection`;
  - e os valores que se chamam: `jinker.cors(...)` e `os.PoolFile(...)`.

  Exemplos: `request.get()` dentro da rota passa no `--check`, e rodando dá
  `TypeError: get() takes exactly one argument (0 given)`; `jinker.cors(0)`
  passa, e rodando dá `cors() argument 1 must be Jinker, not int`. O
  comentário do `mede_aridades.pr` dizia que os de serviço ficavam de fora de
  propósito ("a VM confere rodando"). 395 chamadas na catraca do P2.
  `motor` (checador).
- **motor-12** (achado pelo portão P2, 2026-10-23) Chamar o que não se chama.
  - A propriedade chamada como método (`f.name()`, `regex.compile("a")
    .pattern()`, `app.static_url()`): o `--check` diz `PoolFile.name() takes no
    keyword arguments`, como se fosse método sem parâmetro, e o motor diz
    `'str' object is not callable`.
  - A constante de módulo chamada (`sockets.AF_INET(0)`,
    `qrcode.ERROR_CORRECT_L("")`, `sys.argv([])`): o `--check` aceita, e o
    motor diz `'int' object is not callable`.

  88 chamadas na catraca do P2. `motor` (checador).
- **motor-13** (achado pelo portão P2, 2026-10-23) O tipo que o
  `app.route(...)` devolve tem dois nomes: o `--metadata` publica
  `RouteRegistrar` (e a doc tem `docs/jinker/RouteRegistrar/`), e o motor e o
  checador chamam de `_RouteRegistrar`. `RouteRegistrar r =
  app.route("/x")` o `--check` recusa (`esperava RouteRegistrar, recebeu
  _RouteRegistrar`). E o `--check` não conhece o retorno do `register` dele:
  `int x = app.route("/x").register(f)` passa, e rodando dá `esperava int,
  recebeu Null` (o `--metadata` diz `Null`). Pelo mesmo nome, ele não
  confere os membros: `app.route("/x").p2_nao_existe()` passa, e rodando dá
  `'_RouteRegistrar' object has no method 'p2_nao_existe'` (achado pela
  chamada de membro que não existe do P2, 18.0.0). `motor`.
- ~~**motor-14** (decisão do Kleber, 2026-10-23) O `.type()` sai da linguagem:
  fica só o `type(x)`. Nas palavras dele: "o uso deve ser somente type() não
  .type(), retire ele, isso faz ter menos trabalho". Medido pelo portão P2: o
  `.type()` é o único membro universal (`METODOS_UNIV`, `vm/jinga_vm.c`) e
  devolve `str`, e o `--check` não sabe (`int x = 1.type()` passa e rodando
  dá `esperava int, recebeu str`). Tirar quebra quem usa `.type()`: no
  repositório são 10 linhas (a doc, dois casos de teste, o checador).
  `motor`.~~ Corrigido no 18.0.0: saíram o método (`met_type`, a segunda
  tabela de nomes de tipo), a `METODOS_UNIV` e o `__universal__` do
  `--metadata`. `x.type()` dá o erro de membro que não existe em todo tipo de
  valor, antes de rodar e rodando, e o `type(x)` é a única forma (casos
  ".type() saiu" de `teste/casos_linguagem.c`).
- ~~**motor-15** (achado no conserto do motor-14, 2026-10-23) O `--check`
  aceitava nome de módulo como tipo: `bytes d = b"x"` dava `esperava bytes,
  recebeu byte`, e `os x = os`, `esperava os, recebeu module`, quando o tipo
  que não existe dá `tipo X não existe (variável x)`. O `tp_tipo_existe`
  perguntava se o nome "tinha o membro `type`", e pra módulo a resposta era
  "não tem", não "não sei". O `bytes d` veio da análise da `ssl.pr` dele (o
  `recv.md` diz que o retorno é `bytes`; doc-1). No mesmo caminho: a
  declaração de topo com tipo que não existe saía com dois erros na linha (a
  pré-passada conferia o valor contra o tipo inexistente: `esperava
  naoexiste, recebeu int`). `motor` (checador).~~ Corrigido no 18.0.0: o
  `ps_nativo_eh_tipo` responde "é tipo" sem os módulos, e a pré-passada não
  confere o valor contra tipo que não existe (casos "nome de modulo nao e
  tipo" e o "um erro por declaracao" do `teste/cli_roda.pr`).
- ~~**motor-16** (achado na análise da `ssl.pr` dele, 2026-10-23) O `--check`
  aceitava qualquer membro de um valor sem membro nenhum, `funct` e `Null`:
  `sockets.socket.sendall(s, "x")` (a forma das linhas 90 e 95 da `ssl.pr`,
  o método chamado pela classe, como no Python), `g.foo()` com `g` uma funct
  e `Null.foo()` davam `{"ok":true}`, e rodando `AttributeError: 'funct'
  object has no method 'sendall'`. `motor` (checador).~~ Corrigido no
  18.0.0: funct e Null não têm membro (`ps_nativo_tem_membro`), e o Null só
  fica fora da conferência como lado de uma união (casos "membro de funct" e
  "membro de Null"). A chamada de membro que não existe do P2 achou sozinha,
  rodando contra o 17.0.20.
- **motor-17** (achado no conserto do motor-14, 2026-10-23) O `--check` não
  sabe o tipo de quatro valores, e aceita o que o motor recusa rodando:
  - o nome de um builtin usado como valor: `int x = len` passa, e rodando dá
    `esperava int, recebeu funct`;
  - a chamada de uma funct com `yield`: `str s = gen()` passa, e rodando dá
    `esperava str, recebeu generator`;
  - a chamada de uma `async funct`: com `int async funct f()`, `int x = f()`
    passa, e rodando dá `esperava int, recebeu future`;
  - o membro de enum: `Cor.RED.foo()` passa, e rodando dá `'int' object has
    no method 'foo'`.

  Por isso o membro neles também passa (`len.foo()`, `gen().foo()`). A
  chamada de membro que não existe do P2 acha o do builtin
  (`len.p2_nao_existe()`, na catraca). O gerador de programas do P4 achou
  sozinho o do gerador e o da `async funct` (2026-10-24), encolhidos em
  `teste/p4_achados/960cdd53559c.pr` e `teste/p4_achados/46ee528fae3e.pr`, na
  catraca. `motor` (checador).
- **motor-19** (achado pelo portão P3, 2026-10-23,
  `docs/linguagem/01-estrutura-lexica.md:333`) O `--tokens` lê como código o
  formato da f-string, o trecho depois do `:`. Em
  `post(f"{3.5:.1f}|{7:>4}|{255:x}")` saem, marcados `"em":"fstring"`:
  - um `DOT` e um `SINC` vazio pro `.1f`. O `SINC` é a marca de erro do
    lexer, e aqui o programa é válido e o `--tokens` não lista erro nenhum;
  - um `OP` e um `INT` pro `>4`;
  - um `IDENT` pro `x`.

  A doc (1.6.3) diz que depois do `:` vem o formato do `format()`
  (`[preenche][<>^][0][largura][.precisao][tipo]`), e não uma expressão. Só o
  campo entre chaves dentro do formato (`{x:{w}.2f}`) é expressão. Com
  `int n = 255` e `post(f"{n:x}")`, o hover do editor sobre o `x` diz "nome
  não definido neste arquivo nem por import (rodando dá `NameError`)", e o
  programa passa no `--check` e imprime `ff`, com JIT e sem. `motor` (o
  `--tokens`).

  Medido montando o portão `gramatica` (2026-10-23): a f-string que está
  dentro de um campo de outra (`f"a{f'b{1 + 2}' + 'x'}c"`) vem como um token
  `FSTRING` marcado `"em":"fstring"`, sem o `interp` e sem os tokens de
  dentro dela. O `docs/lsp.md` ("Dentro da f-string") promete os dois pra
  f-string: o intervalo de cada interpolação e os tokens de dentro.
- **motor-20** (achado escrevendo a gramática, 2026-10-23) O parser ainda tem
  o ramo do `match` com `:` e recuo, de quando havia bloco com `:`, e as
  frases dele pedem o `:` que saiu da linguagem. No `--check` e rodando, com
  JIT e sem:
  - `match x:` e embaixo `case 1 { post("um") }` dá `SyntaxError: esperado
    ':' apos padrao do case` (3:12);
  - `match x:` e embaixo `case 1:` com o corpo recuado dá `SyntaxError:
    match indentado nao fechou`, no fim do arquivo (4:19).

  O `if True:` dá a frase que aponta a causa, `bloco com ':' nao existe mais
  — use '{ }'`, e o `case 1:` dentro de `match x {` também. A doc (5.5) só
  tem o `match` de chaves. A gramática (`docs/linguagem/gramatica.md`)
  escreve só a forma de chaves: o ramo do `:` não aceita programa nenhum, e
  os dois recusam os mesmos. `motor` (a frase).
- **motor-21** (achado escrevendo a gramática, 2026-10-23) O parser aceita
  palavra reservada onde vai um valor ou o tipo de um campo, e o checador
  acusa com uma frase que não aponta a causa. No `--check` e rodando, com JIT
  e sem:
  - `x = while` dá `NameError: name 'while' is not defined` (1:5);
  - `class A { if x }` dá `AttributedValueError: tipo if não existe (campo x
    de A)` (1:14), e o fim do erro sai `l2` 0, `c2` 0.

  A palavra reservada no lugar do nome da variável já tem frase própria
  (`while = 1`: `'while' e palavra reservada da linguagem e nao pode ser
  usada como nome de variavel`). A gramática descreve o parser (o `KW` no
  `corpo_primario` e no `tipo_de_campo`), com a nota no fim da página.
  `motor` (a frase).
- ~~**motor-22** (achado escrevendo a gramática, 2026-10-23) No `count each
  <tipo> in <texto literal> { ... }`, o `{` depois do texto é lido como a
  interpolação (`"texto" {expr}`), e não como o bloco: o comando vira a
  expressão `count` sem bloco, e o corpo roda uma vez, como parte do texto.
  `count each str in "abca" { post("dentro") }` e embaixo `post("fim")` passa
  no `--check` e imprime `dentro` e `fim`, com JIT e sem: calado. Com
  `post(_match)` no corpo, a queixa é `NameError: name '_match' is not
  defined` (1:33), que não aponta a causa. Com o texto numa variável (`str s
  = "abca"`, `count each str in s { post(_match) }`), o bloco roda e imprime
  as 4 letras; o `for each c in "ab" { ... }` também abre o bloco. A doc (5.6)
  escreve `count each <Tipo> in <container> { }` sobre um container
  qualquer. A causa: a cabeça do `if`, do `while` e do `for each` é lida no
  modo em que o `{` depois de texto abre o bloco (`chave_abre_bloco`), e o
  container do `count each` não (o `soma` da regra `contagem_com_bloco`, na
  gramática). `motor` (resultado errado calado).~~
  Corrigido no 19.0.1: o contêiner do `count each` com bloco é lido no modo
  cabeça, como o iterável do `for each`, e o `{` depois do texto abre o
  bloco; a gramática lê o contêiner pela regra nova `soma_de_cabeca`. Casos:
  os "motor-22: ..." de `teste/casos_linguagem.c` (o bloco em cada letra,
  `_index` e `_match`, o `return;` e a forma de expressão), e o exemplo da 5.6.
- **motor-23** (achado pelo portão `gramatica`, 2026-10-23) O `--ast` põe os
  erros do lexer na frente dos do parser, e o primeiro erro dele (o `msg`, a
  `linha` e a `coluna` de cima, e o primeiro de `erros`) não é o do `--check`
  nem o que sai rodando. No programa `post(1)` / `int x = = 1` / `post(2)` /
  `post(3)` / `int y = 1 $ 2` (o caso "rodando sai o primeiro erro da lista do
  --check", `teste/casos_linguagem.c`), o `--ast` dá `caractere inesperado:
  '$'` (5:11) e depois `expressao invalida` (2:9); o `--check` e o programa
  rodando dão `expressao invalida` (2:9) primeiro. O `docs/lsp.md` promete a
  ordem do arquivo no `--check` ("os do lexer e os do parser juntos, na ordem
  do arquivo"), e os dois modos do parser têm que dizer o mesmo do mesmo
  programa. 10 lugares na catraca (portão `gramatica`): os 9 blocos da doc
  que mostram um erro com a frase escrita como linha do bloco (a crase e o
  travessão da frase são erro do lexer, depois do erro de verdade) e o caso
  da suíte. `motor` (o `--ast`).
- ~~**motor-24** (achado escrevendo a gramática, 2026-10-23) O valor do `count`
  (`int(7) count in xs`) aceita `*` e `**` e joga a estrela fora. Com `list
  xs = [7, 7, 1]` e `list l = [7]`, `post(int(*l) count in xs, int(7) count
  in xs)` passa no `--check` e imprime `0 2`, com JIT e sem: o `int(*l)` conta
  os itens iguais à lista `l`, e não ao `7`. O mesmo com `int(**d)`. O
  `count_esquerda` (`vm/ps_parser.c`) recusa o argumento com nome e mais de um
  valor, e não olha a estrela. A doc (3.10) promete o valor entre parênteses
  (`Tipo(valor) count in contêiner`). `motor` (resultado errado calado).~~
  Corrigido no 19.0.1: o `*` e o `**` no valor são `SyntaxError: 'count' nao
  aceita '*' no valor: o valor entre parenteses e um so` (ou `'**'`), nas
  quatro formas, no trecho da estrela; a gramática tirou a alternativa da
  estrela do `valor_de_count`. Casos: os "motor-24: ..." e os "motor-34: ..."
  de `teste/casos_linguagem.c`, e o exemplo da 3.10.
- **motor-25** (achado montando o portão `gramatica`, 2026-10-23) O campo
  dentro do formato da f-string (`f"{x:>{w}}"`) não passa pelo parser: quem
  lê é o compilador, com outra frase e no lugar do campo de fora. No
  `--check` e rodando, com JIT e sem:
  - `f"{x:>{a b}}"` passa no `--ast` e dá `SyntaxError: dentro de {} da
    f-string vai uma expressao so (sobrou texto, ou e um comando)` na coluna
    do `x` (3:12), sem `erros` nem fim. O mesmo erro num campo de fora
    (`f"{a b}"`) sai no `--ast` e no `--check` como `sobrou 'b' dentro de {}
    da f-string: cada {} leva uma expressao so`, no `b`;
  - `f"{x:>{a}}"` com o `a` que não existe dá `NameError: name 'a' is not
    defined` na coluna do `x` (2:12), e o fim sai antes do começo (`l2` 1,
    `c2` 2).

  A doc (1.6.3) diz que o campo dentro do formato vale ("`{x:{w}.2f}` usa a
  largura guardada em `w`"), e o `docs/lsp.md` diz que cada item de `erros`
  traz onde o erro começa e onde termina. `motor`.
- ~~**motor-26** (achado pelo gerador do P4, lote 3, 2026-10-24,
  `teste/casos_python.c`) A tabela de caixa do motor é a do Unicode 15.1, e o
  Python 3.15, que a doc cita, usa a do Unicode 17.0: em 110 caracteres o
  `upper` ou o `lower` do Python muda o caractere e o do motor não. São os
  pares novos do Unicode 16.0 e 17.0. Exemplos:
  - o `upper` do U+019B: o Python dá o U+A7DC (a maiúscula, que entrou no
    Unicode 16.0), e o motor devolve o próprio U+019B;
  - as letras do Garay, de U+10D50 em diante.

  A doc diz as duas coisas no mesmo capítulo
  (`docs/linguagem/12-metodos-string-list-dict.md`): "a mesma do
  `upper`/`lower` do Python" (linha 36) e "a tabela do Unicode inteira (UCD
  15.1)" (linha 89). Quem decide é ele: as tabelas do motor passam pro Unicode
  17.0 (o `scripts/gera_unicode.pr` com o UCD 17.0), ou a doc diz que a
  referência é o UCD 15.1. O caso pendente lista os 110. Decisão dele
  (2026-10-24): as tabelas do motor passam pro Unicode 17.0, o do Python 3.15
  que a doc cita, e a doc deixa de dizer 15.1. `motor`   (tabelas do Unicode).~~ Corrigido nesta alteração: `vm/ps_unicode_tab.inc`
  foi regenerado pelo `scripts/gera_unicode.pr` a partir do UCD 17.0.0. O
  caso dos 110 caracteres passou a ser regressão em `teste/casos_python.c`;
  a doc passou a citar UCD 17.0. O commit e a saída dos dois modos ficam
  registrados aqui após os portões.
- ~~**motor-27** (achado pelo gerador do P4, lote 3) O `Parsing.string` junta
  só quatro brancos: o espaço, o tab, o `\n` e o `\r`. O
  `" ".join(s.split())` do Python, que a doc cita, junta também estes:
  - os controles `\x0b`, `\x0c`, `\x1c` a `\x1f` e `\x85`;
  - os espaços do Unicode: `\xa0`, U+1680, U+2000 a U+200A, U+202F, U+205F e
    U+3000;
  - os separadores de linha e de parágrafo, U+2028 e U+2029.

  Programa: `Parsing.string("a\U00003000b")` dá `"a\U00003000b"`, e o Python dá
  `"a b"`. Promessa: `docs/Parsing/string/string.md` ("cada corrida de
  espaço, tab e quebra de linha vira um espaço só, e os das pontas saem (o
  `" ".join(s.split())` do Python)"). Dos 40 casos sorteados do lugar, 32
  estão pendentes. `motor`.~~
  Corrigido no 20.0.1: o `par_string` (`vm/jinga_vm.c`) anda por caractere
  e usa a régua de branco da linguagem, a do `split()` e do `isspace`
  (`cp_eh_branco`). Os separadores U+001C a U+001F ficam no texto, como no
  `split()`: a doc do `Parsing.string` passou a dizer a régua da linguagem
  (o `" ".join(s.split())` do Python, menos esses quatro), e o oráculo do
  `teste/geradores/python_doc.pr` também. Casos: os quatro "motor-27: ..."
  de `teste/casos_libs.c`, e os 32 do `teste/casos_python.c` que estavam
  pendentes, regerado; o exemplo na página do `Parsing.string`.
- **motor-28** (achado pelo gerador do P4, lote 3) O `-o` logo depois do
  arquivo não chega ao programa. `jinga filho.pr -o saida` não roda o
  `filho.pr`: gera um executável chamado `saida` (e diz `gerado: saida`).
  Depois do primeiro argumento, o `-o` chega: `jinga filho.pr x -o y` dá
  `['filho.pr', 'x', '-o', 'y']`. O Python passa pro `sys.argv` tudo o que
  vem depois do script.
  - A promessa do `sys.argv`: `docs/sys/argv/argv.md` ("os seus argumentos
    vêm a partir de `argv[1]` — a mesma convenção de C, Python e JS").
  - A forma de gerar o executável, `jinga arquivo.pr -o nome`, está em
    `docs/LANGUAGE.md` (linhas 46 e 55) e no `jinga --help`.

  Quem decide é ele: a doc do `sys.argv` diz que o `-o` logo depois do
  arquivo é o pedido de gerar o executável, ou o executável sai só por outro
  comando e o `-o` vai pro programa. Decisão dele (2026-10-24): o `-o`
  depois do arquivo vai pro programa, como no Python; o executável sai com a
  opção antes do arquivo (`jinga -o nome arquivo.pr`), e a doc e o `--help`
  acompanham. `motor` (linha de comando).
- ~~**motor-29** (GRAVE, perda de dado; achado no conserto do motor-4,
  2026-10-24) O SQL vai pro banco como texto do C, que acaba no primeiro NUL.
  `cur.execute("delete from t\x00 where id = 1")` roda `delete from t` e apaga
  a tabela inteira, com JIT e sem, no `psodbc` (os quatro drivers e o
  `query`) e no módulo `sqlite3`. O `sqlite3` do Python recusa com
  `ProgrammingError: the query contains a null character`. `motor`.~~
  Corrigido no 18.0.1: uma função só confere o SQL antes de qualquer banco
  (`db_sql_sem_nul`), no `execute`, no `query` (e no `table=`) e no módulo
  `sqlite3`: `DatabaseError: the query contains a null character`, e nada
  roda. Testes: `psodbc: NUL no SQL e erro e nada roda` e o do `sqlite3`.
- ~~**motor-30** (achado no conserto do motor-4) O texto com NUL era cortado
  no caminho do banco, nos dois sentidos. Na ida, o parâmetro ia como texto
  do C: `cur.execute("insert into u values (?)", ("a\x00b",))` gravava `"a"`
  no sqlite, no MySQL e no ODBC, e chegava cortado ao postgres. Na volta, a
  célula do resultado media o texto com `strlen`. E o parâmetro que não virava
  texto (sem memória) ia como `NULL`, calado (lido no código). `motor`.~~
  Corrigido no 18.0.1: o tamanho de cada parâmetro vai até o driver (o
  `bind` do sqlite, o escape do MySQL, o indicador do ODBC), e a célula guarda
  o tamanho. O tipo texto do postgres não guarda NUL: lá o parâmetro com NUL é
  `DatabaseError: A string literal cannot contain NUL (0x00) characters.` (a
  frase do psycopg2). A conversão que falha é `MemoryError`. Testes: `psodbc
  sqlite: parametro com NUL vai e volta inteiro`; no `db.pr`, o postgres
  recusa e o MySQL e o SQL Server devolvem igual.
- ~~**motor-31** (GRAVE, perda de dado; achado no conserto do motor-4) O
  `prepare` do sqlite lê só o primeiro comando, e o resto era jogado fora:
  `delete from t where id = 1; delete from t` rodava só o primeiro, calado,
  no `psodbc` e no módulo `sqlite3`. O `sqlite3` do Python recusa com
  `ProgrammingError: You can only execute one statement at a time.`, sem rodar
  nenhum. `motor`.~~ Corrigido no 18.0.1: depois do `prepare`, o resto tem que
  ser só branco ou comentário (`ps_db_sql_resto`, o `lstrip_sql` do CPython);
  senão é `DatabaseError: You can only execute one statement at a time.`, antes
  de rodar. O `;` no fim, um comentário depois dele e o `;` do começo valem,
  como no Python. Testes: `psodbc sqlite: um comando por vez`, `psodbc query:
  um comando por vez` e o do `sqlite3`.
- ~~**motor-32** (achado no conserto do motor-4, lendo o código) O ODBC (o
  SQL Server) lia cada valor num buffer de 4.096 bytes com um `SQLGetData`
  só: o valor maior voltava cortado, calado (o aviso de corte conta como
  sucesso). O nome de coluna cortava em 127 bytes, o erro de leitura virava
  `NULL`, e o indicador do parâmetro nulo apontava pra memória que já tinha
  saído de escopo quando o driver o lia. `motor`.~~ Corrigido no 18.0.1: a
  leitura repete o `SQLGetData` enquanto o valor não cabe, num buffer que
  cresce; o nome de coluna pede o tamanho inteiro; o erro de leitura é erro; os
  indicadores vivem até o fim da execução. Teste: no `db.pr`, o valor de 10
  KB lido de volta inteiro do SQL Server.
- ~~**motor-33** (achado tirando o WebSocket da jinker, 2026-10-24) O `--check`
  conferia só o primeiro nome do decorador. O membro que não existe e a
  chamada com argumento errado passavam e quebravam rodando: `@app.rota("/x")`
  e `@os.naoexiste` davam `{"ok":true}` no `--check` e `AttributeError`
  rodando; `@log()` com a `funct log(f)` passava e dava `TypeError: log()
  missing 1 required positional argument: 'f'` rodando. Medido no 18.0.1, com
  JIT e sem. A doc promete que o checador pega o membro que o tipo não tem e a
  aridade numa chamada a funct conhecida (`docs/linguagem/10-excecoes.md`,
  10.x). A causa: o `emite_decorador_expr` (`vm/ps_compiler.c`) emitia o
  `GET_MEMBER` e a chamada sem passar pelo checador. `motor` (checador).~~
  Corrigido no 19.0.0: a expressão do decorador vira a árvore de uma
  expressão comum (o nome, um acesso a membro por `.` e a chamada), compilada
  pelo `expr`, e passa pelo checador como `obj.metodo(args)` em qualquer
  lugar. O parser guarda a posição do `.` e do `(` do decorador (`PSNode.i`),
  e o erro sai na mesma linha e coluna no `--check` e rodando. Testes: os
  "motor-33: ..." de `teste/casos_linguagem.c` (o membro e a chamada, antes de
  rodar e rodando pelo `Object`, com a coluna rodando), os dois "--check: ...
  do decorador" de `teste/cli_roda.pr` (a coluna e o fim no JSON do
  `--check`) e o "WebSocket da jinker saiu: @app.socket(...) antes de rodar"
  de `teste/casos_libs.c`.
- ~~**motor-34** (achado no plano do motor-24, 2026-10-24) As quatro formas do
  `count` não diziam o mesmo do mesmo erro no valor. Na forma prefixa e no
  `count each` (`count int(...) in xs`, lidos pelo `count_tipo_valor`), o
  nome dava `'=' e atribuicao; pra comparar use '=='`, o segundo valor
  `faltou ')' no valor de 'count'` e a estrela `expressao invalida`, frases
  que não apontam a causa. Na infixa e na sufixa, as frases certas apontavam
  a palavra `count`, e não o argumento. Nenhuma frase do `count` tinha caso.
  Medido no 19.0.0, com JIT e sem. `motor` (a frase e o lugar).~~ Corrigido
  no 19.0.1: as três frases ficam num lugar só (`COUNT_NOMEADO`,
  `COUNT_UM_SO` e `COUNT_ESTRELA`, em `vm/ps_parser.c`), usadas pelas duas
  leituras, e cada erro aponta o trecho que causa: a estrela, o nome ou o
  segundo valor. Casos: os "motor-34: ..." de `teste/casos_linguagem.c`, com
  a frase e o `^^^`.
- ~~**motor-35** (achado planejando o os-2, 2026-10-25) O
  `bytes.decode("utf-8", "replace")` punha um U+FFFD por byte do trecho
  inválido, e o Python põe um por trecho: `e2 82 28` dava dois antes do `(`,
  e `f0 9f 98` dava três. O texto saía diferente, calado. O Python segue a
  prática recomendada do Unicode ("U+FFFD Substitution of Maximal
  Subparts", seção 3.9 do padrão); o capítulo 12 dizia só que o `replace`
  dá o U+FFFD. Medido no 19.0.3: em 72 combinações (24 sequências e as três
  políticas), 18 diferiam do Python, todas com o trecho de mais de um byte
  (o `replace` e a frase do os-2). `motor` (resultado errado calado).~~
  Corrigido no 19.0.4: o `replace` põe um U+FFFD pelo trecho inteiro e
  salta pro fim dele (o `ignore` salta igual); as 72 combinações saem
  iguais ao Python, com JIT e sem. Casos: o "motor-35: ..." de
  `teste/casos_libs.c`; a regra no capítulo 12 e na página do `decode`.

## os

- ~~**os-1** (achado montando o prazo do portão P1, 2026-10-18) O
  `p.returncode` do `Process` fica `Null` depois que o filho acabou, até
  alguém chamar `p.wait()`. Programa:
  `Object p = os.run(["true"], capture="live")`, `sleep(0.5)`,
  `post(p.returncode)`: sai `Null`, com JIT e sem; o `p.wait()` seguinte dá
  `0`. Promessa: `docs/os/Process/Process.md`, tabela dos membros ("`Null`
  enquanto roda; depois, o código de saída"). Sem isso não há como saber se o
  filho acabou sem ficar preso: o `Process` não tem `poll()` (o Python tem, e
  é ele que atualiza o `returncode` de lá). `motor`.~~
  Corrigido no 19.0.3: a leitura do `returncode` e o `str()` do objeto
  conferem o filho sem esperar (`proc_confere`, `vm/jinga_vm.c`: o
  `waitpid` com `WNOHANG`; no Windows, o `ps_w_proc_terminou` e o
  `ps_w_proc_espera`), e guardam o código como o `wait()`. O `wait()` que
  encontra o filho já recolhido por essa leitura devolve o código guardado.
  Casos: os quatro "os-1: ..." de `teste/casos_libs.c`, e o exemplo da
  página do `Process`.
- ~~**os-2** (achado pelo gerador do P4, lote 3, 2026-10-24,
  `teste/casos_python.c`) Quando o trecho que não é UTF-8 tem mais de um byte,
  o `UnicodeDecodeError` do `os.readFile` diz só o primeiro, e o do Python diz
  a faixa inteira. Programa: o arquivo com os bytes `e2 82` (um caractere de
  três bytes cortado no fim). O motor dá `'utf-8' codec can't decode byte 0xe2
  in position 0: unexpected end of data`, e o Python dá `'utf-8' codec can't
  decode bytes in position 0-1: unexpected end of data`. O mesmo com `e2 82
  28` (`invalid continuation byte`) e com `f0 9f 98`. Promessa:
  `docs/os/readFile/readFile.md` ("lido como UTF-8, como o `open().read()` do
  Python: o que não é UTF-8 é `UnicodeDecodeError`, com a posição do byte").
  `motor`.~~
  Corrigido no 19.0.4: o `utf8_porque` (`vm/jinga_vm.c`) diz também onde
  acaba o trecho inválido máximo, como o decodificador do CPython, e a
  frase sai num lugar só (`frase_utf8`), pro `readFile` e pro `decode`: o
  byte, quando o trecho tem um, ou a faixa dele. Casos: os "os-2: ..." de
  `teste/casos_libs.c`, e os quatro do `teste/casos_python.c` que estavam
  pendentes, regerado.
- ~~**os-3** (achado pelo gerador do P4, lote 3) Com um arquivo no meio do
  caminho, o `NotADirectoryError` do `os.mkdir` diz o caminho inteiro, e o do
  `os.makedirs` diz a pasta que ele tentou criar. Programa: com `f` arquivo,
  `os.mkdir("f/g/h", exist_ok=True)` dá `[Errno 20] Not a directory:
  'f/g/h'`, e o Python dá `[Errno 20] Not a directory: 'f/g'`. Promessa:
  `docs/os/mkdir/mkdir.md` ("cria uma pasta (diretório), e as pastas do
  caminho que faltarem, como o `os.makedirs` do Python"). `motor`.~~
  Corrigido no 20.0.0: no `mod_os_mkdir` (`vm/jinga_vm.c`), a pasta do
  meio que falha com outro erro que não `EEXIST` vira o erro, com o caminho
  até ela, e nada depois dela é criado. Casos: o "os-3: ..." de
  `teste/casos_libs.c`, e o do `teste/casos_python.c` que estava pendente,
  regerado; o exemplo da página do `mkdir`.
- ~~**os-4** (achado pelo gerador do P4, lote 3) O `os.loadFile` de `.csv`
  perde o campo vazio do fim quando a última linha acaba em vírgula e o
  arquivo não termina em quebra de linha. Programa: o arquivo `a,b\n1,` dá
  `[{'a': '1', 'b': Null}]`, e o `csv.DictReader` dá `[{'a': '1', 'b': ''}]`.
  Com um campo a mais, o Python guarda o vazio na chave `None`, e o motor
  perde. Com `\n` ou `\r\n` depois da vírgula, os dois dão o mesmo. Promessa:
  `docs/os/loadFile/loadFile.md` ("lista de dicts (cabeçalho = chaves), como
  o `csv.DictReader` do Python"). `motor`.~~
  Corrigido no 19.0.2: no `csv_para_lista` (`vm/jinga_vm.c`), os laços dos
  campos param no fim da linha, e não no fim do texto: o campo depois da
  vírgula no fim do arquivo é lido, vazio. Casos: os dois "os-4: ..." de
  `teste/casos_libs.c` (sem e com a quebra de linha no fim), e os dois
  casos do `teste/casos_python.c` que estavam pendentes, regerado.
- ~~**os-5** (achado pelo gerador do P4, lote 3) O `os.mkdir` de um caminho que
  acaba em `/.`, com `exist_ok=False`, cria as pastas e depois dá
  `FileExistsError`. O `os.makedirs` não dá erro: ele para quando a última
  parte é `.`. Programa: `os.mkdir("b/f/.")` cria `b/f` e dá `[Errno 17] File
  exists: 'b/f/.'`, e o Python cria `b/f` e não dá erro. Com `exist_ok=True`,
  os dois dão o mesmo. Promessa: a mesma do os-3. `motor`.~~
  Corrigido no 20.0.0: antes de criar, o `mod_os_mkdir` guarda se a pasta
  de cima do `.` já existia; não existindo, volta sem erro depois de criar
  (o `tail == curdir` do `os.makedirs`), e existindo, é o `File exists` de
  sempre. Casos: o "os-5: ..." de `teste/casos_libs.c`, e os dois do
  `teste/casos_python.c` que estavam pendentes, regerado.

O `os.code` saiu no **20.0.0** (decisão dele, 2026-10-25: "tira esse .code,
nem faz sentido"). Ele abria o primeiro editor que achava no `PATH` (`code`,
`cursor`, `zed`, `nano` ou `vim`) e deixava o editor rodando solto. Na rodada
da VPS do conserto do os-3 e do os-5, o exemplo da página abriu o `nano`, e a
tela dele caiu na saída do portão P1. Não era item da lista. Agora o
`os.code()` dá `AttributeError: module 'os' has no attribute 'code'`, no
`--check` e rodando (pelo `Object`), na mesma linha. Casos: os dois "os.code
saiu: ..." de `teste/casos_libs.c`.

## teste

- ~~**teste-1** (achado no conserto do jinker) As quatro conferências de
  `sobra_temp` do `teste/jinker_roda.pr` exigiam zero temporário do motor na
  pasta, mas a resposta MORTA de um `get()` anterior só apaga o temporário
  dela quando o coletor passa. Passavam por sorte de tempo: um bloco de teste
  com texto grande vivo bastou pra derrubar as quatro.~~ Corrigido no 17.0.20:
  a conta é "não aumentou desde antes da operação" (o coletor só tira arquivo).
- ~~**teste-2** (achado na rodada da VPS, 2026-10-22) O portão `roda` matava
  o grupo de processos do exemplo com `kill -9 -<grupo>`. No Linux, o `kill`
  do procps lê só o 1º dígito do número negativo: `kill -9 -1426999` virou
  `kill(-1, 9)`, e a rodada como root matou todos os processos da VPS. Achado
  no mesmo código: sem `bwrap` nem `sandbox-exec`, o portão rodava os
  exemplos sem isolamento e só avisava.~~ Corrigido em 2026-10-23: o
  `kill -9 -- -<grupo>` do POSIX, com o sinal no 1º argumento (`cmd_kill`;
  com `-s KILL`, o procps tomava o grupo 83 por sinal); a prova com o sinal 0
  antes do 1º exemplo (`prova_do_kill`), que reprova a forma sem o `--`; sem
  isolador, nenhum exemplo roda (vermelho no Linux e no macOS; no Windows, o
  portão diz que não rodou).
- **teste-4** (achado na rodada do P1, 2026-10-23) O `teste/e2e/mail_local.pr`
  passa e deixa os servidores falsos vivos, escutando até alguém matar. No
  Mac, depois de `./pool teste/e2e_roda.pr mail_local` (rc 0), sobraram 19:
  o `teste/smtp_falso.py` 8 vezes e o `teste/imap_falso.py` 11. Na VPS, dentro
  do `unshare --pid`, sobraram 18 no fim do `check-e2e-local`. O teste chama
  o `kill()` do `Process` em cada servidor (a funct que encerra,
  `teste/e2e/mail_local.pr:87`), e eles continuam vivos. `teste`.
- **teste-5** (achado no conserto do motor-14, 2026-10-23) O P1 confere só o
  TIPO do erro que um bloco anuncia em comentário (`# TypeError: frase`), não
  a frase: o `scripts/audita_exemplos_doc.pr` e o
  `scripts/confere_saida_doc.pr` (`erro_anunciado`) aceitam o bloco quando o
  nome do tipo aparece no código. `docs/bytes/concat/concat.md:37` anuncia
  `TypeError: sequence item 1: expected a bytes-like object, int found`, o
  motor dá `... expected a byte-like object, int found`, e passa (doc-1).
  `teste`.
- ~~**teste-6** (achado montando o P3, 2026-10-23) O trabalho `portao` do
  `.github/workflows/ci.yml` roda o `make check` sem instalar o servidor do
  editor em `editor/vscode`. O teste do LSP dizia "PULOU" e nunca rodava no
  CI do Linux; o `build.yml` instala no macOS e no Windows. Com o P3, o
  portão novo pararia vermelho lá.~~ Corrigido no commit do P3: o trabalho
  roda o `npm install --omit=dev` em `editor/vscode` antes do `make check`,
  como o `build.yml` (não dá pra conferir no runner enquanto o CI estiver
  parado pela cobrança).
- ~~**teste-7** (achado montando o P3, 2026-10-23, lendo o código) O
  `teste/depurador.pr` montava o `Content-Length` com o `len` do texto, que
  conta caracteres, e o protocolo conta bytes. Na leitura, ele comparava a
  mesma conta com os bytes que chegavam. Com acento numa mensagem, a ida
  sairia cortada, e a volta esperaria bytes que não vêm. O teste passava
  porque as mensagens dele não têm acento (o alvo só tem acento em
  comentário).~~ Corrigido no commit do P3: o depurador usa o
  `scripts/enquadramento.pr`, o mesmo enquadramento do portão, que conta
  bytes.
- ~~**teste-8** (achado montando o P4, 2026-10-23) O `casos_de` do
  `teste/geradores/diferencial.pr`, que lê os programas dos `teste/casos_*.c`
  refazendo a leitura dos literais do C, lê errado dois casos:
  - joga fora a macro `#define` (`DEC_NET`, `teste/casos_linguagem.c:4553`):
    o "dif #914" e o "dif #922" congelaram o programa sem a classe `NET` e
    esperam `NameError: name 'NET' is not defined`;
  - corta como comentário o `/* ... */` de dentro de um literal
    (`"a/* x\ny */b"`, `teste/casos_linguagem.c:3233`): o "dif #539"
    congelou o texto `"ab"`.

  E é lento: ler a suíte toda levou 93 s no Mac. O portão da gramática lê os
  programas pelo `./testar --programas <pasta>`, que grava o texto que o
  próprio compilador C leu (2 s). O conserto é o diferencial ler pelo mesmo
  caminho e ser regerado (lote 2 do P4). `teste`.~~ Corrigido no lote 2 do
  P4: o gerador lê os casos pelo `./testar --programas` (`casos_do_runner`),
  e o `teste/casos_diferencial.c` foi regerado na VPS. O `casos_de` fica,
  porque o `scripts/modelo/indice.pr` usa.
- ~~**teste-9** (achado montando o P4, 2026-10-23, lendo o código) O
  `teste/propriedade.pr` sorteia pelos bits baixos do gerador congruente
  (`return semente % n`, linha 32). Com o módulo potência de 2, os bits
  baixos repetem com período curto (o bit 0 alterna). O `teste/leis.pr` já
  sorteia pelos bits altos (`int(semente / 65536) % n`, linha 68). `teste`.~~
  Corrigido no lote 2 do P4: o resto sai dos bits altos, como nas leis.
- ~~**teste-10** (achado montando o P4, 2026-10-23, lendo o código) O
  `teste/geradores/oraculo.pr` cria a pasta `/tmp/ps_oraculo` (linha 27) e
  não apaga, e o cabeçalho dele diz que "a doc da linguagem diz "igual ao
  Python" em toda parte", e a frase não aparece em nenhuma página de `docs/`
  (medido com `grep`). `teste`.~~ Corrigido no lote 2 do P4: a pasta sai no
  fim, e o cabeçalho diz o que o script faz (a divergência fica marcada pra
  ser julgada).
- ~~**teste-11** (achado no `make check` do lote 3 do P4, 2026-10-24) O
  gerador de programas (`teste/geradores/programas.pr`) gravava o achado
  encolhido sem conferir que a falha se repete. O Mac entrou em sono de
  manutenção no meio da rodada, um programa passou do prazo de 20 s de
  relógio, e virou o achado `teste/p4_achados/891e4223b69f.pr`, que roda em
  0,08 s com JIT e sem. O portão levou 1.214 s, contra 49 s. `teste`.~~
  Corrigido no lote 3 do P4: o achado encolhido roda de novo antes de virar
  arquivo; a falha que não se repete não grava nada, e a rodada reprova
  dizendo isso. O achado falso foi apagado.
- **teste-12** (achado juntando o commit dele `bea30df2`, 2026-10-24) O
  caminho do terminal do `sys.stdin.raw()` não tem teste. A suíte roda com a
  entrada em `/dev/null` e confere só o caminho sem terminal (`raw(True)`
  devolve `False`, e a leitura segue devolvendo `Null`,
  `teste/casos_linguagem.c`). A prova do terminal era o
  `examples/cobrinha.pr`, um jogo que lia tecla por tecla e precisava de pty,
  e ele saiu no `bea30df2`. O que o `docs/sys/sys.md` promete do `raw()`
  ("tecla na hora, sem travar") fica sem conferência, no Linux e no macOS.
  `teste`.
- ~~**teste-13** (achado regerando a tabela de retornos do 20.0.0,
  2026-10-25) O grupo `os` do `scripts/mede_retornos.pr` não grava: ele
  chama `os.loadFile(PNG, mode="r")`, que levanta `UnicodeDecodeError` (o
  texto do `loadFile` é UTF-8, e o PNG não é), e o medidor para com `NAO
  GRAVADO`. Medido no 19.0.4 e no 20.0.0, com JIT e sem. A chamada e a
  conferência do UTF-8 no `loadFile` entraram no mesmo commit, o 17.0.19
  (`58ae207`). `teste`.~~ Corrigido no 20.0.0: o `mode="r"` é medido num
  texto (o `os.json`), e a tabela do `os` foi regerada na VPS.

## doc

Achados pelo portão P1 (`notas/PORTOES.md`) em 2026-10-18: o
`scripts/confere_saida_doc.pr` roda cada exemplo da doc que tem saída e
compara. Nos sockets, as 11 divergências são da doc: o motor faz o que a doc
da linguagem promete, e o exemplo da página está errado. As páginas ficam em
`docs/sockets/`.

- **doc-1** O tipo do valor binário é `byte` (`docs/linguagem/02-tipos-e-valores.md`,
  tabela dos tipos; `docs/bytes/bytes.md`). Três páginas mostram `bytes` na
  saída de `type()`: `recv/recv.md:45` (`bytes 13`),
  `recvfrom/recvfrom.md:47` (`bytes datagrama`) e
  `getsockopt/getsockopt.md:46` (`bytes 4 ...`). O motor dá `byte`. O mesmo
  nome errado está no resumo dos membros de `docs/request/download/download.md:16`
  (`resp.content -> bytes`; no `--metadata`, `Response.content` devolve
  `byte`). E cinco páginas escrevem `bytes-like` na frase do erro (achado na
  análise da `ssl.pr` dele, 2026-10-23): `send/send.md:33`,
  `sendall/sendall.md:29`, `sendto/sendto.md:30`, `setsockopt/setsockopt.md:28`
  (`a bytes-like object is required`) e `docs/bytes/concat/concat.md:37`
  (`expected a bytes-like object`); o motor, o checador e
  `teste/casos_libs.c` dizem `byte-like`, pelo nome do tipo. O P1 não pega:
  a seção de erros não roda, e no erro anunciado em comentário ele confere o
  tipo, não a frase. `doc`.
- **doc-2** `close/close.md:55` imprime o `str(e)` de um `OSError` pego e
  espera `[Errno 9] Bad file descriptor`. A doc da linguagem (10.3) diz que a
  variável do `catch` vale `mensagem (linha N)`, e o motor dá
  `[Errno 9] Bad file descriptor (linha 5)`. `doc`.
- **doc-3** `getpeername/getpeername.md:34`: o programa faz um só
  `post(type(par), par[0], par[1] == porta)`, e a saída da página tem duas
  linhas (`tup 2` e `127.0.0.1 True`), de outro programa. O motor dá
  `tup 127.0.0.1 True`. `doc`.
- **doc-4** `send/send.md:42`: depois de `send("abcd")` e `send("ação")`, a
  página espera `b'abcdac\xcc\xa7ao'`, que não é o UTF-8 de `abcdação` (é o
  `c` com a cedilha combinante e o `a` sem til). O motor dá
  `b'abcda\xc3\xa7\xc3\xa3o'`, os 10 bytes que a própria página conta
  (4 + 6). `doc`.
- **doc-5** (falha fora do Linux) O número do erro do sistema muda de um
  sistema pro outro, e a doc cita o do Linux como se fosse o de todos. Três
  exemplos testam o número e dão `False` no macOS:
  `detach/detach.md:47` (`"Errno 98"`, endereço em uso: 48 no macOS),
  `getpeername/getpeername.md:55` (`"Errno 107"`, sem conexão: 57) e
  `connect_ex/connect_ex.md:43` (`111`, recusa: 61). As seções "Erros" citam
  do mesmo jeito: 107 em `shutdown.md:28` e `getpeername.md:28` e `:73`; 98
  em `bind.md:40`; 95 em `accept.md:40` e `listen.md:26`; 90 em
  `sendto.md:34`; 92 em `getsockopt.md:41` e `setsockopt.md:30`; 89 em
  `send.md:70`; e 36 (63 no macOS) em `sockets.md:110`,
  `docs/exceptions/exceptions.md:154` e `docs/builtins/open/open.md:58` e
  `:61`. O motor passa o número que o sistema dá, como o Python: medido no Mac
  com o Python 3, 48, 57, 61 e 63, os mesmos da Jinga. A forma certa já está
  em `connect/connect.md:30` ("os números e o texto são os do sistema", com o
  de cada um). `doc`.
- **doc-6** (falha fora do Linux) `getsockopt/getsockopt.md:46`,
  `setsockopt/setsockopt.md:35` e `setsockopt/setsockopt.md:70` esperam ler
  `1` de volta de `SO_REUSEADDR` e `SO_KEEPALIVE` ligados. O macOS devolve o
  valor da própria opção (4 e 8), e o Python no Mac dá o mesmo; o motor passa
  o que o sistema dá. O que vale nos dois é "diferente de 0". `doc`.

Achados pelo portão `check` do P1 em 2026-10-18, depois de escrito o contexto
de cada página (os blocos ```ps contexto): o `--check` de cada exemplo com o
que a página pressupõe na frente. Cada um foi medido de novo rodando, com JIT
e sem, e dá o mesmo erro.

- **doc-7** Três exemplos tratam a divisão `/` entre inteiros como inteiro. A
  doc da linguagem diz que `/` dá `flo` sempre (3.2.1), que o retorno da funct
  tipada não é convertido (6.4) e que não há conversão implícita.
  `docs/LANGUAGE.md:455`: `int funct f() { return 1 / 0 }` e
  `bool funct g() { return 1 / 0 }`, pra mostrar o sentinela 500 e `False`; o
  motor recusa antes de rodar (`retorno de f() esperava int, recebeu flo`), e
  o sentinela nunca aparece. `docs/Jinga.md:303`: `x /= 2    # 19.5` com
  `int x` (`variável x esperava int, recebeu flo`). `docs/Jinga.md:462`:
  `int x = 1 / 0` dentro do `try`, então o `catch` que imprimiria
  `Erro: division by zero` nunca roda. `doc`.
- **doc-8** Dois middlewares liberam a rota com `continue`: `docs/Jinga.md:858`
  (linhas 890 e 898) e `docs/hash_jwt.md:152` (linha 179). A doc da linguagem
  diz que `continue` fora de laço é `SyntaxError` (5.4), e a do middleware
  (`docs/jinker/middleware/middleware.md`) diz que quem libera a rota é o
  `pass`; o motor dá `SyntaxError: 'continue' fora de laco`. Com `pass`, os
  dois blocos passam. `doc`.
- **doc-9** `docs/psodbc.md` promete dois parâmetros do `connect` que o motor
  não tem, `odbc_driver` e `trust_server_cert`: na assinatura (linha 40), na
  tabela (linhas 54 e 55), nos parágrafos (75 a 84) e no exemplo 4 do bloco da
  linha 95 (`odbc_driver="ODBC Driver 17 for SQL Server"`, recusado com
  `TypeError: 'odbc_driver' is an invalid keyword argument for connect()`). A
  página da própria função (`docs/psodbc/connect/connect.md:93` e `:98`) já
  diz que os dois "estava documentado e não existe no motor". A linha 75 ainda
  promete que o driver ODBC é achado sozinho (18, 17, 13...), e o motor fixa
  `DRIVER={ODBC Driver 18 for SQL Server}` (`vm/jinga_vm.c:21355`). `doc` (ou
  `motor`, se a decisão for ter os dois parâmetros).
- **doc-10** `docs/Jinga.md:619`: o texto diz que "um valor não-int não mexe
  no contador" do enum, e o exemplo promete `E=12` em
  `enum Mix { A, B = 10, C, D = "x", E }`. A doc da linguagem (8.2) e o motor
  dizem o contrário: `TypeError: enum Mix: o membro E nao tem valor e o
  anterior (D) nao e inteiro`. `doc`.
- ~~**doc-11** `full` e `mei` sem aspas nos exemplos do `manpu`: `cell=full` e
  `amount=full` dão `NameError: name 'full' is not defined`. A assinatura
  (`docs/manpu/remove/remove.md`: `amount="full"`) e o motor (que compara os
  textos `"full"` e `"mei"`) pedem texto. Blocos: `docs/manpu.md:90`, `:124`,
  `:138`, `:161`, `:183` e `:226`; `docs/manpu/open/open.md:30` e `:43`;
  `docs/manpu/remove/remove.md:28` e `:41`;
  `docs/manpu/ManpuFile/ManpuFile.md:36`; `docs/manpu/manpu.md:44` e `:55`;
  `docs/manpu/ManpuFile/write/write.md:24`. `doc`.~~
  Fechado no 19.0.0: as páginas saíram com a lib `manpu` (decisão dele,
  2026-10-24).
- **doc-12** Três páginas do `mail` importam com `from mail import X` e chamam
  `mail.X()`: `docs/mail/MailServer/MailServer.md:5`,
  `docs/mail/MailReader/MailReader.md:5` e
  `docs/mail/MailMessage/MailMessage.md:6`. Pela doc da linguagem (9.2), o
  `from ... import` não liga o nome do módulo; o motor dá
  `NameError: name 'mail' is not defined`. `doc`.
- **doc-13** O `login` do `MailServer` e do `MailReader`: as páginas dele
  (`docs/mail/MailServer/login/login.md` e `docs/mail/MailReader/login/login.md`)
  prometem `login(user, password) -> Null`; o motor tem
  `login(usuario, senha) -> bool` (`--metadata`) e recusa o nome da doc:
  `TypeError: 'user' is an invalid keyword argument for login()`, nos blocos
  `docs/libs_utilitarias.md:54`, `:85`, `:141` e `:167` (e o
  `docs/manpu.md:226`, que saiu com a manpu no 19.0.0). `os dois`: os nomes
  são decisão sua; os da doc são os
  do `smtplib` e do `imaplib` do Python (`login(user, password)`).
- **doc-14** O `body` do `MailMessage` com `byte`: a página dele
  (`docs/mail/MailMessage/body/body.md:12`) diz que o corpo aceita "texto,
  HTML, ou bytes de um arquivo", e `docs/manpu/load/load.md` ensinava passar o
  `byte` do `manpu.load`. O motor só aceita `str`:
  `TypeError: body() argument 1 must be str, not byte`, nos blocos
  `docs/manpu/load/load.md:15`, `docs/manpu.md:42` e `docs/manpu.md:226`. A
  própria `MailMessage.md` diz "texto ou HTML". `os dois`. Os três blocos
  saíram com a manpu no 19.0.0; fica a promessa da `body.md:12`, sem bloco
  que a rode.
- **doc-15** `docs/jinker/Jinker/Jinker.md:59`, `:75`, `:98` e `:118` escrevem
  o `oauth` com chave sem aspas (`{poolip: True, rate: 60}`): pela doc da
  linguagem (2, a chave do dict é uma expressão), é `NameError`, e
  `docs/jinker/tls/tls.md` e `docs/objetos-internos/PoolIp.md` já escrevem
  com aspas. O bloco da
  linha 118 também chama `cors(options=..., origins=...)` sem o servidor, que
  `docs/jinker/cors/cors.md` diz ser erro (rodando: `TypeError: cors() precisa
  do servidor como primeiro argumento`). `doc`.
- **doc-16** Duas páginas descrevem funções que a lib cliente `request` não
  tem: `docs/request/get_json/get_json.md` (`request.get_json()`) e
  `docs/request/text/text.md` (`request.text()`): `AttributeError: module
  'request' has no attribute ...`. As duas são do `request` da jinker (dentro
  da rota), que tem página própria, e o índice `docs/request/request.md` não
  aponta pra elas. `doc`.
- **doc-17** `docs/qrcode.md:68` chama no módulo os métodos do objeto que ele
  mesmo cria: `qrcode.add_data`, `qrcode.make(fit=...)`, `qrcode.make_image` e
  `qrcode.clear` são do `PoolQRCode` (`qr.add_data`...). O commit `5b4cc2f`
  trocou `qr.` por `qrcode.` pra tirar o apelido do módulo e levou a variável
  junto. `doc`.
- **doc-18** Três frases de `docs/LANGUAGE.md` contradizem o motor:
  - linhas 177 a 181: "não existe checagem de redeclaração no motor" e
    `int x = 1` seguido de `str x = "a"` "troca o tipo em silêncio". O motor dá
    `AttributedValueError: variável x já foi declarada como int e não pode ser
    redeclarada como str`, rc 2;
  - linhas 250 a 258: escape desconhecido "perde o backslash silenciosamente"
    (`"C:\Users\..."` vira `"C:Users..."`). O motor guarda a barra e avisa com
    `SyntaxWarning: sequencia de escape invalida '\U' — a barra fica no texto`,
    como o Python 3.12. O exemplo de string crua que a frase anuncia é um
    bloco vazio (já no `HEAD`);
  - linha 785: "Os parênteses após o nome são obrigatórios, mesmo sem
    herança". A regra é a de `class A {` sem `()`, e o motor recusa
    `class A() { }`. `doc`.
- **doc-19** (achado pelo portão `roda`) `docs/psodbc/query/query.md:49` e
  `:62` ensinam passar o comando como tupla `(sql, params)`, com `?` no lugar
  dos valores ("nunca concatene na string"). O motor só aceita texto:
  `TypeError: query() argument 2 must be str, not tup`. Quem segue a página
  não tem como passar parâmetro pelo `query`, e o aviso contra concatenar fica
  sem saída. `os dois` (o motor, se o `query` tiver de aceitar parâmetros como
  a página diz; senão, a doc).
- **doc-20** (achado pelo portão `roda`) Cinco exemplos usam o `request` da
  jinker no topo do programa, fora de uma rota, onde a doc dele não promete
  nada (e o motor-6 faz o `--check` aprovar): `docs/jinker/request/path_param/path_param.md:43`,
  `docs/jinker/request/file/file.md:74`,
  `docs/jinker/request/get_json/get_json.md:54`,
  `docs/jinker/request/header/header.md:41`,
  `docs/psodbc/DbCursor/execute/execute.md:54`, `docs/json.md:43` e
  `docs/json/json.md:43`. Rodando: `NameError: name 'request' is not
  defined`, ou, com o `from jinker import request` do contexto do `json.md`,
  o `Null` do `request.text()` vira `TypeError: parse() argument 1 must be
  str, not Null`. O mesmo em `docs/mail/MailMessage/attach/attach.md:39`
  ("dentro de uma rota que recebe upload", escrito no topo: o `request.file`
  dá `Null` e o `attach(Null)` quebra). A forma que as outras páginas usam é
  o trecho dentro da rota. `doc`.
- **doc-21** (achado pelo portão `roda`) `docs/qrcode/QRImage/to_file/to_file.md:21`
  ensina anexar o QR num e-mail sem gravar no disco:
  `m.attach(img.to_file())`. O `--check` aprova e rodando sai
  `TypeError: attach() argument 1 must be str, not QRPoolFile`, com JIT e
  sem. A página do `attach` (`docs/mail/MailMessage/attach/attach.md`) lista
  o que ele aceita: caminho, `PoolFile` e `PoolFileUpload` (medido: caminho e
  o `PoolFile` de um arquivo binário passam). Duas falhas: as duas páginas se
  contradizem sobre o `QRPoolFile`, e o checador aprova o que o motor recusa.
  `os dois` (o `attach` aceitar o `QRPoolFile`, ou a página do `to_file`
  deixar de prometer; e o checador conferir o tipo do argumento).
- **doc-22** (relato dele, 2026-10-21) O `get` do `Response` do `request`
  cliente. A página (`docs/request/Response/Response.md`) diz só "um header
  (case-insensitive) ou uma chave do JSON". Medido, com JIT e sem: cabeçalho
  sai texto; chave do JSON sai o tipo do JSON (`int`, `bool`, `list`...);
  quando cabeçalho e chave têm o mesmo nome, ganha o cabeçalho; o `null` do
  JSON e a chave que não existe dão os dois `Null`; com corpo que não é JSON,
  só cabeçalho acha. E o `.headers` é um `dict` que diferencia maiúscula
  (`r.headers.get("x-dado")` dá `Null` e `"X-Dado"` dá o valor), ao contrário
  do `get` (o `requests` do Python usa um dicionário que não diferencia). A
  página não diz nada disso. `doc`.
- **doc-23** (relato dele, 2026-10-21: "os documentos estão rasos, só dizem o
  que é, mas não especifica se faz outra tarefa relevante ou como se
  comporta") A doc dos módulos diz o que o membro é e não diz o contrato: o
  tipo exato de cada retorno, o retorno nos casos especiais, o que acontece
  com argumento de borda, os erros e o que muda de sistema pra sistema. O
  jinker-10, o doc-22 e o editor-2 são exemplos medidos. A referência é a das
  linguagens consolidadas: o guia do Javadoc chama a doc de "contrato entre
  quem chama e as implementações", exige o `@param` de todo parâmetro e o
  `@return` de todo método, pede "o retorno nos casos especiais", as
  "condições de borda, faixas de parâmetro e casos de canto", e que o
  comportamento que muda de sistema venha marcado "On <plataforma>"; a doc do
  Python dá o tipo exato do que volta ("um objeto bytes"; "bytes vazio indica
  que o cliente desconectou"). A reavaliação é um plano próprio, pra
  aprovação dele. `doc`.
- ~~**doc-24** (achado pelo portão `roda`, `docs/os/loadFile/loadFile.md:42`)
  `c = os.loadFile("logo.png", mode="r")   # o texto dos bytes`: num PNG de
  verdade (começa com o byte `0x89`) sai `UnicodeDecodeError: 'utf-8' codec
  can't decode byte 0x89 in position 0`, com JIT e sem. O `mode="r"` lê como
  texto UTF-8, e a própria página diz que o que não é UTF-8 dá esse erro; o
  comentário do exemplo promete outra coisa. `doc`.~~
  Corrigido no 20.0.0 (decisão dele, 2026-10-25: "muda só o comentário"): o
  comentário do exemplo diz `UnicodeDecodeError: o PNG não é UTF-8`, e o
  motor fica. O portão `roda` lê o tipo do erro no comentário como erro
  anunciado, e a linha da catraca saiu.
- **doc-25** `docs/os/PoolFile/PoolFile.md:31` comenta
  `os.loadFile("logo.png")   # PoolFile (extensão binária)`. Quem decide é o
  conteúdo, não a extensão: medido, um `.png` com texto dentro volta `str`, e
  um `.txt` com bytes binários volta `PoolFile` (a regra está certa no
  `docs/os/loadFile/loadFile.md`: byte NUL nos primeiros 8 KB). E na mesma
  página faltam duas coisas que se mediu montando o contexto dela: o
  `os.copy` e o `os.move` não criam a pasta de destino (`FileNotFoundError`
  com o caminho de destino), e o `os.rmdir` de pasta com conteúdo dá
  `OSError` com o número do sistema (66, medido no macOS); as páginas
  do `copy`, do `move` e do `rmdir` também não dizem. `doc`.
- **doc-26** (achado pelo portão P2, 2026-10-23, na conferência das páginas
  de método do `scripts/audita_doc.pr`) 14 páginas de método dão aos
  parâmetros outros nomes que o motor. A doc escreve em inglês e o motor em
  português:
  - `MailServer` e `MailReader`: `login(user, password)` na doc e
    `login(usuario, senha)` no motor; `conn(host, port)` e `conn(host, porta)`;
  - `MailMessage`: `subject(title)` e `subject(titulo)`; `fromAddress` e
    `toAddress` (`address` e `endereco`); `body(content, is_html)` e
    `body(conteudo, is_html)`; `attach(file_or_path)` e
    `attach(arquivo_ou_caminho)`;
  - `MongoCollection`: `insert(document)` e `insert(documento)`,
    `insert_many(documents)` e `insert_many(documentos)`,
    `update(query, new_values)` e `update(query, novo)`; `MongoConnection`:
    `collection(name)` e `collection(nome)`;
  - `str.format`: a doc mostra `s.format(a, b)` e o motor recebe `*valores`.

  Quem segue a doc e passa o argumento por nome leva erro, no `--check` e
  rodando: `login(user="x", password="y")` dá `TypeError: 'user' is an invalid
  keyword argument for login()`. Qual dos dois nomes fica é decisão dele. 14
  páginas na catraca (portão `titulo`). `os dois`.
- ~~**doc-27** (achado montando o P3, 2026-10-23) O lote do 18.0.0 deixou
  frases sobre o `.type()` que saiu. No `docs/lsp.md`, o receptor de tipo
  desconhecido "recebe ao menos o que todo valor tem", e o hover de `int n`
  "diz que só tem `type()`". Medido no 18.0.0, o completion desse receptor
  não dá nada, e o hover de `int n` diz **sem membros**. Três testes do
  `editor/vscode/teste_servidor.js` ainda tinham "universais" no nome, e eles
  conferem que o `type` não aparece.~~ Corrigido no commit do P3: as duas
  frases e os três nomes dizem o que o editor faz.
- **doc-28** (achado escrevendo a gramática, 2026-10-23) A precedência do
  capítulo 3 diverge do parser em dois pontos, medidos no `--check` e
  rodando, com JIT e sem:
  - o resumo (3.13) diz "`*`/`/`/`%` < `**` < unários", e a tabela 3.1 e o
    parser dizem o contrário (a potência no nível 13, os unários no 12):
    `post(-2 ** 2)` imprime `-4`;
  - o nível 11 da tabela 3.1 não tem o `//`, que o parser trata junto com
    `*`, `/` e `%`, à esquerda: com `int a = 7` e `int b = 2`, `post(a + b //
    2, a // 2 * 3)` imprime `8 9`.

  A gramática (`docs/linguagem/gramatica.md`) tem a tabela que o portão
  confere. `doc`.
- **doc-29** (achado escrevendo a gramática, 2026-10-23) A tabela de padrões
  do `case` (5.5) não tem o padrão de dicionário nem o número negativo, e o
  motor aceita e roda os dois, com JIT e sem: com `dict d = {"a": 1}`, o
  `match d` com `case {"a": x} { post("a vale", x) }` imprime `a vale 1`; com
  `int n = -3`, o `case -3` casa. A gramática tem os dois
  (`padrao_simples`). `doc`.
- **doc-30** (achado escrevendo a gramática, 2026-10-23) A 4.5 diz que o alvo
  do desempacotamento "é o mesmo da seção 4.4: nome, membro (`o.x`), índice
  (`l[i]`, `d[k]`) e as cadeias deles (`o.d["k"]`)". O desempacotamento
  aceita o membro depois do índice: com `Entity C { int c = 0 }` e `list a =
  [C()]`, `a[0].c, x = 1, 2` roda e `post(a[0].c, x)` imprime `1 2`. A
  atribuição simples não: `a[0].c = 1` dá `SyntaxError: '=' e atribuicao; pra
  comparar use '=='` (5:8), no `--check` e rodando, com JIT e sem, uma frase
  que não aponta a causa. Na gramática, a atribuição simples tem três formas
  (`x = v`, `x.a.b = v` só com nomes, e o alvo que termina num índice:
  `atribuicao`, `atribuicao_de_membro` e `atribuicao_a_indice`), e o
  desempacotamento aceita qualquer cadeia (`alvo_com_sufixos`). Qual dos dois
  fica é decisão dele. `os dois`.
- ~~**doc-31** (achado montando o P4, 2026-10-23) A lista dos passos do
  `make check` no `JINGA-TUDO.md` (5.6) não tinha o `mede_aridades.pr
  --confere` nem os portões P1 (`confere_saida_doc.pr`), P2 e P3.~~
  Corrigido no commit do P4 (lote 1): a lista segue o `Makefile`, com o
  portão `gramatica`.
- **doc-32** (achado pelo gerador do P4, lote 3, 2026-10-24) O exemplo do
  capítulo 12 diz que `"İ".lower()` dá `i`. O resultado é o `i` seguido do
  ponto combinante U+0307, dois caracteres, no motor e no Python. A página do
  `lower` diz certo (`docs/str/lower/lower.md`: "`len("İ".lower())` é 2").
  Lugar: `docs/linguagem/12-metodos-string-list-dict.md`, linha 45 (`# i —
  o I turco com pingo`). `doc`.
- **doc-33** (achado pelo gerador do P4, lote 3) A página do `static_folder`
  diz que o tipo sai "pela tabela padrão do `mimetypes` do Python 3.15 (a
  mesma do `http.server`, 213 extensões)". A da jinker é a
  `_types_map_default` do `mimetypes`, igual nas 213 (conferido no
  `teste/jinker_roda.pr`). O `http.server` soma quatro de compressão:
  - `.gz`, `application/gzip`;
  - `.bz2`, `application/x-bzip2`;
  - `.xz`, `application/x-xz`;
  - `.Z`, `application/octet-stream`.

  As três primeiras a jinker serve como `application/octet-stream`. Quem
  decide é ele: a doc tira o "a mesma do `http.server`", ou a jinker soma as
  quatro. Lugar: `docs/jinker/static_folder/static_folder.md`, linha 84.
  Decisão dele (2026-10-24): a jinker soma as quatro do `http.server`, e a
  doc passa a contar 217 extensões. Com isso o conserto é do motor (a tabela
  da jinker), com a doc junto. `jinker` (e `doc`).
- ~~**doc-34** (achado instalando o 19.0.0 no Mac, 2026-10-24) O `make
  install` copiava as páginas por cima da doc instalada e nunca apagava a
  que saiu do repositório. Depois do 19.0.0, `/usr/local/share/jinga/docs`
  continuava com a `manpu.md`, a `manpu/`, a `jinker/socket/`, a
  `jinker/channel/` e os objetos internos do WebSocket. O servidor do editor
  só pede página de nome que o `--metadata` publica, mas a busca dele pega a
  primeira página que existe entre os caminhos candidatos, e o que lê a pasta
  direto via documentação do que a Jinga não tem mais.~~ Corrigido logo
  depois do 19.0.0: a cópia da doc virou o alvo `install-doc`, que o
  `install` chama e que apaga a pasta instalada antes de copiar. Teste:
  `teste/cli_roda.pr` instala numa pasta com uma página que não existe no
  repositório, e ela sai. `doc`.
