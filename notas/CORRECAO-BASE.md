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
  (checador). O portão P2 achou sozinho, em 2026-10-23: `app.channel(forAll=)`,
  `app.socket(path=)` e `app.socket(channel=)`, o `--check` recusa e o motor
  aceita; com um nome que não existe, os dois recusam com frases diferentes
  (`Jinker.channel() takes no keyword arguments` e `ChannelManager.__call__()
  got an unexpected keyword argument`). 5 chamadas na catraca do P2.
- **jinker-8** Argumento a mais passa calado: `app.channel("a", "b")` e
  `app.socket("/x", True, 3)` rodam sem erro. Promessa: `channel.md` (um
  parâmetro, `forAll`) e `socket.md` (`path`, `channel`). `motor`.

Medidos em 2026-10-22 (relato dele: "o `.get()` do `jinker.request.get()` e o
do `request` não diz se obtém string também"), num servidor da jinker no
loopback, com JIT e sem (`notas/` não guarda o programa; ele está descrito
em cada item):

- **jinker-9** O `request.get(chave)` da rota devolve LISTA quando a chave se
  repete na query string: `?a=1&a=2` dá `['1', '2']`, e `?a=1` dá `'1'`. O
  tipo muda conforme o que o cliente manda, e a rota escrita com tipo
  (`str nome = request.get("nome")`) cai com 500 quando alguém repete o
  parâmetro. A página (`docs/jinker/request/get/get.md`) não diz nada sobre
  chave repetida. No Flask, `request.args.get` devolve o primeiro valor, e
  `getlist` devolve todos. `duvida`: a sua decisão é o primeiro valor, como
  no Flask, com um jeito de pegar todos, ou a lista de hoje escrita na doc.
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

- **editor-1** (relato dele, 2026-10-17) O hover sobre o NOME de um argumento
  (`forAll` em `app.channel(forAll=msg)`) diz "nome não definido neste arquivo
  nem por import (rodando dá `NameError`)": o servidor do editor trata o nome
  do argumento como variável. Medido no `server.js` do 17.0.20, com um pedido
  de hover na coluna do `forAll`. `motor` (editor).
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
- **motor-9** (achado pelo portão `roda`, 2026-10-22, `docs/manpu.md:58`) O
  `mp.write(...)` devolve um `ManpuResult` que se imprime `Success` ou
  `Error` e que é `== True` quando deu certo. As páginas (`docs/manpu.md:58`
  e `docs/manpu/write/write.md:36`) leem o motivo do erro em `nm.status`, e o
  `ManpuResult` não tem membro nenhum (`--metadata`): rodando, `nm.status` dá
  `AttributeError: 'ManpuResult' object has no attribute 'status'`. E quando
  a escrita falha (destino numa pasta que não existe) volta `Error` sem
  motivo nenhum e sem exceção, com JIT e sem: o erro some. O `open` e o
  `os.writeFile` levantam a exceção com o caminho. `os dois`: o motor dá o
  motivo (o `.status` que a doc promete, ou a exceção), e a doc diz qual.
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
  - os da jinker que nascem sem servidor: `ChannelManager`, `SocketNamespace`,
    `SocketEmitter`, `RouteRegistrar`;
  - os do `psodbc`, `DbConnection` e `DbCursor`, e o `ManpuFile`;
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
  recebeu Null` (o `--metadata` diz `Null`). `motor`.
- **motor-14** (decisão do Kleber, 2026-10-23) O `.type()` sai da linguagem:
  fica só o `type(x)`. Nas palavras dele: "o uso deve ser somente type() não
  .type(), retire ele, isso faz ter menos trabalho". Medido pelo portão P2: o
  `.type()` é o único membro universal (`METODOS_UNIV`, `vm/jinga_vm.c`) e
  devolve `str`, e o `--check` não sabe (`int x = 1.type()` passa e rodando
  dá `esperava int, recebeu str`). Tirar quebra quem usa `.type()`: no
  repositório são 10 linhas (a doc, dois casos de teste, o checador).
  `motor`.

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
- **doc-24** (achado pelo portão `roda`, `docs/os/loadFile/loadFile.md:42`)
  `c = os.loadFile("logo.png", mode="r")   # o texto dos bytes`: num PNG de
  verdade (começa com o byte `0x89`) sai `UnicodeDecodeError: 'utf-8' codec
  can't decode byte 0x89 in position 0`, com JIT e sem. O `mode="r"` lê como
  texto UTF-8, e a própria página diz que o que não é UTF-8 dá esse erro; o
  comentário do exemplo promete outra coisa. `doc`.
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
