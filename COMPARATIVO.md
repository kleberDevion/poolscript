# PoolScript × outras linguagens

Comparativo medido em 2026-07-30, nesta máquina (Linux x86-64, 2 núcleos), com
o binário `pool` desta árvore (VM em C, camada 4 completa: 18/18 libs).
Números são o **melhor de 5 execuções** (melhor de 3 nas longas; o
interpretador Python da PS rodou 1 vez nas pesadas — os tempos deixam claro
por quê). Tempo é do processo inteiro: **inclui o startup**, porque é assim
que um script de verdade roda.

Versões: PoolScript `pool` (VM C) e interpretador de referência (Python),
Python 3.12.3, Lua 5.4.6, Node 24.18.0 (V8 com JIT).

## Números

| benchmark | pool (VM C) | PS interpretador | Python 3.12 | Lua 5.4 | Node 24 |
|---|---:|---:|---:|---:|---:|
| hello (startup) | **8 ms** | 95 ms | 12 ms | 1 ms | 30 ms |
| fib(30) recursivo | 169 ms | 53 346 ms | 182 ms | 90 ms | 66 ms |
| loop aritmético 10M | 663 ms | 166 016 ms | 2 176 ms | 77 ms | 57 ms |
| dict: 200k inserções str→int | **133 ms** | 7 318 ms | 153 ms | 228 ms | 312 ms |

O que os números dizem, sem maquiagem:

- **A VM em C fica na classe do CPython** — empata em chamada de função
  (fib: 169 vs 182 ms) e ganha por 3,3× em laço aritmético puro (663 ms vs
  2,2 s). Para uma VM de bytecode sem JIT, é onde dá pra chegar; é o mesmo
  patamar em que o CPython vive há 30 anos.
- **No dict a PS ganha de todo mundo** (133 ms contra 153 do Python, 228 do
  Lua e 312 do Node): o dict compacto em ordem de inserção, copiado do
  desenho do CPython mas sem a caixa de objetos dele, paga menos por
  operação.
- **Lua e Node ganham em aritmética** — Lua porque é o interpretador de
  registradores mais enxuto que existe; Node porque o V8 **compila JIT** o
  laço pra código de máquina. Não é a mesma categoria de máquina: um JIT
  contra uma VM de bytecode sempre vence CPU puro. A PS não persegue esse
  alvo (um JIT custa mais complexidade do que o escopo da linguagem pede).
- **Startup de 8 ms** — mais rápido que Python (12) e Node (30). Só o Lua
  (1 ms, ~300 KB sem stdlib) abre mais rápido. Pra CLI e script curto, o
  custo de subir é o que domina, e aí a PS está do lado certo da tabela.
- **O interpretador de referência (Python) é 100–300× mais lento que a VM**
  — e é por isso que ele é a *autoridade semântica* do diferencial, não o
  runtime de produção. O ganho de migrar pra VM C não é otimização, é
  mudança de categoria.

## O que vem dentro do binário

A comparação de desempenho conta metade da história. A outra metade é o que
cada runtime **traz consigo** num deploy:

| capacidade | pool (16 MB, arquivo único) | Python | Lua | Node |
|---|---|---|---|---|
| JSON | nativo | nativo | **não tem** | nativo |
| HTTP cliente (TLS verificado) | nativo | nativo | não tem | nativo |
| Servidor HTTP + WebSocket + salas | nativo (`jinker`) | stdlib crua (http.server; WS só com pip) | não tem | cru (http; WS só com npm) |
| SQL: SQLite | nativo (estático) | nativo | não tem | não tem |
| SQL: Postgres / MySQL / ODBC | nativo (`psodbc`) | pip (psycopg2 etc.) | luarocks | npm |
| MongoDB | nativo | pip | não tem | npm |
| SMTP + IMAP + MIME | nativo (`mail`) | stdlib crua | não tem | npm |
| QR Code (PNG) | nativo (`qrcode`) | pip | não tem | npm |
| xlsx ler/escrever | nativo (`manpu`) | pip (openpyxl) | não tem | npm |
| Hash/HMAC/PBKDF2/JWT | nativo | stdlib + pip | não tem | nativo/npm |
| Regex | nativo (motor próprio) | nativo | padrões próprios | nativo |
| Rate-limit por IP, upload multipart, TLS c/ cert autogerado | nativo (`jinker`) | monta à mão | — | monta à mão |

O deploy da PS é **copiar um arquivo**. Sem `pip install`, sem
`node_modules`, sem versão de runtime na máquina de destino — a promessa
"portável como Lua" cumprida, mas com as baterias que o Lua nunca teve.
Uma API com banco, upload, WebSocket e QR code é um `.ps` + o `pool`.

Referências de tamanho: `pool` 16 MB (sqlite, OpenSSL, libpq, mysqlclient,
odbc, libpng, expat e zlib **estáticos** dentro dele); Lua ~300 KB (sem
nada); Python ≥ 100 MB instalado (+venv por projeto); Node ≥ 110 MB
(+`node_modules` por projeto).

## Modelo de execução

| | PoolScript (pool) | Python | Lua | Node |
|---|---|---|---|---|
| Máquina | VM bytecode em C (72 opcodes, pilha) | VM bytecode (pilha) | VM registradores | JIT (V8) |
| GC | mark-and-sweep próprio | refcount + ciclos | mark-and-sweep incremental | geracional |
| Concorrência | single-thread; servidor via event loop `poll` | threads c/ GIL, asyncio | coroutines | event loop + workers |
| Tipagem | dinâmica, coerção declarada opcional (`int x = ...`) | dinâmica + hints | dinâmica | dinâmica (TS à parte) |
| Erros | `try/catch` tipado por NOME (`catch (UndefinedTable e)`) | exceções por classe | `pcall` | exceções |

Ponto que não aparece em benchmark: o `catch` tipado da PS casa o **nome**
do erro inclusive nos vindos do banco (`UndefinedTable`, `UndefinedColumn`
via SQLSTATE) — o script trata erro de Postgres sem importar nada.

## Metodologia

Scripts equivalentes nos quatro runtimes (mesma lógica, idiomática em cada
um), `melhor-de-N` via relógio de parede, saída conferida idêntica. Os
fontes ficam em `tests/`/scratchpad da sessão; qualquer número aqui se
reproduz com o `pool` desta árvore. Máquina modesta de propósito: 2 núcleos
é o pior caso que o usuário final tem, não o melhor.
