# Auditoria de engenharia — 2026-08-27

Auditoria externa do motor, da suíte e do ferramental, medindo o projeto contra
a prática de quem mantém linguagem/DSL em produção (SQLite, CPython, Go, Rust,
V8/Chromium, Lua, OSS-Fuzz). Ela **não** repete as auditorias anteriores: parte
do que elas fecharam e procura o que nenhuma ferramenta do repositório olha.

**Árvore auditada:** `858f51e` (v8.3.78). A árvore ANDOU durante a auditoria —
a suíte e a cobertura foram medidas em `2c3b831` (v8.3.78/8.3.72), e os três
commits seguintes (`750ee3a`, `61c37b1`, `858f51e`) tocaram só `lsp/`,
`editor/` e `ps_versao.h`. Nenhum fonte `.c` do motor mudou entre os dois
pontos; os defeitos de motor abaixo foram **re-verificados em `858f51e`**.

**Ambiente:** Linux 7.0.0-30-generic, x86-64, gcc, 7,7 GB de RAM (máquina de
desenvolvimento; sem Postgres/MySQL/mongod/SMTP de pé).

Classificação de evidência usada em cada item (skill `poolscript-evidence-gate`):
**[V]** verificado nesta execução · **[T]** testado no repositório · **[L]**
lido no código, não executado · **[NV]** não verificado.

---

## 1. Evidência executada

| Comando | Resultado |
|---|---|
| `./testar` (suíte inteira, em `2c3b831`) | **7848/7848 passaram, 0 falharam** [V] |
| `./pool lsp/teste_lsp.ps` | 18/18 [V] |
| `./pool scripts/audita_exemplos_doc.ps` (auditor NOVO, criado aqui) | 275 blocos, **1 recusado pelo motor**, rc=1 [V] |
| `make avisos` | 2 avisos [V] — e o alvo está **inerte**, ver §4.8 |
| gcc com as flags do build, `-fsyntax-only`, 17 fontes | **0 avisos** [V] |
| gcc real (`-c -o /dev/null`) + `-Wformat-truncation=2 -Wshadow` | **31 truncamentos + 1 shadow** que o alvo `avisos` não mostra [V] |
| `lcov --summary cob/vm.info` (medição de 27/08 15:00) | linha 55,0% · função 54,9% · **ramo 41,4%** [V] |
| Repros de crash (§3.1–§3.3) | 3 SIGSEGV reproduzidos em `858f51e` [V] |
| Carga de GC, laço de 1M, dict de 200k | medidos, §4.6 [V] |
| `make check-e2e`, `make oom`, `make check-asan`, `make fuzz`, `make analisa`, `make cobertura` | **não rodados nesta execução** [NV] — máquina sensível; a cobertura usada é o artefato de 15:00 do mesmo dia |

---

## 2. Cobertura de ramos (o número que interessa)

Medição de `cob/vm.info` (suíte inteira contra binário instrumentado, sem e2e).
Percentual agregado **não decide nada** — por arquivo:

| Arquivo | Linha | **Ramo** | Funções nunca executadas |
|---|---|---|---|
| `poolscript_vm.c` | 53,3% (7010/13157) | **39,8%** (6236/15687) | **420 de 949 (44%)** |
| `ps_parser.c` | 93,6% | 67,1% | 1 de 59 |
| `ps_compiler.c` | 91,5% | 70,5% | 1 de 47 |
| `ps_lexer.c` | 75,6% | 67,5% | 6 de 32 |
| `ps_qr.c` | 86,0% | 69,0% | 3 de 27 |
| `ps_xlsx.c` | 81,1% | 53,2% | 3 de 26 |
| `ps_regex.c` | 62,2% | 45,0% | 13 de 40 |
| `ps_hash.c` | 41,5% | 37,7% | 13 de 22 |
| `ps_ast.c` | 44,0% | 21,2% | 1 de 7 |
| `main.c` | 22,7% | 16,6% | 12 de 17 |
| `ps_http.c` | 13,3% | **6,4%** | 11 de 16 |
| `ps_mail.c` | 2,8% | **0,6%** | 35 de 38 |
| `ps_db.c` | 0% | **0%** | 20 de 20 |
| `ps_jinker.c` | 0% | **0%** | 39 de 39 |
| `ps_pkg.c` | 0% | **0%** | 26 de 26 |
| `ps_mongo.c` | 0% | **0%** | 8 de 8 |
| `ps_guzer.c` | 0% | **0%** | 12 de 12 |
| **total vm/** | 55,0% | **41,4%** | **624 de 1385 (45%)** |

### 2.1. Os ramos descobertos são exatamente os prioritários

A ordem de prioridade da skill é: (1) alocação/cleanup/**finalizadores**;
(2) **raízes do GC e troca de contexto**; (3) exceção entre frames; (4) parse de
entrada não confiável; (5) auth/limite de protocolo; (6) conversão numérica.

Das 420 funções nunca executadas de `poolscript_vm.c`:

- **17 finalizadores** nunca rodaram: `fin_dbconn fin_dbcur fin_jchan fin_jchst
  fin_jemit fin_jproxy fin_jreq fin_jsockns fin_jupload fin_mongocol
  fin_mongoconn fin_qrbuild fin_qrfile fin_response fin_socket fin_sqlcur
  fin_wsconn`. A skill já dizia: *finalizador existente não é finalizador
  testado* — agora tem número.
- **8 tracers de GC** nunca rodaram: `gct_enum gct_futuro gct_gerador
  gct_jinker gct_jreg gct_jreq gct_jresp gct_response`. Dois deles
  (`gct_gerador`, `gct_futuro`) **não dependem de serviço externo nenhum**:
  são gerador e future, recurso puro da linguagem.
- 152 `mod_*` + 102 `met_*` — nativas de biblioteca nunca chamadas.

Escrevi o caso que cobre `gct_gerador` (gerador suspenso segurando valores
enquanto o GC roda 300 mil vezes) e ele **passa** [V] — ou seja, é teste
faltando, não defeito. Custa 20 linhas em `casos_linguagem.c`:

```ps
action fonte() {
    marca = ["sobrevivi", [1, 2, 3]]
    i = 0
    while (i < 5) { yield marca
        i = i + 1 }
}
g = fonte()
for each v in g { primeiro = v
    break }
j = 0
while (j < 300000) { lixo = [j, j + 1]
    j = j + 1 }
for each v in g { post(v) break }   // ['sobrevivi', [1, 2, 3]]
```

### 2.2. O que a medição de cobertura não faz (e devia)

- **Não é portão.** `make cobertura` roda só no cron da CI e sobe HTML como
  artefato. Sem baseline versionado, sem meta por arquivo, sem cobertura de
  DIFF, sem MC/DC em decisão composta. Queda de 10 pontos num arquivo passa
  despercebida — é o que o Codecov/patch-coverage resolve em qualquer projeto
  desse porte, e o que o SQLite trata como requisito (100% de ramo + MC/DC).
- **Não inclui e2e.** Os 0% de `ps_db/ps_jinker/ps_pkg/ps_mongo/ps_guzer` não
  significam "código morto": significam "só o e2e toca, e o e2e não entra na
  conta nem na CI" (§4.3).
- O `cob/` medido tem `-O0` correto (o `CFLAGS_BASE` já separa otimização),
  ponto que a auditoria anterior consertou. Confirmado lendo o alvo [L].

---

## 3. Defeitos verificados (com reprodução)

### 3.1. Parser sem teto de profundidade → SIGSEGV no caminho do editor **[V]**

```bash
python3 -c "print('post(' + '('*30000 + '1' + ')'*30000 + ')')" > /tmp/d.ps
./pool --check /tmp/d.ps      # rc=139, "Falha de segmentação (core dumped)"
./pool --check < /tmp/d.ps    # rc=139 — o MESMO caminho que o LSP usa
```

Entre 10.000 (ok) e 20.000 (morre) níveis de aninhamento. A descida recursiva
não tem limite de profundidade; a pilha do C acaba.

Por que importa mais do que parece: (a) é o caminho de **entrada não confiável**
por definição — arquivo aberto no editor, `psl install` de pacote, `--check` de
CI; (b) o **fuzzer roda exatamente esse harness** com `-max_len=65536` e nunca
achou, o que mede a profundidade real do fuzz de hoje (§4.2); (c) dentro do
jinker, a pilha da fibra é de 128 KB (§3.2), então o limiar cai ~64x.

Correção da casa: contador de profundidade no parser com erro de sintaxe ao
estourar (CPython: `MAXSTACK`/`RecursionError`; Go: `maxNestLev`; Clang:
`MaxDepth` no parser de expressão).

### 3.2. `str()`/`post()` sem teto → SIGSEGV; dentro do jinker, o servidor inteiro cai **[V]**

CLI (pilha de 8 MB): estrutura acíclica com 100 mil níveis mata o processo.
Dentro de uma rota do jinker, **300 níveis bastam**:

```ps
# servidor
@app.route("/fundo/<prof>")
action fundo() {
    prof = int(request.path_param("prof"))
    x = []
    i = 0
    while (i < prof) { x = [x]
        i = i + 1 }
    return {"tam": str(x).len()}
}
```

```
/fundo/100 -> 200 {"tam": 202}
/fundo/300 -> Falha de segmentação (imagem do núcleo gravada)  → servidor MORTO
              cliente seguinte: "Connection refused"
```

Causa: `escreve_valor`/`valor_para_texto` recorrem sem teto, e a fibra do jinker
roda numa pilha de C de `malloc(128 KB)` (`FIB_CSTACK`), **sem página de
guarda** — `ps_ctx_make` monta o contexto sobre memória de heap comum. Fora do
alcance da pilha, o estouro não vira SIGSEGV limpo: vira escrita em heap alheio.

Comparação: Boost.Context, Go e libmill alocam pilha de corrotina com `mmap` +
`PROT_NONE` justamente para transformar estouro em fault determinístico.

O JSON já tem teto de 64 nos dois sentidos (`json aninhado demais`) [V] — logo,
payload remoto não chega lá. O buraco é o caminho `str`/`post`, e ele é
alcançável com profundidade vinda de parâmetro do cliente, como no repro.

### 3.3. Detecção de ciclo cega acima de 256 → SIGSEGV com estrutura legal **[V]**

`PS_CICLO_MAX 256`: o vetor `ps_pilha_texto` só REGISTRA os 256 primeiros
níveis (`if (ps_prof_texto < PS_CICLO_MAX)`), mas o contador continua subindo.
Um ciclo que fecha abaixo de 256 é detectado; um que fecha **acima** não existe
para o detector:

```ps
raiz = []
x = raiz
alvo = null
i = 0
while (i < 400) { novo = []
    addEnd(x, novo)
    x = novo
    if (i == 300) { alvo = novo }
    i = i + 1 }
addEnd(x, alvo)      # ciclo fecha na profundidade 300
post(str(raiz))      # rc=139
```

O comentário do código diz, textualmente, que escolheu detectar ciclo *"em vez
de um teto de profundidade"*. Sem o teto, a defesa tem um alçapão. O `repr` do
CPython não tem esse limite (conjunto por thread, sem teto fixo).

### 3.4. SMTP/IMAP com TLS sem verificação, entregando senha **[L]**

`vm/ps_mail.c:142` — `SSL_CTX_set_verify(c->ctx, SSL_VERIFY_NONE, NULL)`, e o
comentário justifica: *"é o contexto stdlib do Python"*. **É o contrário**:
`smtplib.SMTP_SSL`/`starttls()` usam `ssl.create_default_context()` desde o
Python 3.6 (PEP 476 / bpo-25008), que verifica cadeia E hostname.

No mesmo arquivo, `ps_smtp_login` manda `AUTH PLAIN`/`AUTH LOGIN` (usuário e
senha em base64) e `ps_imap_login` manda `LOGIN user senha`. Sem verificação de
certificado, qualquer MITM no caminho recebe as credenciais em claro depois de
um handshake que "funciona".

O cliente HTTP do mesmo projeto faz o certo (`ps_http.c:110-116`:
`SSL_CTX_set_verify` + **`SSL_set1_host`**), então o padrão correto já está no
repositório — falta aplicá-lo ao mail, com escape explícito (`verificar=false`)
para o caso do certificado self-signed do teste.

### 3.5. Chave de máscara de WebSocket com `rand()` sem semente **[L]**

`vm/ps_jinker.c:690` (e 634): `masc[i] = (unsigned char)(rand() & 0xff)`. Não
há `srand()` em lugar nenhum do projeto [V, grep], então a sequência é
**idêntica a cada processo**. A RFC 6455 §5.3 exige que a chave de máscara venha
de fonte forte e imprevisível — é a defesa contra envenenamento de cache em
intermediário. `ps_random_bytes()` (que lê `/dev/urandom`) já existe em
`ps_hash.c:451`, ao lado.

### 3.6. Hash de dict sem semente + endereçamento aberto = HashDoS **[L]**

`hash_str` é FNV-1a com as constantes públicas (`2166136261` / `16777619`),
sem semente por processo, e `acha_slot` faz sondagem linear. Chaves em colisão
de 32 bits são pré-computáveis offline (colisão de bloco + concatenação, já que
FNV-1a é dependente do sufixo), e N chaves colidentes custam O(N²).

O runtime serve HTTP (jinker) e transforma header, query, form e JSON do cliente
em dict. É o ataque de 2011 (n.runs SA-2011.004) que rendeu CVE-2011-4885 (PHP),
CVE-2012-1150 (Python) e correções em Ruby, Java e V8 — todos passaram a
randomizar a semente por processo. Correção: semear com `ps_random_bytes` no
boot e misturar a semente no estado inicial do FNV (ou trocar por SipHash-1-3).

### 3.7. LSP escreve o buffer do editor em `/tmp/ps_lsp` (0755) **[V]**

`lsp/servidor.ps:24` — `TMP = "/tmp/ps_lsp"`, caminho FIXO e previsível;
`os.mkdir(TMP, exist_ok=true)` aceita a pasta já existente, de qualquer dono.
Medido depois de `./pool lsp/teste_lsp.ps`:

```
drwxr-xr-x 2 ... /tmp/ps_lsp
-rw-rw-r-- 1 ... check.ps      <- o arquivo que está aberto no editor
-rw-rw-r-- 1 ... meta.json  c.out  tok.json
```

Em máquina compartilhada: (a) todo mundo lê o fonte que você está editando;
(b) outro usuário cria `/tmp/ps_lsp` antes (ou `check.ps` como symlink) e passa
a receber/entregar conteúdo — CWE-377/CWE-59. `--tokens` já lê de stdin (`<`),
então metade do problema some trocando o arquivo por pipe; o resto pede
`mkdtemp` com 0700.

### 3.8. `psl install` sem versão, sem lockfile, com sha256 opcional **[L]**

`vm/ps_pkg.c`: o registry é `{"nome": "url"}` ou `{"nome": {"url":..,
"sha256":..}}`; `baixa_fonte` só confere o hash `if (sha_esperado &&
sha_esperado[0])`. Não há conceito de **versão** no arquivo inteiro (grep por
`versao|version|lock` não casa nada), logo `psl install nome` hoje e amanhã
podem instalar código diferente, sem registro do que entrou. HTTPS é exigido
(bom), mas o índice manda em tudo: quem controla o registry controla o código
executado. E o arquivo tem **0% de cobertura** — 26 de 26 funções nunca
executadas.

Prática de referência: npm/cargo/pip têm lockfile + integridade obrigatória;
PyPI (PEP 458/TUF) e Sigstore assinam o índice, não só o pacote.

---

## 4. Ferramental que falta

O portão de hoje (`make check` = suíte + metadata + doc + padrões em C + LSP +
`-fanalyzer`) é sólido, e a CI de 27/08 finalmente tornou ASan/OOM/fuzz/
cobertura obrigatórios no cron. O que segue é o degrau seguinte.

### 4.1. Não existe build de asserção — o buraco de maior alavancagem

Em 41 mil linhas: **8 `_Static_assert`** (7 no guzer, 1 no X11) e **6
`assert()`**, todos em `ps_guzer.c`. O motor — pilha, frames, GC, fibras — não
tem uma invariante checada.

Consequência direta: ASan e fuzzer só acham **morte**, nunca **estado errado**.
Um `sp` desequilibrado, um operando de tipo impossível chegando num opcode, uma
raiz esquecida — nada disso vira falha enquanto não virar segfault, e com
39,8% de ramo cobertos há muito caminho onde não vira.

Referência: CPython compila `--with-pydebug` com `assert` por toda parte e roda
a suíte nos dois modos; V8 tem `DCHECK`; SQLite roda a suíte com
`SQLITE_DEBUG` (assert-heavy) e sem. É a diferença entre "não quebrou" e
"as invariantes valeram".

Proposta concreta: `PS_ASSERT(x)` compilado com `-DPS_DEBUG`, alvo
`make check-debug` rodando a MESMA suíte, e a CI noturna rodando os dois.

### 4.2. Um alvo de fuzz para oito superfícies de entrada hostil

`teste/ps_fuzz.c` cobre `ps_verifica_fonte` (lexer+parser+compilador). Não há
alvo para: parser HTTP do jinker (rede), MIME/IMAP (`ps_mail.c`), regex
(compilador e executor), base64/JWT, XLSX/ZIP, QR/PNG, JSON, índice do registry.
Todos consomem byte de terceiro; nenhum tem harness.

Pior, o corpus **não persiste**: `teste/fuzz_corpus/` e `teste/fuzz_achados/`
estão no `.gitignore`, a CI não usa cache, e `make fuzz` depende de `semeia`.
Cada noite recomeça do zero, e 30 minutos de mutação guiada por cobertura são
jogados fora. O modelo do Go (`testdata/fuzz/`, reproducer versionado) e do
OSS-Fuzz (corpus persistido + replay de crash como regressão) existem
justamente para isso — e o §3.1 é a prova empírica de que o fuzz de hoje não
chega fundo: o estouro de pilha do parser cabe em 60 KB de entrada.

Falta também **replay**: um `crash-*` achado às 3h vira artefato do GitHub, não
caso da suíte. Ninguém reprova amanhã se ele voltar.

### 4.3. E2E fora da CI, inclusive o que não precisa de serviço

`teste/e2e/LEIAME.md` lista cinco scripts que **não precisam de nada**:
`arquivo.ps`, `sqlite.ps`, `socket.ps`, `guzer.ps` (headless), `jinker_srv+cli`
(loopback). Nenhum roda na CI. É o caminho mais barato para tirar
`ps_jinker.c`/`ps_db.c(sqlite)`/`ps_guzer.c` do 0%.

### 4.4. Nenhuma análise estática além do `-fanalyzer`

Sem clang-tidy, sem cppcheck, sem CodeQL, sem Semgrep, sem Coverity/Infer. O
`scripts/audita_c.ps` (4 regras próprias, boas) e o `-fanalyzer` — que localmente
**exclui `poolscript_vm.c`**, metade do motor — são tudo. CodeQL roda de graça
em repositório público e pega classe inteira (uso de não-inicializado, TOCTOU,
formato variável) que o `-fanalyzer` não persegue.

### 4.5. Sanitizer sem TSan e cego para fibra

- `make check-asan` roda com `ASAN_OPTIONS=detect_leaks=0` [L]: **vazamento
  não reprova** na suíte (só no fuzz, que usa `detect_leaks=1`).
- Sem TSan, e existe `pthread_create` (`poolscript_vm.c:16737`) com estado
  global mutável ao lado (`vm_corrente`, `ps_pilha_texto`, `ps_prof_texto`,
  documentados como "single-threaded").
- Sem `__sanitizer_start_switch_fiber`/`finish_switch_fiber` em volta do
  `ps_fctx_swap` (0 ocorrências [V]): o ASan **não sabe** que a pilha trocou,
  então o portão ASan não valida de verdade o caminho async/jinker — que é
  exatamente onde mora o §3.2.
- Sem MSan (o `memcheck` do valgrind existe, mas é manual e fora da CI).

### 4.6. Zero benchmark — e o GC não é geracional

Não há alvo, arquivo ou número de desempenho no repositório. Medido aqui [V]:

| Carga | PoolScript | Referência |
|---|---|---|
| laço `while` de 1M com soma | **0,70 s** | CPython 3: 2,14 s (3× mais lento) |
| 200 mil inserções em dict | 0,08 s, 43 MB | escala linear, ok |
| 200 mil alocações curtas, heap vivo = 0 | 1,09 s | — |
| idem, heap vivo = 200 mil objetos | 3,90 s | **3,5×** |
| idem, heap vivo = 800 mil objetos | 5,94 s | **5,3×** |

O motor é rápido; o custo aparece com heap vivo grande, que é o comportamento
esperado de mark-sweep stop-the-world não geracional (`proximo_gc = alocado*2`).
Para um servidor HTTP de longa duração isso é latência por pausa proporcional ao
heap. Lua ficou incremental/geracional por esse motivo; CPython tem geracional.
Antes de mexer no coletor, o que falta mesmo é **medição versionada**: sem
benchmark no repositório, qualquer regressão de desempenho entra sem ninguém
ver (CPython tem pyperformance; V8 e LuaJIT têm bot de perf).

### 4.7. CI de uma máquina só

`ubuntu-24.04`, gcc, x86-64. Sem clang no portão (só no fuzz), sem macOS/BSD,
sem ARM64, sem 32 bits, sem big-endian. E `vm/ps_gmp_min.h` declara **à mão** o
layout da `__mpz_struct` com o comentário "Layout da ABI (64-bit)", sem um
`_Static_assert` sequer — em 32 bits, ou com limb diferente, isso é corrupção
silenciosa. (O `ps_x11_min.h` faz o certo: `_Static_assert` de layout.)
Mínimo defensável: `_Static_assert(sizeof(mp_limb_t) == sizeof(void*))` e
conferir `__gmp_bits_per_limb` no boot.

### 4.8. `-Werror` ausente e o alvo `avisos` está inerte

O build de hoje é limpo com as flags do projeto [V]. Mas nada impede que um
aviso entre: a CI não usa `-Werror`, e aviso que não reprova é aviso que
acumula.

Pior, o alvo que deveria mostrar os avisos "barulhentos" usa `-fsyntax-only` —
e `-Wformat-truncation` só existe depois do GIMPLE. Medido [V]:

```
make avisos                                   -> 2 avisos
mesmos arquivos compilados de verdade         -> 31 truncamentos + 1 shadow
ps_mail.c com -fsyntax-only                   -> 0
ps_mail.c compilado de verdade                -> 1
```

É a MESMA classe de defeito que o Makefile já documenta e corrigiu no alvo
`analisa` ("`-fsyntax-only` NÃO serve aqui: o gcc para antes do GIMPLE"). Ficou
por corrigir no vizinho. E o comentário do Makefile que fala em "~20
truncamentos deliberados" descreve um número que o alvo não consegue mostrar.

### 4.9. Nenhum exemplo da documentação é executado

1296 blocos de código em 459 arquivos `.md`, e nada os roda. O `audita_doc.ps`
confere assinatura de título; o código dentro das páginas nunca foi analisado.

Escrevi o auditor que faltava — **`scripts/audita_exemplos_doc.ps`** (não
commitado; decisão sua) — e ele achou de primeira [V]:

```
blocos ```ps conferidos: 275
BLOCOS QUE O MOTOR RECUSA: 1
  docs/swagger/swagger.md:19
    SyntaxError: bloco com ':' nao existe mais — use '{ }'
```

Ou seja: o exemplo principal da lib `swagger` (lib bundled, voltada ao usuário)
está escrito na sintaxe de bloco que a linguagem removeu. Quem copiar, toma
erro. Mais um, achado à mão, entre os 6 blocos que o auditor pula por conterem
`...`: `docs/linguagem/06-funcoes.md:148` apresenta

```
int async reaction f():   ...
```

como forma "válida" — o motor responde `bloco com ':' nao existe mais`.

O auditor é deliberadamente conservador (só cercas ```ps, dedenta a margem do
markdown, ignora bloco que mostra erro de propósito) e **nomeia** o que pulou,
para não esconder corte. Python (doctest), Rust (`cargo test --doc`) e Go
(Example) executam a doc por esse motivo. `examples/*.ps` (16 arquivos) também
não é rodado por nada [V] — analisa, mas ninguém executa.

### 4.10. Ferramenta de usuário: o que a linguagem não oferece a quem escreve `.ps`

Sem **formatador** (gofmt/rustfmt/black), sem **linter**, sem **depurador**
(nem DAP), sem **framework de teste** (não há `assert`/`test` na stdlib nem
`psl test`), sem **cobertura de código PoolScript**, sem **profiler**. Para uma
linguagem que já tem servidor HTTP, ORM-ish, GUI e gerenciador de pacotes, a
ausência de "como eu testo meu programa?" é o buraco mais visível de fora.

Faltam também no runtime: **`math`** (sem `sqrt`, `floor`, `ceil`, `log`),
**`random`** e — mais sério — **nenhum gerador criptográfico exposto**: o módulo
`hash` oferece `crypt/check/sha256/b64encode/b64decode`, e `ps_random_bytes`
existe só em C. Quem for gerar token de sessão no jinker não tem por onde.

### 4.11. Opcode duplicado à mão nos dois arquivos

`vm/ps_compiler.c:28` e `vm/poolscript_vm.c:84` repetem o enum inteiro (87
opcodes numerados). Hoje eles são **idênticos** [V, diff]. Não há
`_Static_assert`, header comum, X-macro nem teste que compare — a próxima
divergência é despacho no opcode errado, isto é, corrupção. Um header
`ps_opcodes.def` com X-macro resolve e ainda dá o desmontador de graça (que hoje
existe pela metade: `nome_op()` só no compilador, sem `pool --dis`).

### 4.12. Release engineering

Sem `LICENSE`, sem `SECURITY.md`, sem `CONTRIBUTING.md`, sem CHANGELOG. As tags
param em `v8.2.54` (e `v8_2_17`, `Linux_standalone_ELF_8_2_31` — três padrões
diferentes) enquanto a versão é **8.3.78**: não há release rastreável nem forma
de alguém dizer "estou na 8.3.7x".

E há um item legal concreto: o `pool` entra em produção com **`libmysqlclient`
estático** (`-Wl,-Bstatic ... -lmysqlclient`), que é GPLv2 (com FOSS Exception
condicionada). Sem `LICENSE` no repositório, o binário distribuído em
`dist/pool-portable.tar.gz` fica numa posição indefensável se alguém perguntar.
Uma SBOM (`syft`/CycloneDX) do bundle resolveria a parte técnica.

### 4.13. Código morto da era CPython dentro do motor

- `vm/poolscript_vm.c`: **256 linhas** sob `#ifdef PS_MODULO_PYTHON`
  (2078–2159 e 21985–22160), com `PyObject`, `PyList_New`, `PyErr_SetString`.
  Nenhum alvo define esse símbolo [V] — nunca compila, nunca é conferido, e
  engorda justamente o arquivo que o `-fanalyzer` não consegue terminar.
- `vm/ps_lexer_bind.c`, `ps_parser_bind.c`, `ps_compiler_bind.c` — **569 linhas**
  com `#include <Python.h>`, fora do `FONTES`. O cabeçalho do primeiro diz:
  *"quando o binário standalone estiver pronto, este arquivo é deletado"*. Está
  pronto há muito tempo.
- `.gitignore` ainda cheio de `mypyc`, `.pyd`, `pyinstaller`, `psl-poolscript-vsix`,
  `audita_doc_sigs.py` — regras para coisas que não existem.

O `notas/CLAUDE.md` diz "não há Python nem JavaScript no projeto". São 825
linhas de Python-era ainda no `vm/`.

---

## 5. Catálogo de ilogismos

Comportamentos incoerentes entre si — não são "opinião de design", são regras
que se contradizem dentro do mesmo motor. Todos verificados em `858f51e`.

| # | O que acontece | Por que é ilógico |
|---|---|---|
| **I1** | `null == 0` → `True`, mas `null <= 0` → `False`. E `null == null` é `True` com `null <= null` `False` | igualdade sem ordem coerente: `a == b` não implica `a <= b`. Verificado por matriz completa: 7 pares quebram a regra |
| **I2** | Qualquer comparação com `null` devolve `False` calado (`null < 1`, `null > 1`, `null == 1` todos `False`), enquanto `"abc" < 5` **levanta** `SomeValueUnexpected` | tricotomia quebrada em 8 pares. Duas políticas para o mesmo erro: null é silencioso, o resto grita. `if (x < 10)` com `x = null` cai no `else` sem avisar |
| **I3** | `sorted([3, null, 1])` **levanta** erro, mas `3 < null` devolve `False` | a biblioteca é mais rígida que o operador que ela usa |
| **I4** | `l[99]` numa lista de 3: escreve `IndexOutOfBoundsWarning` no **stderr**, devolve `null`, **rc=0**. `l[99] = x`: `IndexError` e rc=1. `d["falta"]`: `KeyError` | três políticas para "índice/chave inexistente". A leitura fora de faixa é a única que corrompe dado em silêncio dentro de um pipeline |
| **I5** | `try { post(l[99]) } catch (e) { ... }` **não pega** nada | o "Warning" é `fprintf(stderr, ...)` cru (`poolscript_vm.c:19114`), não exceção: não tem tipo, linha, coluna, nem como ser tratado |
| **I6** | `SomeValueUnexpected` aparece **466 vezes** no motor; `TypeError` 2, `ValueError` 2, `IndexError` 4 | `catch (TypeError e)` é inútil na prática: divisão por zero, chave mutável, comparação incompatível e falha de I/O caem todas no mesmo balde |
| **I7** | `AtributtedValueError` | tipo de erro público com o nome escrito errado ("Attributed"), e ele nem aparece em `docs/exceptions/` |
| **I8** | `1 / 0` → `SomeValueUnexpected: divisão por zero: division by zero` | mensagem duplicada em dois idiomas, e o tipo não diz nada sobre divisão |
| **I9** | Doc de exceções lista 12 tipos; o motor emite **18** | faltam `AtributtedValueError`, `IndexError`, `TypeError`, `ValueError`, `NotImplementedError`, `FileNotFoundError`, `SyntaxError`. O `audita_doc` confere assinatura, não taxonomia de erro |
| **I10** | `is` é documentado como operador de **tipo**, mas `5 is 5` → `True` e `5 is 6` → `False` | com lado direito que não é tipo, cai em igualdade de valor sem avisar; um `x is 0` digitado por engano vira comparação, não erro |
| **I11** | `x = 10000000000000001` (int exato, bignum ok), `x / 1` → `1e+16`, `int(x / 1)` → `10000000000000000` | `/` é sempre real, e **não existe `//` nem `**` nem `pow()`**: uma linguagem com inteiro de precisão arbitrária sem divisão inteira nem potência perde precisão calada na única divisão que tem |
| **I12** | `while (i < 3) { i = i + 1 }` numa linha: ok. Fechar `}` na linha do último comando de bloco multi-linha: **SyntaxError** — apontando a linha **seguinte** | regra de chave que muda conforme o bloco ser de uma linha ou não, com diagnóstico na linha errada |
| **I13** | `def f(x) { ... }` (quem vem de Python) → `SyntaxError: faltou ':' no dicionario`, apontando para dentro do corpo | o erro fala de dicionário porque o `{` virou literal; nada sugere `action` |
| **I14** | `"a" + 1` → `'+' entre tipos incompativeis` | a mensagem não diz **quais** tipos (compare: CPython diz `can only concatenate str (not "int") to str`) |
| **I15** | `docs/sys/argv/argv.md` afirma `sys.argv == ["app.ps", "entrada.txt", ...]` e ensina `sys.argv[1]` para o primeiro argumento. O motor devolve `['um', 'dois']` | doc e motor discordam por um índice: todo programa que seguir a doc lê o argumento errado |
| **I16** | `pool --check arquivo_quebrado.ps` imprime `{"ok":false,...}` e sai com **rc=0** | conferidor que nunca reprova não serve de portão: `pool --check f.ps \|\| exit 1` nunca dispara |
| **I17** | `docs/swagger/swagger.md` e `docs/linguagem/06-funcoes.md` ensinam `reaction f():` | sintaxe removida da linguagem; o motor responde "bloco com ':' nao existe mais" |
| **I18** | Módulos com nome de ecossistema alheio (`flask`, `smtplib`, `mimetext`, `requests`) e semântica diferente | `@app.route("/user/<id>")` + `action perfil()` **não** injeta `id` (é `request.path_param("id")`), ao contrário do Flask que o nome promete |
| **I19** | Aliases duplicados sem canônico anunciado: `request`/`requests`, `qr`/`qrcode`, `manpu`/`mp`, `psodbc`/`db`, `sqlite`/`sqlite3` | dois nomes para a mesma coisa multiplicam a superfície de doc, de teste e de completion |
| **I20** | `--metadata` declara tipo de retorno em **86 de 838** membros (10,3%); `notas/CLAUDE.md` diz que o hover do LSP mostra "a assinatura real e o tipo de retorno" | o hover mostra assinatura **sem** retorno em 9 de cada 10 casos. E o default só existe metade: métodos de tipo expõem 4591 defaults, os 341 parâmetros de **módulo** expõem **zero** — então `audita_doc` não pode conferir default de nativa nenhuma |
| **I21b** | 14 parâmetros do `--metadata` têm **espaço no começo do nome** (`str.find(' inicio', ' fim')`) | vem de `split(",")` sem `strip` na tabela do VM; aparece assim no hover do editor e em qualquer doc gerada dela |
| **I22** | `post(true)` → `True`; `post(null)` → `null`; `type(null)` → `Null`; `type(true)` → `bool` | três convenções de caixa no mesmo sistema de valores. `true`/`True` e `null`/`Null` são aceitos como literal, e cada um imprime de um jeito |
| **I23** | Defaults no `--metadata` grafados `Null` 4557 vezes e `null` 5 vezes | a mesma tabela discorda de si mesma sobre como se escreve nulo |
| **I21** | Indentação tem que ser múltiplo de 4 espaços — e as próprias páginas de `docs/linguagem/` tinham blocos com 2 e 3 espaços | regra do motor que a doc do motor não segue (esses casos eram margem do markdown; ver §4.9 — o auditor novo dedenta antes de acusar) |

---

## 6. O que está certo (verificado, para não passar mensagem torta)

- **Suíte:** 7848/7848 [V]. Modelo de subprocesso por caso (morte vira
  resultado) é a decisão certa para um motor que já morreu em teste.
- **Igualdade é relação de equivalência de verdade**: matriz 12×12 sobre
  `null,false,true,0,1,0.0,1.0,"","0",[],[0],{}` — reflexiva, simétrica e
  transitiva, zero violação [V].
- **ReDoS fechado**: `regex.match("(a+)+$", "aaa…!")` responde
  `regex: backtracking demais` em 30 ms [V].
- **JSON com teto de profundidade 64** nos dois sentidos [V].
- **Jinker HTTP**: header limitado a 64 KB (medido: conexão resetada; RSS do
  servidor foi de 4 MB para 12,6 MB e estabilizou [V]), corpo a 64 MB,
  `SO_RCVTIMEO` de 30 s, `Content-Length` validado dígito a dígito,
  TE+CL recusado — as correções da auditoria anterior estão de pé.
- **Alocação hostil barrada**: `[0] * 10**12`, `"a" * 10**12`, índice gigante,
  todos com erro limpo [V].
- **Cliente HTTP verifica hostname** (`SSL_set1_host`) [L].
- **Comparação em tempo constante** (`ps_iguais_constante`) existe e é usada [L].
- **Injeção de falha de alocação** com `--wrap` do linker (técnica do SQLite) —
  código de produção sob teste, sem `#ifdef` no motor [L].
- **`gc_valida_tabela()`** aborta no boot se um tipo esqueceu tracer [L].
- **Casos pendentes com inversão**: caso que passa e não devia acusa
  "JÁ FUNCIONA" — impede fila fantasma [L].
- **Zero `TODO`/`FIXME`/`HACK`** no `vm/` [V].
- **Build sem aviso** com as flags do projeto [V].
- **CI de 27/08** já torna ASan/OOM/fuzz/analisador-total/cobertura
  obrigatórios no cron, com o arquivo grande incluído no analisador [L].

---

## 7. Não verificado / risco residual

- `make oom`, `make check-asan`, `make fuzz`, `make analisa`, `make check-e2e`,
  `make cobertura` **não foram executados nesta sessão** (máquina sensível). A
  cobertura citada é o artefato de 27/08 15:00, da árvore `2c3b831`.
- Nenhum serviço externo estava de pé: Postgres, MySQL, mongod e SMTP são
  **não verificados** — e é justamente onde a cobertura é 0%.
- §3.4 (mail TLS), §3.5 (`rand()`), §3.6 (HashDoS), §3.8 (pacotes) são
  **lidos no código, não explorados**: não escrevi MITM, não gerei conjunto de
  chaves colidentes, não montei registry hostil. A leitura é direta e as
  referências são públicas, mas a exploração não foi executada.
- O impacto de §3.2 **dentro do jinker sob ASan** não foi medido — e não pode
  ser, hoje, sem as anotações de fibra do §4.5.
- A árvore mudou durante a auditoria (`2c3b831` → `858f51e`); as medições de
  LSP valem para o servidor de `858f51e`, que ganhou completion de volta.

---

## 8. Ordem de ataque sugerida

Por retorno sobre esforço, não por gravidade nominal:

1. **Teto de profundidade** no parser e em `escreve_valor`/`valor_para_texto`,
   e teto no detector de ciclo (§3.1, §3.2, §3.3). Três SIGSEGV, uma tarde.
2. **Página de guarda** na pilha da fibra (`mmap` + `PROT_NONE`) — transforma o
   pior caso do §3.2 em fault determinístico em vez de heap corrompido.
3. **`PS_ASSERT` + `make check-debug`** (§4.1). Multiplica o valor de tudo que
   já existe: suíte, fuzz, ASan e OOM passam a achar estado errado, não só morte.
4. **`SSL_VERIFY_PEER` + `SSL_set1_host` no mail** com escape explícito (§3.4),
   e `ps_random_bytes` na máscara do WebSocket (§3.5).
5. **Semente de hash por processo** (§3.6).
6. **Corpus e crashes de fuzz versionados + replay na suíte**, e mais dois ou
   três alvos (HTTP, MIME, regex) (§4.2).
7. **E2E sem serviço na CI** e **cobertura como portão** com baseline por
   arquivo (§4.3, §2.2).
8. **`ps_opcodes.def` com X-macro** (§4.11) e **remoção das 825 linhas de
   Python-era** (§4.13).
9. **`scripts/audita_exemplos_doc.ps` no `make check`** e correção de
   `docs/swagger/swagger.md` (§4.9).
10. **LICENSE + SECURITY.md + tags coerentes + SBOM do bundle** (§4.12).

---

## 9. A suíte em Python cobria MAIS que a de hoje

Pergunta levantada depois da auditoria: entre a suíte antiga (pytest, apagada em
`f90845d`) e a de hoje (C, `./testar`), qual cobre mais? Não havia resposta no
repositório — `AUDITORIA-TESTES.md` registra que **cobertura nunca havia sido
medida** antes de 2026-08-26, e o primeiro número (42,1% de ramo) já é da suíte
em C. Então medi as duas, com o mesmo método.

**Método** [V]: worktree em `db0bcb6` (2026-08-24 — último commit com
`src/poolscript/` E `tests/`: 87 arquivos, 1300 funções de teste, 3066 casos
coletados). Compilei o `pool` e a extensão CPython com `-O0 -g --coverage`,
rodei `pytest tests/`, capturei com `lcov --rc branch_coverage=1`. Do lado de
hoje, o mesmo `cob/vm.info` do §2. Os `*_bind.c` (que não existem no build de
hoje) ficaram fora do total das duas colunas.

| | suíte em **Python** (db0bcb6) | suíte em **C** (hoje) |
|---|---|---|
| casos | 3066 testes, **113 s** | 7852 casos, **73 s** |
| linhas | **78,8%** (15669/19886) | 55,0% (11772/21388) |
| funções | **83,8%** (1102/1315) | 54,9% (761/1385) |
| **ramos** | **53,3%** (11774/22079) | **41,4%** (9987/24146) |

Resultado: `3062 passed, 4 failed` (as 4 são `test_cli` de `git update` e
`test_swagger`, que já batia no `SyntaxError` do bloco `:`).

Por arquivo, é onde a diferença mora:

| arquivo | Python linha/ramo | C hoje linha/ramo |
|---|---|---|
| `poolscript_vm.c` | **82,0 / 53,4** | 53,3 / 39,8 |
| `ps_db.c` | **71,5 / 48,9** | 0 / 0 |
| `ps_jinker.c` | **74,0 / 52,1** | 0 / 0 |
| `ps_mongo.c` | **81,7 / 40,8** | 0 / 0 |
| `ps_pkg.c` | **61,2 / 41,8** | 0 / 0 |
| `ps_http.c` | **80,5 / 64,2** | 13,3 / 6,4 |
| `ps_hash.c` | **95,4 / 76,9** | 41,5 / 37,7 |
| `ps_regex.c` | **85,3 / 64,9** | 62,2 / 45,0 |
| `main.c` | **70,8 / 52,6** | 22,7 / 16,6 |
| `ps_qr.c` | 90,7 / 65,9 | 86,0 / **69,0** |
| `ps_xlsx.c` | 84,3 / 55,0 | 81,1 / 53,2 |
| `ps_compiler.c` | 93,3 / 72,5 | 91,5 / **70,5** |
| `ps_parser.c` | 81,4 / 54,4 | **93,6 / 67,1** |
| `ps_lexer.c` | 68,2 / 59,3 | **75,6 / 67,5** |
| `ps_mail.c` | 0 / 0 | **2,8 / 0,6** |
| `ps_guzer.c` | 0 / 0 | 0 / 0 |

**Por que a antiga cobria mais, e o que isso quer dizer**

- Ela testava **em processo**: `from poolscript import ...` alcançava banco,
  jinker, http, pkgmgr e mongo com mock e loopback. A de hoje é 100%
  fork/exec do `./pool`, e tudo que precisa de serviço foi empurrado pro
  `teste/e2e/`, que até hoje não entrava em portão nenhum. Os cinco 0% do §2
  **não são código novo sem teste — são teste que existia e foi perdido**.
- Front-end é o contrário: `ps_parser.c` e `ps_lexer.c` estão melhor hoje
  (93,6 vs 81,4 de linha), efeito dos geradores (`oraculo`, `robustez`,
  `diferencial`). Ressalva honesta: nesse ponto o número do Python está
  **subestimado** — os `.gcda` de `ps_parser/ps_lexer/ps_ast` da extensão
  colidiram com os do bind e foram descartados, então esses três só contam o
  caminho do binário.
- **Quantidade de caso não é cobertura.** 7852 casos fixos cobrem menos que
  3066 testes, porque 6891 deles (`oraculo` + `diferencial`) são snapshot de
  expressão, que passam pelo mesmo caminho de código.

Não é argumento para voltar ao Python: é a conta do que a migração custou em
alcance, e a lista exata do que precisa ser reconstruído em `.ps`/C — começando
pelos cinco arquivos em zero.

---

## 10. Estado dos achados em `6ca7def` (verificado)

Depois desta auditoria entraram `febff9d` e `6ca7def`. Conferido rodando, não
lendo o diff:

**Fechado** — §3.1 parser com `PS_PARSE_PROF_MAX` (30 mil parênteses → erro de
sintaxe limpo; 1500 níveis ainda passam) · §3.2 teto no texto **e** página de
guarda `mmap`+`PROT_NONE` na fibra (`/fundo/300`, `/fundo/5000` e `/fundo/50000`
respondem 200; o servidor sobrevive) · §3.3 ciclo profundo · §3.4 mail com
`SSL_VERIFY_PEER` + `SSL_set1_host` e escape `PS_MAIL_TLS_INSEGURO=1` ·
§3.5 `ps_random_bytes` na máscara WS · §3.6 semente de hash por processo
(`PS_HASH_SEED` fixa) · §4.1 `vm/ps_assert.h` + `make check-debug`
(**7852/7852 sob `-DPS_DEBUG`**) · §4.3 `make check-e2e-local` (**rc=0**) ·
§4.9 auditor de exemplos no `make check` (**276 blocos, 0 recusados**) ·
§2.2 `teste/cobertura_portao.ps` com baseline por arquivo (**rc=0**) ·
replay de achado de fuzz versionado (`teste/fuzz_achados/crash-parser-profundo`).

**Aberto** — §3.7 `/tmp/ps_lsp` 0755 · §3.8 pacotes sem versão/lockfile ·
§4.4 clang-tidy/CodeQL · §4.5 `detect_leaks=0`, sem TSan, sem
`__sanitizer_start_switch_fiber` (agora mais relevante: a pilha virou `mmap`) ·
§4.6 zero benchmark (a era Python tinha `bench_async.sh`/`bench_webhook.sh`) ·
§4.7 matriz de CI e `_Static_assert` no GMP · §4.8 `avisos` ainda com
`-fsyntax-only`, sem `-Werror` · §4.10 ferramenta de usuário, `math`, `random`,
CSPRNG · §4.11 opcode duplicado · §4.12 LICENSE/SECURITY/tags/SBOM ·
§4.13 código morto de CPython · catálogo §5 inteiro menos I17.

**Novo, pequeno:** `pool-debug` (3,2 MB, artefato de build) está *staged* e não
está no `.gitignore`, ao lado de `pool-oom`/`pool-fuzz`/`pool-asan` que estão.

---

*Auditoria feita sem alterar o motor. Os arquivos criados foram
`scripts/audita_exemplos_doc.ps` (auditor de exemplos da doc, já commitado em
`febff9d`) e este relatório.*
