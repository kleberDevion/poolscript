# Levantamento: tipos de erro que o motor emite

Medido por 14 agentes em 2026-09-11, 941 chamadas de ferramenta, a VM inteira.
Dado bruto em `w8fh28p9w.output.json`. Este resumo e derivado dele.

- tipos distintos emitidos: **43**
- tipos que nenhum programa consegue disparar: ConnectionFailure, OutOfMemory
- tipos que o `catch` do nome exato NAO pega: 0
- tipos medidos que `catch (Exception e)` pega: 72

## Inventario, por numero de sitios

| tipo | sitios | primeiro sitio |
|---|---:|---|
| `MemoryError` | 423 | vm/poolscript_vm.c:3608 |
| `TypeError` | 333 | vm/poolscript_vm.c:3551 |
| `RuntimeError` | 245 | vm/poolscript_vm.c:3990 |
| `ValueError` | 65 | vm/poolscript_vm.c:3373 |
| `FileNotFoundError` | 42 | vm/poolscript_vm.c:3167 |
| `FileExistsError` | 42 | vm/poolscript_vm.c:3166 |
| `IsADirectoryError` | 42 | vm/poolscript_vm.c:3168 |
| `NotADirectoryError` | 42 | vm/poolscript_vm.c:3169 |
| `PermissionError` | 42 | vm/poolscript_vm.c:3171 |
| `OSError` | 35 | vm/poolscript_vm.c:3172 |
| `DatabaseError` | 25 | vm/poolscript_vm.c:12623 |
| `RecursionError` | 17 | vm/poolscript_vm.c:10041 |
| `IOError` | 15 | vm/poolscript_vm.c:12396 |
| `NetworkError` | 14 | vm/poolscript_vm.c:15083 |
| `ZeroDivisionError` | 12 | vm/poolscript_vm.c:3620 |
| `AttributeError` | 10 | vm/poolscript_vm.c:23546 |
| `IndexError` | 8 | vm/poolscript_vm.c:5887 |
| `SyntaxError` | 8 | vm/poolscript_vm.c:25333 |
| `OverflowError` | 4 | vm/poolscript_vm.c:3342 |
| `UnicodeDecodeError` | 4 | vm/poolscript_vm.c:6844 |
| `KeyError` | 4 | vm/poolscript_vm.c:5686 |
| `AttributedValueError` | 4 | vm/poolscript_vm.c:9228 |
| `TimeoutError` | 4 | vm/poolscript_vm.c:15086 |
| `AssertionError` | 4 | vm/poolscript_vm.c:20482 |
| `NameError` | 4 | vm/poolscript_vm.c:21923 |
| `ImportError` | 4 | vm/poolscript_vm.c:25302 |
| `NotImplementedError` | 3 | vm/poolscript_vm.c:25354 |
| `UnicodeEncodeError` | 2 | vm/poolscript_vm.c:6787 |
| `LookupError` | 2 | vm/poolscript_vm.c:6672 |
| `%.60s do buffer tp (vem de vm/ps_db.c) —…` | 2 | vm/poolscript_vm.c:16599 |
| `(nenhum — o sitio escreve so vm->erro e …` | 2 | vm/poolscript_vm.c:20710 |
| `ConversionError` | 1 | vm/poolscript_vm.c:23330 |
| `UniqueViolation` | 1 | vm/ps_pgerr.h:101 |
| `UndefinedTable` | 1 | vm/ps_pgerr.h:173 |
| `UndefinedColumn` | 1 | vm/ps_pgerr.h:159 |
| `DivisionByZero` | 1 | vm/ps_pgerr.h:51 |
| `InsufficientPrivilege` | 1 | vm/ps_pgerr.h:152 |
| `StringDataRightTruncation` | 1 | vm/ps_pgerr.h:30 |
| `ConnectionFailure` | 1 | vm/ps_pgerr.h:14 |
| `OutOfMemory` | 1 | vm/ps_pgerr.h:198 |
| `DiskFull` | 1 | vm/ps_pgerr.h:197 |
| `SystemError` | 1 | vm/ps_pgerr.h:217 |
| `AssertFailure` | 1 | vm/ps_pgerr.h:255 |

## Suspeitas levantadas, por tipo

### `MemoryError`

- vm-b: 131 sitios sao MERRO/BERRO "sem memoria" (falha real de malloc, inalcancavel por programa), mas 3 sitios (5412, 5456, 9967) levantam MemoryError para um ARGUMENTO grande demais, sem nenhuma alocacao ter falhado — o mesmo caso que em 5319 seria ValueError. Mesma condicao logica, dois tipos diferentes.

- vm-e: os 7 sitios cobrem so a repeticao de str e de bytes (21571-21599). A MESMA mensagem literal, "sem memoria na repeticao", e MemoryError em 21577 e RuntimeError em 21556 (repeticao de lista). Fora dai, todos os 69 sitios de falta de memoria da regiao usam ERRO( e saem como RuntimeError.

- libs: so existe num sitio — e o unico MemoryError da regiao inteira, e nao e falta de memoria: e o teto de max_size que o proprio programa pediu. Os 31 sitios de falta de memoria de verdade da regiao (13 em ps_db.c, 9 em ps_mail.c, 5 em ps_regex.c, 3 em ps_mongo.c, 1 em ps_jinker.c) NAO viram MemoryError nenhum.

- frente: Nome correto e na hierarquia (poolscript_vm.c: MemoryError->Exception). Os 4 sitios da regiao sao inalcancaveis por programa; sao rede de seguranca de alocacao.

### `RuntimeError`

- vm-a: so existe num sitio

- vm-b: macro-guarda-chuva: os 23 sitios misturam 3 causas sem relacao — "estouro da pilha" do fixa_raiz, "regex: backtracking demais" (9561, 9597, 9613, 9666...) e "sem fonte de aleatoriedade do sistema" (10200). Um catch (RuntimeError e) pega as tres de uma vez e nao consegue distinguir.

- vm-c: mesmo erro com tipos diferentes: SK_ERRNO (vm/poolscript_vm.c:13206) manda TODO errno de socket pra RuntimeError, ECONNREFUSED/ETIMEDOUT inclusive, enquanto o MESMO errno no modulo os vira subclasse de OSError (vm/poolscript_vm.c:11849)

- vm-e: e o tipo POR OMISSAO, nao uma escolha: 103 dos 125 sitios usam a macro ERRO(vm, msg) (definida em 20538), que grava "RuntimeError" sem o sitio digitar nada. 69 desses 103 sao falta de memoria. Resultado medido: [1,2]*9999999999 -> RuntimeError e "ab"*9999999999 -> MemoryError, mesmo opcode OP_MUL, 21552 vs 21571. E 5[1:2] -> RuntimeError (23089) enquanto 5[1] -> TypeError (22890), mesmo valor, mesma falha.

- frente: main.c:451: ramo morto, condicao impossivel (PS_ERRO_RUNTIME so nasce em ps_roda_fonte, e quem chama e ps_verifica_fonte). main.c:153: default defensivo que nao consegui alcancar.

### `ValueError`

- vm-b: o sitio 5319 e o unico da regiao que ainda carrega o prefixo em portugues "valor invalido: " (e sem acento), justamente o prefixo que o comentario de 10322 diz ter sido removido de todas as mensagens. Todos os outros 28 ValueError da regiao usam o texto seco.

- vm-d: so existe num sitio

### `FileNotFoundError`

- vm-a: subclasse sem pai / so existe num sitio

- vm-b: subclasse sem pai: sai de tipo_do_errno (vm/poolscript_vm.c:3163, ENOENT), chamado pelos 20 erro_sistema() da regiao. catch (OSError e) NAO pega, porque o catch compara a string por igualdade.

- vm-c: subclasse sem pai pelo nome, mas MEDIDO no binario de hoje: catch (OSError e) PEGA (confere vm/poolscript_vm.c:3186 EXCECOES + excecao_eh em 3241). As 21 ocorrencias sao os 21 sitios erro_sistema da regiao, um funil so: tipo_do_errno (vm/poolscript_vm.c:3250) escolhe qual dos 6 nomes sai, pelo errno

### `FileExistsError`

- vm-a: subclasse sem pai / so existe num sitio

- vm-b: subclasse sem pai E aparentemente inalcancavel pelo open(): tipo_do_errno (3163) so devolve FileExistsError com EEXIST, e o unico modo do open() que produziria EEXIST e o 'x'. O open() valida 'x' como modo primario valido (6560) e depois entrega a string de modo crua ao fopen(3) (6575-6577, 6589), que nao aceita um 'x' sozinho. Medido: open("/etc/hostname", "x") num arquivo QUE EXISTE devolve OSError: [Errno 22] Invalid argument: '/etc/hostname' — nunca FileExistsError.

- vm-c: subclasse sem pai pelo nome; medido: catch (OSError e) pega. Mesmo funil de 21 sitios erro_sistema

### `IsADirectoryError`

- vm-a: subclasse sem pai / so existe num sitio

- vm-b: subclasse sem pai. O sitio 6587 e o unico da regiao que FORCA o errno na mao (erro_sistema(vm, EISDIR, ...)) — ali o nome IsADirectoryError e deterministico, nao vem do sistema.

- vm-c: subclasse sem pai pelo nome; medido: catch (OSError e) pega. Mesmo funil de 21 sitios erro_sistema, o errno EISDIR e passado fixo aqui

### `NotADirectoryError`

- vm-a: subclasse sem pai / so existe num sitio

- vm-b: subclasse sem pai: tipo_do_errno (3163) para ENOTDIR. catch (OSError e) NAO pega.

- vm-c: subclasse sem pai pelo nome; medido: catch (OSError e) pega. Mesmo funil de 21 sitios erro_sistema

### `PermissionError`

- vm-a: subclasse sem pai / so existe num sitio

- vm-b: subclasse sem pai: tipo_do_errno (3163) para EACCES/EPERM. catch (OSError e) NAO pega.

- vm-c: subclasse sem pai pelo nome; medido: catch (OSError e) pega. Mesmo funil de 21 sitios erro_sistema

### `OSError`

- vm-a: so existe num sitio

- vm-b: o comentario de 3152-3162 chama isto de "OSError puro" e diz que "o resto cai no OSError" — escrito como se houvesse hierarquia. Nao ha: o catch compara a string, entao catch (OSError e) pega SOMENTE o default do switch (errno fora de EEXIST/ENOENT/EISDIR/ENOTDIR/EACCES/EPERM) e deixa passar os 5 casos que o proprio switch se deu ao trabalho de nomear.

- libs: so existe num sitio — e alem de unico, e codigo morto: nenhum programa consegue levanta-lo, e no lugar dele o motor manda uma URL truncada em silencio.

### `RecursionError`

- vm-b: os 3 sitios da regiao (10041, 10043, 10046) ficam todos dentro de ger_retoma e dao a MESMA mensagem "maximum recursion depth exceeded" para 3 limites diferentes (frames, locais, pilha). Nao consta da tabela de tipos embutidos de docs/linguagem/10-excecoes.md:108-123.

- vm-d: so existe num sitio

### `IOError`

- vm-c: mesmo erro com tipos diferentes: ENOENT sai FileNotFoundError no os.loadFile e IOError aqui. Medido: catch (OSError e) pega o loadFile e NAO pega o os.run; so catch (Exception e) ou catch (IOError e) pegam. Em vm/poolscript_vm.c:12205 o tipo que erro_sistema acabou de escrever e SOBRESCRITO a mao por "IOError"

- frente: so existe num sitio — e, pior, e um SEGUNDO nome para uma condicao que o resto do motor ja batizou de outro jeito. MEDIDO com o mesmo arquivo inexistente: `os.loadFile("/nao/existe/xyz.ps")` -> `FileNotFoundError: [Errno 2] No such file or directory`, porque tipo_do_errno (poolscript_vm.c:3250) devolve FileNotFoundError para ENOENT. O `--check` chama isso de IOError. E os dois nomes nao se encontram: a tabela EXCECOES pendura IOError em Exception de proposito (poolscript_vm.c:3196-3200, comentario: "IOError fica IRMAO de OSError, nao pai nem filho"), entao `catch (IOError e)` NAO pega o FileNotFoundError — medido: o erro escapou do catch (`FileNotFoundError: [Errno 2] ...`, rc=1).

### `NetworkError`

- vm-c: so existe num sitio. E o UNICO ponto da regiao onde o tipo da excecao nao e literal: vm/poolscript_vm.c:15086 copia hr.erro_tipo cru com "%.30s", entao o nome do tipo e um dado vindo de vm/ps_http.c, sem validacao nenhuma contra a tabela de excecoes

- vm-d: subclasse sem pai

### `IndexError`

- vm-c: so existe num sitio

### `SyntaxError`

- vm-e: 2 sitios literais (25333 lexer, 25343 parser) mais um ramo do ternario 25354, que decide entre SyntaxError e NotImplementedError pela flag prog->erro_do_programa.

- libs: so existe num sitio + subclasse sem pai — emitido por vm/ps_db.c:161 (SQLSTATE 42601). Pior: colide com o SyntaxError da propria linguagem (vm/poolscript_vm.c:3226), que e erro de compilacao e nem e capturavel em runtime. Um catch (SyntaxError e) escrito por engano ao redor de uma query passa a pegar erro de banco.

- frente: O nome esta certo e esta na tabela de hierarquia (poolscript_vm.c:3226 SyntaxError->Exception). O suspeito e outro: os 4 sitios (main.c:141, 449, 715, 727) so IMPRIMEM o nome. Os 104 sitios que de fato levantam o erro (76 perro em ps_parser.c, 18 erro/erro_em em ps_lexer.c, 10 cerro_sx em ps_compiler.c) gravam SO a mensagem num buffer char[256] e nunca escrevem tipo nenhum. O nome nasce em main.c mapeando um enum de 4 valores (PSTipoErro, ps_vm.h:19-26). Efeito medido: o MESMO defeito e pegavel ou nao dependendo de estar no arquivo rodado (rc=2, nao pegavel) ou num modulo importado (pegavel).

### `UnicodeDecodeError`

- vm-b: nao consta da tabela de tipos embutidos de docs/linguagem/10-excecoes.md:108-123 — quem so leu a doc nao sabe escrever este nome no catch. E um nome de SUBCLASSE (a raiz seria UnicodeError/ValueError), que nada pega.

### `AttributedValueError`

- vm-b: so existe num sitio na regiao (9228, dentro de checa_param_tipos). O nome nao existe em nenhuma outra linguagem e e escrito a mao em 4 lugares do arquivo (9228 e 23243/23251/23278, fora da minha regiao): quatro strings soltas que precisam continuar identicas pra o mesmo catch pegar as quatro.

### `TimeoutError`

- vm-c: so existe num sitio, e o sitio nem escreve o nome: vem de vm/ps_http.c:324/350/376 por dentro de hr.erro_tipo. TimeoutError NAO esta na tabela EXCECOES (vm/poolscript_vm.c:3186-3226), entao e o unico tipo levantado nesta regiao que catch (Exception e) nao pega. Medido acima

### `ImportError`

- vm-e: o sitio 24343 e um remendo: `if (!vm->erro_tipo[0]) ... "ImportError"`. Ele so cobre os 7 sitios de carrega_modulo_ps que nao escrevem tipo nenhum QUANDO erro_tipo esta vazio; se sobrou tipo de um erro anterior, o remendo nao dispara e o tipo velho vaza (medido, ver observacoes).

### `NotImplementedError`

- vm-e: so existe num sitio: 25354 e a UNICA ocorrencia de "NotImplementedError" como tipo de excecao em todo o vm/poolscript_vm.c, e e o unico tipo emitido pelo motor que ficou de FORA da tabela EXCECOES (3187-3227). Medido: `catch (Exception e)` NAO pega esse erro, enquanto pega todos os outros 14 tipos da regiao. Alem disso a doc chama o tipo de `NotImplemented`, sem Error, e afirma isso por escrito em docs/PoolScript.md:374 e docs/LANGUAGE.md:1162 - medido: `catch (NotImplemented e)` nao pega nada.

- frente: NAO ESTA NA TABELA DE HIERARQUIA. A tabela EXCECOES (vm/poolscript_vm.c:3195-3227) nao tem entrada para NotImplementedError, entao excecao_pai() devolve NULL e so casa pelo proprio nome. MEDIDO: `try { import rg_mod_ni } catch (Exception e) {...}` NAO pega — o erro escapa (`NotImplementedError: rg_mod_ni: 'break' fora de laco (linha 2)`, rc=1), enquanto o mesmo teste com SyntaxError pega. Alem disso o nome MENTE: 63 dos 73 sitios do compilador usam cerro() (erro_do_programa=0 -> NotImplementedError) e muitos sao erro de quem escreveu, nao feature faltando: ps_compiler.c:2904 "'break' fora de laco", :2923 "'continue' fora de laco", :2583 "import mal formado", :1460 e :1463 "cor invalida", :1569 "argumento posicional depois de nomeado", :2058 "alvo de desempacotamento invalido", :1512 "tipo desconhecido", :1032 "chave nao fechada na f-string". O editor recebe esses como 'nao implementado'.

### `UnicodeEncodeError`

- vm-b: nao consta da tabela de tipos embutidos de docs/linguagem/10-excecoes.md:108-123. Nome de SUBCLASSE (raiz seria UnicodeError/ValueError), que nada pega.

### `LookupError`

- vm-b: so existe nestes 2 sitios em TODO o vm/poolscript_vm.c (grep -c LookupError = 2, ambos na regiao) e nao aparece em nenhuma tabela de docs/linguagem/10-excecoes.md. Nome inventado que nenhum programa sabe que existe pra escrever no catch.

### `%.60s do buffer tp (vem de vm/ps_db.c) — "DatabaseError" ou `

- vm-d: so existe num sitio

### `(nenhum — o sitio escreve so vm->erro e faz return -1, erro_`

- vm-d: so existe num sitio

### `ConversionError`

- vm-e: so existe num sitio: 23330 e o UNICO ponto da regiao que levanta ConversionError, e o unico do arquivo inteiro fora da propria tabela EXCECOES (3213). Alcanca apenas `char x = <inteiro fora de 0..0x10FFFF>`.

### `UniqueViolation`

- libs: so existe num sitio + subclasse sem pai — o caso classico: quem escreve catch (DatabaseError e) para tratar chave duplicada NAO pega nada. E o mesmo UNIQUE no sqlite DA DatabaseError (medido).

### `UndefinedTable`

- libs: so existe num sitio + subclasse sem pai — vm/ps_db.c:161, SQLSTATE 42P01. No sqlite o mesmo SELECT sai como DatabaseError (medido).

### `UndefinedColumn`

- libs: so existe num sitio + subclasse sem pai — vm/ps_db.c:161, SQLSTATE 42703.

### `DivisionByZero`

- libs: digitado errado (divergencia de nome) + subclasse sem pai — a linguagem chama divisao por zero de ZeroDivisionError (vm/poolscript_vm.c:3207, 21634-21725); o mesmo conceito vindo do banco se chama DivisionByZero. Dois nomes para a mesma coisa no mesmo motor, e nenhum pega o outro.

### `InsufficientPrivilege`

- libs: so existe num sitio + subclasse sem pai — vm/ps_db.c:161, SQLSTATE 42501. A mensagem diz "permission denied" e o motor TEM PermissionError (vm/poolscript_vm.c:3194), mas o tipo emitido nao e nem PermissionError nem DatabaseError.

### `StringDataRightTruncation`

- libs: so existe num sitio + subclasse sem pai — vm/ps_db.c:161, SQLSTATE 22001. Valor que nao serve para a coluna: nem ValueError nem DatabaseError.

### `ConnectionFailure`

- libs: so existe num sitio + subclasse sem pai — nome de falha de conexao que nao e NetworkError. A falha de conexao do MESMO driver, quando acontece no connect, vira NetworkError (medido: "4) NetworkError: falha de conexao: connection to server at \"127.0.0.1\", port 9 failed"). Mesmo evento, dois tipos, conforme a hora em que acontece.

### `OutOfMemory`

- libs: so existe num sitio + subclasse sem pai — falta de memoria com um nome que nao e MemoryError (vm/poolscript_vm.c:3223). Nem catch (MemoryError e) nem catch (DatabaseError e) pegam.

### `DiskFull`

- libs: so existe num sitio + subclasse sem pai — nem OSError nem DatabaseError pegam.

### `SystemError`

- libs: so existe num sitio + subclasse sem pai — nome generico de "erro do sistema" que existe SO por causa de um SQLSTATE de banco. Nao esta na tabela de excecoes da linguagem (vm/poolscript_vm.c:3178-3228), entao so casa por igualdade literal e ninguem sabe que existe pra escrever.

### `AssertFailure`

- libs: digitado errado (divergencia de nome) + subclasse sem pai — a linguagem chama falha de assert de AssertionError (vm/poolscript_vm.c:3225); vindo do banco chama AssertFailure. Nenhum pega o outro.

