# Binário standalone da PoolScript — zero CPython.
#
# O mesmo `poolscript_vm.c` também compila como extensão do CPython, com
# -DPS_MODULO_PYTHON (ver setup_vm.py). Aqui a flag NÃO entra: nada de
# Python.h, nada de libpython, nenhum símbolo do interpretador no binário.
CC      ?= gcc
# A versão sai da FONTE ÚNICA (src/poolscript/__init__.py) e entra como
# -DPS_VERSAO — assim o `pool --version` acompanha o bump sem um 4º lugar
# hardcoded pra dessincronizar (o main.c só tem o fallback do #ifndef).
PS_VER  := $(shell sed -n 's/^__version__ = "\(.*\)"/\1/p' src/poolscript/__init__.py)
CFLAGS  ?= -O2 -Wall -Wextra -Wno-unused-parameter -I/usr/include/postgresql -I/usr/include/mysql -DUTF8PROC_EXPORTS -I/usr/include/libmongoc-1.0 -I/usr/include/libbson-1.0
CFLAGS  += -DPS_VERSAO='"$(PS_VER)"'
VM      := vm
FONTES  := $(VM)/ps_lexer.c $(VM)/ps_ast.c $(VM)/ps_parser.c \
           $(VM)/ps_compiler.c $(VM)/ps_hash.c $(VM)/ps_regex.c $(VM)/ps_mail.c $(VM)/ps_http.c $(VM)/ps_qr.c $(VM)/ps_xlsx.c $(VM)/ps_db.c $(VM)/ps_mongo.c $(VM)/ps_jinker.c $(VM)/ps_guzer.c $(VM)/ps_pkg.c $(VM)/poolscript_vm.c $(VM)/main.c

# A sqlite entra ESTÁTICA (libsqlite3.a): o binário continua rodando em
# máquina que não tem libsqlite3.so. Ela é domínio público, sem custo de
# licença; -lpthread e -ldl são dependências dela, não nossas.
# Depende do __init__.py e do Makefile também: um bump de versão (ou mudança
# de flag) força o relink, senão o `pool --version` fica preso no valor antigo
# porque as fontes .c não mudaram.
pool: $(FONTES) src/poolscript/__init__.py Makefile
	$(CC) $(CFLAGS) -I$(VM) -o $@ $(FONTES) \
	  -L/usr/lib/postgresql/16/lib -Wl,-Bstatic -lsqlite3 -lpq -lpgcommon -lpgport -lmysqlclient -lodbc -lssl -lcrypto -lpng -lexpat -lz -Wl,-Bdynamic -lstdc++ -lzstd -lltdl -lldap -llber -lgssapi_krb5 -lmongoc-1.0 -lbson-1.0 -lrt  -lpthread -ldl -lm -l:libX11.so.6 -l:libgmp.so.10

# Bundle PORTÁTIL: pool + todas as .so numa pasta lib/, com wrapper. Roda em
# qualquer VPS x86-64 (glibc compatível) SEM apt install — mongo, gnutls, krb5,
# ldap etc. vão junto. Gera dist/pool-portable/ e dist/pool-portable.tar.gz.
bundle: pool
	./build_bundle.sh

# Confere que não sobrou nada de Python no binário.
verifica: pool
	@echo "== dependências dinâmicas =="; ldd ./pool
	@echo "== símbolos de Python (deve ser vazio) =="; \
	  nm -uD ./pool 2>/dev/null | grep -i python || echo "  nenhum"
	@echo "== tamanho =="; ls -lh ./pool | awk '{print "  " $$5}'

limpa:
	rm -f pool

.PHONY: verifica limpa
