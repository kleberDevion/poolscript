# Binário standalone da Jinga — tudo em C.
#
# Os clientes de banco (sqlite, libpq, MariaDB Connector/C, unixODBC), o TLS, o PNG e
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
           -I/usr/include/postgresql -I/usr/include/mariadb -DUTF8PROC_EXPORTS -I/usr/include/libmongoc-1.0 -I/usr/include/libbson-1.0
# O laço da VM despacha por goto calculado. O GCC junta todos os gotos num
# salto indireto só e depois os duplica de volta em cada tratador — mas só
# quando a cauda de despacho cabe em `max-goto-duplication-insns` (8 de
# fábrica). Com os blocos do código de máquina no laço a cauda passou de 8,
# 52 sítios viraram 2 e o interpretador ficou 16% mais lento em instruções
# (medido com perf stat). O clang não tem o parâmetro (e o fuzz usa flags
# próprias).
ifeq ($(findstring clang,$(CC)),)
CFLAGS_BASE += --param=max-goto-duplication-insns=64
endif
CFLAGS  ?= -O2 $(CFLAGS_BASE)

# O QUE O `pool` LIGA. Os clientes de banco — Postgres, MySQL, ODBC e Mongo —
# NÃO entram: cada driver abre a biblioteca dele no primeiro `connect`
# (vm/ps_dl.h). Ligados, eles e o que arrastam (Kerberos, LDAP, gnutls, SASL:
# 36 bibliotecas) eram carregados na partida de TODO programa — medido com
# callgrind, 90% do trabalho de um `pool --version` era o carregador dinâmico
# relocando essas bibliotecas. Só carrega a biblioteca que o programa usa.
# Os cabeçalhos continuam no CFLAGS: o código compila contra eles, só não liga.
#
# O sqlite, o TLS, o PNG, o XML e o zlib entram ESTÁTICOS no `pool`: o binário
# roda em máquina sem as .so deles, e estático não custa nada na partida. O
# gmp (inteiro grande) é da linguagem, não de um módulo.
LIBS_SISTEMA   := -lrt -lpthread -ldl -lm -l:libgmp.so.10
LIBS_POOL      := -Wl,-Bstatic -lsqlite3 -lssl -lcrypto -lpng -lexpat -lz -Wl,-Bdynamic $(LIBS_SISTEMA)
# as variantes de teste (asan, debug, fuzz, oom, cobertura) ligam dinâmico
LIBS_TESTE     := -lsqlite3 -lssl -lcrypto -lpng -lexpat -lz $(LIBS_SISTEMA)
VM      := vm
FONTES  := $(VM)/ps_lexer.c $(VM)/ps_ast.c $(VM)/ps_parser.c \
           $(VM)/ps_compiler.c $(VM)/ps_pilha.c $(VM)/ps_hash.c $(VM)/ps_regex.c $(VM)/ps_mail.c $(VM)/ps_http.c $(VM)/ps_qr.c $(VM)/ps_xlsx.c $(VM)/ps_db.c $(VM)/ps_mongo.c $(VM)/ps_jinker.c $(VM)/ps_pkg.c $(VM)/ps_debug.c $(VM)/ps_retornos.c $(VM)/jinga_vm.c $(VM)/main.c

# A sqlite entra ESTÁTICA (libsqlite3.a): o binário continua rodando em
# máquina que não tem libsqlite3.so. Ela é domínio público, sem custo de
# licença; -lpthread e -ldl são dependências dela, não nossas.
# Depende do header de versão e do Makefile: um bump (ou mudança de flag)
# força o relink, senão o `pool --version` fica preso no valor antigo porque
# as fontes .c não mudaram.
# Makefile, fontes e binário ficam todos na RAIZ do repositório: `make <alvo>`
# de lá, sem `-f`.
MK := $(lastword $(MAKEFILE_LIST))

# `retornos_medidos.inc` é GERADO (scripts/mede_retornos.pr) e incluído pelo
# `ps_retornos.c`. Sem ele nesta lista, regerar o .inc não recompilava nada:
# o make olhava só os .c, via tudo em dia, e o binário seguia com a tabela
# velha — a medição nova ficava no arquivo sem chegar no `--metadata`.
# `ps_build.h` é GERADO: a DATA da compilação, pra o `--version` distinguir
# dois binários da mesma versão (um instalado e um recém-compilado respondiam
# igual, e quem perguntava "o que tem nesta máquina" não tinha como saber).
# Escreve num temporário e só troca quando MUDA (`cmp -s`): trocar sempre
# relinkaria o binário a cada `make`. Um `-D` no CFLAGS não serve — o make não
# rastreia mudança de receita, e o valor ficaria preso no da compilação
# anterior, que é o mesmo defeito que este alvo existe pra evitar.
FORCE:
$(VM)/ps_build.h: FORCE
	@printf '/* GERADO pelo Makefile — a data desta compilacao. */\n#define PS_BUILD_DATA "%s"\n' \
	    "$$(date +%Y-%m-%d)" > $@.novo
	@cmp -s $@.novo $@ 2>/dev/null || mv $@.novo $@
	@rm -f $@.novo

# Os CABEÇALHOS da VM também: mudar só um `.h` (uma macro, um campo de struct)
# não refazia o binário — o make olhava os `.c`, via tudo em dia, e o `pool`
# seguia com o comportamento velho.
CABECALHOS := $(wildcard $(VM)/*.h)

pool: $(FONTES) $(CABECALHOS) $(VM)/ps_versao.h $(VM)/ps_build.h $(VM)/retornos_medidos.inc $(MK)
	$(CC) $(CFLAGS) -I$(VM) -o $@ $(FONTES) $(LIBS_POOL)

# Bundle PORTÁTIL: o binário + todas as .so numa pasta lib/, com wrapper. Roda
# em qualquer VPS x86-64 (glibc compatível) SEM apt install — mongo, gnutls,
# krb5, ldap etc. vão junto. Gera dist/jinga-portable/ e dist/jinga-portable.tar.gz.
bundle: pool
	./build_bundle.sh

# ── extensão do VS Code ─────────────────────────────────────────────────────
# Ela vivia SÓ em ~/.vscode/extensions, sem estar no repositório e sem forma de
# reconstruir: não havia `.vsix` em lugar nenhum da máquina. Editar a cópia
# instalada funciona até a hora de levar pra outro PC, e aí não há o que levar.
#
# `node_modules` NÃO é versionado — o `npm install` o traz a partir do
# package.json, e é ele que garante a mesma versão do cliente LSP em qualquer
# máquina.
#
#     make logo           regenera a logo (SVG do desktop + PNG dos editores) de scripts/gera_logo.pr
#     make vsix           gera editor/vscode/jinga-<versao>.vsix (Node >= 22)
#     make instala-vsix   gera e instala no VS Code local
EXT := editor/vscode

$(EXT)/node_modules:
	cd $(EXT) && npm install --omit=dev --no-audit --no-fund

# A logo (o J) nasce de UMA geometria, em Jinga: o SVG do tipo MIME e os PNG
# do VS Code e do IntelliJ saem do mesmo script — mudou o desenho, roda isto.
logo: pool
	./pool scripts/gera_logo.pr

.PHONY: logo

vsix: $(EXT)/node_modules
	# O @vscode/vsce 4 (2026-09) exige Node >= 22, e os transitivos das versoes
	# 2 e 3 tambem passaram a exigir Node 20+: com Node 18 o `npx` baixa e
	# estoura no meio ("File is not defined", "styleText is not a function").
	# Dizer isso ANTES e melhor que a pilha do npx. O servidor LSP nao depende
	# disto: o cliente sobe o `jinga-lsp` INSTALADO (o mesmo do IntelliJ e
	# do Neovim), entao `make install` atualiza o servidor do VS Code tambem. A
	# vsix so precisa ser refeita quando o CLIENTE (extension.js) ou a gramatica
	# mudam — o server.js dela e so a reserva de quem nao fez `make install`.
	@n=$$(node -p 'Number(process.versions.node.split(".")[0])'); \
	if [ "$$n" -lt 22 ]; then \
	  echo "vsix: empacotar exige Node >= 22 (aqui: $$(node -v)); o @vscode/vsce e seus transitivos nao rodam mais no 18."; \
	  echo "      Instale um Node novo (nvm/apt) ou aponte um: PATH=/caminho/do/node22/bin:\$$PATH make vsix"; \
	  exit 1; \
	fi
	cd $(EXT) && npx --yes @vscode/vsce package --allow-missing-repository --skip-license
	@ls -1 $(EXT)/*.vsix

instala-vsix: vsix
	code --install-extension $$(ls -t $(EXT)/*.vsix | head -1) --force
	@echo
	@echo "Recarregue a janela: Ctrl+Shift+P -> Developer: Reload Window"
	@echo "O VS Code mantem o processo antigo do servidor ate isso."

.PHONY: vsix instala-vsix

# ── Neovim ──────────────────────────────────────────────────────────────────
# Cliente LSP NATIVO: sem extensão, sem empacotamento, sem cliente de terceiro
# no meio. É por isso que ele serve de controle — se o completion funciona aqui
# e não no VS Code, o servidor está certo e o defeito é do outro lado.
#
# O tema `ariake-dark` NÃO é uma imitação: as cores saíram do mesmo tema que ele
# usa no VS Code (`wart.ariake-dark`), escopo por escopo, do color-theme.json
# (interface) e do tmTheme (código).
#
#     make nvim     instala em ~/.config/nvim
NVIM_CFG ?= $(HOME)/.config/nvim

nvim:
	@install -d $(NVIM_CFG)/colors $(NVIM_CFG)/syntax
	install -m644 editor/nvim/colors/ariake-dark.lua $(NVIM_CFG)/colors/
	# O REALCE. Ficou meses só na maquina, fora do repositorio: a copia
	# instalada era de antes da reforma, nao conhecia `funct` e pintava `//`
	# de comentario (nesta linguagem `//` e divisao inteira). Fora do git,
	# nada disso aparecia num diff.
	install -m644 editor/nvim/syntax/jinga.vim $(NVIM_CFG)/syntax/
	rm -f $(NVIM_CFG)/syntax/poolscript.vim
	# GUARDA COPIA E INSTALA, em vez de recusar.
	#
	# Recusar parecia prudente e nao era: o init.lua da maquina ficou DIAS preso
	# numa versao truncada — sem o fechamento automatico de () [] {} e sem o modo
	# de edicao permanente — e o alvo dizia "nao vou sobrescrever" toda vez, o que
	# se le como "esta tudo certo". Um backup datado resolve o medo real (perder
	# edicao sua) sem deixar a config apodrecer.
	@if [ -f $(NVIM_CFG)/init.lua ] && ! cmp -s editor/nvim/jinga.lua $(NVIM_CFG)/init.lua; then \
	  cp $(NVIM_CFG)/init.lua $(NVIM_CFG)/init.lua.bak-$$(date +%Y%m%d-%H%M%S); \
	  echo "  o init.lua anterior virou init.lua.bak-<data> (ele DIFERIA)"; \
	fi
	install -m644 editor/nvim/jinga.lua $(NVIM_CFG)/init.lua
	@echo "  init.lua instalado"
	@echo "  tema + LSP em $(NVIM_CFG)"

.PHONY: nvim

# ── IntelliJ / JetBrains ────────────────────────────────────────────────────
# O plugin (ícone por extensão, indentação no Enter, auto-fechamento com
# type-over) MORAVA em `ideia-icons/` e foi apagado junto com centenas de
# arquivos no commit d91f2e9. O `.jar` seguiu instalado e funcionando, então
# nada acusou — mas sem o fonte no repositório ele não se reconstrói, não
# acompanha a gramática e não vai pra outra máquina. Um binário instalado não
# é uma entrega.
#
# O bundle TextMate (realce) é CÓPIA da gramática do vsix — fonte única lá,
# sincronizada aqui, então o IDEA colore igual (inclusive a paleta do C#). Ele
# vai DENTRO do jar e o plugin o registra no boot: sem passo manual no IDEA.
#
#     make intellij     compila, sincroniza a gramática e instala no IDEA
IJ      := editor/intellij
IJ_HOME ?= $(HOME)/.local/share/JetBrains

intellij:
	$(IJ)/bundle/atualiza.sh
	$(IJ)/plugin/build.sh
	@destino=$$(ls -d $(IJ_HOME)/IntelliJIdea* $(IJ_HOME)/IdeaIC* 2>/dev/null | head -1); \
	if [ -z "$$destino" ]; then \
	  echo "  nao achei instalacao do IntelliJ em $(IJ_HOME)"; \
	  echo "  o jar esta em $(IJ)/plugin/dist/jinga-icons.jar — instale pelo"; \
	  echo "  Settings > Plugins > engrenagem > Install Plugin from Disk"; \
	else \
	  rm -rf "$$destino/poolscript-icons"; \
	  install -d "$$destino/jinga-icons/lib"; \
	  install -m644 $(IJ)/plugin/dist/jinga-icons.jar "$$destino/jinga-icons/lib/"; \
	  echo "  plugin instalado em $$destino/jinga-icons"; \
	  echo "  REINICIE o IDEA (plugin so recarrega no boot)"; \
	fi
	@echo "  realce: automatico — o bundle vai dentro do jar e o plugin registra no boot"
	@echo "  LSP:    automatico — o plugin declara o servidor ao LSP4IJ (plugin do marketplace);"
	@echo "          se ha um 'Jinga' (ou 'PoolScript') cadastrado A MAO em Settings > Language Servers,"
	@echo "          apague-o: senao sobem dois servidores"
	@$(IJ)/plugin/teste_intellij.sh

.PHONY: intellij

# Instala no sistema: o binário (como `jinga` e `jpkg`, que são o mesmo — e
# como `pool` e `psl`, os nomes de antes do rename, pra nada quebrar) e o
# servidor LSP. O `jinga-lsp` (e o antigo `poolscript-lsp`) é o atalho que o
# editor chama.
PREFIXO ?= /usr/local
install: pool
	install -d $(PREFIXO)/bin $(PREFIXO)/share/jinga/lsp
	install -m755 pool $(PREFIXO)/bin/jinga
	install -m755 pool $(PREFIXO)/bin/jpkg
	install -m755 pool $(PREFIXO)/bin/pool
	install -m755 pool $(PREFIXO)/bin/psl
	# o que sobrou de uma instalacao com o nome antigo
	rm -rf $(PREFIXO)/share/poolscript
	# O SERVIDOR LSP e `editor/vscode/server.js`, sobre `vscode-languageserver`.
	# Vai junto com as bibliotecas que ele importa (~3 MB) pra que Neovim,
	# IntelliJ e qualquer editor que fale LSP tenham o MESMO cerebro que o VS
	# Code — antes cada um dependia de um servidor em Jinga que respondia
	# `-32601` pra quase tudo.
	# `analise.js` vai JUNTO: o server exige ele (`require('./analise.js')`), e
	# instalar so o server.js deixava o Neovim e o IntelliJ com um servidor que
	# morre no boot por MODULE_NOT_FOUND. O VS Code nao via porque a vsix leva a
	# pasta inteira — o defeito so aparecia nos outros dois editores.
	install -m644 editor/vscode/server.js editor/vscode/analise.js \
	        $(PREFIXO)/share/jinga/lsp/
	@for m in vscode-languageserver vscode-languageserver-protocol \
	          vscode-languageserver-types vscode-jsonrpc \
	          vscode-languageserver-textdocument semver; do \
	    if [ -d editor/vscode/node_modules/$$m ]; then \
	      rm -rf $(PREFIXO)/share/jinga/lsp/node_modules/$$m; \
	      mkdir -p $(PREFIXO)/share/jinga/lsp/node_modules; \
	      cp -r editor/vscode/node_modules/$$m $(PREFIXO)/share/jinga/lsp/node_modules/; \
	    fi; \
	  done
	# A DOC vai junto: a prosa das sugestões e do hover sai de
	# `docs/<escopo>/<nome>/<nome>.md`. Sem ela instalada, o servidor funciona
	# mas responde sem explicação nenhuma — que é justamente o que o completion
	# não pode voltar a ser. Só as páginas, não o resto do repositório.
	@cd docs && find . -name '*.md' -exec install -Dm644 {} \
	        $(PREFIXO)/share/jinga/docs/{} \;
	printf '#!/bin/sh\n# Servidor LSP da Jinga. Ver docs/lsp.md.\nif ! command -v node >/dev/null 2>&1; then\n  echo "jinga-lsp precisa do node (o servidor usa vscode-languageserver)" >&2\n  exit 1\nfi\nexec node %s/share/jinga/lsp/server.js "$${@:---stdio}"\n' \
	        '$(PREFIXO)' > $(PREFIXO)/bin/jinga-lsp
	chmod 755 $(PREFIXO)/bin/jinga-lsp
	# o nome antigo do servidor continua respondendo (config de editor feita antes)
	cp $(PREFIXO)/bin/jinga-lsp $(PREFIXO)/bin/poolscript-lsp
	# O MIME NÃO derruba o install. Ele escreve em $(DADOS) (/usr/share por
	# padrão, ver a nota abaixo), então sem root ele falha — e falhava levando
	# junto um install que já tinha copiado o `jinga`, o `jpkg` e o LSP com
	# sucesso. O ícone do arquivo no gerenciador é uma comodidade do desktop;
	# ele não pode dar "Erro 2" num install que deu certo.
	# Sem root ele cai no `install-icone` — que atualiza a LOGO no diretório do
	# usuário, onde o GTK olha primeiro. O que fica faltando aí é só o registro
	# do TIPO (`$(DADOS)/mime`), que é o que precisa de root de verdade; se ele
	# já foi feito uma vez, o desktop continua inteiro.
	@$(MAKE) --no-print-directory install-mime PREFIXO=$(PREFIXO) 2>/dev/null \
	  || { $(MAKE) --no-print-directory install-icone; \
	       echo "  (o TIPO MIME em $(DADOS)/mime pede root: sudo make install-mime)"; }
	@echo "instalado em $(PREFIXO): jinga, jpkg, jinga-lsp (e os nomes antigos pool, psl, poolscript-lsp)"

# Tipo MIME + ícone do `.pr` pro desktop (GNOME/KDE/XFCE/…). Fica separado
# porque num servidor sem ambiente gráfico ele não faz falta e as ferramentas
# (`update-mime-database`) podem nem existir — daí o `|| true`.
# A base de MIME é /usr/share por padrão, NÃO $(PREFIXO): era obrigatório
# enquanto a extensão era `.ps` (o `glob-deleteall` que a tirava do PostScript
# só valia dentro da MESMA base onde o PostScript está definido), e continua
# sendo o lugar certo — é lá que está o arquivo das versões de antes, que esta
# instalação precisa sobrescrever.
DADOS ?= /usr/share
install-mime:
	install -d $(DADOS)/mime/packages \
	           $(DADOS)/icons/hicolor/scalable/mimetypes
	install -m644 dados/zz-jinga.xml $(DADOS)/mime/packages/
	install -m644 dados/icones/text-jinga.svg \
	        $(DADOS)/icons/hicolor/scalable/mimetypes/
	# o registro do nome antigo sai: senao ficam dois tipos pro mesmo `.pr`
	rm -f $(DADOS)/mime/packages/zz-poolscript.xml \
	      $(DADOS)/icons/hicolor/scalable/mimetypes/text-poolscript.svg
	-update-mime-database $(DADOS)/mime 2>/dev/null || true
	@./dados/espalha_icone.sh "$(DADOS)" instalar

# Só a LOGO, no diretório do usuário — não precisa de root. Serve pra trocar o
# desenho sem reinstalar nada: `~/.local/share/icons/<tema>` vem antes do
# `/usr/share/icons/<tema>` na busca do GTK, e o `espalha_icone.sh` espelha os
# temas do sistema pra lá. Só o registro do TIPO (o `.xml` em mime/packages)
# continua sendo coisa de root, e ele muda muito mais raramente que o desenho.
DADOS_USUARIO ?= $(HOME)/.local/share
install-icone:
	@./dados/espalha_icone.sh "$(DADOS_USUARIO)" instalar
	@echo "  logo do .pr atualizada em $(DADOS_USUARIO)/icons (sem root)"

.PHONY: install install-mime install-icone desinstala

desinstala:
	rm -f $(PREFIXO)/bin/jinga $(PREFIXO)/bin/jpkg $(PREFIXO)/bin/jinga-lsp \
	      $(PREFIXO)/bin/pool $(PREFIXO)/bin/psl $(PREFIXO)/bin/poolscript-lsp
	rm -rf $(PREFIXO)/share/jinga $(PREFIXO)/share/poolscript
	rm -f $(DADOS)/mime/packages/zz-jinga.xml $(DADOS)/mime/packages/zz-poolscript.xml
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
                teste/casos_oraculo.c teste/casos_robustez.c \
                teste/casos_libs.c

# o binário se chama `testar` porque `teste` é a PASTA dos casos
testar: $(TESTE_FONTES) teste/ps_teste.h $(VM)/ps_ext.h
	$(CC) -O2 -Wall -Wextra -Wno-unused-parameter -Wno-missing-field-initializers -Iteste -I$(VM) -o $@ $(TESTE_FONTES)

# `make check` = compila os dois e roda a suíte
# ── ferramentas prontas: sanitizers e analisador estático ───────────────────
# Não é caso escrito à mão: o ASan/UBSan instrumenta CADA acesso de memória e
# CADA operação com comportamento indefinido, e para no ponto exato. Acha a
# classe inteira (estouro de buffer, uso após liberar, shift inválido, overflow
# de signed) sem ninguém adivinhar o caso.
pool-asan: $(FONTES) $(CABECALHOS) $(VM)/ps_versao.h $(MK)
	$(CC) $(CFLAGS) -g -fsanitize=address,undefined -fno-omit-frame-pointer \
	  -I$(VM) -o $@ $(FONTES) $(LIBS_TESTE)

# Análise estática do gcc: caminho de execução simbólico, acha vazamento,
# desreferência de NULL e uso de não-inicializado sem rodar o programa.
#
# `-fsyntax-only` NÃO serve aqui: o gcc para antes do GIMPLE e o analisador
# nunca chega a rodar — o alvo imprimia "nada" acontecesse o que acontecesse.
# Tem que compilar de verdade, jogando o objeto fora.
#
# Um arquivo por invocação, e não todos de uma vez: o `jinga_vm.c` sozinho
# esgota a memória desta máquina quando analisado junto com os outros.
# TETO DE MEMÓRIA, e ele não é opcional. Esta máquina tem 7,7 GB e costuma
# estar com ~3,5 GB livres; o `-fanalyzer` num fonte grande passa de 6 GB
# sozinho e o OOM killer derruba a sessão inteira, não só o gcc. O `ulimit -v`
# faz o GCC desistir e reportar, em vez de levar a máquina junto.
#
# `jinga_vm.c` (30 mil linhas) fica FORA por padrão pelo mesmo motivo: ele
# nunca terminou aqui. Ele é relatado como não coberto — em voz alta, porque
# omitir isso seria dizer "limpo" sobre metade do motor. Pra rodar mesmo assim,
# numa máquina que aguente:
#
#     make analisa ANALISA_TUDO=1 ANALISA_MB=12000
ANALISA_MB   ?= 2000
ANALISA_TUDO ?=
ANALISA_FORA := $(VM)/jinga_vm.c

# Falso positivo CONFERIDO, um por linha, com o motivo. O `-fanalyzer` não
# segue posse através de struct nem de parâmetro de saída, então acusa como
# vazamento o ponteiro que o dono libera em outro lugar. Cada entrada aqui foi
# verificada lendo o código; a alternativa (sair 0 pra tudo) é o portão que não
# reprova, que foi o que este alvo já era uma vez.
#
#   ps_xlsx.c  leak of `nb`     — o `realloc` de `o->b` do Out (out_add); liberado
#                                 por quem monta o ZIP (free(zip.b))
#   ps_db.c    leak of `strdup` — `res->cols[i]` (res_col_nome); liberado em
#                                 ps_db_res_libera
# Cada entrada e um padrao de `grep` (regex basica, sem espaco: `.` casa o
# espaco e as aspas do gcc) casando ARQUIVO + MENSAGEM, nao arquivo:linha —
# a linha anda a cada edicao e o portao reprovava o mesmo falso positivo de
# antes, ja conferido, so porque o codigo acima dele cresceu (345 -> 357).
ANALISA_ACEITOS := ps_xlsx.c:[0-9]*:[0-9]*:.*leak.of..nb. ps_db.c:[0-9]*:[0-9]*:.*leak.of..strdup
ANALISA_ALVO := $(if $(ANALISA_TUDO),$(FONTES),$(filter-out $(ANALISA_FORA),$(FONTES)))

analisa:
	@set -f; achou=0; faltou=""; \
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
#     make memcheck PS=teste/e2e/ws.pr
PS ?= /dev/null
memcheck: pool
	valgrind --leak-check=full --show-leak-kinds=definite,indirect \
	         --track-origins=yes --error-exitcode=1 ./pool $(PS)

.PHONY: memcheck

# Avisos barulhentos, pra revisão deliberada — não entram no build de todo dia
# porque o volume de truncamento intencional esconderia o achado de verdade.
# Só compila (`-fsyntax-only` basta: aqui não há analisador envolvido).
# Avisos EXTRA que o build normal não liga. Reprova se achar algum.
#
# Duas correções que valem lembrar:
#
# 1. Era `-fsyntax-only`, que PULA a geração de código — e metade dos avisos
#    úteis (`-Wformat-truncation` inteiro) só nasce depois dela. O alvo mostrava
#    2 avisos onde uma compilação de verdade mostra 34. É o mesmo defeito que já
#    tinha sido corrigido no `analisa` e que ficou aqui.
# 2. Era `| grep`, então o alvo saía 0 sempre: nada reprovava. Agora sai != 0.
#
# `-Wformat-truncation` fica no nível 1, não no 2: o nível 2 acusa `snprintf`
# com precisão EXPLÍCITA (`%.200s`, `%.*s`), que é justamente o jeito certo de
# escrever, e os 31 avisos que ele dá aqui são todos desses. Portão que grita à
# toa é portão desligado. O nível 2 continua acessível pra revisão:
#
#     make avisos AVISOS_NIVEL=2
# Benchmark do motor. FORA do `make check` de propósito: tempo é ruidoso, e
# portão que reprova porque outro processo estava rodando é portão que se
# aprende a ignorar. Existe pra que otimização no motor tenha número — sem ele,
# "ficou mais rápido" é palpite (ver notas/PERFORMANCE.md).
#
#   make bench            mede e compara com teste/bench_base.txt
#   make bench BENCH=--grava     adota a medição atual como referência
#   make bench BENCH=--portao    sai != 0 se piorou além da folga (CI noturna)
# LEIS da linguagem: regra, não exemplo. Cada lei vale pra TODO valor sorteado,
# e a semente muda a cada execução — o oposto do caso congelado.
#
# Entra no `make check` com poucas rodadas (é barato: 15 mil checagens em 3 s) e
# roda FUNDO no noturno, com a semente do dia. Achado vem com o contraexemplo
# encolhido e a semente pra repetir.
BENCH ?=
bench: pool
	@nice -n 19 ./pool teste/bench.pr $(BENCH)

AVISOS_NIVEL ?= 1
avisos:
	@saida=$$(for f in $(FONTES); do \
	    nice -n 19 $(CC) $(CFLAGS) -Wlogical-op -Wshadow \
	      -Wformat-truncation=$(AVISOS_NIVEL) \
	      -I$(VM) -c -o /dev/null $$f 2>&1; \
	  done | grep -E "warning:" | sort -u); \
	if [ -n "$$saida" ]; then \
	  printf '%s\n' "$$saida"; \
	  echo; \
	  echo "avisos: $$(printf '%s\n' "$$saida" | wc -l) achado(s) — reprovando."; \
	  exit 1; \
	fi; \
	echo "  avisos extra (nivel $(AVISOS_NIVEL)): nenhum"

.PHONY: avisos

.PHONY: analisa

check: pool testar
	./testar
	@echo
	# A MESMA suite com o codigo de maquina desligado: o interpretador
	# continua sendo a referencia, e os dois tem que concordar em tudo.
	JINGA_JIT=0 ./testar
	@echo
	# Unidade em C: os ramos de erro dos modulos puros, que fonte .pr nao alcanca.
	@$(MAKE) --no-print-directory unidade
	@echo
	@./pool teste/confere_metadata.pr
	@echo
	# ASSERCOES: nenhum caso pode conferir menos do que o runner sabe conferir.
	# Eram 1577 gravados como `NULL, "<stderr>", -1` — sem stdout e sem rc.
	@./pool teste/confere_assercoes.pr
	@echo
	# DUPLICADOS: dois casos com o mesmo fonte sao um teste e uma copia. Eram
	# 232 — o contador de casos mentindo sobre o alcance da suite.
	@./pool teste/confere_duplicados.pr
	@echo
	@./pool scripts/audita_doc.pr
	@echo
	# COBERTURA da doc, com catraca. O `audita_doc` acima confere a página que
	# EXISTE contra o motor e passa dizendo "nenhuma divergencia" — o que se le
	# como "esta completa". Ele nunca perguntou se a pagina existe: media 132 e
	# calava sobre 721 membros sem nenhuma, `hash.sha256` e `sys.stdin` entre
	# eles. Portao que aprova medindo 15% sem dizer que sao 15% e' pior que
	# portao nenhum, porque vira base pra afirmar que esta tudo conferido.
	@./pool teste/confere_cobertura_doc.pr
	@echo
	# `//` na doc. O I11 tirou `//` de comentario e o fez divisao inteira; a doc
	# nao acompanhou e ficaram 179 paginas (725 ocorrencias) ensinando
	# `// comentario`. Quem copia um exemplo escreve codigo que nao compila — e
	# eu mesmo aprendi errado lendo a doc e escrevi `//` num exemplo novo. O
	# `audita_exemplos_doc` nao pegava: essas cercas nao sao ```ps.
	@./pool scripts/conserta_barra_doc.pr --portao
	@echo
	# STDLIB que a suite inteira nao chamava uma vez. O benchmark de cobertura
	# (`teste/bench_cobertura.pr`) lista as funcoes em ZERO execucao, e eram 115
	# so em jinga_vm.c: `os.cwd`, `sys.platform`, `sys.stdin.read`,
	# `date.now`, as 63 constantes de socket. Documentadas e nao testadas.
	@./pool teste/cobre_stdlib.pr
	@echo
	@./pool scripts/audita_c.pr
	@echo
	@./pool scripts/audita_exemplos_doc.pr
	@echo
	@./pool teste/fuzz_replay.pr
	@echo
	# DEPURADOR: fala DAP com o motor igual o editor faz — breakpoint na linha
	# pedida, pilha, variaveis VIVAS no ponto (slot e reaproveitado entre
	# blocos, entao nome errado aqui e' silencioso), passo a passo, parada na
	# excecao e o grafico de execucao. Sobe dois processos `pool --debug`.
	@./pool teste/depurador.pr
	@echo
	# Os EXEMPLOS de `examples/` — 16 programas que ninguem rodava. Sao a
	# primeira coisa que se le pra aprender a linguagem, e apodreciam em
	# silencio: a mudanca do indice quebrou dois de uma vez sem o gate ver.
	@./pool teste/exemplos_roda.pr
	@echo
	# LEIS: o que vale pra TODO valor, não pra um exemplo. Barato o bastante
	# pro portão; o noturno roda fundo com a semente do dia.
	@$(MAKE) --no-print-directory leis
	@echo
	# Drivers de LOOPBACK: CLI+jpkg, sockets e o jinker a fundo. Ficavam fora de
	# qualquer alvo — escritos, passando, e sem ninguém rodando.
	@./pool teste/cli_roda.pr
	@echo
	@./pool teste/sockets_roda.pr
	@echo
	@./pool teste/jinker_roda.pr
	@echo
	# Mongo: sobe o proprio mongod em /tmp e derruba. PULA se nao houver binario.
	@./pool teste/mongo_roda.pr
	@echo
	# LSP: o servidor agora e `editor/vscode/server.js`, sobre
	# `vscode-languageserver` (a implementacao de REFERENCIA do protocolo). O
	# teste o dirige como o VS Code faz. PULA sem node — o portao nao exige
	# ambiente de editor, mas nao finge que passou.
	@if command -v node >/dev/null 2>&1 && [ -d editor/vscode/node_modules/vscode-languageserver ]; then \
	    node editor/vscode/teste_servidor.js ./pool; \
	  else \
	    echo "PULOU o teste do LSP — falta node ou 'npm install' em editor/vscode"; \
	  fi
	@echo
	# REALCE: as listas (tipos, builtins, excecoes) da gramatica do VS Code e do
	# syntax do Neovim saem do `--metadata`. Ficaram meses sem portao: a
	# gramatica divergiu (JSON, Long, PoolFile) e ninguem viu.
	@./pool scripts/gera_realce.pr --portao
	@echo
	# NEOVIM: a config do editor tambem apodrece. A do desenvolvedor ficou DIAS
	# truncada, sem o fechamento de () [] {}, e nada acusava.
	@editor/nvim/teste_nvim.sh
	@echo
	# INTELLIJ: o plugin contra os jars REAIS do IDEA e do LSP4IJ (stub que nao
	# casa so quebra em runtime), e o servidor respondendo ao LSP4J. PULA sem IDEA.
	@editor/intellij/plugin/teste_intellij.sh
	@echo
	@$(MAKE) --no-print-directory analisa
	@echo
	# Avisos extra (-Wlogical-op, -Wshadow, -Wformat-truncation) que o build
	# normal nao liga. Barato: e so recompilar com mais flags.
	@$(MAKE) --no-print-directory avisos
	@echo
	# A MESMA suite com as invariantes do motor ligadas. Sem isto o portao so
	# acha MORTE; com isto acha estado errado antes de virar morte.
	@$(MAKE) --no-print-directory check-debug
	@echo
	# E2E que nao precisa de servico externo. Ficou FORA do portao por um tempo,
	# e o preco foi ps_jinker.c e ps_db.c em 0% de cobertura: nao por
	# falta de teste, mas porque o teste que os cobre nao entrava em portao nenhum.
	@$(MAKE) --no-print-directory check-e2e-local
	@echo
	@echo "FORA deste portao, e cada um tem alvo proprio porque e caro:"
	@echo "  make check-e2e   banco/mail/socket/mongo (servico externo de verdade)"
	@echo "  make check-asan  a MESMA suite sob ASan+UBSan (~4 min)"
	@echo "  make oom         falha de alocacao ponto a ponto (~5 min)"
	@echo "  make fuzz        fuzzer no front-end (FUZZ_T=<segundos>)"
	@echo "  make cobertura   linha E RAMO, por arquivo, com catraca"

# E2E: cada script sobe o que precisa e checa de ponta a ponta. Fica fora do
# `check` porque depende de serviço externo (Postgres, MySQL, mongod, SMTP) e
# porque é pesado.
#
# A ORDEM IMPORTA, e por isso não é mais um `for` sobre `*.pr`: o shell ordena
# alfabeticamente e `jinker_cli.pr` vinha ANTES de `jinker_srv.pr` — o cliente
# subia sem servidor, morria com "Connection refused", e o servidor ficava
# servindo até o timeout. O driver conhece o papel de cada script (par,
# sozinho, servidor sem cliente), espera a porta ABRIR em vez de dormir no
# escuro, e mata o servidor aconteça o que acontecer.
#
#     make check-e2e            # tudo
#     make check-e2e E2E=jinker # só o que casa com o filtro
E2E ?=
check-e2e: pool
	@./pool teste/e2e_roda.pr $(E2E)

# E2E que NÃO precisa de serviço externo: arquivo, sqlite (embutida), socket
# (loopback) e o par jinker (loopback). São scripts que
# rodam em qualquer máquina, e são o caminho mais barato pra tirar
# `ps_jinker.c` e `ps_db.c` dos 0% de cobertura — eles estão em
# zero não por serem código morto, mas porque só o e2e os toca e o e2e não
# entrava em portão nenhum.
E2E_SEM_SERVICO := arquivo sqlite socket pkg mail_local jinker manpu qrcode c d f
# DIZ QUAL alvo caiu. Antes o laço só levantava uma flag e o fim imprimia
# "e2e local: FALHOU" — treze alvos rodados, nenhum nome. Portão que reprova
# sem dizer o quê obriga a rodar tudo de novo à mão pra descobrir, que é o
# oposto do que ele existe pra fazer.
#
# Cada alvo com TIMEOUT próprio: um servidor que não sobe pendurava o alvo
# inteiro, e o que se via era a máquina travando, não um teste falhando.
E2E_TIMEOUT ?= 180

check-e2e-local: pool
	@caidos=""; \
	for alvo in $(E2E_SEM_SERVICO); do \
	  timeout $(E2E_TIMEOUT) ./pool teste/e2e_roda.pr $$alvo || caidos="$$caidos $$alvo"; \
	done; \
	if [ -n "$$caidos" ]; then \
	  echo "e2e local: FALHARAM ->$$caidos"; \
	  echo "  rode um por vez: ./pool teste/e2e_roda.pr <alvo>"; \
	  exit 1; \
	fi; \
	echo "e2e local: ok"

.PHONY: check-e2e check-e2e-local

# ── injeção de falha de alocação (a técnica do SQLite) ──────────────────────
# Toda correção da classe C da auditoria é um caminho de falta de memória, e
# NENHUM jamais executou: o gcov mostrava `if (!mn)` avaliado 42x com o ramo
# verdadeiro nunca tomado. O Linux dá /dev/full de graça e por isso o I/O pôde
# ser testado; memória não tem equivalente, então a gente fabrica.
#
# `--wrap` é do linker: nenhuma linha do motor muda, o código testado é o
# código de produção. Ver teste/ps_oom.c.
# Testes de UNIDADE em C: linka SÓ os módulos puros (hash, regex, ast) e chama
# as funções direto. É o que fura o teto de ~60% de ramo da suíte `.pr`, que
# por construção não alcança tratamento de erro — não existe programa Jinga
# que faça um `malloc` falhar ou passe um buffer curto pro base64.
unidade: teste/unidade.c $(VM)/ps_pilha.c $(VM)/ps_hash.c $(VM)/ps_regex.c $(VM)/ps_ast.c $(VM)/ps_xlsx.c $(MK)
	$(CC) $(CFLAGS) -g -I$(VM) -o $@ teste/unidade.c \
	  $(VM)/ps_pilha.c $(VM)/ps_hash.c $(VM)/ps_regex.c $(VM)/ps_ast.c $(VM)/ps_xlsx.c -lz -lexpat -lm
	@./$@

pool-oom: $(FONTES) $(CABECALHOS) teste/ps_oom.c $(VM)/ps_versao.h $(MK)
	$(CC) $(CFLAGS) -g -I$(VM) -o $@ $(FONTES) teste/ps_oom.c \
	  -Wl,--wrap=malloc,--wrap=calloc,--wrap=realloc,--wrap=strdup \
	  $(LIBS_POOL)

# Falha a i-ésima alocação de cada programa de teste/oom_varre.pr. Passar não é
# "não deu erro": é "morreu limpo" — segfault, liberação dupla e trava reprovam.
oom: pool-oom
	@./pool teste/oom_varre.pr

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

pool-fuzz: $(FUZZ_FONTES) $(CABECALHOS) teste/ps_fuzz.c $(VM)/ps_versao.h $(MK)
	clang -O1 -g -fsanitize=fuzzer,address,undefined -fno-omit-frame-pointer \
	  -Wno-everything -I$(VM) -I/usr/include/postgresql -I/usr/include/mariadb \
	  -DUTF8PROC_EXPORTS -I/usr/include/libmongoc-1.0 -I/usr/include/libbson-1.0 \
	  -o $@ $(FUZZ_FONTES) teste/ps_fuzz.c $(LIBS_TESTE)

# Semeia o corpus com os programas que a suíte já tem: o fuzzer parte de
# entrada VÁLIDA e muta a partir dela, em vez de descobrir a sintaxe do zero.
semeia: pool
	@./pool teste/fuzz_semeia.pr

# Depois de fuzzar, o que foi achado FICA: `teste/fuzz_achados/` é versionado
# (ver .gitignore) e o `make check` replaya tudo em todo portão. Sem isso, o
# crash de hoje é redescoberto amanhã.
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
	@./pool teste/propriedade.pr $(PROP_N) $(PROP_SEMENTE)

.PHONY: propriedade

# ── LEIS da linguagem (property testing com shrinking) ──────────────────────
# A única família de teste onde não existe valor esperado gravado: cada lei
# vale pra QUALQUER valor sorteado, então ela não encoda a saída de ontem —
# que é o defeito estrutural do oráculo e do diferencial (fotografia).
# `nice -n 19` porque a máquina de desenvolvimento trava com carga, e este
# alvo entrou no `make check`, que roda o tempo todo.
LEIS_N ?= 300
LEIS_SEMENTE ?= 1
leis: pool
	@nice -n 19 ./pool teste/leis.pr $(LEIS_N) $(LEIS_SEMENTE)

.PHONY: leis

# ── a MESMA suíte, sob ASan+UBSan ───────────────────────────────────────────
# A suíte roda o binário -O2, que é exatamente o que ESCONDE a classe de
# defeito de memória: leitura fora de faixa em -O2 costuma "funcionar". O
# `pool-asan` já existia e nada o rodava — este alvo é a ligação que faltava.
# VAZAMENTO CONTA: a detecção ficava desligada (`detect_leaks=0`), e com ela a
# tabela de capturas de toda closure, que nunca era solta, passava em todo
# portão. Ligada, qualquer memória que o motor não devolve reprova o caso.
check-asan: pool-asan testar
	@echo "suite inteira sob AddressSanitizer + UBSan + LeakSanitizer…"
	@PS_POOL=pool-asan ASAN_OPTIONS=detect_leaks=1:abort_on_error=0 \
	  UBSAN_OPTIONS=print_stacktrace=1 nice -n 19 ./testar

.PHONY: check-asan

# ── build de asserção: a MESMA suíte com as invariantes valendo ─────────────
# Sem isto, ASan, fuzzer e injeção de falha só acham MORTE, nunca ESTADO
# ERRADO: um `sp` desequilibrado ou um ObjType impossível só viram falha
# quando viram segfault, e com 41% de ramo coberto há muito caminho onde não
# viram. É o `--with-pydebug` do CPython, o `DCHECK` do V8, o `SQLITE_DEBUG`.
#
# Roda a suíte inteira contra o binário com `-DPS_DEBUG`. Invariante quebrada
# vira `abort()`, e o runner (subprocesso por caso) reporta como morte por
# sinal em vez de derrubar a bateria.
pool-debug: $(FONTES) $(CABECALHOS) $(VM)/ps_versao.h $(MK)
	$(CC) -O1 -g -DPS_DEBUG $(CFLAGS_BASE) -I$(VM) -o $@ $(FONTES) $(LIBS_TESTE)

check-debug: pool-debug testar
	@echo "suite inteira com as invariantes do motor ligadas…"
	@PS_POOL=pool-debug nice -n 19 ./testar

.PHONY: check-debug

# ── cobertura ───────────────────────────────────────────────────────────────
# A auditoria de testes apontou que cobertura NUNCA tinha sido medida. E a
# métrica que importa é RAMO TOMADO, não linha: 93% de linha no parser eram 67%
# de ramos. Linha superestima — é a razão de o SQLite medir MC-DC.
# `pool` como pré-requisito: o alvo TERMINA no portão (`./pool
# teste/cobertura_portao.pr`), e no CI não havia `./pool` — o job morria no
# último passo sem ter comparado nada.
cobertura: pool testar
	@rm -rf cob && mkdir -p cob
	$(CC) -O0 -g --coverage $(CFLAGS_BASE) -I$(VM) -o cob/pool $(FONTES) $(LIBS_TESTE)
	@echo "rodando a suite contra o binario instrumentado…"
	@PS_POOL=cob/pool nice -n 19 ./testar 2>&1 | tail -2
	# O PORTÃO INTEIRO, não só o `testar`. Enquanto a medição rodava apenas a
	# suíte, `ps_jinker.c`, `ps_db.c` e os 113 nativos de socket
	# apareciam em 0% — não por falta de teste, mas porque o teste que os cobre
	# (e2e local, drivers .pr) rodava FORA da medição. Número que ignora metade
	# do portão manda corrigir o que já está coberto.
	# A unidade em C tem que entrar na MEDIÇÃO, senão os ramos que só ela
	# alcança continuam contando como descobertos.
	# O MESMO link do alvo `unidade` (com ps_xlsx.c -lz -lexpat): sem o xlsx o
	# link falhava, o `2>/dev/null` escondia e o `|| true` seguia — a unidade
	# nunca entrou na medição. Falha de link ou de teste agora derruba o alvo.
	@echo "rodando os testes de unidade instrumentados…"
	@$(CC) -O0 -g --coverage $(CFLAGS_BASE) -I$(VM) -o cob/unidade teste/unidade.c \
	  $(VM)/ps_pilha.c $(VM)/ps_hash.c $(VM)/ps_regex.c $(VM)/ps_ast.c $(VM)/ps_xlsx.c -lz -lexpat -lm
	@./cob/unidade > /dev/null
	@echo "rodando os drivers .pr e o e2e local contra o mesmo binario…"
	@for d in teste/confere_metadata.pr scripts/audita_doc.pr \
	          scripts/audita_exemplos_doc.pr \
	          teste/fuzz_replay.pr teste/cli_roda.pr teste/sockets_roda.pr \
	          teste/jinker_roda.pr teste/mongo_roda.pr teste/depurador.pr; do \
	  nice -n 19 ./cob/pool $$d >/dev/null 2>&1 || true; \
	done
	# E os que PRECISAM de serviço também, quando ele existe: `db` (PostgreSQL,
	# MySQL) e `mongo` cobrem os drivers de `ps_db.c` e `ps_mongo.c` que o
	# sqlite não alcança — pg_exec, a conversão de `?` pra `$1`, o mapa de
	# SQLSTATE. Cada um imprime PULOU e sai 0 quando o serviço não está de pé,
	# então isto é seguro: na máquina com banco a medição sobe, na CI sem banco
	# ela fica igual. O `|| true` não esconde falha de teste — quem cobra esses
	# scripts é o `make check-e2e`; aqui eles só MEDEM.
	@for alvo in $(E2E_SEM_SERVICO) db mongo mail ws; do \
	  nice -n 19 ./cob/pool teste/e2e_roda.pr $$alvo >/dev/null 2>&1 || true; \
	done
	@lcov --capture --directory . --output-file cob/bruto.info \
	  --rc branch_coverage=1 --ignore-errors mismatch,source,empty >/dev/null 2>&1
	@lcov --extract cob/bruto.info "*/vm/*" --output-file cob/vm.info \
	  --rc branch_coverage=1 --ignore-errors empty >/dev/null 2>&1
	@genhtml cob/vm.info --output-directory cob/html --branch-coverage \
	  >/dev/null 2>&1 || true
	@echo; lcov --summary cob/vm.info --rc branch_coverage=1 2>/dev/null | tail -6
	@echo; echo "detalhe por arquivo:  cob/html/index.html"
	@echo
	# PORTÃO: compara ARQUIVO POR ARQUIVO com o baseline versionado. Percentual
	# agregado não decide nada — queda de 10 pontos num arquivo some na média de
	# 17, e foi assim que a mesma cobertura reapareceu em três auditorias
	# seguidas sem ninguém saber quando piorou.
	@./pool teste/cobertura_portao.pr

.PHONY: cobertura
