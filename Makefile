# Binário standalone da PoolScript — tudo em C.
#
# Os clientes de banco (sqlite, libpq, mysqlclient, unixODBC), o TLS, o PNG e
# o zlib entram ESTÁTICOS: o `pool` roda em máquina que não tem nenhum deles.
CC      ?= gcc
# A versão é constante de header (vm/ps_versao.h) — o compilador resolve, sem
# shell nenhum no meio.
# `-Wduplicated-branches` não vem no `-Wall -Wextra` e pegou dois defeitos
# reais de uma vez: um `if` com os dois ramos iguais no lexer e um ternário
# escolhendo entre dois nomes de tipo IDÊNTICOS no desempacotamento — este
# último parecia distinguir "valores demais" de "insuficientes" pra quem faz
# `catch (Tipo e)`, e não distinguia. Zero falso positivo aqui, então entra no
# build de todo dia.
#
# `-Wlogical-op` e `-Wformat-truncation=2` NÃO entram: o primeiro acusa
# `errno == EAGAIN || errno == EWOULDBLOCK` (que no Linux são o mesmo valor, e
# escrever os dois é o idioma portável), e o segundo acusa ~20 truncamentos
# deliberados (`%.200s`). Ruído nesse volume esconde o aviso de verdade. Os
# dois ficam no alvo `avisos`, pra revisão de propósito.
# As flags são separadas do NÍVEL DE OTIMIZAÇÃO de propósito.
#
# O alvo `cobertura` fazia `-O0 -g --coverage $(CFLAGS)`, e o `-O2` de dentro do
# CFLAGS vinha DEPOIS: o gcc obedece o último, então a medição inteira rodava
# otimizada. Instrumentação em -O2 embaralha atribuição de linha e ramo — o
# número saía, mas não era o que o alvo dizia estar medindo.
CFLAGS_BASE ?= -Wall -Wextra -Wno-unused-parameter -Wduplicated-branches \
           -I/usr/include/postgresql -I/usr/include/mysql -DUTF8PROC_EXPORTS -I/usr/include/libmongoc-1.0 -I/usr/include/libbson-1.0
CFLAGS  ?= -O2 $(CFLAGS_BASE)
VM      := vm
FONTES  := $(VM)/ps_lexer.c $(VM)/ps_ast.c $(VM)/ps_parser.c \
           $(VM)/ps_compiler.c $(VM)/ps_hash.c $(VM)/ps_regex.c $(VM)/ps_mail.c $(VM)/ps_http.c $(VM)/ps_qr.c $(VM)/ps_xlsx.c $(VM)/ps_db.c $(VM)/ps_mongo.c $(VM)/ps_jinker.c $(VM)/ps_guzer.c $(VM)/ps_pkg.c $(VM)/poolscript_vm.c $(VM)/main.c

# A sqlite entra ESTÁTICA (libsqlite3.a): o binário continua rodando em
# máquina que não tem libsqlite3.so. Ela é domínio público, sem custo de
# licença; -lpthread e -ldl são dependências dela, não nossas.
# Depende do header de versão e do Makefile: um bump (ou mudança de flag)
# força o relink, senão o `pool --version` fica preso no valor antigo porque
# as fontes .c não mudaram.
# Makefile, fontes e binário ficam todos na RAIZ do repositório: `make <alvo>`
# de lá, sem `-f`.
MK := $(lastword $(MAKEFILE_LIST))

pool: $(FONTES) $(VM)/ps_versao.h $(MK)
	$(CC) $(CFLAGS) -I$(VM) -o $@ $(FONTES) \
	  -L/usr/lib/postgresql/16/lib -Wl,-Bstatic -lsqlite3 -lpq -lpgcommon -lpgport -lmysqlclient -lodbc -lssl -lcrypto -lpng -lexpat -lz -Wl,-Bdynamic -lstdc++ -lzstd -lltdl -lldap -llber -lgssapi_krb5 -lmongoc-1.0 -lbson-1.0 -lrt  -lpthread -ldl -lm -l:libX11.so.6 -l:libgmp.so.10

# Bundle PORTÁTIL: pool + todas as .so numa pasta lib/, com wrapper. Roda em
# qualquer VPS x86-64 (glibc compatível) SEM apt install — mongo, gnutls, krb5,
# ldap etc. vão junto. Gera dist/pool-portable/ e dist/pool-portable.tar.gz.
bundle: pool
	./build_bundle.sh

# Instala no sistema: o binário (como `pool` e `psl`, que são o mesmo) e o
# servidor LSP, que é PoolScript e por isso precisa dos .ps ao lado. O
# `poolscript-lsp` é o atalho que o editor chama.
PREFIXO ?= /usr/local
install: pool
	install -d $(PREFIXO)/bin $(PREFIXO)/share/poolscript/lsp
	install -m755 pool $(PREFIXO)/bin/pool
	install -m755 pool $(PREFIXO)/bin/psl
	install -m644 lsp/protocolo.ps lsp/modelo.ps lsp/servidor.ps \
	        $(PREFIXO)/share/poolscript/lsp/
	printf '#!/bin/sh\n# Atalho do servidor LSP. O servidor e PoolScript; ver docs/lsp.md.\nexec %s/bin/pool %s/share/poolscript/lsp/servidor.ps "$$@"\n' \
	        '$(PREFIXO)' '$(PREFIXO)' > $(PREFIXO)/bin/poolscript-lsp
	chmod 755 $(PREFIXO)/bin/poolscript-lsp
	@$(MAKE) --no-print-directory install-mime PREFIXO=$(PREFIXO)
	@echo "instalado em $(PREFIXO): pool, psl, poolscript-lsp, tipo MIME e icone"

# Tipo MIME + ícone do `.ps` pro desktop (GNOME/KDE/XFCE/…). Fica separado
# porque num servidor sem ambiente gráfico ele não faz falta e as ferramentas
# (`update-mime-database`) podem nem existir — daí o `|| true`.
# A base de MIME é /usr/share por padrão, NÃO $(PREFIXO): o `glob-deleteall`
# que tira o `.ps` do PostScript só tem efeito dentro da MESMA base onde o
# PostScript está definido. Instalar em /usr/local/share deixaria as duas
# definições convivendo e o PostScript ganharia pela magic.
DADOS ?= /usr/share
install-mime:
	install -d $(DADOS)/mime/packages \
	           $(DADOS)/icons/hicolor/scalable/mimetypes
	install -m644 dados/zz-poolscript.xml $(DADOS)/mime/packages/
	install -m644 dados/icones/text-poolscript.svg \
	        $(DADOS)/icons/hicolor/scalable/mimetypes/
	-update-mime-database $(DADOS)/mime 2>/dev/null || true
	@./dados/espalha_icone.sh "$(DADOS)" instalar

.PHONY: install install-mime desinstala

desinstala:
	rm -f $(PREFIXO)/bin/pool $(PREFIXO)/bin/psl $(PREFIXO)/bin/poolscript-lsp
	rm -rf $(PREFIXO)/share/poolscript
	rm -f $(DADOS)/mime/packages/zz-poolscript.xml
	-update-mime-database $(DADOS)/mime 2>/dev/null || true
	-./dados/espalha_icone.sh "$(DADOS)" remover

# Confere que não sobrou nada de Python no binário.
verifica: pool
	@echo "== dependências dinâmicas =="; ldd ./pool
	@echo "== símbolos de Python (deve ser vazio) =="; \
	  nm -uD ./pool 2>/dev/null | grep -i python || echo "  nenhum"
	@echo "== tamanho =="; ls -lh ./pool | awk '{print "  " $$5}'

limpa:
	rm -f pool

.PHONY: verifica limpa

# ── suíte de testes (em C, sem Python) ──────────────────────────────────────
# Roda o `pool` de VERDADE em subprocesso, um por caso: caso que mata a VM
# (segfault/SIGFPE) vira falha relatada em vez de derrubar a bateria.
TESTE_FONTES := teste/ps_teste.c teste/casos_crash.c teste/casos_inteiros.c \
                teste/casos_erros.c teste/casos_linguagem.c \
                teste/casos_pendentes.c teste/casos_cobertura.c \
                teste/casos_diferencial.c teste/casos_equivalencia.c \
                teste/casos_oraculo.c teste/casos_robustez.c

# o binário se chama `testar` porque `teste` é a PASTA dos casos
testar: $(TESTE_FONTES) teste/ps_teste.h
	$(CC) -O2 -Wall -Wextra -Wno-unused-parameter -Wno-missing-field-initializers -Iteste -o $@ $(TESTE_FONTES)

# `make check` = compila os dois e roda a suíte
# ── ferramentas prontas: sanitizers e analisador estático ───────────────────
# Não é caso escrito à mão: o ASan/UBSan instrumenta CADA acesso de memória e
# CADA operação com comportamento indefinido, e para no ponto exato. Acha a
# classe inteira (estouro de buffer, uso após liberar, shift inválido, overflow
# de signed) sem ninguém adivinhar o caso.
pool-asan: $(FONTES) $(VM)/ps_versao.h $(MK)
	$(CC) $(CFLAGS) -g -fsanitize=address,undefined -fno-omit-frame-pointer \
	  -I$(VM) -o $@ $(FONTES) \
	  -L/usr/lib/postgresql/16/lib -lsqlite3 -lpq -lmysqlclient -lodbc -lssl \
	  -lcrypto -lpng -lexpat -lz -lstdc++ -lzstd -lltdl -lldap -llber \
	  -lgssapi_krb5 -lmongoc-1.0 -lbson-1.0 -lrt -lpthread -ldl -lm \
	  -l:libX11.so.6 -l:libgmp.so.10

# Análise estática do gcc: caminho de execução simbólico, acha vazamento,
# desreferência de NULL e uso de não-inicializado sem rodar o programa.
#
# `-fsyntax-only` NÃO serve aqui: o gcc para antes do GIMPLE e o analisador
# nunca chega a rodar — o alvo imprimia "nada" acontecesse o que acontecesse.
# Tem que compilar de verdade, jogando o objeto fora.
#
# Um arquivo por invocação, e não todos de uma vez: o `poolscript_vm.c` sozinho
# esgota a memória desta máquina quando analisado junto com os outros.
# TETO DE MEMÓRIA, e ele não é opcional. Esta máquina tem 7,7 GB e costuma
# estar com ~3,5 GB livres; o `-fanalyzer` num fonte grande passa de 6 GB
# sozinho e o OOM killer derruba a sessão inteira, não só o gcc. O `ulimit -v`
# faz o GCC desistir e reportar, em vez de levar a máquina junto.
#
# `poolscript_vm.c` (21.908 linhas) fica FORA por padrão pelo mesmo motivo: ele
# nunca terminou aqui. Ele é relatado como não coberto — em voz alta, porque
# omitir isso seria dizer "limpo" sobre metade do motor. Pra rodar mesmo assim,
# numa máquina que aguente:
#
#     make analisa ANALISA_TUDO=1 ANALISA_MB=12000
ANALISA_MB   ?= 2000
ANALISA_TUDO ?=
ANALISA_FORA := $(VM)/poolscript_vm.c

# Falso positivo CONFERIDO, um por linha, com o motivo. O `-fanalyzer` não
# segue posse através de struct nem de parâmetro de saída, então acusa como
# vazamento o ponteiro que o dono libera em outro lugar. Cada entrada aqui foi
# verificada lendo o código; a alternativa (sair 0 pra tudo) é o portão que não
# reprova, que foi o que este alvo já era uma vez.
#
#   ps_xlsx.c:345   `o->b` do Out — liberado por quem monta o ZIP (free(zip.b))
#   ps_db.c:75      `res->cols[i]` — liberado em ps_db_res_libera:31
ANALISA_ACEITOS := ps_xlsx.c:345 ps_db.c:75
ANALISA_ALVO := $(if $(ANALISA_TUDO),$(FONTES),$(filter-out $(ANALISA_FORA),$(FONTES)))

analisa:
	@achou=0; faltou=""; \
	for f in $(ANALISA_ALVO); do \
	  saida=$$( (ulimit -v $$(($(ANALISA_MB) * 1024)); \
	             nice -n 19 $(CC) $(CFLAGS) -I$(VM) -fanalyzer -c -o /dev/null $$f) 2>&1 ); \
	  rc=$$?; \
	  aviso=$$(printf '%s\n' "$$saida" | grep -E "warning:|error:" | grep -v "^cc1"); \
	  for ac in $(ANALISA_ACEITOS); do \
	    aviso=$$(printf '%s\n' "$$aviso" | grep -v "$$ac" || true); \
	  done; \
	  aviso=$$(printf '%s\n' "$$aviso" | grep -v '^$$' || true); \
	  if [ -n "$$aviso" ]; then achou=1; printf '%s\n' "$$aviso"; fi; \
	  if [ $$rc -ne 0 ] && [ -z "$$aviso" ]; then faltou="$$faltou $$f"; fi; \
	done; \
	if [ $$achou -eq 0 ]; then echo "  -fanalyzer: nada nos analisados"; fi; \
	if [ -n "$$faltou" ]; then \
	  echo "  NAO TERMINARAM (teto de $(ANALISA_MB) MB):$$faltou"; fi; \
	$(if $(ANALISA_TUDO),,echo "  FORA por padrao (grande demais p/ esta maquina): $(ANALISA_FORA)"); \
	if [ $$achou -ne 0 ]; then \
	  echo; \
	  echo "  Achado do analisador REPROVA. Falso positivo conferido vai pra"; \
	  echo "  ANALISA_ACEITOS (com o motivo escrito), nunca ignorado no silencio."; \
	  exit 1; \
	fi

# Valgrind: o que o ASan não pega — leitura de não-inicializado e o mapa de
# vazamento com a pilha de quem alocou. Roda UM script por vez; é ~30x mais
# lento que nativo, então não entra no `check`.
#
#     make memcheck PS=teste/e2e/ws.ps
PS ?= /dev/null
memcheck: pool
	valgrind --leak-check=full --show-leak-kinds=definite,indirect \
	         --track-origins=yes --error-exitcode=1 ./pool $(PS)

.PHONY: memcheck

# Avisos barulhentos, pra revisão deliberada — não entram no build de todo dia
# porque o volume de truncamento intencional esconderia o achado de verdade.
# Só compila (`-fsyntax-only` basta: aqui não há analisador envolvido).
avisos:
	@$(CC) $(CFLAGS) -Wlogical-op -Wformat-truncation=2 -Wshadow \
	  -I$(VM) -fsyntax-only $(FONTES) 2>&1 | grep -E "warning:" | sort -u

.PHONY: avisos

.PHONY: analisa

check: pool testar
	./testar
	@echo
	@./pool teste/confere_metadata.ps
	@echo
	@./pool scripts/audita_doc.ps
	@echo
	@./pool scripts/audita_c.ps
	@echo
	@./pool lsp/teste_lsp.ps
	@echo
	@$(MAKE) --no-print-directory analisa
	@echo
	@echo "FORA deste portao, e cada um tem alvo proprio porque e caro:"
	@echo "  make check-e2e   banco/mail/socket/jinker/guzer/mongo/qr (servico externo)"
	@echo "  make check-asan  a MESMA suite sob ASan+UBSan (~4 min)"
	@echo "  make oom         falha de alocacao ponto a ponto (~5 min)"
	@echo "  make fuzz        fuzzer no front-end (FUZZ_T=<segundos>)"
	@echo "  make cobertura   linha E RAMO, por arquivo"

# E2E: cada script sobe o que precisa e checa de ponta a ponta. Fica fora do
# `check` porque depende de serviço externo (Postgres, MySQL, mongod, SMTP) e
# porque é pesado.
#
# A ORDEM IMPORTA, e por isso não é mais um `for` sobre `*.ps`: o shell ordena
# alfabeticamente e `jinker_cli.ps` vinha ANTES de `jinker_srv.ps` — o cliente
# subia sem servidor, morria com "Connection refused", e o servidor ficava
# servindo até o timeout. O driver conhece o papel de cada script (par,
# sozinho, servidor sem cliente), espera a porta ABRIR em vez de dormir no
# escuro, e mata o servidor aconteça o que acontecer.
#
#     make check-e2e            # tudo
#     make check-e2e E2E=jinker # só o que casa com o filtro
E2E ?=
check-e2e: pool
	@./pool teste/e2e_roda.ps $(E2E)

.PHONY: check-e2e

# ── injeção de falha de alocação (a técnica do SQLite) ──────────────────────
# Toda correção da classe C da auditoria é um caminho de falta de memória, e
# NENHUM jamais executou: o gcov mostrava `if (!mn)` avaliado 42x com o ramo
# verdadeiro nunca tomado. O Linux dá /dev/full de graça e por isso o I/O pôde
# ser testado; memória não tem equivalente, então a gente fabrica.
#
# `--wrap` é do linker: nenhuma linha do motor muda, o código testado é o
# código de produção. Ver teste/ps_oom.c.
pool-oom: $(FONTES) teste/ps_oom.c $(VM)/ps_versao.h $(MK)
	$(CC) $(CFLAGS) -g -I$(VM) -o $@ $(FONTES) teste/ps_oom.c \
	  -Wl,--wrap=malloc,--wrap=calloc,--wrap=realloc,--wrap=strdup \
	  -L/usr/lib/postgresql/16/lib -Wl,-Bstatic -lsqlite3 -lpq -lpgcommon \
	  -lpgport -lmysqlclient -lodbc -lssl -lcrypto -lpng -lexpat -lz \
	  -Wl,-Bdynamic -lstdc++ -lzstd -lltdl -lldap -llber -lgssapi_krb5 \
	  -lmongoc-1.0 -lbson-1.0 -lrt -lpthread -ldl -lm \
	  -l:libX11.so.6 -l:libgmp.so.10

# Falha a i-ésima alocação de cada programa de teste/oom_varre.ps. Passar não é
# "não deu erro": é "morreu limpo" — segfault, liberação dupla e trava reprovam.
oom: pool-oom
	@./pool teste/oom_varre.ps

.PHONY: oom

# ── fuzzer no front-end (libFuzzer, clang) ──────────────────────────────────
# A suíte é 100% de entradas FIXAS: nada gera entrada nova. A libFuzzer muta
# guiada por COBERTURA — vê que ramos cada entrada alcançou e prioriza as que
# abrem caminho novo. Roda só o `--check` (lexer+parser+compilador), sem
# executar bytecode: é o caminho onde entrada torta faz estrago.
#
# Achado vira arquivo `crash-*`, que JÁ É o caso de regressão (modelo do Go).
FUZZ_FONTES := $(filter-out $(VM)/main.c,$(FONTES))
FUZZ_T ?= 120
FUZZ_CORPUS := teste/fuzz_corpus

pool-fuzz: $(FUZZ_FONTES) teste/ps_fuzz.c $(VM)/ps_versao.h $(MK)
	clang -O1 -g -fsanitize=fuzzer,address,undefined -fno-omit-frame-pointer \
	  -Wno-everything -I$(VM) -I/usr/include/postgresql -I/usr/include/mysql \
	  -DUTF8PROC_EXPORTS -I/usr/include/libmongoc-1.0 -I/usr/include/libbson-1.0 \
	  -o $@ $(FUZZ_FONTES) teste/ps_fuzz.c \
	  -L/usr/lib/postgresql/16/lib -lsqlite3 -lpq -lmysqlclient -lodbc -lssl \
	  -lcrypto -lpng -lexpat -lz -lstdc++ -lzstd -lltdl -lldap -llber \
	  -lgssapi_krb5 -lmongoc-1.0 -lbson-1.0 -lrt -lpthread -ldl -lm \
	  -l:libX11.so.6 -l:libgmp.so.10

# Semeia o corpus com os programas que a suíte já tem: o fuzzer parte de
# entrada VÁLIDA e muta a partir dela, em vez de descobrir a sintaxe do zero.
semeia: pool
	@./pool teste/fuzz_semeia.ps

fuzz: pool-fuzz semeia
	@mkdir -p teste/fuzz_achados
	@echo "fuzzando por $(FUZZ_T)s (ajuste com FUZZ_T=<segundos>)"
	@ASAN_OPTIONS=detect_leaks=1 ./pool-fuzz $(FUZZ_CORPUS) \
	  -max_total_time=$(FUZZ_T) -max_len=65536 -timeout=10 \
	  -artifact_prefix=teste/fuzz_achados/ -print_final_stats=1

.PHONY: fuzz semeia

# ── teste de propriedade (metamorphic), com entrada NOVA a cada execução ────
# `casos_equivalencia` é a melhor ideia da suíte e estava congelada em 148
# exemplos. Congelado não acha nada novo.
PROP_N ?= 200
PROP_SEMENTE ?= 1
propriedade: pool
	@./pool teste/propriedade.ps $(PROP_N) $(PROP_SEMENTE)

.PHONY: propriedade

# ── a MESMA suíte, sob ASan+UBSan ───────────────────────────────────────────
# A suíte roda o binário -O2, que é exatamente o que ESCONDE a classe de
# defeito de memória: leitura fora de faixa em -O2 costuma "funcionar". O
# `pool-asan` já existia e nada o rodava — este alvo é a ligação que faltava.
check-asan: pool-asan testar
	@echo "suite inteira sob AddressSanitizer + UBSan…"
	@PS_POOL=pool-asan ASAN_OPTIONS=detect_leaks=0:abort_on_error=0 \
	  UBSAN_OPTIONS=print_stacktrace=1 nice -n 19 ./testar

.PHONY: check-asan

# ── cobertura ───────────────────────────────────────────────────────────────
# A auditoria de testes apontou que cobertura NUNCA tinha sido medida. E a
# métrica que importa é RAMO TOMADO, não linha: 93% de linha no parser eram 67%
# de ramos. Linha superestima — é a razão de o SQLite medir MC-DC.
cobertura: testar
	@rm -rf cob && mkdir -p cob
	$(CC) -O0 -g --coverage $(CFLAGS_BASE) -I$(VM) -o cob/pool $(FONTES) \
	  -L/usr/lib/postgresql/16/lib -lsqlite3 -lpq -lmysqlclient -lodbc -lssl \
	  -lcrypto -lpng -lexpat -lz -lstdc++ -lzstd -lltdl -lldap -llber \
	  -lgssapi_krb5 -lmongoc-1.0 -lbson-1.0 -lrt -lpthread -ldl -lm \
	  -l:libX11.so.6 -l:libgmp.so.10
	@echo "rodando a suite contra o binario instrumentado…"
	@PS_POOL=cob/pool nice -n 19 ./testar 2>&1 | tail -2
	@lcov --capture --directory . --output-file cob/bruto.info \
	  --rc branch_coverage=1 --ignore-errors mismatch,source,empty >/dev/null 2>&1
	@lcov --extract cob/bruto.info "*/vm/*" --output-file cob/vm.info \
	  --rc branch_coverage=1 --ignore-errors empty >/dev/null 2>&1
	@genhtml cob/vm.info --output-directory cob/html --branch-coverage \
	  >/dev/null 2>&1 || true
	@echo; lcov --summary cob/vm.info --rc branch_coverage=1 2>/dev/null | tail -6
	@echo; echo "detalhe por arquivo:  cob/html/index.html"

.PHONY: cobertura
