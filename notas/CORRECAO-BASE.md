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

## teste

- ~~**teste-1** (achado no conserto do jinker) As quatro conferências de
  `sobra_temp` do `teste/jinker_roda.pr` exigiam zero temporário do motor na
  pasta, mas a resposta MORTA de um `get()` anterior só apaga o temporário
  dela quando o coletor passa. Passavam por sorte de tempo: um bloco de teste
  com texto grande vivo bastou pra derrubar as quatro.~~ Corrigido no 17.0.20:
  a conta é "não aumentou desde antes da operação" (o coletor só tira arquivo).
