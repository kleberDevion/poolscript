# Migração da PoolScript para C

Objetivo: **tudo nativo, zero CPython em tempo de execução**. A linguagem deve
rodar como um binário próprio, portável como Lua.

Documento de estado + plano. Números medidos, não estimados.

---

## Estado por camada

A ordem importa: cada camada depende da anterior. Não adianta otimizar o
runtime enquanto o parser ainda é Python — o `.ps` não chega até lá sozinho.

```
Camada 1  Lexer .................... ████████████ 100%  ✅ fechada
Camada 2  Parser (AST) ............. ████████████ 100%  ✅ fechada
Camada 3  Compilador (AST→bytecode)  ████████████ 100%  ✅ fechada
Camada 4  Runtime (VM + builtins) .. ██████████░░  ~85%  🔨 falta biblioteca
Camada 5  Binário standalone ....... ████████████ 100%  ✅ fechada
```

**2278 testes passando.** A VM roda **1353 dos 1355** trechos `.ps` da suíte
que o interpretador roda (99,9%) — medido, não estimado. Os 2 que faltam são
`db.query`, que depende da lib `psodbc` (categoria B abaixo).

Camadas 1–3 e 5 sem pendência. Na 4: **todos os 38 builtins**, **53 métodos
de string de 53**, **9 libs reais de 18** (+ as 5 stubs). O que trava um
`.ps` real hoje é biblioteca que linka C externo — HTTP, banco, e-mail —,
não a linguagem.

### O binário

```
pool arquivo.ps        16 MB, startup 0,7 ms (bancos estáticos; mongoc/ldap dinâmicos)
```

Contra 48 MB e 637 ms do `dist/pool-linux` do PyInstaller, e 70 ms rodando
por dentro do `python3`. Empata com o Lua no startup.

O crescimento do binário é o preço da portabilidade **estática**: sqlite
(~1,5 MB) e OpenSSL (~5 MB) entram embutidos, pra `import sqlite3`/`import
mail` rodarem em máquina que não tem essas libs. Cada nova lib de rede
(libcurl no `request`, etc.) soma o seu. É a troca de "link, não reescreva":
binário maior, zero dependência de runtime — o oposto do PyInstaller, que era
grande E lento.

O mesmo `poolscript_vm.c` gera os dois: `make` faz o binário, `setup_vm.py`
faz a extensão com `-DPS_MODULO_PYTHON`. Os dois chamam `ps_roda_fonte`, então
não existe "funciona num e não no outro" — e `tests/test_binario_c.py` checa
que `ldd` não lista libpython e que nenhum símbolo Python ficou por resolver.

---

## O que já é C

| Arquivo | Linhas | O que faz |
|---|---:|---|
| `ps_lexer.c` / `.h` | 668 | Lexer completo. Sem `Python.h`. |
| `ps_ast.c` / `.h` | 321 | AST + arena. Sem `Python.h`. |
| `ps_parser.c` / `.h` | 2085 | Parser recursivo-descendente. Sem `Python.h`. |
| `ps_compiler.c` / `.h` | 576 | Compilador AST→bytecode. Sem `Python.h`. **Não integrado ainda.** |
| `poolscript_vm.c` | ~9800 | VM completa: `Value`, GC, laço, builtins, métodos, todos os módulos nativos e o binding Python. |
| `ps_hash.c` / `.h` | ~600 | SHA-2, HMAC, PBKDF2, base64 — sem OpenSSL. |
| `ps_regex.c` / `.h` | ~620 | Motor de regex por backtracking. |
| `ps_mail.c` / `.h` | ~800 | SMTP, IMAP e MIME (parse + construção). TLS via OpenSSL. |
| `ps_http.c` / `.h` | ~440 | Cliente HTTP/HTTPS. Socket + OpenSSL, **sem libcurl**. |
| `ps_qr.c` / `.h` | ~560 | Encoder de QR (ISO 18004) + render PNG (libpng). **Sem libqrencode**. |
| `ps_qr_tables.h` | ~3620 | Tabelas do padrão QR, geradas da lib `qrcode`. |
| `ps_xlsx.c` / `.h` | ~470 | Leitura/escrita de .xlsx: ZIP sobre zlib + XML com expat. |
| `ps_db.c` / `.h` | ~520 | Fachada SQL: sqlite + postgres + mysql + ODBC. Resultado tipado. |
| `ps_pgerr.h` | ~260 | SQLSTATE → nome de classe psycopg2, gerado da lib. |
| `ps_mongo.c` / `.h` | ~180 | MongoDB via libmongoc, ponte JSON↔BSON. |
| `ps_lexer_bind.c` | 76 | Binding **temporário** — só teste diferencial. Descartável. |
| `ps_parser_bind.c` | 308 | Binding **temporário** — só teste diferencial. Descartável. |

O executor não faz **nenhuma** chamada à C-API dentro de `vm_executa` — os
usos restantes de `PyObject` estão só na borda (carregar bytecode, devolver
resultado), e somem quando a camada 5 existir.

---

## Camada 3 — Compilador — fechada

`ps_compiler.c` compila **todo nó da linguagem** que não é `async`/`await`
(fora de escopo por decisão). **72 opcodes**; bytecode conferido contra o
compilador Python por desmonte textual, e a semântica contra o interpretador
pela suíte diferencial. Inclui os operadores lógicos com curto-circuito
(`and`/`or`/`not` viram salto, não opcode binário), tipo declarado
(`COERCE_DECL`), tipo de retorno de `int`/`bool` action (`COERCE_RET` + `try`
implícito) e repropagação de `catch` não casado (`RERAISE`).

O único uso de `NotImplementedError` restante é o que deve ser: `await`,
decorador com caminho pontuado e nó que ainda não existe na linguagem.

---

## Camada 4 — Runtime

### Já nativo

`Value` (union etiquetada: `V_NULL`, `V_BOOL`, `V_INT` como `int64_t`,
`V_FLOAT` como `double`, `V_FUNC`, `V_NATIVE`, `V_OBJ`), `PSString` (com
hash), `PSList`, `PSDict` (hash aberto, sondagem linear, lápides, rehash),
GC mark-and-sweep com pilha cinza.

### Builtins: 38 de 38 — fechado

```
post   len    str    int    flo    bool   type   abs    round
hex    bin    oct    ord    chr    range  list   sum    min
max    sorted reversed enumerate  zip    id     input  load
open   sleep  gather
addEnd addStart removeEnd removeStart  map  filter
```

`Parsing`, `PoolFile` e `__name__` não são builtins: são globais pré-ligadas
(módulo oculto, referência de tipo e o nome do script).

`map`/`filter` foram os primeiros a **reentrar na VM**: `vm_executa_base()`
aceita uma base de frame/pilha/locais, então um builtin em C chama uma action
da PoolScript sem pisar no frame de quem o chamou. O resultado parcial precisa
ser fixado como raiz na pilha da VM (`fixa_raiz`) — o GC não varre a pilha do
C, e sem isso a lista some no meio da construção.

### String methods: 53 de 53 — fechado

O **A** delas era despacho de método em tipo builtin: `GET_MEMBER` só resolvia
em instância de Entity. Agora string (e número, por conversão automática)
devolve um `OBJ_METODO_NAT` — o método preso ao valor. É objeto, e não chamada
direta, porque `f = texto.upper` é código válido.

Tudo trabalha em **codepoint**, não em byte: a string é UTF-8 e `"ção"` tem 3
caracteres em 5 bytes. Caixa cobre ASCII + Latin-1 Suplementar + Latin
Estendido-A — a faixa do português.

```
upper lower title capitalize swapcase casefold
strip lstrip rstrip
startswith endswith contains has find rfind index rindex count len
isalpha isdigit isnumeric isdecimal isalnum isspace
isupper islower isascii istitle isprintable
split rsplit splitlines join replace
partition rpartition removeprefix removesuffix
ljust rjust center zfill expandtabs
```

Inclui `encode` (tipo `bytes` próprio), `format`/`format_map`,
`maketrans`/`translate`, `get_json`/`get` e chamada com argumento nomeado
(`s.replace("a", "b", count=2)`).

### Stdlib: 18 de 18 módulos reais (+ 5 stubs)

Nativos em C: **`json`** (parser e serializador próprios, JSON estrito),
**`date`** (via `time.h`), **`regex`** (motor próprio em `ps_regex.c`),
**`datasentity`**, **`hash`** (SHA-256/HMAC/PBKDF2/base64 em `ps_hash.c`, sem
OpenSSL), **`jwt`** (HS256), **`sys`**, **`dotenv`**, **`Parsing`** (builtin
global, com as conversões do `parsing_lib.py` à risca), **`sqlite3`**
(libsqlite3 estática no binário; transação implícita antes de DML e `using`
que comita, como o wrapper Python; linha como dict; erro `DatabaseError`),
**`mail`** (SMTP+IMAP+MIME próprios em `ps_mail.c`, TLS via OpenSSL estático;
`MailServer`/`MailMessage`/`MailReader`; MIME multipart com base64 em 76 e
assunto RFC 2047), **`request`**/**`requests`** (HTTP/HTTPS próprio em
`ps_http.c`, socket + OpenSSL, **sem libcurl**; `Response` com
text/content/json/save; redirect com porta preservada; TLS verifica o
certificado, como o urllib; `ws_connect` é um cliente WebSocket REAL —
handshake de cliente + frames mascarados sobre o transporte do jinker, com
`send`/`on_message`/`close`; sem thread, o callback é drenado nas operações
da conexão),
**`qrcode`**/**`qr`** (encoder próprio em `ps_qr.c`, ISO 18004, **sem
libqrencode**; Reed-Solomon, seleção de máscara e format/version info
validados grade a grade contra a lib; PNG via libpng estática; `gen`, `make`,
constantes L/M/Q/H, e o builder `qrcode.QRCode` com
add_data/make/make_image + `QRImage` com save/resize/to_file),
**`manpu`**/**`mp`** (read/write/remove/src/load de csv,
txt, json, xml, html e **xlsx**; o xlsx é `ps_xlsx.c` — ZIP à mão sobre zlib +
XML com expat, **sem openpyxl nem libzip**; célula tipada int/float/null;
`mp.open()`/ManpuFile com write/read/save e `using` que salva ao sair — o CSV
sai byte a byte igual ao csv.writer do Python), **`psodbc`**/**`db`** (fachada SQL única em `ps_db.c`
sobre sqlite + libpq — postgres; DbConnection/DbCursor com execute/fetch*,
resultado tipado, placeholder `?` no sqlite e `%s` no pg, e erro do postgres
com o nome de classe exato da psycopg2 via SQLSTATE — `catch (UndefinedTable e)`
funciona; mysql/mssql/mongo têm as libs prontas mas faltam servidores neste
ambiente pra validar), **`os`** (com o tipo
`PoolFile`: campos `name`/`ext`/`size`, métodos `move`/`copy`/`delete`/
`bytes`/`path`; `loadFile` decide texto/binário pela extensão e decodifica
`.json` e `.csv`; `cmd` passa pelo shell, `run` executa direto) e
**`jinker`** (servidor HTTP/1.1 + WebSocket próprio em `ps_jinker.c`, socket +
OpenSSL, **sem http.server nem websockets**: `Jinker`, `cors`, `jsonify`,
`render`, `request`, `JinkerResponse`; `@app.route(...)` com path-params `:id`
e `/<id>`, retorno como `jsonify`/dict/list/(corpo,status)/str; `@app.socket`
com salas, `channel`/`emit`/`exclude_self`; upload multipart com validação de
extensão; PoolIp — rate limit e ban por IP; TLS com cert self-signed
autogerado; CORS com allowlist de origens. Single-thread: um event loop com
`poll` atende HTTP e todas as conexões WebSocket, e o handler `.ps` reentra na
VM via `chama_valor` — sem thread, sem GC concorrente).

As 5 stubs (`sqlite`, `smtplib`, `mimetext`, `multipart`, `flask`) importam e
falham com `NotImplemented` capturável na chamada, como no interpretador.

Formato **intercambiável** com o lado Python é requisito, não detalhe: hash e
token gerados num motor têm que validar no outro, senão trocar de runtime
derruba login. Verificado nos dois sentidos, com teste permanente.

`import`, `from x import a as b` e `PUSH x [as y] [GET a, b]` compilam para
`IMPORT_MOD` (+ `GET_MEMBER` por nome pedido). Módulo nativo, `.ps` vizinho e
lib instalada em `~/.poolscript/libs` resolvem em runtime, nessa ordem de
prioridade — nativo ganha de arquivo, arquivo local ganha de lib instalada.

As maiores, todas já portadas, escrevem o protocolo à mão em vez de puxar a
lib pesada como dependência de runtime:

| Módulo | Linhas | Reescrito à mão sobre |
|---|---:|---|
| `jinker_lib` | 1465 | socket + OpenSSL (HTTP/1.1 + WebSocket), sem http.server |
| `manpu_lib` | 412 | zlib + expat (xlsx é ZIP + XML), sem libzip/openpyxl |
| `psodbc_lib` | 380 | libpq, libmysqlclient, unixODBC, mongo-c-driver |
| `qrcode_lib` | 240 | encoder QR próprio + libpng, sem libqrencode |

Nenhuma depende de Python, e os headers já
estão instalados nesta máquina. Quando as deps estáticas de uma lib faltam
(como aconteceu com a libcurl, que precisaria de nghttp2/idn2/zstd/brotli em
`.a`), o protocolo é escrito à mão sobre socket + OpenSSL em vez de virar
dependência dinâmica — foi assim que o `request` saiu (`ps_http.c`).

---

## O que falta

Camadas 1, 2, 3 e 5 fechadas. O que resta é **biblioteca**, não linguagem.

**Nada está bloqueado.** As bibliotecas C necessárias já estão instaladas
(`libssl-dev libcurl4-openssl-dev libsqlite3-dev libpq-dev libzip-dev
libexpat1-dev libqrencode-dev libpng-dev`) — o que separa as libs abaixo é
**tamanho de trabalho**, não impedimento.

### B. Libs que linkam biblioteca C — **concluído**

Decisão de 2026-07-29: **linkar quando há `.a` autossuficiente, escrever o
protocolo à mão quando não há** — nunca dependência dinâmica de rede, nunca
lib Python. As 18 libs reais estão portadas, incluindo o `jinker` (último):
servidor HTTP/1.1 + WebSocket próprio em `ps_jinker.c` sobre socket + OpenSSL,
validado por diferencial contra o `jinker_lib.py` (rotas, path-params, CORS,
upload, PoolIp, TLS e salas de WebSocket batem entre binário e interpretador).

**RS/PS/ES/EdDSA no `jwt`** exigem mais que o OpenSSL do lado C: o
interpretador (a autoridade do diferencial) também precisa suportá-los, e ele
usa só `hashlib`/`hmac` — RSA/curva elíptica seria uma dependência Python nova
(`cryptography`). Enquanto os dois não expandem juntos, os dois recusam por
nome, que é o estado consistente. É decisão de escopo, não trabalho pendente.

### C. Reorganização de arquivos

`.py` junto de `.py`, `.c`/`.h` junto dos irmãos, interpretador antigo em
árvore própria. Combinado de fazer "bem depois" — é aqui, com a migração
parada de mover código todo dia.

### Fora de escopo

`async`/`await` continua só no interpretador. Não está na fila.

---

## Performance: onde estão os ganhos

Medido, não estimado (números em [`MEMORIA.md`](MEMORIA.md)). Em ordem de
impacto por esforço:

1. **`acha_metodo_str` faz strcmp linear em 44 entradas** a cada `.upper()`.
   Hash perfeito ou cache inline no call site. É o mais fácil e o mais visível.
2. **`LOAD_NAME`/`STORE_NAME` resolvem local-vs-global em runtime.** Uma
   análise de escopo no compilador resolveria a maioria em compilação.
3. **String aloca a cada operação.** Interning de string curta.
4. **VM de pilha com instrução de 2 slots.** Virar registrador é reescrever o
   compilador inteiro — só vale se 1–3 não bastarem.

---

## Números medidos

Benchmarks reais desta máquina, não estimativa.

| Teste | tree-walker | VM em C | CPython |
|---|---:|---:|---|
| `fib(22)` | 591 ms | **1,5 ms** (405x) | 1,8 ms → **1,3x mais rápida** |
| loop 300k | 1195 ms | **8,1 ms** (148x) | 30,5 ms → **3,8x mais rápida** |
| concat 100k | 431 ms | **6,3 ms** (68x) | 6,0 ms → 1,1x mais lenta |

Lexer em C: **11x** mais rápido que o lexer Python.

### Por que mypyc não bastou

Medido, não suposto: mypyc sobre a VM em Python deu **~1,2x** no fib e deixou
o loop **mais lento**. Mesmo com tipagem perfeita, o teto foi **1,5x** —
porque o `int` do mypyc continua sendo objeto do CPython. O ganho de 405x veio
do **modelo de valores próprio** (`int64_t` de verdade), não de "compilar
para C".

---

## Disciplina de trabalho (o que funcionou)

### 1. Teste diferencial, sempre

Nenhuma expectativa escrita à mão. As duas implementações rodam o mesmo
fonte e o resultado tem que bater:

- lexer → token a token (tipo, valor, linha, coluna)
- parser → AST serializada em S-expression
- compilador → bytecode desmontado em texto (opcode, argumento, pool)
- VM → saída comparada contra o interpretador

Isso pegou bugs que inspeção **não** pegaria:

| Bug | Como apareceu |
|---|---|
| Coluna contando byte em vez de caractere | 3 arquivos com acento divergiram |
| Use-after-free na AST | serialização saiu como lixo binário |
| `async int action` perdia o `async` | silencioso, sem erro |
| `list nums = [...]` virava VarDecl indevidamente | estrutura diferente |
| Decorador dentro de `Entity` engolia a action | um nó a menos |
| Action aninhada virava global | bytecode divergiu do interpretador |
| Pool de constantes com slot morto | índices deslocados |

### 2. ASan em todo lote de C

`-fsanitize=address,undefined`, incluindo **caminhos de erro** (é onde mais
se corrompe memória). Prova de ausência de vazamento por **invariância de
carga**: 1 execução e 20 execuções pesadas vazam os mesmos 3824 bytes — que
são do CPython, não nossos.

### 3. `./rebuild_vm.sh`, nunca `setup_vm.py` direto

O setuptools compara timestamps com granularidade de **segundo**. Editar um
`.c` e recompilar no mesmo segundo deixa o `.so` velho no lugar, e o teste
passa a medir código que não existe mais. Isso já custou tempo duas vezes.

### 4. Nó não suportado para com erro explícito

Nunca gerar AST ou bytecode errado em silêncio.

---

## Achados no lado Python (decisões em aberto)

Bugs do interpretador/parser originais, expostos pela migração. **Não
corrigidos** — são decisões de design:

1. **`post([Null])` imprime `[None]`** — o `None` do Python vaza pelo `repr`
   da list. No topo, `post(Null)` imprime `null`. A VM em C é consistente
   (`[null]`). Registrado em teste.
2. **`lista[0] = x` não funciona** — nem no interpretador. Limitação do
   parser. O `INDEX_SET` da VM já existe, esperando o parser.
3. **`docs/PoolScript.md` documenta `nomes[0] = "joao"`** com saída
   esperada — algo que a linguagem nunca fez.

---

## Próximo passo concreto

1. `jinker` (servidor HTTP+WS; o loop de eventos interno em C é permitido — o
   veto era só às keywords `async`/`await` do `.ps`).
   Decisão de link firmada:
   **estático quando o `.a` é autossuficiente** (sqlite, OpenSSL); libs cujas
   deps estáticas faltam a gente escreve o protocolo à mão sobre socket
   (foi o caso do HTTP — libcurl exigiria nghttp2/idn2/zstd/brotli em `.a`,
   que não existem aqui — e do QR, já que libqrencode só tem `.so`). Nunca
   dependência Python.
2. Reorganização de arquivos (C) — por último, combinado.
