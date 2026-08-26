# Binário standalone da PoolScript — tudo em C.
#
# Os clientes de banco (sqlite, libpq, mysqlclient, unixODBC), o TLS, o PNG e
# o zlib entram ESTÁTICOS: o `pool` roda em máquina que não tem nenhum deles.
CC      ?= gcc
# A versão é constante de header (vm/ps_versao.h) — o compilador resolve, sem
# shell nenhum no meio.
CFLAGS  ?= -O2 -Wall -Wextra -Wno-unused-parameter -I/usr/include/postgresql -I/usr/include/mysql -DUTF8PROC_EXPORTS -I/usr/include/libmongoc-1.0 -I/usr/include/libbson-1.0
VM      := vm
FONTES  := $(VM)/ps_lexer.c $(VM)/ps_ast.c $(VM)/ps_parser.c \
           $(VM)/ps_compiler.c $(VM)/ps_hash.c $(VM)/ps_regex.c $(VM)/ps_mail.c $(VM)/ps_http.c $(VM)/ps_qr.c $(VM)/ps_xlsx.c $(VM)/ps_db.c $(VM)/ps_mongo.c $(VM)/ps_jinker.c $(VM)/ps_guzer.c $(VM)/ps_pkg.c $(VM)/poolscript_vm.c $(VM)/main.c

# A sqlite entra ESTÁTICA (libsqlite3.a): o binário continua rodando em
# máquina que não tem libsqlite3.so. Ela é domínio público, sem custo de
# licença; -lpthread e -ldl são dependências dela, não nossas.
# Depende do header de versão e do Makefile: um bump (ou mudança de flag)
# força o relink, senão o `pool --version` fica preso no valor antigo porque
# as fontes .c não mudaram.
# O Makefile mora em rebuild/, mas as fontes e o binário são da RAIZ do
# repositório: rode sempre `make -f rebuild/Makefile <alvo>` de lá.
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
	install -m644 dados/icones/text-x-poolscript.svg \
	        $(DADOS)/icons/hicolor/scalable/mimetypes/
	-update-mime-database $(DADOS)/mime 2>/dev/null || true
	-gtk-update-icon-cache -f -t $(DADOS)/icons/hicolor 2>/dev/null || true

.PHONY: install install-mime desinstala

desinstala:
	rm -f $(PREFIXO)/bin/pool $(PREFIXO)/bin/psl $(PREFIXO)/bin/poolscript-lsp
	rm -rf $(PREFIXO)/share/poolscript
	rm -f $(DADOS)/mime/packages/zz-poolscript.xml
	rm -f $(DADOS)/icons/hicolor/scalable/mimetypes/text-x-poolscript.svg
	-update-mime-database $(DADOS)/mime 2>/dev/null || true
	-gtk-update-icon-cache -f -t $(DADOS)/icons/hicolor 2>/dev/null || true

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
analisa:
	@$(CC) $(CFLAGS) -I$(VM) -fanalyzer -fsyntax-only $(FONTES) 2>&1 \
	  | grep -E "warning|error" || echo "  -fanalyzer: nada"

.PHONY: analisa

check: pool testar
	./testar
	@echo
	@./pool teste/confere_metadata.ps
	@echo
	@./pool lsp/teste_lsp.ps
