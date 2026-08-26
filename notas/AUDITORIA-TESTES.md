# Auditoria da suíte: cobertura medida e o que falta

Medição de 2026-08-26. Nada foi alterado no repositório: o binário instrumentado
foi construído numa cópia isolada em `/tmp`, com `gcc -O0 --coverage`, e a suíte
rodou lá.

**Resposta curta:** a suíte não é rasa em quantidade — 7.822 casos, todos verdes
em 65 s. Ela é rasa em **alcance**. O núcleo da linguagem está bem coberto; a
biblioteca padrão quase não está.

---

## Estado em 2026-08-26 (depois de atacar a lista)

Os seis problemas estruturais foram atacados, e a "ordem de retorno" do fim
deste documento foi seguida na sequência proposta. Cada um virou **alvo do
Makefile**, porque ferramenta que não tem alvo ninguém roda:

| Item | Virou | O que já achou |
|---|---|---|
| **1. Fuzzer no front-end** | `make fuzz` — libFuzzer (clang) sobre `ps_verifica_fonte`, corpus semeado com os 16.461 programas que a suíte já tinha (`make semeia`) | 480.512 execuções em 3 min, **zero crash e zero vazamento**, 4.932 entradas novas de cobertura |
| **2. Injeção de falha de malloc** | `make oom` — `pool-oom` ligado com `-Wl,--wrap=malloc,calloc,realloc,strdup`; nenhuma linha do motor muda | **55 mortes violentas em 2.766 pontos**, todas corrigidas: liberação dupla no gerador, `free()` de ponteiro-lixo na Entity, `strdup(NULL)`, `memcpy` em NULL no `split`, e o realloc duplo do `ps_grade_set` |
| **3. Suíte sob ASan+UBSan** | `make check-asan` — a MESMA suíte, `PS_POOL` troca o binário | o `pool-asan` existia e nada o rodava; agora roda |
| **4. Cobertura com meta em ramos** | `make cobertura` — gcov+lcov, relatório HTML por arquivo | primeiro número de RAMO que existiu: **42,1%** (contra 55,3% de linha — a linha superestimava mesmo) |
| **5. e2e no portão** | `make check-e2e`, e o `make check` **anuncia** o que ficou de fora | gap declarado em voz alta, não escondido |
| **6. `equivalencia` como property test** | `make propriedade` — entrada SORTEADA a cada execução, semente impressa pra repetir o achado | 2.392 formas em 3 sementes, zero divergência |

**O que a injeção de malloc provou na prática:** era mesmo o item de maior
retorno. As correções da classe C eram 80% código não exercitado; hoje todos
os 2.766 pontos de alocação passam, e o contrato é explícito — *ou o programa
termina, ou levanta erro; segfault, liberação dupla e trava reprovam*.

**Defeito achado escrevendo o próprio semeador**, e este é o melhor argumento
do documento a favor de dogfooding: o casador de regex é recursivo, um quadro
de pilha C por caractere. O teto de PASSOS (2 milhões) não protegia disso — a
pilha de 8 MB acaba antes, e o `pool` **morria de SIGSEGV sem mensagem**
casando `"((?:[^"\\]|\\.)*)"` contra um trecho de 29 mil caracteres. Agora há
teto de PROFUNDIDADE (`RX_MAX_PROF`), e o mesmo caso vira
`RuntimeError: regex: backtracking demais`, com linha e coluna. Três casos de
regressão entraram na suíte.

**O que continua verdade e não foi resolvido:**

- **problema 1 (um único método de teste)** — continua não havendo teste de
  unidade em C. O dict compacto, o bignum, a arena e o `utf8_byte_de` seguem
  sem teste direto. O caminho é o do CPython (`_testcapi`): um módulo que
  exponha as internas só para teste;
- **problema 2 (88% snapshot)** — `diferencial` e `oraculo` continuam
  congelando o comportamento de hoje. O `propriedade` é o começo da saída, não
  a saída;
- **problema 3 (sete módulos a 0%)** — `check-e2e` existe, mas os módulos só
  sobem com serviço externo no ar; a cobertura deles continua zero numa
  máquina sem banco;
- **o `oraculo` não parou de crescer** — a recomendação de congelá-lo e
  investir no `equivalencia` é decisão de projeto, não foi tomada aqui.

**Limitação que o teto de profundidade expõe:** o casador não dá conta de
`(?:...)*` sobre alguns milhares de caracteres, coisa que o Python faz. Hoje
isso é erro claro em vez de morte, mas continua sendo limite real — anotado em
`notas/LIMITACOES.md`. A saída definitiva é tornar a repetição de grupo
iterativa em vez de recursiva.

---

## 1. Como reproduzir a medição

```bash
S=/tmp/cov && rm -rf $S && mkdir -p $S
cp -r vm teste Makefile $S/ && cd $S

# mesmo comando de link do Makefile, trocando -O2 por -O0 -g --coverage
gcc -O0 -g --coverage -Wall -Wno-unused-parameter \
  -I/usr/include/postgresql -I/usr/include/mysql -DUTF8PROC_EXPORTS \
  -I/usr/include/libmongoc-1.0 -I/usr/include/libbson-1.0 -Ivm -o pool \
  vm/ps_lexer.c vm/ps_ast.c vm/ps_parser.c vm/ps_compiler.c vm/ps_hash.c \
  vm/ps_regex.c vm/ps_mail.c vm/ps_http.c vm/ps_qr.c vm/ps_xlsx.c vm/ps_db.c \
  vm/ps_mongo.c vm/ps_jinker.c vm/ps_guzer.c vm/ps_pkg.c vm/poolscript_vm.c \
  vm/main.c \
  -L/usr/lib/postgresql/16/lib -Wl,-Bstatic -lsqlite3 -lpq -lpgcommon -lpgport \
  -lmysqlclient -lodbc -lssl -lcrypto -lpng -lexpat -lz -Wl,-Bdynamic \
  -lstdc++ -lzstd -lltdl -lldap -llber -lgssapi_krb5 -lmongoc-1.0 -lbson-1.0 \
  -lrt -lpthread -ldl -lm -l:libX11.so.6 -l:libgmp.so.10

gcc -O2 -w -Iteste -o testar teste/ps_teste.c teste/casos_*.c
./testar

gcov -n  pool-*.gcda                  # cobertura de linha por arquivo
gcov -b -n pool-poolscript_vm.gcda    # ramos
gcov -f -n pool-poolscript_vm.gcda    # por função
```

O build instrumentado leva ~3 min; a suíte, 65 s.

---

## 2. Cobertura medida

| Arquivo | Linhas executadas | Ramos tomados ao menos 1× |
|---|---|---|
| `vm/ps_parser.c` | 93,58% (1728) | 67,10% |
| `vm/ps_compiler.c` | 91,45% (1742) | 70,54% |
| `vm/ps_qr.c` | 85,99% (357) | — |
| `vm/ps_lexer.c` | 74,84% (477) | 67,82% |
| `vm/ps_http.c` | 69,76% (248) | — |
| `vm/ps_regex.c` | 61,80% (521) | — |
| **`vm/poolscript_vm.c`** | **52,71% (13202)** | **39,78%** |
| `vm/ps_ast.c` | 43,97% (116) | — |
| `vm/main.c` | 22,71% (361) | — |
| `vm/ps_mail.c` | 2,80% (571) | — |
| `vm/ps_xlsx.c` | 0,00% (307) | — |
| `vm/ps_pkg.c` | 0,00% (335) | — |
| `vm/ps_mongo.c` | 0,00% (106) | — |
| `vm/ps_jinker.c` | 0,00% (514) | — |
| `vm/ps_hash.c` | 0,00% (245) | — |
| `vm/ps_guzer.c` | 0,00% (221) | — |
| `vm/ps_db.c` | 0,00% (306) | — |
| **TOTAL** | **53,74% — 11.477 de 21.357** | |

Duas leituras importantes:

- **Linha superestima.** No parser, 93,58% de linha são 67,10% de ramos tomados.
  No interpretador, 52,71% de linha são **39,78% de ramos**: seis em cada dez
  desvios do motor nunca foram exercitados em nenhuma direção.
- **2.034 linhas executáveis a zero.** Sete módulos (`db`, `jinker`, `guzer`,
  `hash`, `mongo`, `pkg`, `xlsx`) nunca executam uma linha sequer, e `mail` fica
  em 2,80%. Eles vivem em `teste/e2e/`, que o `make check` não roda.

### Por função, dentro da VM

**415 das 947 funções (43,8%) nunca são chamadas por nenhum dos 7.822 casos.**

| Categoria | Mortas / total | |
|---|---|---|
| `mod_*` — API dos módulos | 158 / 206 | **77%** |
| `met_*` — métodos de objeto | 89 / 225 | 40% |
| `fin_*` — finalizadores do GC | 18 / 42 | 43% |
| `nativa_*` — builtins | 2 / 35 | 6% |

O perfil fica nítido: o núcleo da linguagem é sólido (94% dos builtins
exercitados), e a **biblioteca padrão é quase inteiramente descoberta**. Os
finalizadores recém-refatorados estão em 43% sem nenhum teste que os dispare.

---

## 3. O número que mais dói

Com a instrumentação zerada, rodando a suíte **sem** o grupo `oraculo`:

```
poolscript_vm.c   Lines executed:52.64% of 13202
```

Acrescentando de volta os 4.483 casos do `oraculo`:

```
poolscript_vm.c   Lines executed:52.71% of 13202
```

**57% da suíte inteira acrescenta 10 linhas de 13.202.**

O `oraculo` é produto cartesiano de receptor × método × argumento, e o produto
repassa o mesmo caminho de código milhares de vezes. É custo de manutenção e de
tempo de execução sem retorno de detecção. O mesmo vale, em menor grau, para o
`diferencial`.

### Composição da suíte

| Grupo | Casos | Natureza |
|---|---|---|
| `oraculo` | 4.483 | snapshot gerado (Python como oráculo na geração) |
| `diferencial` | 2.408 | snapshot da própria VM no tempo |
| `cobertura` | 483 | gerado do corpus antigo |
| `linguagem` | 189 | escrito à mão, semântica |
| `equivalencia` | 148 | **metamorphic** — gerado |
| `pendentes` | 29 | regressão (fila hoje vazia) |
| `inteiros` | 29 | bignum |
| `erros` | 28 | erro que tem que aparecer |
| `robustez` | 14 | gerado — aridade/tipo errado |
| `crash` | 11 | o que já matou o processo |
| **total** | **7.822** | **88% é snapshot** |

---

## 4. Seis problemas estruturais

**1 · Um único método de teste.** Os 7.822 casos são a mesma coisa: escreve `.ps`
em `/tmp`, roda `./pool` em subprocesso, compara stdout/stderr como texto. Não
existe um único teste de unidade em C. O dict compacto, o bignum, a arena,
`utf8_byte_de`, o GC — nenhum é testado diretamente; só se algum programa `.ps`
passar por ali por acaso. É a causa direta dos `fin_*` a 43%.

**2 · 88% da suíte é snapshot, não oráculo.** `diferencial` e `oraculo` congelam
o comportamento de hoje. O cabeçalho do `oraculo` admite: "caso marcado DIVERGE
tem o comportamento do Python no comentário". O snapshot trava o comportamento
errado com a mesma força que trava o certo.

**3 · Sete módulos a 0%.** Estão só em `teste/e2e/`, fora do `make check`.

**4 · Nada gera entrada nova.** Todos os casos são fixos. Zero fuzzing, zero
property-based. Um compilador em C sem fuzzer no parser é o caso de manual.

**5 · Nenhum portão dinâmico.** `pool-asan` existe e nada o roda; a suíte roda o
binário `-O2`, que é exatamente o que esconde os achados C1–C6 de
`AUDITORIA.md`. Cobertura nunca havia sido medida — os números acima são os
primeiros.

**6 · `casos_equivalencia` é a melhor ideia da suíte e está parada.** 148 casos
estáticos. Isso é *metamorphic testing*, a técnica certa para uma linguagem sem
segundo motor: se `for each` e `while` dão respostas diferentes para a mesma
conta, uma das duas está quebrada, e não interessa o que o Python faria. Deveria
gerar entradas novas a cada execução, não 148 exemplos congelados.

---

## 5. Como as grandes resolvem

### SQLite — o análogo direto

C, embarcado, poucos autores, muito código. As duas técnicas dele atacam
exatamente as classes achadas em `AUDITORIA.md`:

- a métrica é **MC-DC 100%**, não cobertura de linha (aqui, 93% de linha no
  parser são 67% de ramos tomados — linha engana);
- **injeção de falha de malloc**: a suíte roda cada teste N vezes fazendo a
  i-ésima alocação falhar. Fecharia C1, C2, C4 e C5 sozinha;
- **injeção de falha de I/O**: o mesmo para escrita e disco cheio. Fecharia
  B1–B4.

<https://www.sqlite.org/testing.html>

### CPython

`_testcapi` / `_testinternalcapi`: módulos C que expõem as APIs internas só para
teste — a resposta ao problema 1. `regrtest -R 3:3` roda cada teste 4× comparando
contagem de referências para caçar vazamento. Hypothesis (property-based) para o
`re` e para o repr de float.

<https://devguide.python.org/testing/run-write-tests/> ·
<https://hypothesis.readthedocs.io/en/latest/>

### V8 / SpiderMonkey

**Fuzzilli** gera programas JS *sintaticamente válidos* mutando uma IR, guiado
por cobertura, em vez de bytes aleatórios. E o *correctness fuzzing* roda o mesmo
programa com JIT ligado e desligado, comparando o resultado — diferencial de
verdade, não snapshot. Test262 é a suíte de conformidade da linguagem, separada
do motor, que todos os motores rodam.

<https://github.com/googleprojectzero/fuzzilli> · <https://github.com/tc39/test262>

### Rust

`compiletest`: o teste é o próprio `.rs` com `//~ ERROR` anotado **na linha
exata**, e o snapshot vive num `.stderr` versionado, regravável com `--bless`.
Muito mais preciso que o `strstr(erro, esperado)` do runner atual, que aceita
qualquer erro que contenha a substring — inclusive o erro errado.

<https://rustc-dev-guide.rust-lang.org/tests/ui.html>

### Go

Fuzzing na própria linguagem desde a 1.18: quando o fuzzer acha uma falha, ele
grava o caso em `testdata/fuzz` **como teste de regressão, automaticamente**. O
corpus cresce sozinho e vai pro controle de versão.

<https://go.dev/doc/security/fuzz/>

### LLVM / Clang

`FileCheck` verifica saída **estruturada** embutida no fonte, não igualdade de
texto inteiro. Csmith gera programas C aleatórios válidos e sem UB para teste
diferencial entre compiladores.

<https://llvm.org/docs/CommandGuide/FileCheck.html> ·
<https://github.com/csmith-project/csmith>

---

## 6. Ordem de retorno

1. **Fuzzer no front-end.** O harness já existe: `pool --check` faz parse +
   compile sem executar. libFuzzer ou AFL++ com o corpus semeado pelos 7.822
   programas que já estão escritos. Maior retorno por hora investida, e é onde a
   cobertura já é alta — o que sobra ali são os caminhos que só entrada
   malformada alcança.
   <https://llvm.org/docs/LibFuzzer.html> · <https://github.com/google/AFL>
2. **Injeção de falha de malloc** (`PS_TESTE_MALLOC_FALHA=N`). Fecha C1–C6 de
   `AUDITORIA.md` e prova que as mensagens de "sem memoria" funcionam — hoje
   nenhuma delas jamais executou.
3. **Suíte sob ASan + UBSan dentro do `check`.** 65 s viram uns 4 min. O
   `pool-asan` já está pronto; falta rodá-lo.
4. **`make cobertura`, com meta em ramos tomados.** Sem métrica não há direção, e
   os números deste arquivo são os primeiros que existiram.
5. **e2e no portão.** É o que tira os sete módulos de 0%.
6. **Parar de crescer o `oraculo`** e investir o esforço no `equivalencia` como
   property test com entradas geradas a cada execução.

Referências conferidas (HTTP 200) em 2026-08-26.
