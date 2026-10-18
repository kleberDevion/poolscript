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

Achados em 2026-10-17, no conserto acima e no relato dele (abertos):

- **jinker-6** Rota que devolve um `dict` que não vira JSON responde 200 com o
  corpo vazio, calada (nem com `debug=True` sai erro). Programa: rota com
  `return {"status": app.channel.status}`; o `ChannelStatus` não vira JSON
  (`json.stringify` dá "Object of type ChannelStatus is not JSON
  serializable"). Saída no 17.0.19 e no 17.0.20: `HTTP/1.1 200 OK`,
  `Content-Length: 0`. Promessa: `docs/jinker/route/route.md` ("dict/lista →
  JSON 200"); o que não vira JSON tem que dar erro visível (500 com a frase),
  como no Flask. `motor`.
- **jinker-7** (relato dele) `app.channel(forAll=msg)`, a forma que
  `channel.md`, `jinker.md` e `ChannelManager.md` ensinam, é recusada antes de
  rodar com `TypeError: Jinker.channel() takes no keyword arguments` quando o
  `app` tem tipo (`Jinker app = Jinker(__name__)`) ou é atribuído direto; o
  `app.socket(path="/x")` igual (`Jinker.socket()`). Sem o checador, roda. A
  causa: o `--metadata` publica a propriedade (`channel`, `socket`) com
  `params: []`, sem os parâmetros da chamada do valor dela (`forAll`; `path`,
  `channel`), e o medidor de aridades gravou "não aceita nome". `motor`
  (checador).
- **jinker-8** Argumento a mais passa calado: `app.channel("a", "b")` e
  `app.socket("/x", True, 3)` rodam sem erro. Promessa: `channel.md` (um
  parâmetro, `forAll`) e `socket.md` (`path`, `channel`). `motor`.

## editor

- **editor-1** (relato dele, 2026-10-17) O hover sobre o NOME de um argumento
  (`forAll` em `app.channel(forAll=msg)`) diz "nome não definido neste arquivo
  nem por import (rodando dá `NameError`)": o servidor do editor trata o nome
  do argumento como variável. Medido no `server.js` do 17.0.20, com um pedido
  de hover na coluna do `forAll`. `motor` (editor).

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
- **motor-4** (GRAVE, perda de dado; achado lendo o `psodbc.query` em
  2026-10-18, por causa do doc-19) O `psodbc.query` copia o SQL num buffer
  fixo de 4.096 bytes com `snprintf` (`mod_db_query`, `vm/jinga_vm.c:21408`) e
  executa o pedaço que coube, calado. Programa: tabela `t` com 3 linhas e
  `psodbc.query(base="corte.db", cmd="DELETE FROM t" + " " * 4090 + "WHERE id = 1")`
  (4.115 bytes): sobram 0 linhas, não 2, rc 0, com JIT e sem. O `WHERE` caiu
  no corte e o `DELETE` apagou tudo. No mesmo assunto, o caminho do MySQL
  (`mysql_exec`, `vm/ps_db.c:472`) recusa com "sql grande demais" o SQL acima
  de 16 KB: não corta, mas recusa uso válido (um `INSERT` em lote). `motor`.
- **motor-5** (achado pelo portão `roda`, `docs/manpu/read/read.md:27`) O
  `manpu.read` de um arquivo que não existe não levanta erro: devolve o texto
  `Error: arquivo não encontrado: nao_existe.csv` como se fosse o conteúdo,
  rc 0. O exemplo da página quebra depois, no índice
  (`TypeError: string indices must be integers, not 'str'`), longe da causa.
  O `os.readFile` e o `open` da Jinga levantam `FileNotFoundError` no mesmo
  caso, como o `open` do Python. `motor`.

## os

- **os-1** (achado montando o prazo do portão P1, 2026-10-18) O
  `p.returncode` do `Process` fica `Null` depois que o filho acabou, até
  alguém chamar `p.wait()`. Programa:
  `Object p = os.run(["true"], capture="live")`, `sleep(0.5)`,
  `post(p.returncode)`: sai `Null`, com JIT e sem; o `p.wait()` seguinte dá
  `0`. Promessa: `docs/os/Process/Process.md`, tabela dos membros ("`Null`
  enquanto roda; depois, o código de saída"). Sem isso não há como saber se o
  filho acabou sem ficar preso: o `Process` não tem `poll()` (o Python tem, e
  é ele que atualiza o `returncode` de lá). `motor`.

## teste

- ~~**teste-1** (achado no conserto do jinker) As quatro conferências de
  `sobra_temp` do `teste/jinker_roda.pr` exigiam zero temporário do motor na
  pasta, mas a resposta MORTA de um `get()` anterior só apaga o temporário
  dela quando o coletor passa. Passavam por sorte de tempo: um bloco de teste
  com texto grande vivo bastou pra derrubar as quatro.~~ Corrigido no 17.0.20:
  a conta é "não aumentou desde antes da operação" (o coletor só tira arquivo).

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
  `byte`). `doc`.
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
- **doc-11** `full` e `mei` sem aspas nos exemplos do `manpu`: `cell=full` e
  `amount=full` dão `NameError: name 'full' is not defined`. A assinatura
  (`docs/manpu/remove/remove.md`: `amount="full"`) e o motor (que compara os
  textos `"full"` e `"mei"`) pedem texto. Blocos: `docs/manpu.md:90`, `:124`,
  `:138`, `:161`, `:183` e `:226`; `docs/manpu/open/open.md:30` e `:43`;
  `docs/manpu/remove/remove.md:28` e `:41`;
  `docs/manpu/ManpuFile/ManpuFile.md:36`; `docs/manpu/manpu.md:44` e `:55`;
  `docs/manpu/ManpuFile/write/write.md:24`. `doc`.
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
  `docs/libs_utilitarias.md:54`, `:85`, `:141` e `:167` e
  `docs/manpu.md:226`. `os dois`: os nomes são decisão sua; os da doc são os
  do `smtplib` e do `imaplib` do Python (`login(user, password)`).
- **doc-14** O `body` do `MailMessage` com `byte`: a página dele
  (`docs/mail/MailMessage/body/body.md:12`) diz que o corpo aceita "texto,
  HTML, ou bytes de um arquivo", e `docs/manpu/load/load.md` ensina passar o
  `byte` do `manpu.load`. O motor só aceita `str`:
  `TypeError: body() argument 1 must be str, not byte`, nos blocos
  `docs/manpu/load/load.md:15`, `docs/manpu.md:42` e `docs/manpu.md:226`. A
  própria `MailMessage.md` diz "texto ou HTML". `os dois`.
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
