# Auditoria: erros lógicos e remendos

Varredura das 40.874 linhas de C, do Makefile, da suíte e das notas, em
2026-08-26 (`main` @ 820d2f4). 22 achados, cada um reproduzido no código — não
inferido.

A metade que mais importa não é bug de memória: é **verificação que não
verifica** e **I/O que falha calado**.

**O que não foi coberto:** `vm/poolscript_vm.c` (21.908 linhas) não passou pelo
analisador — o processo foi morto por falta de memória nas duas tentativas. Os
16 fontes restantes passaram inteiros. A suíte completa (~2.600 casos) também
não foi executada: rodei o grupo `pendentes` e dois casos do `diferencial`. Os
achados B1–B4, C4 e F2 no arquivo grande vieram de leitura e de
`-Wduplicated-branches`, não do analisador.

---

## Estado em 2026-08-26 (depois da correção)

**22 de 22 tratados.** Reprovado com: build limpo (`-Wall -Wextra
-Wduplicated-branches`), `make analisa` de verdade, 7828/7828 na suíte, 15/15
no LSP, metadata conferida.

| # | O que foi feito |
|---|---|
| A1 | `-fsyntax-only` trocado por compilação real; **provado** com um use-after-free plantado. Teto `ulimit -v` (`ANALISA_MB=2000`) porque o analisador sem limite derrubou esta máquina 2x. `poolscript_vm.c` fica FORA por padrão, e o alvo **diz isso em voz alta** — não conta como limpo |
| A2 | Campo `pendente` no `Caso`: pendência que PASSA vira falha ("JA FUNCIONA, tire daqui"). Provado nos dois sentidos. Cabeçalho reescrito: a fila está vazia, e agora não esvazia em silêncio |
| A3 | `analisa` entrou no `check`; `check-e2e` criado, e o `check` **anuncia** que o e2e está fora — gap declarado, não escondido |
| B1 | `close()` confere o `fclose`. Provado com `/dev/full`: **antes gravava, fechava e saía 0**; agora IOError |
| B2 | `copy()` confere `fwrite`, `ferror(src)` e `fclose(dst)`, e apaga a meia-cópia |
| B3 | `writelines()` confere cada gravação e diz **qual linha** falhou |
| B4 | `write()` levanta IOError em gravação curta; o retorno continua sendo a contagem |
| C1 | Cada campo é publicado assim que o `realloc` dele dá certo — acabou o ponteiro pendurado no descritor da Entity |
| C2 | `realloc` por temporária no `pool build`, com liberação do que já havia |
| C3 | `SQLDescribeCol` com `SQL_SUCCEEDED`, e `nome`/`dt` inicializados ANTES da chamada |
| C4 | 7 `x = realloc(x, …)` restantes convertidos (mongo, xlsx, árvore XML, `split` 2x) |
| C5 | `res_cols_reserva`/`res_col_nome`: os 4 drivers passam pelo mesmo lugar checado |
| C6 | Vazamentos fechados; o `-fanalyzer` caiu de **16 avisos para 2**, e os 2 são falso positivo conferido (posse que ele não segue através de struct) |
| C7 | Pior que truncar: `n` do `snprintf` era passado adiante e o `escrever_txt` **lia além do buffer**. Os dois caminhos recusam agora |
| D1 | Andaime revertido. Não existe mais migração pela metade — o `expr`/`stmt` com salvar-restaurar, que é o que funciona, ficou |
| D2 | Sem efeito depois da reversão: não há mais fallback pro ambiente |
| E1 | Cada caso roda em diretório próprio (`mkdtemp` + `chdir`), apagado no fim. `__DIR__/` e `__DOCS__/` saíram do git. A classe ficou impossível, não só os 2 casos |
| E2 | `poll` nos dois canos. Caso de ~200 KB no stderr entrou na suíte |
| E3 | Fonte que não cabe é **recusado em voz alta**, em vez de rodar pela metade |
| F1 | Ramos idênticos removidos |
| F2 | Um tipo só, como já era na linha de baixo; quem distingue é a mensagem |
| F3 | Comentário do Makefile corrigido (`rebuild/` não existe) |

`-Wduplicated-branches` entrou nas `CFLAGS` (achou F1 e F2, zero falso
positivo). `-Wlogical-op` e `-Wformat-truncation=2` **não** entraram: o
primeiro acusa `errno == EAGAIN || errno == EWOULDBLOCK`, que no Linux são o
mesmo valor e escrever os dois é o idioma portável; o segundo acusa ~20
truncamentos deliberados. Ruído nesse volume esconde o aviso de verdade — os
dois ficam no alvo `avisos`. Alvo `memcheck` (valgrind) criado.

**Achado novo, fora desta lista:** `mp.write(column=, cell=, content=[...])`
grava a lista inteira como o texto de UMA célula, então ida-e-volta
(`write` → `mp.read`) devolve `[]`. O `.xlsx` gerado é válido e o leitor está
certo — quem decide se `content=` deve espalhar em colunas é o dono da API.
Comportamento igual no binário de 21/08: não é regressão.

---

## Tabela

| # | Local | Defeito | Efeito | Peso |
|---|---|---|---|---|
| **A1** | `Makefile`, alvo `analisa` | `-fanalyzer` junto com `-fsyntax-only`: o gcc encerra antes do GIMPLE e o analisador nunca é acionado | Imprime `-fanalyzer: nada` aconteça o que acontecer. Rodando de verdade: **15 avisos reais** | grave |
| **A2** | `teste/casos_pendentes.c:1`, `notas/LIMITACOES.md` | Cabeçalho declara "fila de trabalho, cada caso falha até o motor fazer certo". `./testar pendentes` → **29 de 29 passam** | A doc aponta para uma fila vazia; e um caso que falhe de verdade fica indistinguível de pendência velha | grave |
| **A3** | `Makefile`, alvo `check` | Não roda `analisa` nem os 16 scripts de `teste/e2e/` | Banco, mail, socket, jinker, guzer, mongo e qrcode ficam fora do portão | real |
| **B1** | `vm/poolscript_vm.c:5693`, `:1750` | `fclose()` sem checar retorno, no `close()` e no finalizador do GC. Nenhum dos 20 `fclose` do arquivo checa | O buffer da libc só desce ao disco no `fclose`. Disco cheio devolve EOF que ninguém lê: grava, fecha, e o arquivo está truncado | grave |
| **B2** | `vm/poolscript_vm.c:5732` (`PoolFile.copy()`) | O laço `fread`/`fwrite` não consulta `fwrite`, nem `ferror(src)`, nem `fclose(dst)` | Cópia truncada volta como sucesso, devolvendo o caminho do destino | grave |
| **B3** | `vm/poolscript_vm.c:5676,5681` (`writelines()`) | Os dois `fwrite` têm o retorno descartado; o método devolve `Null` | Nem erro nem contagem — não há como saber que uma linha não foi gravada | real |
| **B4** | `vm/poolscript_vm.c:5648–5660` (`write()`) | Devolve a contagem do `fwrite`, mas nunca levanta erro nem consulta `ferror` | Escrita parcial vira um número menor no retorno, que nenhum script confere | real |
| **C1** | `vm/ps_compiler.c:2234` | Dois `realloc` seguidos; se o 2º falha, `free(mn)` solta o bloco novo enquanto `def->met_nomes` ainda aponta pro antigo — já invalidado pelo 1º | Ponteiro pendurado no descritor da Entity e liberação dupla na destruição da classe | grave |
| **C2** | `vm/main.c:179` | `nomes = realloc(nomes, …)` sem checagem, com `nomes[n++]` na linha seguinte | `pool build` sob pressão de memória escreve em NULL. Analisador: *dereference of NULL 'nomes'* | grave |
| **C3** | `vm/ps_db.c:322` | `SQLDescribeCol()` com retorno ignorado; em falha, `SQLCHAR nome[128]` continua não inicializado e vai para `strdup` | Nome de coluna do ODBC formado por lixo de pilha, sem terminador garantido | grave |
| **C4** | `vm/poolscript_vm.c:13465,13943,13956`, `vm/ps_mongo.c:74`, `vm/ps_xlsx.c:157,178,291` | 7× o padrão `x = realloc(x, …)` sem temporária nem checagem | Em falha o ponteiro original vaza e vira NULL, e o código seguinte escreve nele | real |
| **C5** | `vm/ps_db.c:266,318,319` | `calloc` sem checagem seguido de `res->cols[i] = strdup(…)` | Consulta MySQL/ODBC com muitas colunas, sob memória curta, escreve em NULL em vez de levantar o erro | real |
| **C6** | `vm/ps_xlsx.c:157,291,444`, `vm/ps_db.c:96,176,324`, `vm/ps_jinker.c:753`, `vm/ps_mongo.c:67` | 8 vazamentos confirmados nos caminhos de erro | O jinker é processo longo: o de `ps_jinker.c:753` (campo de formulário) acumula requisição após requisição | real |
| **C7** | `vm/ps_pkg.c:231` | `-Wformat-truncation`: `%s` de até 255 bytes numa região que pode ter 172 | Caminho de pacote longo cortado em silêncio pelo `snprintf` | resíduo |
| **D1** | `vm/ps_compiler.c:315–365` (não commitado) | O comentário diz "Existe só enquanto a migração dos pontos de emissão não termina; some no fim". Estado: **3 chamadas de `emite_em` contra 248 de `emite`** | O commit 820d2f4 anuncia que a posição deixa de ser estado global, mas 99% das emissões continuam lendo `c->linha_atual` | real |
| **D2** | `vm/ps_compiler.c:355` | `int32_t l = pos.linha ? pos.linha : c->linha_atual;` — linha 0 é tratada como "sem posição" e cai de volta no global | É o caso que a mudança existe para eliminar: o nó sem linha (interior de f-string re-lexado) volta a herdar a linha ambiente | real |
| **E1** | `teste/casos_diferencial.c:11425`, `:11694` | Dois casos gravam `__DIR__/sub/nota.txt` e `__DOCS__/*` no diretório corrente (a raiz do repo) e não limpam | Os quatro arquivos estão **versionados** (`git ls-files` os lista) | real |
| **E2** | `teste/ps_teste.c:144` | `le_tudo(po[0])` drena stdout até EOF e só então lê stderr; os dois canos têm 64 KB | Caso com >64 KB em stderr enche o cano, o filho bloqueia, o pai espera stdout — impasse quebrado pelo `alarm(20)` e relatado como "TRAVOU" | real |
| **E3** | `teste/ps_teste.c:85,102` | `char fonte_ajustada[8192]` preenchido com corte em `sizeof - 256` | Caso com arquivo auxiliar maior que ~8 KB roda um programa truncado, comparado como se fosse inteiro | resíduo |
| **F1** | `vm/ps_lexer.c:589` | `if (tipo != T_NULL) guarda_texto(…); else guarda_texto(…);` — ramos idênticos | Condição inerte, sobra de quando `T_NULL` era tratado à parte | resíduo |
| **F2** | `vm/poolscript_vm.c:20034` | `l->len > n_alvos ? "OutputUnexpectedValues" : "OutputUnexpectedValues"` | A mensagem ao lado distingue "demais" de "insuficientes"; o tipo pretendia distinguir e não distingue. Muda o que `catch (Tipo e)` consegue tratar | resíduo |
| **F3** | `Makefile:20` | "O Makefile mora em `rebuild/` … rode sempre `make -f rebuild/Makefile <alvo>` de lá" | Ele está na raiz e `rebuild/` não existe. Instrução impossível de seguir, dentro do arquivo que ela descreve | resíduo |

Contagem: 7 graves, 10 reais, 5 resíduo.

---

## Como consertar cada classe

A ordem é a que economiza retrabalho: primeiro enxergar, depois consertar. Cada
bloco fecha com o texto normativo, não com tutorial de blog. Todos os links
foram conferidos (HTTP 200) em 2026-08-26.

### A1 — fazer o analisador realmente analisar

Trocar `-fsyntax-only` por compilação real descartando o objeto; o gcc precisa
chegar ao GIMPLE para o analisador existir. Um arquivo por invocação, e não
todos de uma vez: o `poolscript_vm.c` sozinho esgota a memória desta máquina.

```make
analisa:
	@for f in $(FONTES); do \
	  $(CC) $(CFLAGS) -I$(VM) -fanalyzer -c -o /dev/null $$f 2>&1 \
	    | grep -E "warning:|error:"; \
	done; true
```

Para provar que o alvo funciona, plante um `use-after-free` num arquivo qualquer
e confirme que ele aparece — foi assim que este achado saiu.

- <https://gcc.gnu.org/onlinedocs/gcc/Static-Analyzer-Options.html> — a lista
  completa de `-Wanalyzer-*` e o que cada um decide
- <https://gcc.gnu.org/onlinedocs/gcc/Warning-Options.html> —
  `-Wduplicated-branches`, `-Wlogical-op`, `-Wshadow` e `-Wformat-truncation`
  não estão nas `CFLAGS`; sozinhos pegariam F1, F2 e C7
- <https://github.com/google/sanitizers/wiki/AddressSanitizer> — o alvo
  `pool-asan` já existe; o que falta é rodá-lo dentro do `check`

### A2 — a fila que não é fila

Duas saídas honestas, e a escolha é de projeto. Ou os 29 casos migram para o
grupo de regressão a que pertencem e `casos_pendentes.c` volta a ficar vazio —
ou o runner ganha um campo `esperado_falhar`, e aí um caso pendente que *passa*
vira falha ("isto já funciona, mova daqui"). A segunda é a que o arquivo promete
no cabeçalho.

- <https://docs.pytest.org/en/stable/how-to/skipping.html> — a semântica exata
  de "falha esperada", e por que um xfail que passa precisa virar erro

### B — I/O que avisa quando falha

A regra é uma só: **toda função da biblioteca padrão que pode falhar tem
retorno, e o retorno tem que ser lido.** Em arquivo isso são três pontos, não
um:

- `fwrite` devolve quantos itens gravou — menor que o pedido é erro;
- `ferror(f)` acumula a falha do fluxo, e é o que pega o erro que só aparece no
  flush;
- `fclose` devolve `EOF` quando o flush final falha. É o último lugar onde
  "disco cheio" ainda pode ser dito, e hoje é descartado nos 20 pontos.

Para o `close()` da linguagem, o `IOError` já existe no motor e é o tipo certo.
Para o finalizador do GC (`poolscript_vm.c:1750`) não há a quem levantar — e é
justamente por isso que o `close()` explícito precisa reportar: o arquivo
esquecido é o caso em que ninguém nunca saberá.

- <https://man7.org/linux/man-pages/man3/fwrite.3.html> — retorno, `ferror` e a
  distinção entre fim de arquivo e erro
- <https://man7.org/linux/man-pages/man3/fclose.3.html>
- <https://wiki.sei.cmu.edu/confluence/display/c/ERR33-C.+Detect+and+handle+standard+library+errors>
  — a tabela de quem devolve o quê
- <https://cwe.mitre.org/data/definitions/252.html> — CWE-252, a classe, com
  exemplos em C

### C1, C2, C4 — o padrão do realloc

`x = realloc(x, n)` é errado por construção: em falha o `realloc` devolve NULL
*e não libera o bloco antigo*, então a atribuição perde a única referência a
ele. Sempre por temporária:

```c
char **novo = realloc(nomes, sizeof(char *) * (size_t)cap);
if (!novo) { /* nomes ainda é válido: libere ou propague */ }
nomes = novo;
```

C1 é o mesmo erro com uma volta a mais. Quando o segundo `realloc` falha,
`free(mn)` solta o bloco novo — mas `def->met_nomes` ficou apontando para o
antigo, que o *primeiro* `realloc` já invalidou. A saída é atribuir cada campo
assim que o seu `realloc` der certo, ou alocar os dois em blocos independentes e
só publicar no descritor quando ambos existirem.

- <https://en.cppreference.com/w/c/memory/realloc> — o parágrafo sobre falha é a
  regra inteira
- <https://wiki.sei.cmu.edu/confluence/display/c/MEM12-C.+Consider+using+a+goto+chain+when+leaving+a+function+on+error+when+using+and+releasing+resources>
  — cadeia de `goto` para sair de função com vários recursos abertos; resolve C6
  de uma vez
- <https://cwe.mitre.org/data/definitions/415.html> (double free) e
  <https://cwe.mitre.org/data/definitions/401.html> (leak)

### C3 — o buffer que ninguém preencheu

`SQLDescribeCol` devolve `SQL_SUCCESS`, `SQL_SUCCESS_WITH_INFO`, `SQL_ERROR` ou
`SQL_INVALID_HANDLE`. Só nos dois primeiros o buffer `nome` foi escrito. O
conserto tem duas partes, e as duas importam: envolver a chamada em
`SQL_SUCCEEDED(...)` **e** inicializar `nome[0] = '\0'` antes, para que o caminho
de erro tenha um valor definido em vez de pilha.

- <https://learn.microsoft.com/en-us/sql/odbc/reference/syntax/sqldescribecol-function>
  — a tabela de retornos e quais argumentos ficam intactos em cada um
- <https://cwe.mitre.org/data/definitions/457.html> — CWE-457

### D — terminar a migração da posição

O desenho está certo — é o do CPython, e o comentário no arquivo cita a fonte
certa. O problema é o estado intermediário: com 3 pontos migrados de 248, quem
ler o compilador amanhã não sabe qual das duas formas é a válida.

D2 é mais que dívida. Enquanto o fallback for
`pos.linha ? pos.linha : c->linha_atual`, migrar um ponto de emissão *não muda
nada* para o nó sem linha — que é exatamente o caso do interior de f-string, o
defeito que motivou a mudança. Uma posição ausente precisa ser ausente de
verdade (herdar do *nó pai*, passado como argumento), nunca cair no ambiente.

O teste que prova o conserto já está escrito na forma de um
`raise Boom(f"erro: {e}")` cuja linha de traceback tem que ser a do `raise`.

- <https://peps.python.org/pep-0626/> — por que a posição é dado do nó e não
  estado do gerador, e o que quebra quando não é
- <https://github.com/python/cpython/blob/main/InternalDocs/compiler.md> — o
  compilador que serviu de modelo, com o `location` explícito em cada emissão

### E — suíte hermética

Um caso de teste não pode escrever no diretório de onde foi chamado. O runner já
resolve isso para o fonte principal (`mkstemp` em `/tmp`); o que falta é os casos
com caminho relativo herdarem o mesmo tratamento — `chdir` do filho para um
diretório temporário próprio, apagado no fim. Isso conserta E1 e o torna
impossível de repetir. Os quatro arquivos já versionados saem com
`git rm -r --cached __DIR__ __DOCS__`.

E2 é o impasse clássico de dois canos: ler um até o fim enquanto o outro enche.
As saídas são `poll()` nos dois descritores, ou redirecionar stderr para o mesmo
cano do stdout quando o caso não precisar separá-los.

- <https://man7.org/linux/man-pages/man7/pipe.7.html> — a capacidade de 64 KB e o
  bloqueio na escrita quando o cano enche
- <https://man7.org/linux/man-pages/man2/poll.2.html>
- <https://testing.googleblog.com/2012/10/hermetic-servers.html> — a definição de
  teste que não depende (nem mexe) no ambiente

### F — resíduo

F1 e F2 são de uma linha cada e o compilador já os aponta com
`-Wduplicated-branches` — que basta acrescentar às `CFLAGS` para que não voltem.

Em F2 há uma decisão real por baixo: se "valores demais" e "valores
insuficientes" merecem tipos de erro distintos, o ternário estava certo e o
segundo nome é que ficou faltando; se não merecem, o ternário some. Como
`catch (Tipo e)` compara por tipo, isso muda o que um script consegue tratar.

F3 é o comentário do Makefile descrevendo uma organização de arquivos que não
existe mais.
