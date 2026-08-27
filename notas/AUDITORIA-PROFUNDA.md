# Auditoria profunda: onde as ferramentas não olham

Varredura de 2026-08-26, feita **sem** usar os alvos do projeto (`analisa`,
`oom`, `fuzz`, `propriedade`). O motivo é o ponto central desta rodada:

> ferramenta só acha o que foi feita para achar, e a cobertura ainda não é 100%.

---

## Estado em 2026-08-26 (depois da correção)

**9 de 9 tratados.** Reprovado com: 7846/7846 na suíte, e2e do jinker com os
11 casos crus novos, `audita_doc` 87/87, LSP 15/15, `oom` limpo, build sem
aviso.

| # | O que era | O que ficou |
|---|---|---|
| **1** | `unsigned char hdr[256]` fixo — `kid` de 219 passava, de 220 rejeitava token VÁLIDO | o buffer é dimensionado pelo header real (base64 encolhe 4→3, então o tamanho é sabido antes de decodificar), com teto de 64 KB contra abuso. Provado com tokens HS256 de verdade: `kid` de 3000 passa, e assinatura errada / `alg: none` continuam recusados |
| **2** | `chunked` servido com **200 e corpo vazio** | 501, e TE junto com Content-Length é 400 (os dois dizem onde o corpo acaba; discordar É o ataque) |
| **3** | pipelining travava: 2 requisições num write → 1 resposta | a conexão com bytes ainda no buffer vai pra FILA que o laço já drena, em vez de re-armar no epoll esperando dado que já saiu do socket. Sem recursão por requisição |
| **4** | `Content-Length` duplicado usava o primeiro, calado | valores diferentes → 400; repetição do MESMO valor colapsa em um e passa |
| **5** | `Content-Length: abc` virava 0 via `atol` | dígito do começo ao fim, senão 400 |
| **6** | linha de header sem `:` ignorada | 400 |
| **7** | `Content-Length : 5` virava header de nome `"Content-Length "` — sumia | 400, que é o que a RFC 9112 §5.1 manda nominalmente |
| **8** | `ps_base64_decode` parava no primeiro `=` e devolvia dado parcial | padding continua OPCIONAL (base64url de JWT não tem), mas quando existe tem que estar no fim e na quantidade exata; `n % 4 == 1` é sempre erro |
| **9** | `scripts/audita_doc.ps` sumiu do repo | reescrito contra `pool --metadata` (o antigo dependia de um `audita_doc_sigs.py` que não existe desde o porte pra C). Entrou no `make check` |

**O que o achado 9 destapou, e vale mais que ele:** o `--metadata` mentia por
omissão. 21 nativas declaravam ZERO parâmetro (`bytes.hex()`, `hash.crypt()`,
todas as `Parsing.*`, `sys.exit()`), e **o LSP lê essa mesma tabela** — ele
oferecia hover e completion com assinatura vazia. Preenchido, conferindo a
aridade no corpo de cada nativa. Hoje: 87 páginas conferidas, 87 batem, 0
lacunas.

Duas páginas estavam erradas de verdade, e uma delas era perigosa:
`psodbc.connect` documentava `odbc_driver=` e `trust_server_cert=`, que **o
motor recusa** com "argumento nomeado desconhecido" — quem seguisse a doc
tomava erro. O `TrustServerCertificate=yes` é fixo no código, não parâmetro.

**O que o auditor de doc NÃO confere, dito de propósito:** valores default.
`--metadata` não os expõe — 309 parâmetros, zero defaults — porque cada nativa
resolve o dela no corpo em C. Então `flags=0` continua sem verificação; só a
existência e a ordem de `flags`. Dizer "87/87 OK" sem essa ressalva seria
conferir metade e anunciar inteiro.

**Um defeito do próprio auditor, achado ao rodá-lo:** `docs/jinker/request/get/`
documenta `jinker.request.get(key)`, e o índice colidia com o módulo `request`
de HTTP, marcando a página como divergente sem ser. O caminho do arquivo passou
a decidir o namespace. Auditor que inventa achado é tão ruim quanto o que
esconde.

**O que continua fora:** `chunked` é recusado, não implementado. Recusar com
501 é correto por RFC e fecha o buraco de segurança; implementar a codificação
é outra tarefa.

Então a busca foi dirigida para onde a cobertura medida é **zero ou quase**, com
**oráculos externos** — vetores oficiais NIST/RFC, `badssl.com`, requisições HTTP
cruas — em vez das tabelas do próprio projeto.

Nada foi alterado no repositório. Todos os harnesses ficaram em `/tmp`.

---

## Onde procurei, e por quê

| Alvo | Cobertura medida | Por que é o lugar certo |
|---|---|---|
| `ps_hash.c` | **0,00%** | cripto escrita à mão; errada, ninguém saberia |
| `ps_jinker.c` | **0,00%** | parser HTTP, exposto à rede, entrada hostil |
| `ps_pkg.c` | **0,00%** | baixa e **executa** código de terceiro |
| `ps_mail.c` | 2,98% | parser MIME; e-mail é o dado menos confiável que existe |
| `ps_http.c` (TLS) | 69,76% | o caminho do TLS não é exercitado por nenhum teste |
| caminho JWT na VM | — | verificação de token de terceiro |

---

## Achados

### 1. `jwt.check()` rejeita token válido com header ≥ 256 bytes

**GRAVE (interoperabilidade).** `vm/poolscript_vm.c:9029`:

```c
unsigned char hdr[256];
long nhdr = ps_base64_decode(tk->chars, (size_t)(p1 - tk->chars), hdr, sizeof(hdr) - 1);
if (nhdr <= 0) return 0;
```

O `ps_base64_decode` devolve `-1` quando a saída não cabe no `cap`, e o `return 0`
transforma isso em **“token inválido”**.

Provado com tokens HS256 **de verdade**, assinados com o `ps_hmac` do próprio
motor e o mesmo segredo:

| `kid` | header decodificado | `jwt.check` |
|---|---|---|
| 218 chars | 254 bytes | `{'sub': 'ana'}` |
| 219 chars | **255 bytes** | `{'sub': 'ana'}` |
| 220 chars | **256 bytes** | **`null`** |
| 300 chars | 341 bytes | `null` |

Emissores reais estouram isso sem esforço: `kid` longo, `jku`, `x5t` e
principalmente `x5c` (cadeia de certificados no header, medida em kilobytes).
Falha **fechado** — não é brecha de segurança —, mas o token bom é recusado e a
causa não aparece em lugar nenhum.

Nenhum teste cobre porque `ps_hash.c` está a 0% e este caminho da VM não é
exercitado.

### 2. jinker: `Transfer-Encoding: chunked` descarta o corpo em silêncio

**GRAVE (perda de dado).** O parser (`vm/ps_jinker.c:328`) só conhece
`Content-Length`. Não há **uma única menção** a `chunked` no servidor.

```
POST /eco HTTP/1.1
Transfer-Encoding: chunked

5
OLAAA
0
```
→ `HTTP/1.1 200 OK` · `{"len": 0, "corpo": ""}`

O mesmo POST com `Content-Length: 5` devolve `{"len": 5, "corpo": "OLAAA"}`.

Chunked é HTTP/1.1 válido e é o que qualquer cliente usa quando o tamanho do
corpo não é conhecido de antemão (streaming, upload de arquivo por biblioteca).
O handler recebe corpo vazio e responde **200**, como se o cliente não tivesse
mandado nada. A RFC 9112 §6.3 manda responder **501 Not Implemented** quando o
`Transfer-Encoding` não é entendido.

### 3. jinker: pipelining HTTP trava

**REAL.** Duas requisições GET válidas num único `write`:

```
respostas recebidas: 1   (esperado: 2)
```

As mesmas duas, em `write`s separados com 1 s de intervalo: **2 respostas** — ou
seja, keep-alive funciona; o que quebra é só o pipelining (RFC 9112 §9.3.2).

Causa: `conn_consome()` (`ps_jinker.c:99`) faz `memmove` e **mantém** o resto no
buffer, mas o loop de eventos só volta à conexão quando o `poll` acusa dados
**novos no socket**. Bytes já bufferizados não acordam ninguém. A função que
checaria isso existe — `ps_jk_ws_tem_dados`, com `if (c->n > 0) return 1;` — e é
usada **só** no caminho WebSocket.

O lado bom do mesmo defeito: **não há request smuggling**. Testei
`Content-Length: 0` com uma requisição completa embutida no corpo — a requisição
embutida simplesmente nunca é processada.

### 4–7. jinker: quatro requisições malformadas aceitas com 200

Nenhuma delas devolve **400**; todas são processadas como se estivessem certas.

| # | Requisição | Resultado | O que a RFC manda |
|---|---|---|---|
| 4 | `Content-Length: 5` **duas vezes** | aceita, usa a primeira | 9112 §6.3.5 → **400** |
| 5 | `Content-Length: abc` | `atol`→0, corpo vazio, **200** | 9112 §6.3 → **400** |
| 6 | linha de header **sem `:`** | ignorada em silêncio, **200** | 9112 §5 → **400** |
| 7 | `Content-Length : 5` (espaço antes do `:`) | header perdido, corpo vazio, **200** | 9112 §5.1 → **400**, citado como vetor de smuggling |

O padrão é um só: **o parser é permissivo e nunca recusa**. Isolado, o jinker
não é contrabandeável (achado 3). Atrás de um proxy reverso que interprete esses
casos de outro jeito, a discordância é exatamente o que o smuggling explora.

### 8. `ps_base64_decode`: padding inválido vira dado parcial, não erro

`vm/ps_hash.c:400` devolve `-1` **só** para caractere fora do alfabeto. Todo o
resto de entrada malformada sai como “decodifiquei N bytes”:

| Entrada | Devolveu | Deveria |
|---|---|---|
| `Zg=` (padding curto) | 1 byte | erro |
| `Zg===` (padding longo) | 1 byte | erro |
| `=Zm9v` (padding no início) | 0 bytes | erro |
| `Zm==9v` (padding no meio) | 1 byte | erro |
| `Z` (len % 4 == 1) | 0 bytes | erro |
| `----` / `____` (urlsafe no decode padrão) | 3 bytes | — permissivo |

Não abre buraco no JWT, porque a verificação exige tamanho de assinatura exato
(`if (nveio != (long)nesp) return 0;`). Mas o contrato é frouxo para os outros
consumidores (`hash.b64decode`, MIME, `request`): entrada corrompida vira dado
truncado, sem sinal.

Espaço e quebra de linha no meio **são** aceitos corretamente — RFC 2045 manda
ignorar.

### 9. `scripts/audita_doc.ps` não existe mais

A auditoria de assinatura doc↔código (que já rodou 261/263) sumiu do repo.
Restaram `casos_para_chaves`, `converte_tudo`, `docs_para_chaves`,
`para_chaves`, `regrava_dif`. Hoje só o `confere_metadata.ps` cruza doc com
motor, e ele cobre apenas o metadata do editor.

---

## O que verifiquei e está CERTO

Registrado porque auditoria que só lista defeito não informa onde já se pode
confiar.

**Cripto — 24 vetores oficiais, todos passam.** FIPS 180-4, RFC 4231, RFC 6070,
RFC 4648. Inclui os casos que costumam quebrar implementação caseira:

- `sha256` de **1.000.000** bytes `'a'` (múltiplos blocos + padding de tamanho);
- `sha256` de 56 bytes (o caso de borda do padding);
- `sha512` e `sha384`;
- HMAC com **chave maior que o bloco** (RFC 4231 #6), que exige hash da chave;
- PBKDF2 com 1 e com 4096 iterações;
- base64 dos 7 comprimentos do RFC 4648, ida e volta.

**TLS — validação completa.** `ps_http.c:105`: `SSL_CTX_set_default_verify_paths`
+ `SSL_VERIFY_PEER` + `SSL_set_tlsext_host_name` (SNI) + **`SSL_set1_host`**
(casamento de hostname). Testado contra `badssl.com`:

```
CONECTOU  200  https://example.com/
recusou        https://wrong.host.badssl.com/
recusou        https://expired.badssl.com/
recusou        https://self-signed.badssl.com/
recusou        https://untrusted-root.badssl.com/
```

**JWT — o desenho está certo.** `alg` não é confiado (só a família HMAC; `none` e
`RS256` são recusados), o HMAC é calculado sobre `header.payload`, a assinatura
precisa ter **tamanho exato**, e a comparação é em tempo constante
(`ps_iguais_constante`). O único problema é o buffer do achado 1.

**MIME — robusto sob entrada hostil.** 17 mensagens tortas sob ASan+UBSan:
`=?utf-8?B?` sem fechar, charset vazio, base64 inválido, encoding desconhecido,
header sem valor, header duplicado, folding, 200 palavras RFC 2047 encadeadas,
multipart sem boundary de fechamento. **Nenhum crash, nenhum UB.** E o
casamento de nome de header é exato — `SubjectX:` não casa `Subject`, que é o
bug clássico desse parser.

**`ps_pkg` — instalador cuidadoso.** Exige HTTPS (`https_ok`), confere o sha256
quando o índice o fornece, e `derive_name` usa `strrchr(target, '/') + 1` — um
nome `../../etc/x` vira `x`, sem path traversal. O sha256 é **opcional**: sem ele
a integridade fica só no TLS.

**Opcodes — nenhum morto, nenhum sem `case`.** Cruzei os emitidos pelo
compilador com os tratados na VM: batem (os que pareciam faltar são a macro
`CMP(OP_LT, <)`).

**Keep-alive HTTP funciona.**

---

## Estrutura e histórico

- **240 commits em 27 dias** (2026-07-31 a 2026-08-26), ~9 por dia.
- Hotspot absoluto: **`vm/poolscript_vm.c`, 87 alterações**. É também o maior
  arquivo (21.9k linhas), o de menor cobertura relativa entre os ativos (53,8%)
  e o único que fica **fora** do `make analisa` por padrão. Três razões
  independentes apontando para o mesmo arquivo.
- Maiores funções: `stmt_no` **946 linhas** (`ps_compiler.c:1610`), `statement`
  **824** (`ps_parser.c:1700`), `expr_no` 428, `primario` 374. São switches de
  compilador — idiomático em C, e é onde o risco se concentra.

---

## Nota de método

Três vezes nesta varredura a **minha própria busca** produziu falso positivo:

1. um `awk` de fronteira de função deu 2.790 linhas para `chama_valor`, que tem
   103;
2. um `grep` de opcodes acusou 7 sem `case` na VM — todos eram a macro `CMP`;
3. um `grep` de validação TLS não encontrou `SSL_set1_host`, que estava duas
   linhas abaixo do que eu havia lido.

Nos três casos o erro só apareceu porque fui conferir o resultado no código.
É o mesmo argumento que motivou esta rodada, aplicado a mim: **a ferramenta
mede o que ela sabe medir, e a conferência é o que transforma medida em fato.**

---

## Gaps anteriores que continuam abertos

Da rodada passada, ainda válidos:

- `check-e2e` roda `jinker_cli.ps` (7º na ordem alfabética) antes de
  `jinker_srv.ps` (8º) — o cliente morre com `Connection refused`;
- `make check` sai com **código 0** mesmo com aviso do `-fanalyzer`; o portão
  não fecha;
- `poolscript_vm.c` fora do `analisa` por padrão — 62% das linhas executáveis;
- seis módulos a 0%: `db`, `guzer`, `hash`, `jinker`, `mongo`, `pkg`. Destes,
  **`hash` é código puro** — não depende de serviço externo nenhum e poderia
  estar coberto hoje, com os 24 vetores deste documento;
- `make check-asan` não roda inteiro nesta máquina (OOM killer); só passa
  quebrado em lotes por grupo.
