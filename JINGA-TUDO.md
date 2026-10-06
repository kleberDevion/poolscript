# Jinga — tudo (para a mudança de laptop)

Este arquivo tem credenciais: a senha do sudo deste laptop, as senhas de teste e o caminho de cada chave. Ele está no git desde que o repositório virou **privado** (conferido em 2026-09-28: `github.com/kleberDevion/poolscript` responde 404 sem login). Junto dele entram `.vscode/` e o `.claude/settings.json`; o `.claude/settings.local.json` continua fora (é o pessoal, e o daqui tem ~250 permissões da época do interpretador antigo com nome de biblioteca de outra linguagem). Se o repositório voltar a ser público, este arquivo e o `.claude/settings.json` voltam pro `.gitignore` antes.

Montado em 2026-09-28. As seções foram levantadas ao longo desse dia, enquanto a versão subia de 16.1.6 a 16.1.9: um número marcado com versão ou commit anterior é a medida daquela hora. O estado final está em 6.2 e na seção 9.

## Sumário

1. [Quem você é e como trabalhamos](#1-quem-você-é-e-como-trabalhamos)
2. [A linguagem Jinga](#2-a-linguagem-jinga)
3. [O motor (`vm/`)](#3-o-motor-vm)
4. [Editor](#4-editor)
5. [Build, testes, portões, VPS, CI, release e versão](#5-build-testes-portões-vps-ci-release-e-versão)
6. [Trilha de dev e pendências](#6-trilha-de-dev-e-pendências)
7. [Máquina, contas e credenciais](#7-máquina-contas-e-credenciais)
8. [Seus projetos](#8-seus-projetos)
9. [Mudança pro Mac, passo a passo](#9-mudança-pro-mac-passo-a-passo)

## 1. Quem você é e como trabalhamos

Fonte desta seção: as 24 notas `user-*` e `feedback-*` da memória do assistente, mais as notas de regra que ficam fora desse prefixo (`bump-a-cada-alteracao`, `nada-de-falso-verde`, `maquina-sensivel-drivers`, `suite-na-vps`, `paridade-docs-vsix-testes`, `design-de-api-e-do-usuario`, `rebase-e-github-dele`, `doc-nivel-python-java`, `ci-runner-nao-e-limpo`, `git-identidade-sumiu`). A pasta da memória é `~/.claude/projects/-home-kleberdevion-poolscript-lang/memory/`. São 75 arquivos: 74 notas e o índice `MEMORY.md`.

### 1.1 Quem você é

- Kleber (usuário `kleberDevion` no GitHub, e-mail `kleberdevion@proton.me`), criador da **Jinga**. Até 2026-09-24 a linguagem se chamava PoolScript, e entre 2026-09-15 e 09-24 a extensão do editor usou o nome Pyrite.
- Você criou a linguagem pra ela caber no seu jeito de aprender. Você tem dificuldade de aprendizado e esquece coisas ("tenho amnésia"). Começou a programar pelo celular, porque não tinha notebook. Hoje usa a Jinga pra tocar a sua própria API sem depender de outra linguagem. O norte de todas as features é a **autonomia da linguagem**.
- Você não carrega o histórico entre sessões. O assistente reancora o essencial com clareza, sem tom de "você já sabe disso".
- Você escreve em português, direto, às vezes sob estresse. A resposta vai no mesmo idioma, objetiva e sem floreio. O estresse não é com o assistente: segue-se em frente.
- Você se define como "muito pessimista". Mostrar o que já funciona, com o número medido, ajuda.
- Você decide rápido e manda. Quando reclama de um comportamento, quase sempre é bug do motor: em 2026-08-24 foram 11 bugs reais num dia.
- Você edita arquivos direto no GitHub e roda `git rebase` em cima de `origin/main` enquanto o assistente trabalha.
- Seu editor é o **Neovim**, mesmo quando você diz "vim": `vim` e `vi` apontam pra `/usr/bin/nvim`, e a config lida é `~/.config/nvim/init.lua`. O modo de edição é permanente: abre em inserção, `Ctrl+S` salva, `Ctrl+Z` alterna. Motivo: os comandos de uma tecla do modo normal apagavam coisa sem avisar. Você também usa VS Code e IntelliJ.

### 1.2 O nome do assistente e a assinatura dos commits

- Em 2026-09-05, depois de um dia de regressões, você disse: "Mude tudo de Claude pra Regressão, pois seu nome vai ser esse agora". O nome serve de lembrete de que cada mudança do assistente quebrava algo que existia. Ele só perde o sentido quando o assistente mede antes de afirmar e não quebra o que existe.
- Trailer de todo commit: `Co-Authored-By: Regressão <noreply@anthropic.com>`. O e-mail fica como está, porque é ele que diz de onde veio o commit.
- Menção em prosa no repositório que cite "Claude" como autor passa a ser "Regressão".
- Caminhos de ferramenta **não mudam**: a pasta `.claude/` (o `settings.json` dela é versionado desde que o repositório é privado) e o nome de arquivo `CLAUDE.md`. Renomear quebra o carregamento.
- Identidade do git: autor `kleberDevion <klebersantanadeoliveira07@gmail.com>`.
  - De 2026-09-19 a 09-27 não havia `~/.gitconfig`, e o commit exigia `-c`.
  - Hoje existe `~/.gitconfig`, criado em 2026-09-27, com essa identidade.
  - A config da máquina é sua. No Mac, ou você cria a config global, ou o assistente commita com `-c`:

```
# no Mac, feito por você (a config global é sua):
git config --global user.name "kleberDevion"
git config --global user.email "klebersantanadeoliveira07@gmail.com"

# sem config global, o assistente commita assim:
git -c user.name="kleberDevion" -c user.email="klebersantanadeoliveira07@gmail.com" commit -F - <<'EOF'
Assunto: o que mudou, no tom dos commits do repo

- o que foi feito, com o número medido
- Versao X.Y.Z.

Co-Authored-By: Regressão <noreply@anthropic.com>
EOF
```

### 1.3 Regras de trabalho

Cada linha traz a regra e o porquê.

#### Processo

| Regra | Por quê |
|---|---|
| Toda mudança não trivial (mais de uma função, mudança de comportamento ou decisão de API) começa em **modo de plano**. Sequência: explorar e mapear todo consumidor com arquivo:linha; planejar com as decisões já resolvidas; escrever o arquivo do plano (contexto, decisões, passos que compilam um a um, testes, verificação com `timeout` e `nice`); esperar a aprovação; executar como está escrito. | Editar direto gerava acréscimo não pedido e conserto de um em um. Com plano, em 2026-09-11 foram 13 edições de uma vez e 7733 casos verdes. Você pediu "prossiga sempre assim". |
| O plano **propõe** nome ou API com o porquê e avisa que você pode derrubar qualquer decisão. Achou algo fora do plano depois de aprovado: relata, não edita. | Nome e API são decisão sua. |
| Fazer **só o que foi pedido**. Citar, apontar, colar um trecho ou reclamar é diagnóstico, não ordem. Editar só com verbo no imperativo dirigido àquilo. | Em 2026-09-05 o que quebrou veio de acréscimos (aviso de escopo, bloqueio de `.ps` no estático), e você bloqueou a execução. |
| Achado em volta do pedido vira **uma linha** no relato, sem mexer. Exceção: buraco de segurança medido, e aí fecha só o buraco. | Cada "melhoria" em volta é regressão em potencial num sistema que você conhece inteiro. |
| Linha que você já editou está decidida. Não "terminar o trabalho" por conta própria. | Em 2026-09-11 o assistente trocou `name`, `modo` e `mode` três vezes sem pedido. |
| Execução negada: não repetir nem variar o comando. Deixar o estado escrito e os comandos prontos pra você. | Insistir no comando negado é desobedecer. |
| **Nada pela metade, nada "pra depois"**. Fecha 100% ou diz de cara por que não fecha. Não para no meio pra relatar. Não termina resposta com "quer que eu faça X depois?". | "Ou faz ou nem faz." Pra você a palavra "pendente" é falha, e item solto gera ansiedade. |
| Feature entregue = **quatro eixos**: motor, doc verificada, editor (completion, hover, definição) e teste. Ao tocar uma superfície, auditar os membros vizinhos. | Feature "jogada" derruba a sua confiança na linguagem inteira. O que o motor aceita e o editor não mostra é "ofuscado" pra você. |
| **Bump de versão em toda alteração**, no mesmo commit: `jinga scripts/bump.pr patch\|minor\|major`, depois `make pool`, depois instalar binário e docs, depois derrubar o processo do LSP. | Você confere `jinga --version`. Sem bump, roda o binário velho e conclui que o assistente mentiu (aconteceu em 2026-09-09/10). |
| Rótulo do bump decidido pelo conteúdo: patch = correção sem mudar comportamento válido; minor = novidade compatível; major = quebra código existente. Os dígitos 2 e 3 viram em 100 (`8.2.99 -> 8.3.0`). A extensão tem versão própria (`bump.pr ext patch`). Nunca ceder pra agradar. | Em 2026-09-26 a classe sem `()` quebrou compatibilidade e virou 16.0.0. Você rejeitou "então uso só patch". |
| Doc, `examples/`, testes e notas que descrevem o comportamento mudam **no mesmo commit**. Antes de fechar, dar `grep` pelo nome antigo. | Doc velha fez o assistente caçar bug inexistente: os `examples/` com `//` quebraram e foram investigados dias depois. |
| **Número medido nunca vai em prosa**: sai de um comando ou não entra no texto. | Número em prosa fez a doc dizer `ps_mail.c 16,9%` quando eram 56,9%. |
| `make check` inteiro verde antes de qualquer commit. Neste laptop, rodar em lotes ou na VPS. | `./testar` sozinho não pega doc, editor nem exemplos. |
| Commitar cada pedaço fechado assim que o portão passa. | Em 2026-09-06 um rebase seu apagou do disco 12 arquivos sem commit. Salvou o autostash, não o assistente. |
| Se o repositório mudar por baixo (rebase seu, edição no GitHub): parar, levantar `git status`, `git reflog` e `.git/rebase-merge/`, contar pra você e esperar. No conflito, a sua versão manda. | O rebase em curso é operação sua. |
| Depois de instalar o servidor do editor, derrubar o processo antigo com `kill` no pid de `pgrep -f "^node /usr/local/share/jinga/lsp/server.js"`. Nunca pedir pra você recarregar, e nunca usar `pkill -f` com o padrão dentro do próprio comando (mata o shell). | O VS Code segue com o processo velho, você vê o comportamento antigo e conclui que nada mudou. |
| Quando você disser "anota na trilha", escrever em `memory/trilha-de-dev.md` na hora e confirmar na resposta. | A trilha existe pra você não ter que lembrar o assistente. |

#### Verdade e medição

| Regra | Por quê |
|---|---|
| **Nada de falso verde**. Nenhum skip, nenhuma gambiarra. Teste que não rodou é declarado como não rodado. Saída do motor só vira expectativa depois de conferida. | Um `skipif` escondeu um segfault da VM (2026-08-22): "EU QUERO RESULTADO VERDADEIRO". |
| Nunca afirmar "o motor faz/não faz X" sem ter rodado X. Se não deu tempo, escrever "vou medir" e medir. | Em 2026-09-05 "o motor nunca serve `.ps`" era falso, e você chamou de mentira. |
| Toda classe de vetor entra na suíte como **lista** (laço), nunca um ou dois exemplos. | Testar 2 formas de travessia deixou 23 de fora (25 vetores no total). |
| Regra que existe em dois lugares do motor é bug latente: unificar numa função, nunca copiar lookahead. | O decorador tinha uma cópia do lookahead que perdia `int async`, e a rota sumia calada (404). |
| Método de correção: gerar a matriz combinatória, medir a categoria inteira, mostrar a lista com os números, corrigir em lote, medir de novo. | Patch no sintoma fazia você achar o bug irmão minutos depois. Sobre o método: "se sempre fosse assim não demoraria tanto". |
| Desempenho se mede com `sudo perf stat -e instructions,cycles`: faixa de 5 rodadas, binário instalado contra o novo (`perf_event_paranoid=4`; senha do sudo na seção 7). O `teste/bench.pr` mede em rodízio. | O wall-clock deste laptop varia 2x entre execuções iguais (máquina térmica). "Se o tempo aumentar vc refaz." |
| O `perf` é do kernel Linux e não existe no macOS. A comparação de instruções continua numa máquina Linux (a VPS nunca foi medida pra contador de hardware). | Fato da plataforma. |
| Queixa sua com arquivo real: rodar **o seu arquivo** primeiro e achar a causa antes de sondar variantes. | Em 2026-09-28 a causa era um typo, e a sonda virou tabela de padrões. |
| CI verde só vale se medido na imagem do CI: container `ubuntu:24.04` com a preparação da imagem do GitHub, passo do `ci.yml` com o texto exato, suíte como usuário comum. Nunca "Ubuntu limpa". | Depois de "CI compila de novo" (7d734a9), o CI ficou vermelho de 20/09 a 22/09 pelo libpq 18 pré-instalado no runner. |
| Tropeço escrevendo Jinga é bug ou limitação candidata: corrigir no motor, nunca contornar. | Compromisso do self-hosting: "dane-se as limitações, anota e corrige". |

#### Código e correção

| Regra | Por quê |
|---|---|
| Correção por **regra geral**, do tipo que outras linguagens têm (sugestão por distância, erro na causa). Nunca `if (texto == "...")` pra um engano específico. | "Sério que vc criou uma tabela com o caso que te dei?" (2026-09-28). O mecanismo geral (b51dfd7) você aprovou: "top, prossiga sempre assim". |
| Erro sai na **causa** (declaração, assinatura), um só por defeito, com recuperação pra não cascatear. | O `self` saiu sublinhado três linhas abaixo da assinatura errada: "pq diabos não mostra o erro no local correto" (2026-09-17). |
| Você diz que a mensagem de erro "está errada": melhorar a mensagem (apontar a causa, sugerir o nome, dizer o local) no motor e no checador, com teste e doc. Nunca provar que o typo era seu. | Caso `raplace`: "deixa de ser muito literal" (2026-09-20). |
| Reclamação sua = **hipótese de bug do motor** até prova em contrário. "Não tem X?" é buraco. Reproduzir a partir do código do motor, nunca apontando erro no seu código. | Placar de 2026-08-24: 11 bugs reais. |
| Nunca transformar uso válido em erro nem limitar funcionamento por conta própria. Caso difícil se implementa do jeito consagrado. | O commit db50393 recusou decorador em método e quebrou o `~/projetos/painel` (2026-09-14). |
| Não perguntar o que decorre de um princípio que você já deu. Perguntar só nome ou API das suas libs, gosto, ou restrição que você queira impor. | "Se fosse incapaz de decidir não saberia acusar o erro." |
| **Sem conversão implícita**: só `tipo(valor)`. `char c = 64` dando `"@"` e `long y = 5` **não** são conversão e não se mexe neles. | Quando o `char` foi mexido em 2026-09-09, você explodiu, e a mudança foi revertida. |
| Palavra que sai da linguagem some do motor, do `--ast`, do editor e dos comentários **no mesmo lote**. Compatibilidade só vale pra nomes de comando, binário e caminho. | Você viu `kind === 'action'` no `server.js` (2026-09-26): "um monte de nome legado, isso me irrita". |
| O editor nunca copia regra do motor. Lê `--ast`, `--metadata`, `--tokens` e `--check`, e não tem **nenhuma expressão regular** nos `.js`. | Lista cravada no `server.js`: "vc ta definindo denovo". Regex nos `.js` era uma segunda gramática que sempre ficava sem um ramo. |
| Queixa de hover ou completion vai direto em `editor/vscode/server.js`/`analise.js` e no `--metadata`, sem repro no runtime. Uma leitura, uma edição, o teste de regressão. Uma posição só. | Três sondas seguidas: "200000 agents em tudo, vc começou me irritar". |
| Nunca pedir o seu arquivo nem prova: reproduzir pelo padrão que você descreveu (`cur = con.cursor()`). "Continue" = voltar à tarefa anterior sem perguntar. | "Se acha que é mentira nem faz." |
| Caso isolado não se corrige sozinho: matriz sobre todo o `--metadata` e regra geral. | "Definir a mão como vc ta fazendo não dá" (2026-09-26, `sys.stdout.`). |

#### Texto

| Regra | Por quê |
|---|---|
| **Sem emoji** em lugar nenhum: código, comentário, doc, teste, commit, resposta. Nem símbolo pictográfico (estrela, check, sinal de aviso). Antes de commitar, `grep` no diff pelas faixas U+1F000-1FAFF, U+2600-27BF, U+2B00-2BFF, U+2300-23FF, FE0F, 200D e U+1D100-1D1FF. Teste de caractere de 4 bytes usa a letra CJK U+20BB7. Aviso em doc é texto ("Cuidado", "PERIGO"). | "Eu odeio emoji, isso é de IA burra" (2026-09-22). |
| **Sem nome de biblioteca ou framework de outra linguagem** em lugar nenhum: código, comentário, doc, dica de erro, teste, nota, commit. Explicar o conceito pelo que ele faz. Antes de commitar, `git grep -n -i` no diff. Módulo de fachada (stub) não existe. | A Jinga é sua, e nome alheio é a linguagem se apresentando como cópia. Você apagou `notas/` inteiro por isso (2026-09-05). |
| **Sem hedge**: nunca "risco", "arriscado", "talvez". Dizer o fato medido e a consequência concreta ("código que usa X para de compilar"). Quando não sabe, dizer "não medi isso". | Ressalva atrasa a sua decisão sem acrescentar informação. |
| Doc no nível das referências oficiais de Python e Java: sintaxe, parâmetros (tipo, padrão, efeito), retorno, erros, bordas, exemplos. Cada afirmação é verificada rodando. A assinatura do título sai do código (`scripts/audita_doc.pr`). Corrigir a categoria, não só o caso apontado. | "Você faz na velocidade da luz mas SEM QUALIDADE." |
| Não fragmentar doc em um `.md` por membro pequeno quando um só dá conta. | "Isso é ridículo pra quem ler." |
| Ajuda, doc e exemplos respeitam a divisão: `jinga` roda, `jpkg` cuida de pacotes. | Em 2026-08-09 o assistente trocou todo o help pra `psl` lendo uma frase sua sobre pacotes. |

#### Ferramentas

| Regra | Por quê |
|---|---|
| Script de apoio, automação, auditoria e gerador de caso são **escritos em Jinga** (`.pr` rodado no `./pool`). | "A partir de hoje vc vai usar só pool pra programar" (2026-08-24). No 1º dia rendeu 3 bugs reais. |
| Todo trecho `.pr` que o assistente escreve (teste, doc, exemplo) passa por `jinga --check` antes de virar arquivo. | O assistente erra o básico por analogia com Python (ver 1.4). |
| Editar arquivo **só com as ferramentas de edição** (Edit/Write). Nunca `sed -i`, `awk`, nem script de substituição. Ler com `grep` ou `sed -n` pode. | Substituição por script casa onde não devia, fica calada quando não casa e não deixa rastro. Você cobrou "para de preguiça". |
| Edit com `new_string` terminado em espaço perde o espaço e cola palavras (`functaninhada`): conferir com `grep` depois. | Aconteceu nos renames de `action` e Jinga. |
| Faltou banco, binário ou pacote pra testar: instalar e usar sem pedir (na VPS: apt oficial, Docker). Seus dados ficam intocados, o serviço fica desligado fora da rodada, e a limpeza é total. | "Era só baixar os bancos. Eu nunca proibi você disso" (2026-09-21). |

#### Máquina

| Regra | Por quê |
|---|---|
| `timeout` em **todo** comando, e `nice -n 19` no que é pesado (`timeout 900 nice -n 19 ...`). | `nice` sem `timeout` deixou processo pendurado segurando a máquina (2026-09-04). |
| **Um alvo pesado por comando**, esperando terminar. `&&` e `;` contam como vários. | O VS Code caiu duas vezes com `make pool && make testar && ./testar` (2026-09-08/09). |
| Suíte em lotes. `make check` inteiro neste laptop morre no OOM killer (exit 137). | 7,7 GB de RAM, cerca de 3,5 GB livres com o desktop aberto (medido 2026-08-26). |
| `uptime` antes de job pesado: com load acima de 8, espera. | O load chegou a 21 com o IDEA indexando. |
| Matar todo processo que o assistente criou (servidor LSP, jinker, xvfb). | Processo órfão segura a máquina. |
| `ulimit -v` em toda ferramenta de análise. | `gcc -fanalyzer` no arquivo grande da VM passa de 6 GB, e o OOM derrubou a sessão duas vezes. |
| E2E com editor real só na validação final. | É o job mais pesado. |
| Build, suíte inteira, ASan, OOM e e2e rodam na **VPS `tecsol-vps`** (desde 2026-09-20). Detalhes logo abaixo da tabela. | "Tá impossível essa máquina, faça build e rode a suíte lá"; "não deixe resquício nenhum". |
| Na VPS, o Postgres 16 da porta 5432 é **seu**. O banco `ps_teste` é criado antes e apagado depois de cada rodada. | Dado seu não se destrói. |
| As regras de carga foram escritas pra este laptop (i3-1115G4, 7,7 GB). O Mac (i9-9880H, 16 GB) ainda não foi medido: medir lá antes de relaxar qualquer uma. | Número só medido. |

Roteiro da VPS: pasta temporária em `/tmp/ps-lote-N`, suíte rodando como `nobody`, `nohup setsid`, log fora da pasta, e limpeza total no mesmo script (pasta, sobras em `/tmp`, cache do npm). Localmente fica só o leve.

#### Seu código e suas decisões

| Regra | Por quê |
|---|---|
| **Design de API é seu**: nomes, classes, métodos, assinaturas, parâmetros, e o texto de mensagem que você só apontou. O assistente faz o encanamento do motor e o ferramental (editor, doc, teste) em cima do que você entrega. | "O DESIGN DE API SOU EU QUE FAÇO." Uma classe `CorsConfig` inventada te irritou. |
| Suas libs (`~/.jinga/libs`: pondy, roken, random, swagger, Controllers, e as outras) são território seu. Nem crítica, nem FYI. Quando o assistente lê uma delas só pra reproduzir bug do motor, usa o arquivo como dado e fica calado sobre o conteúdo. | "O pondy é meu, o arg eu boto o que eu quiser" (2026-08-22). |
| Seus projetos: entregar a **lista** de erros achados, não editar. Exceção: o que um plano aprovado autorizar. | Na tipagem estática só as 6 chaves do `oauth` do painel foram migradas, por autorização. |
| Conserto não se estende a caminhos que você não pediu. | "O que é meu é meu, não mexe" (`@static` no callback). |
| Decisão que você já deu não volta como pergunta. A lista está em 6.7. | "Eu não vou responder pois eu já respondi lá em cima da sessão." |

### 1.4 O básico da linguagem que o assistente erra

Conferido rodando `jinga` 16.1.7 em 2026-09-28 quando marcado "medido"; o resto vem da nota `feedback-basico-da-linguagem`.

| Escrito errado | Certo |
|---|---|
| `// comentário` | `#`. O `//` é divisão inteira (medido: `7 // 2` dá `3`; `x = 5 // comentario` dá NameError). |
| `class X() {` | `class X {`. Herança: `class X(Pai) {`. `()` vazio é SyntaxError (medido). O `model X() {` mantém os parênteses. |
| `nome: str[20]` | `nome: str(length=20)` |
| `open(x).read()` / `open(x, "w").write(y)` | `os.readFile(x)` / `os.writeFile(x, y)`. Você proibiu a forma com `open`. |
| `(x := 1)` | Não existe. |
| `for i in range(n)` / `l.addEnd(x)` | `for each i in range(n)` (medido) / `l.append(x)` |
| `os.cmd(...)` devolvendo a saída | Ecoa e devolve `Null` (medido). Redirecione pra arquivo e leia. |
| Linha nova começando com `+ b` | Não continua a expressão: o resto é descartado calado (medido: `c = a` seguido de `+ b` deixa `c = 1`). Use parênteses. |
| `os.remove(arq)` | Não existe (medido: "Did you mean: 'move'?"). Use `PoolFile.delete()` ou `os.cmd("rm -f ...")`. |
| Variável criada dentro de `if`/`for each` e usada depois | Escopo de bloco: rodando dá NameError (medido). Declare antes do bloco. |
| `type(e)` no `catch (e)` pra saber o tipo | `e` é `str` (medido). O tipo se confere com `catch (ValueError e)`. |
| `count=` como parâmetro, variável `base` | `count` e `base` são palavras reservadas. `base(Pai).membro` é o acesso ao pai. |
| Handler do jinker com `(req)` | O handler não recebe a requisição. Lê `jinker.request`. Parâmetro com nome de path param recebe o valor dele (I18). |
| `"\d+"` em regex | A string come a barra: use `r"\d+"`. `regex.match` casa a string **inteira**. |
| `x in d` pra testar valor | `in` testa a chave. Valor: `x in d.value()` (medido). |
| `"\x89PNG...".encode()` como binário | `\x89` vira dois bytes UTF-8. Binário de verdade: `bytes.fromhex(...)`. |
| `printf '...%s...'` no shell pra gerar `.pr` | O printf come o `%s`: gere o arquivo com Write. |
| `.ps`, `.psl`, `.p` | A extensão é só `.pr` (desde 2026-09-15). |
| `{k: 1}` achando que `k` vira texto | A chave do literal de dict é **expressão** (medido: com `k = "z"`, `{k: 2}` dá `{'z': 2}`). Texto vai entre aspas. |
| `sys.argv` sem o nome do script | `sys.argv[0]` é o caminho do script como foi passado (medido: `['d.pr', 'um', 'dois']`). A nota antiga dizia o contrário. |

### 1.5 O que mais irrita (casos que já aconteceram)

- Emoji ou símbolo pictográfico em qualquer lugar.
- Nome de lib de outra linguagem: no import, na dica de erro, no comentário.
- Perguntar de novo o que você já decidiu, ou perguntar o óbvio.
- Acréscimo não pedido, e mexer no que você só citou.
- "Risco", hedge, relatório que afirma sucesso sem o dado que prova.
- Falso verde: skip, teste que "passou" sem rodar, CI dito verde sem medir.
- Feature sem editor, sem doc ou sem teste.
- Nome legado vazando no `server.js` ou no `--ast`.
- Tabela de padrões no lugar de regra geral.
- Rodada de sondas antes de agir, ou pedir o seu arquivo como prova.
- Provar que o typo era seu em vez de melhorar a mensagem.
- Mexer no `char`.
- Editar com `sed` ou script.
- Máquina travada por job pesado.
- Deixar coisa pela metade, ou parar pra resumir em vez de continuar.

## 2. A linguagem Jinga

Guia de referência para retomar o projeto. Cada exemplo `ps` abaixo foi rodado no binário desta máquina (`./pool`, que respondia `Jinga 16.1.6 [PSVM] (2026-09-28)` na hora; no fim do dia o fonte e o binário estão em 16.1.9). Cada mensagem de erro citada é a que o motor imprime hoje. A referência completa fica em `docs/LANGUAGE.md` e `docs/linguagem/01..14`. Quando esses textos discordam do motor, o motor está certo (ver 2.21).

### 2.1 Visão geral

| Item | Fato |
|---|---|
| Nome | Jinga. Antes se chamava PoolScript. O rename foi em 2026-09-24 |
| Extensão | `.pr`, e só ela. `.ps`, `.psl` e `.p` saíram. Para migrar: `jinga scripts/migra_pr.pr <pasta> --aplica --libs` |
| Tipagem | Estática: tudo que tem tipo é conferido **antes de rodar**. Não existe conversão implícita |
| Bloco | `{ }`, e só isso. A indentação é estética. `:` não abre bloco (a única exceção é o guard `main`, ver 2.7.4) |
| Motor | VM em C (`vm/`): lexer, parser (AST), compilador, bytecode e VM. No x86-64 cada funct vira código de máquina na primeira execução (`vm/ps_jit_x64.h`) |
| Binário | `jinga` roda, `jpkg` gerencia pacotes. É o mesmo binário. `pool` e `psl` são atalhos antigos que continuam instalados |
| Versão | A fonte é `vm/ps_versao.h`, hoje em `16.1.9` (bump no commit `3c245e7`). O `./pool` e o `/usr/local/bin/jinga` desta máquina respondem `16.1.9` (medido no fim de 2026-09-28). `make pool` e a instalação alinham os dois |
| Lista viva de keywords, builtins, módulos e exceções | `jinga --metadata` (JSON) |

### 2.2 Rodar e conferir

```bash
jinga arquivo.pr              # roda
jinga arquivo.pr a b          # sys.argv == ['arquivo.pr', 'a', 'b']
jinga -e 'post(1 + 1)'        # código inline
jinga build                   # roda todos os .pr da pasta atual
jinga arquivo.pr -o app       # executável que roda sozinho (fonte + módulos embutidos)
jinga --check arquivo.pr      # só analisa; JSON com TODOS os erros de tipo
cat x.pr | jinga --check      # lê da entrada padrão
jinga --check --path x.pr     # stdin conferido como se fosse x.pr (acha vizinhos)
jinga --ast / --tokens / --bytecode arquivo.pr
jinga --sem-jit arquivo.pr    # só interpretador (ou JINGA_JIT=0)
jinga --debug <porta> arquivo.pr   # Debug Adapter Protocol (F5 no VS Code)
jinga --metadata              # keywords, builtins, módulos, exceções em JSON
jinga --version
```

Códigos de saída medidos:

| Situação | rc |
|---|---|
| Rodou sem erro | 0 |
| Erro de sintaxe ou de tipo detectado antes de rodar. O programa **não roda** e imprime `N erros de tipo — o programa nao rodou.` | 2 |
| Exceção não capturada rodando (inclusive erro de tarefa `async` sem `await`) | 1 |
| `--check` com `{"ok":true}` / `{"ok":false,...}` | 0 / 1 |

Formato do `--check` com erro (saída real):

```
{"ok":false,"tipo":"AttributedValueError","msg":"variável x é int (tipo fixado na primeira atribuição), recebeu str","linha":2,"coluna":5,"erros":[{...,"l2":2,"c2":8},{"tipo":"NameError","msg":"name 'vlaor' is not defined. Did you mean: 'valor'?",...}]}
```

O REPL não existe: `jinga repl` responde isso e sugere `-e`.

### 2.3 Estrutura léxica

- **Comentário**: `# linha` e `""" bloco """`. **`//` é divisão inteira**, não comentário.
- **String**: `"..."` e `'...'` são equivalentes. Multilinha usa `'''...'''` (aspas **simples** triplas, porque `"""` é comentário). Também existem `f"...{x}..."`, `r"..."` (cru), `f'''...'''` e `r'''...'''`.
- **Escapes**: `\n \t \r \\ \" \' \a \b \f \v \e` (ESC), `\033`, `\xHH`, `\uXXXX`, `\UXXXXXXXX`. Um escape desconhecido mantém a barra e gera `SyntaxWarning` no stderr (o programa roda).
- **Bytes**: `b"\x89PNG"` é do tipo `byte`. `br"..."` é cru e `b'''...'''` é multilinha. `\u` não vale em bytes.
- **Cor**: `<red>"texto"`, `<2196f3>"texto"`, `<f00>"texto"` geram ANSI 24-bit e fecham sozinhos. Nomes aceitos: `red green blue yellow cyan magenta white black purple orange pink gray grey lime teal`. Hex de 4 ou 5 dígitos não é cor.
- **Booleano**: `True`, `False`. **Nulo**: `Null`. Só essa grafia, a mesma que o motor imprime (desde o 16.2.7); `true`, `false`, `null`, `None` e `none` são nomes comuns, e usar um deles dá `NameError` antes de rodar, com a dica da grafia certa.
- **Números**: `42`, `3.14`, `1e5` (sempre `flo`). `.5` é erro: o dígito antes do ponto é obrigatório. Um inteiro maior que 64 bits vira bignum automaticamente, e `type()` continua `int`.
- **Identificador**: `[A-Za-z_][A-Za-z0-9_]*`, **sem acento**. Maiúscula inicial é convenção para classe, lib e tipo de erro. `catch (Tipo e)` e `raise Tipo(...)` exigem maiúscula.
- **Continuação de linha**: dentro de `( [ {` a quebra de linha é livre. Uma linha que começa com `.membro` continua a expressão anterior. Uma linha que começa com `+` **não** continua a anterior: dá `SyntaxError: expressao invalida`. Use parênteses.
- **Palavras reservadas** (doc 01 §1.5): `if elif else while for each in is match case break continue pass return and or not Not funct async await yield str int long flo bool list dict tup json char Object object Entity class Class self base model enum private public import from as PUSH GET try catch finally raise global using to count`. `static`, `nonnull` e os apelidos de tipo valem só **por posição**. `base` e `count` não podem ser nome de variável nem de parâmetro.

```ps
# comentario de linha
"""
comentario de bloco,
pode cruzar linhas
"""
nome = "mundo"
post(f"ola, {nome}!")
post('aspas simples valem igual')
sql = '''SELECT *
FROM usuarios'''
post(sql)
post(r"C:\temp\novo")
post(<red>"erro", <2196f3>"azul", "sem cor")
b = b"\x89PNG"
post(type(b), len(b), b[0])            # byte 4 137
post(7 // 2, 7 / 2, 2 ** 10, -7 % 3)   # 3 3.5 1024 2
post(True, True, Null, Null, Null)     # True True Null Null Null
```

### 2.4 Tipos e tipagem estática

#### 2.4.1 Tipos

| Tipo | `type()` | Literal | Observação |
|---|---|---|---|
| `int` | `int` | `42` | 64 bits com promoção a bignum. Uma declaração `int` recusa bignum; use `long` |
| `long` | `int` | — | Inteiro de qualquer tamanho. É restrição da declaração, não tipo de runtime |
| `flo` | `flo` | `3.14` | double IEEE-754 |
| `bool` | `bool` | `True` | Subtipo de int em aritmética (`True + True == 2`) |
| `str` | `str` | `"x"` | UTF-8. Índice, fatia e `len` contam **caracteres** |
| `char` | `str` | `"x"` ou `64` | Um caractere. Um inteiro vira codepoint **só na declaração** |
| `list` | `list` | `[1, 2]` | Mutável |
| `tup` | `tup` | `(1, 2)` | Imutável. Só tem `index count contains has len` |
| `dict` / `json` | `dict` | `{"k": v}` | Mutável. Ordem de inserção |
| `byte` | `byte` | `b"..."` | Imutável. Indexar dá int. `bytes` é o **módulo** |
| `Object` / `object` | nome da classe | — | Qualquer objeto: instância, servidor, conexão, arquivo, funct |
| `Null` | `Null` | `Null` | Ausência de valor |

Apelidos (valem só onde cabe um tipo): `string`/`String` = `str`, `integer`/`Integer` = `int`, `tuple`/`Tuple` = `tup`, `dictionary`/`Dictionary`/`json`/`JSON` = `dict`, `object` = `Object`, `Long` = `long`.

#### 2.4.2 Regras da tipagem estática (aprovadas pelo dono em 2026-09-16)

1. Tudo que tem tipo é conferido **antes de rodar**: declaração, reatribuição, parâmetro e o padrão dele, retorno, saída da funct tipada, campo de class e model, `self.campo`, `obj.campo`. O tipo escrito tem que existir.
2. Uma variável **sem tipo** tem o tipo fixado na primeira atribuição, como o `var` do Java. `Null` não fixa o tipo nem conflita com ele.
3. **Sem conversão implícita.** Converter é sempre `tipo(valor)`. `char c = 64` (vira `"@"`) e `long y = 5` **não** são conversão e devem ficar como estão.
4. Uma chamada a funct ou método conhecido (inclusive de `.pr` importado) tem aridade, nomes e tipos conferidos antes de rodar. Método que o tipo não tem é erro antes de rodar.
5. Todo código é conferido, inclusive o que nunca executa.
6. O que só se sabe rodando (`json.parse`, `d["k"]`, `l[0]`, `d.get`) é conferido rodando, no lugar tipado. A frase do erro é a mesma nos dois momentos.
7. Um nome que o arquivo nunca liga é `NameError` antes de rodar, com `Did you mean`.
8. A chave do literal de dict é uma **expressão**: `{k: 1}` usa a variável `k`, e texto vai entre aspas.
9. Redeclarar com **outro** tipo é erro (`int y = 1` e depois `long y = 2`). Redeclarar com o mesmo tipo passa.

| Alvo | Aceita | Recusa |
|---|---|---|
| `int` | int | flo (mesmo `5.0`), str (mesmo `"7"`), bool |
| `flo` | flo | int (mesmo `5`), str, bool |
| `str` | str | int, flo, bool |
| `bool` | bool | int (mesmo `1`) |
| `long` | inteiro de qualquer tamanho | o resto |
| `byte` | bytes | str |
| `Object` | instância, servidor, conexão, arquivo, funct | str, list, dict, tup, byte, números, Null |

```ps
int idade = 30
str nome = "ana"
flo preco = 9.9
bool ativo = True
char letra = "x"
char arroba = 64
long grande = 99999999999999999999
list itens = [1, 2]
dict mapa = {"a": 1}
tup par = (1, 2)
String apelido = "tambem str"
x = 10
n = int("7")
f = flo(5)
s = str(42)
post(idade, nome, preco, ativo, letra, arroba, grande, itens, mapa, par, apelido, x, n, f, s)
post(type(arroba), type(grande), 5 is int, "a" is not int)   # str int True True
```

Erros de tipo (mensagens reais, todas antes de rodar, rc 2):

```
int n = "7"      -> AttributedValueError: variável n esperava int, recebeu str
str s = 10       -> AttributedValueError: variável s esperava str, recebeu int
x = 1
x = "a"          -> AttributedValueError: variável x é int (tipo fixado na primeira atribuição), recebeu str
int y = 1
long y = 2       -> AttributedValueError: variável y já foi declarada como int e não pode ser redeclarada como long
int n = input()  -> AttributedValueError: variável n esperava int, recebeu str|Null
p.nome = 5       -> AttributedValueError: campo nome de P esperava str, recebeu int
```

#### 2.4.3 Conversão explícita

- Builtins `int()`, `flo()`, `str()`, `bool()`, `list()`, `tup()`, `dict()`, `long()`. Se não der para converter, estouram (`int("abc")` dá `ValueError`).
- `Parsing` é global, sem import, e faz conversão tolerante com alvo `to <tipo>`: `Parsing.integer("  127.abc ", to int)` devolve `127`. `"7" to int` sozinho não converte nada.
- `input()` devolve `str`, ou `Null` no fim da entrada. Para número, escreva `int(input())`.

#### 2.4.4 Dict: chave é expressão

```ps
k = "a"
d = {k: 1, "tls": True}
post(d)                                    # {'a': 1, 'tls': True}
post("a" in d, 1 in d.value())             # True True  (in olha a CHAVE; valor via .value())
d.nova = 2                                 # chave por atributo == d["nova"] = 2
post(d["nova"], d.get("falta", "padrao"))  # 2 padrao
for each chave, valor in d.items() {
    post(chave, valor)
}
```

`{tls: True}` sem a variável `tls` dá `NameError: name 'tls' is not defined`. Uma chave ausente em `d["x"]` ou `d.x` dá `KeyError`, e `d.get(k, padrao)` nunca dá erro. Um método tem prioridade sobre uma chave de mesmo nome.

### 2.5 Variáveis, escopo e atribuição

- **Escopo de bloco**: cada `if`, `elif`, `else`, `while`, `for each`, `count each`, `try`, `catch`, `finally`, `case` e o guard `main` abre um escopo. Um nome **novo** criado no bloco não sai dele. Atribuir a um nome que já existe fora atualiza esse nome (write-through). O corpo de um laço reinicia a cada volta.
- **Exceção**: as variáveis do `using` e do corpo dele sobrevivem depois do bloco.
- `global nome` dentro de uma funct é obrigatório para **criar** uma global lá de dentro.
- Não existe atribuição encadeada (`a = b = 5` dá `SyntaxError: expressao invalida`), walrus (`:=`), `++x` prefixo nem atribuição por fatia.
- `++` e `--` são só pós-fixos e devolvem o valor anterior.
- O desempacotamento aceita `*resto`, aninhamento e alvo membro ou índice (`l[i], l[j] = l[j], l[i]`).
- A declaração tipada exige valor: `int x` sozinho dá `declaracao de variavel exige '='`.
- `private` só no topo do arquivo: `private x = 1`, `private int n = 1`, `private a, b = 1, 2`.

```ps
total = 0
for each i in [1, 2, 3] {
    total += i
}
post(total)                # 6

resultado = ""             # declarada FORA para sobreviver ao if
if total > 5 {
    resultado = "grande"
}
post(resultado)

contador = 0
funct soma_um() {
    global contador
    contador = contador + 1
}
soma_um()
post(contador)             # 1

a, *resto = [1, 2, 3]
b = 9
a, b = b, a
post(a, b, resto)          # 9 1 [2, 3]
x = 5
y = x++
post(x, y)                 # 6 5
```

### 2.6 Operadores

Precedência, do mais fraco para o mais forte: ternário `A if C else B` < `or`/`||` < `and`/`&&` < `not`/`!` < comparação, `is`, `in`, `count` < `|` < `^` < `&` < `<<`/`>>` < `+ -` < `* / // %` < unários `+ - ~ await` < `**` (à direita) < pós-fixos `() . [] ++ --`.

| Regra | Fato |
|---|---|
| `/` | Sempre `flo` (`10 / 5` dá `2.0`) |
| `//` | Divisão inteira com piso (`-7 // 2` dá `-4`) |
| `%` | Tem o sinal do divisor (`-7 % 3` dá `2`) |
| `**` e `pow(b, e, mod)` | Potência. `-2 ** 2` dá `-4`. `2 ** 3 ** 2` dá `512` |
| `+` | Soma números ou concatena sequências do mesmo tipo. **Não coage**: `"a" + 1` dá `TypeError: can only concatenate str (not "int") to str` |
| `*` | Sequência vezes int repete: `"-" * 40`, `[0] * 3` |
| `==` | Compara valor: `1 == 1.0`, `1 == True`, estrutura de list, tup e dict. `5 == "5"` é False |
| `===`, `!==` | Não existem: `` `===` nao existe nesta linguagem; use `==` `` |
| `<` com `Null` | `TypeError: '<' not supported between instances of 'Null' and 'int'`. `Null == 0` é False |
| `a < b < c` | Não encadeia, é `(a < b) < c`. Escreva `a < b and b < c` |
| `and`, `or` | Devolvem **sempre bool** (`0 or "x"` dá `True`). Para valor padrão use o ternário |
| `is`, `is not` | Testam **tipo**: `x is int`, `x is Null`. `x is 5` é SyntaxError |
| `in`, `not in` | Lista/tup: elemento. str: substring. dict: **chave** |
| Bitwise | Só entre int (bool entra como 0/1, e `~True` é erro) |
| `count` | `int(7) count in l` e `count int(7) in l` contam o valor. `int in l count` e `count int in l` contam o tipo |

### 2.7 Controle de fluxo

#### 2.7.1 if / while / for each / compreensão

```ps
nota = 8
if nota >= 7 {
    post("aprovado")
} elif nota >= 5 {
    post("recuperacao")
} else {
    post("reprovado")
}

n = 0
while n < 3 {
    n += 1
}

for each i in range(10, 0, -2) {
    if i == 4 {
        break
    }
    post(i)                              # 10 8 6
}

for each nome, idade in [["ana", 30], ["bia", 25]] {
    post(nome, idade)
}

post([x * 2 for each x in [1, 2, 3] if x > 1])   # [4, 6]
post(sum(v for each v in [1, 2, 3]))             # 6  (argumento único dispensa colchetes)
sinal = "positivo" if n > 0 else "zero"
post(sinal)
```

- A palavra é `elif`. `else if` dá `SyntaxError: esperado inicio de bloco com '{'`.
- `for` exige `each`: `for i in` dá `SyntaxError: esperado 'each' em 'for each'`.
- `for each` itera lista, tup, str, range e gerador. Um **dict não itera** (`TypeError: 'dict' object is not iterable`). Use `.keys()`, `.values()` ou `.items()`.
- `range(fim)`, `range(ini, fim)` e `range(ini, fim, passo)` devolvem uma lista concreta.
- Não existe `while ... else`. A compreensão só existe para lista. Com mais de um argumento na chamada, os colchetes são obrigatórios.
- `break` e `continue` fora de laço dão `'break' fora de laco`. `pass` é no-op em qualquer lugar, e `{ }` vazio também vale.

#### 2.7.2 match / case

```ps
ponto = [3, 0]
match ponto {
    case [0, 0] {
        post("origem")
    }
    case [x, 0] {
        post("eixo X em", x)
    }
    case [x, y] if x == y {
        post("diagonal")
    }
    case _ {
        post("outro")
    }
}

dia = "Sabado"
match dia {
    case "Sabado" | "Domingo" {
        post("fim de semana")
    }
    case _ {
        post("dia util")
    }
}
```

Os padrões são: literal, captura (`case x`), `_`, alternativa `|`, lista `[a, b]` e guarda `if`. Se nenhum casar, nada acontece (não é erro). Os nomes ligados valem só no bloco do case.

#### 2.7.3 count each

```ps
nums = [7, "x", 7, 3]
post(int(7) count in nums)     # 2
post(int in nums count)        # 3
count each int in nums {
    post(_match, _index, _count)   # 7 0 3 / 7 2 3 / 3 3 3
}
count each int(7) in nums {
    post(_index)                   # 0 / 2
}
```

Dentro do bloco existem `_match` (o elemento), `_index` (a posição no container), `_count` ou `self` (o total). Dentro de uma funct, `return;` vazio acumula e devolve o total, e `return valor` interrompe.

#### 2.7.4 Guard de entrada

```ps
funct principal() {
    post("ola, Jinga")
}

if __name__ == "main" {
    principal()
}
```

- No arquivo executado, `__name__` vale `"main"`, dentro e fora do `if`. Num módulo importado, vale o nome do módulo. O caminho do arquivo está em `sys.argv[0]`.
- O guard só é reconhecido com o literal `"main"`, e é pulado em **todo** import, inclusive o de um arquivo chamado `main.pr`.
- É o único lugar em que `:` ainda abre bloco (`if __name__ == "main":` seguido de uma linha indentada funciona).

### 2.8 Funções — `funct`

```ps
funct soma(int a, int b = 10) {
    return a + b
}
post(soma(1), soma(1, 2), soma(b=5, a=1))   # 11 3 6

funct tudo(a, *args, **kwarg) {
    post(a, args, kwarg)
}
tudo(1, 2, 3, x=4)                  # 1 (2, 3) {'x': 4}
tudo(*[1, 2], **{"y": 3})           # 1 (2,) {'y': 3}

str funct nome() {
    return "ana"
}
post(nome())

int funct status() {
    raise Falha("x")
}
post(status())                      # 500, e o traceback sai no stderr

bool funct ok() {
    return 1 / 0 == 0
}
post(ok())                          # False, e o ZeroDivisionError sai no stderr

dobro = funct(n) {
    return n * 2
}
post(dobro(21), map([1, 2], dobro))                              # 42 [2, 4]
post(filter([1, 2, 3, 4], funct(n) { return n % 2 == 0 }))       # [2, 4]

funct conta(n) {
    i = 0
    while i < n {
        yield i
        i += 1
    }
}
post(list(conta(3)))                # [0, 1, 2]

nonnull funct exige(v) {
    return v
}
post(exige(1))
```

- `funct` é a única palavra. `action` e `reaction` saíram: `'action' saiu da linguagem; a funcao se declara com 'funct': funct f(args) { ... }`.
- No parâmetro tipado o tipo vem **antes** do nome (`int a`). `funct f(x: int)` dá `no parametro o tipo vem ANTES do nome: escreva funct f(int x), nao funct f(x: int)`. O argumento é conferido e nunca convertido.
- `*args` é tup e `**kwarg` é dict. Os dois vêm depois dos comuns, sem tipo e sem padrão. `f(*lista)` e `f(**dict)` espalham na chamada. O limite é 256 parâmetros fixos.
- `return` sem valor, ou chegar ao fim do corpo, devolve `Null`.
- **Funct tipada** (`str funct`, `Pessoa funct`...): todo `return` **e toda saída** do corpo (`post`, `sys.stdout`/`stderr`, arquivo) têm que ser do tipo declarado. Qualquer tipo vale como retorno, inclusive uma classe.
  - `int funct`: erro no corpo dá `500` (o erro sai no stderr) e ausência de return dá `0`.
  - `bool funct`: erro dá `False` e ausência de return dá `True`. `return 1` é erro de tipo.
  - Os outros tipos: chegar ao fim sem return é erro (`f() devolve str e pode chegar ao fim sem return`).
  - `post("x")` dentro de `int funct` dá `saída de conta() esperava int, recebeu str`.
- **Modificadores em ordem livre**: tipo, `async`, `public`/`private`, `static`, `nonnull`. Exemplo: `private static int funct f()`. Sem o `funct`: `faltou 'funct' antes de 'f': int async funct f(...)`.
- `nonnull` recusa `Null` em qualquer parâmetro, inclusive vindo de um valor padrão: `RuntimeError: nonnull: parametro 'v' em 'exige' nao pode ser Null`. `@NonNull` e `@static` em linha própria são a grafia antiga e continuam valendo.
- `static funct f(self)` fora de classe dá `funct static f(self, ...) nao recebe self: static nao tem instancia, tire o self`.
- Funct é valor (`type()` dá `"funct"`). A recursão sem fim é `RecursionError`. Uma funct com `yield` é gerador, e chamá-la não executa nada até o primeiro pedido.
- `map(lista, fn)` e `filter(lista, fn)` recebem **a lista primeiro**.

### 2.9 async / await / fibras

```ps
async funct dobro(n) {
    sleep(0.2)
    return n * 2
}

f = dobro(21)
post(await f)                                  # 42
post(gather(dobro(1), dobro(2), dobro(3)))     # [2, 4, 6] em ~0,2 s
post(await [dobro(4), 99])                     # [8, 99]
```

- `async funct` devolve um `future` e começa a rodar **na chamada**, até o primeiro ponto de cedência (`sleep`, I/O, `await`).
- As tarefas andam sempre que o principal espera: no `await`, no `gather`, no `sleep` e na I/O dele. No fim do programa, uma tarefa pendente não é esperada.
- Erro **antes** da primeira cedência sobe na chamada. Erro **depois** sobe no `await`. Se ninguém aguardou, a mensagem sai no stderr no fim e o programa termina com rc 1. O tipo original da exceção é preservado.
- `await` numa lista funciona como `gather` achatado. `await 5` devolve 5. `gather(lista)` devolve lista dentro de lista.
- O future não tem `.done()` nem `.result()`. `__init__` não pode ser `async`.
- Por baixo são fibras (pilha própria por tarefa) com epoll no Linux e kqueue no macOS (`vm/ps_so.c`). Toda I/O de saída do jinker cede. Handler de rota ou middleware `async` roda na fibra da conexão.

### 2.10 Classes — `class` / `Entity` / `Class`

As três palavras são sinônimas e podem herdar umas das outras.

```ps
class Animal {
    nome: str
    funct fala(self) {
        return "..."
    }
}

class Cao(Animal) {
    funct __init__(self, nome, raca) {
        base().__init__(nome)
        self.raca = raca
    }
    funct fala(self) {
        return base().fala() + " au"
    }
}

c = Cao("rex", "vira-lata")
post(c.nome, c.raca, c.fala())   # rex vira-lata ... au
post(c)                          # <Cao {'nome': 'rex', 'raca': 'vira-lata'}>
```

- **Sem `()` no cabeçalho**. O `()` aparece só na instanciação. `class A() {` dá ``o '()' fica so na instanciacao: escreva `class A {` (heranca: `class A(Pai) {`)``. Herança: `class Filha(Pai, Mae) {`.
- **Campos**: `nome: tipo`, `tipo nome` ou `nome = valor` (sem tipo), com padrão opcional. Eles sintetizam o `__init__` (um argumento por campo, na ordem, aceitando posicional e nomeado). Um `funct __init__(self, ...)` próprio substitui o sintetizado. Um filho com campos próprios sintetiza só com os campos dele.
- **Método** tem `self` como primeiro parâmetro, senão dá `TypeError: método m() sem self: o primeiro parâmetro de um método é self (ou marque static)` na linha do `funct`. `self.x = ...` num método cria o campo.
- **Campo declarado no construtor**: `private str name = nome` dentro de `__init__` vira campo privado do objeto.
- Um campo sem `static` não é alcançado pelo nome do tipo: `Livro.autor` dá `RuntimeError: Entity 'Livro' não tem campo estático 'autor' — instancie primeiro`.

#### 2.10.1 static e visibilidade de dentro

```ps
class Conta {
    private saldo: int = 0
    public static int total = 0

    public funct deposita(self, int v) {
        self.saldo = self.saldo + v
        conta_mais()
        return self.saldo
    }
    static funct conta_mais() {
        Conta.total = Conta.total + 1
    }
    static funct taxa(flo v) {
        return v * 0.01
    }
}

c = Conta()
post(c.deposita(100), Conta.total, Conta.taxa(50.0))   # 100 1 0.5
```

- Um método `static` é chamado pela classe (`Conta.taxa(...)`). Chamar pela instância dá `funct 's' e static: chame pela Entity (Tipo.s(...)), nao pela instancia`. Um método static que declara `self` tem o `self` descartado na chamada pela classe.
- Um campo `static` é avaliado uma vez, na declaração da classe, e é compartilhado. Ele não entra no construtor.
- **Regra do dono**: o que `App.x` alcança de fora, `x` alcança de dentro. Isso vale para campo e método `static`, da classe e dos pais, pelo nome solto, dentro de qualquer método. Um método comum solto (`m()`) é `NameError`, porque é `self.m()`. Parâmetro ou local com o mesmo nome tem prioridade.
- Um decorador no corpo da classe roda na declaração, então o objeto que ele usa tem que ser `static`. O motor avisa: `'mapp' e campo de instancia — ... declare-o static`.

#### 2.10.2 private / public

- **Membro**: `private` em campo ou método só é acessível de dentro de um método da própria classe (regra Java). De fora dá `RuntimeError: acesso negado: 'x' e private de C (so acessivel de dentro da classe)`. O `private` do pai continua invisível para o filho. `public` é o padrão.
- **Topo do arquivo**: `private` vale em **qualquer** declaração (funct, class, model, enum, variável). O nome funciona dentro do arquivo e não sai pelo import (ver 2.15).
- Em qualquer outro lugar dá erro: `'private' aqui nao tem efeito: so vale no topo do arquivo (o que nao sai pelo import) ou no corpo da Entity`.
- É disciplina de código, não proteção de memória.

#### 2.10.3 Herança e `base()`

```ps
class Motor {
    funct __init__(self, cavalos) {
        self.cavalos = cavalos
    }
}
class Roda {
    funct __init__(self, qtd) {
        self.qtd = qtd
    }
}
class Carro(Motor, Roda) {
    funct __init__(self) {
        base(Motor).__init__(300)
        base(Roda).__init__(4)
    }
}
c = Carro()
post(c.cavalos, c.qtd)   # 300 4
```

- `base()` (um pai) e `base(Pai)` (qualquer ancestral) alcançam tudo do pai com o `self` atual: `.__init__(args)`, `.metodo(args)` (mesmo sobrescrito) e `.campo` (leitura).
- Com dois pais, `base()` dá `base() com mais de um pai: diga qual, base(A) ou base(B)`.
- A forma antiga `base(x)` dá `base(...) recebe so o nome do pai: base().__init__(x) ou base(Pai).__init__(x)`.
- `base().x = 1` dá `atribua pelo self: self.x = ...`. `base(X)` com X que não é ancestral dá `'X' nao e pai de 'Filha' (pais: A, B)`.
- Resolução de método na herança múltipla: esquerda para a direita, e em profundidade pela cadeia de cada pai.

#### 2.10.4 `@dataentity`

```ps
from datasentity import dataentity, asdict, asjson

@dataentity
class Pessoa {
    nome: str
    idade: int = 18
}

p = Pessoa(nome="Ana")
post(asdict(p), asjson(p))   # {'nome': 'Ana', 'idade': 18} {"nome": "Ana", "idade": 18}
```

### 2.11 model e enum

`model` **mantém** os `()` (decisão do dono em 2026-09-25). Ele valida um dict com `==` e responde True/False.

```ps
model Endereco() {
    cep: str(regex="^[0-9]{5}-[0-9]{3}$")
}

model Usuario() {
    nome:     str(length=20)
    idade:    int(min=18, max=120)
    papel:    str(in=["admin", "user"])
    apelido:  str(optional=True)
    endereco: Endereco
    tags:     list(of=str, length=5)
}

dado = {"nome": "ana", "idade": 30, "papel": "user", "endereco": {"cep": "12345-678"}, "tags": ["a"]}
post(dado == Usuario)    # True
dado["idade"] = 12
post(dado == Usuario)    # False
```

| Parâmetro | Vale em | Confere |
|---|---|---|
| `length=N` | str, int, list | Máximo de caracteres, dígitos ou itens |
| `regex="..."` | str | A string inteira casa o padrão (fullmatch). Padrão inválido é erro de compilação |
| `in=[...]` / `not_in=[...]` | str, int, flo, bool | O valor está (ou não está) na lista |
| `min=N` / `max=N` | int, flo | Faixa inclusiva |
| `optional=True` | qualquer | O campo pode faltar ou vir `Null` |
| `of=T` | list | Cada item é do tipo T ou do model T |

- Os tipos de campo são `str int flo bool list dict` ou outro model (aninhado, que pode ser declarado depois ou vir de import).
- Os valores dos parâmetros são literais. Chaves a mais no dict são ignoradas.
- Na rota do jinker, `model=` responde 422 com o caminho do campo (`campo 'endereco.cep': ...`).
- `model M {` sem parênteses dá `esperado '(' apos nome do model`.

```ps
enum Status {
    OK = 200,
    NOT_FOUND = 404
}
enum Cor {
    RED
    GREEN
}
post(Status.OK, Cor.GREEN, type(Cor.RED))   # 200 1 int
```

`enum` **não** tem parênteses. Os membros são separados por vírgula ou quebra de linha, e a numeração começa em 0 ou continua a partir de um valor explícito. Um membro inexistente dá `AttributeError`.

### 2.12 Decoradores

```ps
funct log(f) {
    funct w(*args, **kwarg) {
        post("chamando")
        return f(*args, **kwarg)
    }
    return w
}

@log
funct soma(a, b) {
    return a + b
}
post(soma(1, 2))   # chamando / 3
```

- Três são resolvidos na compilação: `static`, `nonnull` (grafias `@static` e `@NonNull`) e `@dataentity`.
- Os demais seguem um protocolo único. Com parênteses (`@app.get("/x")`) é uma chamada, sem parênteses (`@log`) é o valor. Se o valor tem `.register(f)`, o motor registra e o nome continua sendo a funct. Senão, se o valor é chamável, o nome passa a valer o que ele devolveu (`Null` mantém a funct original).
- Os decoradores empilham, e o de baixo aplica primeiro.
- Valem em cima de funct solta, de classe (registra o primeiro método que não é `__init__`) e de método (o método envolvido recebe o receptor como primeiro argumento).
- Um nome inexistente dá `NameError` na linha do `@`. Um valor inútil dá `TypeError: decorador @x vale 'int', que nao registra (.register) nem envolve (chamavel) a funct`. Sem nada embaixo dá `SyntaxError: decorador sem funct ou Entity embaixo`.

### 2.13 Exceções, raise, assert

```ps
try {
    n = int("abc")
} catch (ValueError e) {
    post("nao e numero:", e)
} catch (e) {
    post("outro erro:", e)
} finally {
    post("sempre roda")
}

try {
    raise SaldoInsuficiente("faltou 10")
} catch (SaldoInsuficiente e) {
    post(e)                                  # faltou 10 (linha 12)
}

try {
    x = [1][5]
} catch (LookupError e) {
    post("pegou pela arvore:", e)            # list index out of range (linha 18)
}

assert(1 + 1 == 2)
assert(len("abc"), 3, "tamanho")
```

- `catch` exige parênteses e tem quatro formas: `(Tipo e)`, `(Tipo)`, `(e)` e `()`. O `e` é **texto** (`mensagem (linha N)`), e `type(e)` é `str`. O tipo se filtra no próprio `catch`.
- `try` com `finally` e sem `catch` é válido (o erro propaga depois do finally). `try` sozinho dá `esperado 'catch' ou 'finally' apos bloco do try`.
- `raise Nome("msg")` aceita um nome livre com maiúscula, sem declaração prévia. `raise "texto"` gera `RuntimeError`. Os nomes da árvore são valores (`erro = KeyError`, `raise erro`, `ValueError is Exception`).
- Um `SyntaxError`, e todo erro de tipo, nome, aridade ou membro que o compilador já conhece, sai antes de rodar e não chega ao `catch`.
- `assert(cond)`, `assert(cond, "mensagem")` e `assert(veio, esperado, "nota")` levantam `AssertionError`. Exemplo: `AssertionError: soma — veio 4, esperava 5`. Quando passa, devolve `True`. Um `esperado` do tipo str é lido como mensagem, então para comparar com texto use a forma de três.

Árvore de exceções (`jinga --metadata`, campo `excecoes`):

```
Exception
  OSError: FileExistsError, FileNotFoundError, IsADirectoryError, NotADirectoryError,
           PermissionError, NetworkError, TimeoutError
  IOError
  LookupError: IndexError, KeyError
  ArithmeticError: ZeroDivisionError, OverflowError
  ValueError: UnicodeError (UnicodeDecodeError, UnicodeEncodeError), AttributedValueError, ConversionError
  RuntimeError: RecursionError, NotImplementedError
  TypeError, AttributeError, NameError, ImportError, MemoryError, AssertionError, SyntaxError, DatabaseError
```

A regra de uso: `TypeError` quando o tipo está errado e `ValueError` quando o tipo está certo e o valor não serve.

### 2.14 using e arquivos

```ps
import os
os.writeFile("saida.txt", "linha 1\n")
using open("saida.txt", "a") as f {
    f.write("linha 2\n")
}
post(os.readFile("saida.txt"))
```

- `using` fecha o recurso na saída, mesmo com erro: arquivo, conexão de banco (faz commit antes de fechar) e planilha do `manpu` (salva).
- Para ler ou gravar um arquivo inteiro, o idioma do projeto é `os.readFile(path)` e `os.writeFile(path, conteudo)`. O dono proíbe `open(x).read()`.
- `open(path, mode="r", encoding="utf-8")` devolve um `PoolFile` com os métodos `read readline readlines write writelines close save path move copy delete bytes` e os campos `name ext size`.
- Remover arquivo é `PoolFile.delete()`. `os.remove` não existe.

### 2.15 Imports

```ps
import json
import os as sistema
from json import parse, stringify
from regex import compile as rc
import json *
PUSH date GET today
import '../pacote/modulo.pr'
from './irmao.pr' import w
from .irmao import w as w2
import lib
post(modulo.z(), w(), w2(), lib.publica, stringify(parse("[1]")))
```

| Forma | Efeito |
|---|---|
| `import m` / `import m as x` | Liga o módulo e dá acesso por `m.x`. `import a.b` liga `b` |
| `from m import a, b as c` | Liga nomes soltos |
| `from m import *` = `import m *` = `PUSH m GET *` | Liga cada nome exportado, mas não o nome `m`. Resolvido na compilação e **só no topo do arquivo** |
| `PUSH m [as x] [GET a, b]` | É `import` / `from ... import`. Trocar `GET` por `import` (e vice-versa) é SyntaxError com a forma certa |
| `import '../pasta/arq.pr' [as x]` | Caminho relativo à pasta **do arquivo que importa**. Liga o nome do arquivo. Nome com `-` exige `as` |
| `from .mod import x` / `from ..pkg.mod import y` | Relativo com pontos, só com `from`. `import .mod` é erro que mostra as duas formas certas |
| `import 'json'` | Sem `/` e sem extensão, a string é nome de módulo |
| `import jinga.libs.minhalib` | Nome qualificado de lib instalada. Liga `minhalib` e é o mesmo módulo de `import minhalib`. `poolscript.libs.X` também funciona |

Ordem de resolução de `import nome`: (1) stdlib, (2) lib instalada com `jpkg install x.pr -asLib` em `~/.jinga/libs` (ou `$JINGA_HOME/libs`), (3) arquivo ao lado de quem importa, (4) arquivo do projeto a partir da pasta do arquivo executado. **Uma lib sempre ganha de um arquivo local de mesmo nome.**

O que o módulo exporta: funct, class, model, enum e variável do topo, os nomes que os imports dele ligam e o `global x`. Ficam de fora o que é `private`, o que só existe dentro de bloco (inclusive no guard) e os builtins que ele só usa. Um nome começado por `_` é exportado normalmente.

```ps
# lib.pr
private segredo = 5
private funct interna() {
    return segredo
}
publica = 9
```

`post(lib.segredo)` em outro arquivo dá, antes de rodar:

```
AttributeError: module 'lib' has no attribute 'segredo' (existe, mas é private: declarada em lib.pr, linha 1)
```

`private class` também não sai pelo import (`from k4lib import Oculta` recebe a mesma frase). O `--check` confere chamadas contra a assinatura do `.pr` importado: aridade, tipos, construtor, métodos e membros. Um módulo que não compila é acusado na linha do `import`, com o quadro do defeito dentro do módulo. A importação **não** dispara o guard `main` do módulo.

Apelidos de módulo **não existem**: `import db`, `mp`, `qr`, `sqlite`, `JSON`, `dataentity` e o plural de `request` dão `ImportError: No module named '...'`.

### 2.16 Builtins (40, sem import)

`assert post input open load len type range str int long flo bool list tup dict abs pow round hex bin oct ord chr sum min max sorted reversed enumerate zip map filter addEnd addStart removeEnd removeStart sleep gather id`

- `post(...)` imprime separando por espaço. `print` **não existe** (`NameError`).
- `input(prompt)` devolve str, ou `Null` no fim da entrada.
- `load(caminho)` carrega o `.env` e devolve um dict.
- `len` conta caracteres, e `len(Null)` dá 0.
- `range` devolve uma lista.
- `round` arredonda metade para par.
- `sorted` e `reversed` devolvem cópia.
- `enumerate` e `zip` devolvem listas de tuplas.
- `addEnd(l, v)`, `addStart(l, v)`, `removeEnd(l)` e `removeStart(l)` mudam a lista no lugar. Em lista vazia, `removeEnd` e `removeStart` dão `Null`.
- `sleep(s)` cede para as tarefas.
- `gather(...)` espera futures.
- `id(x)` devolve a identidade.

### 2.17 Métodos embutidos (sem import)

| Tipo | Quantidade | Destaques |
|---|---|---|
| `str` | 55 | Caixa Unicode (`upper lower title capitalize swapcase casefold`), `strip` com conjunto, `find` (-1) e `index` (erro) com `inicio` e `fim`, `startswith`/`endswith` com tupla de opções, testes `is...`, `split rsplit splitlines join partition rpartition`, `replace` com **lista** de alvos, `removeprefix removesuffix ljust rjust center zfill expandtabs`, `format format_map maketrans translate`, regex `match` (string inteira), `findall` e `sub`, e também `encode`, `get_json`, `get`, `len`, `contains`/`has` |
| `list` | 14 | `append extend insert pop remove reverse sort clear index count contains has copy len`. Os que mudam a lista devolvem `Null` |
| `dict` | 12 | `keys values value items get has contains pop update clear copy len` |
| `tup` | 5 | `index count contains has len`. `t.append(3)` dá `AttributeError: 'tup' object has no attribute 'append'` |
| `byte` | 44 | Os nomes do str, com caixa só ASCII e busca que aceita int de 0 a 255. Indexar e iterar dão int, fatiar dá byte. Inclui `decode(encoding, errors)` e `hex` |
| qualquer valor | 1 | `.type()` |

Encodings aceitos em `encode`/`decode`: `utf-8`, `latin-1`, `ascii`, `utf-16-le/be`, `utf-32-le/be`. Os modos de `errors` são `strict`, `ignore` e `replace`.

Em regex dentro de string, use `r"\d+"` ou `"\\d+"`. Com `"\d"` a barra fica no texto e sai um aviso de escape inválido.

### 2.18 Biblioteca padrão (18 módulos com `import`, mais `Parsing` global)

A lista viva está em `jinga --help` e em `jinga --metadata` (`modulos`). Todos são nativos, embutidos na VM e carregados só no import.

| Módulo | Uma linha |
|---|---|
| `os` | Arquivos e pastas (`readFile writeFile exists isfile isdir mkdir rmdir ls rename copy move size cwd chdir`), ambiente (`getenv environ loadFile`), `pathFile`/`pathFolder` (olham a pasta do `.pr`), `cmd(c, capture=False)` via shell (com `capture=True` devolve a saída), `run(lista)` sem shell (a forma segura contra injeção), `ipmach`, `warn`, `code`, `PoolFile` |
| `sys` | `argv` (`argv[0]` é o caminho do `.pr`), `exit(code)`, `platform`, `executable`, `RelativePath`, `stdout`/`stderr` (`write writeln flush`), `stdin` (`read readline raw`) |
| `json` | `parse(texto)` e `stringify(valor)`. `Null` vira `null` no texto JSON |
| `date` | `time today datahora now timestamp timestamp_ms monotonic` e `hora(hours, minutes, days)`, que devolve segundos |
| `regex` | `compile` (devolve `Pattern`), `match`/`fullmatch` (string inteira), `search` (bool), `findall sub split escape` |
| `dotenv` | `load(caminho)` carrega o `.env` no ambiente e devolve um dict |
| `jwt` | `gen` e `check` para tokens de autenticação |
| `hash` | `crypt(senha)`/`check(hash, senha)` para senha, `sha256`, `b64encode`, `b64decode` |
| `bytes` | Criar e converter dados binários: `new fromhex hex base64 frombase64 fromint toint tolist concat slice get xor` |
| `sqlite3` | `connect`, depois `cursor`, `execute`, `fetch`. Banco local embutido |
| `psodbc` | `connect(driver=...)` único para SQLite, PostgreSQL, MySQL, SQL Server e MongoDB. `query` é o atalho de SQLite |
| `mail` | `MailServer` (SMTP, porta 587 por padrão), `MailMessage`, `MailReader` (IMAP, 993) |
| `request` | Cliente HTTP (`get post put patch delete head`, com `fields=` e `file=` para multipart) e WebSocket de saída `ws_connect`. Devolve `Response` |
| `jinker` | Servidor HTTP e WebSocket: `Jinker cors jsonify render request JinkerResponse JinkerRequest`. Rotas por decorador, `model=` que responde 422, `route_prefix`, fibras e epoll |
| `sockets` | 91 membros: TCP, UDP e UNIX crus (cliente e servidor), resolução de nomes, byte-order, constantes `AF_*`, `SOCK_*` e `SO_*` |
| `qrcode` | `gen`, `make` (PNG), `QRCode`, `ERROR_CORRECT_L/M/Q/H` |
| `manpu` | CSV, XLSX, XML, HTML, JSON e texto: `read load write remove src open` (use `open` com `using`) |
| `datasentity` | `dataentity asdict astuple aslist asjson` |
| `Parsing` (global) | Conversão tolerante: `string integer floating boolean TransientValue JSONformatt Arrayformatt Tuplasformatt`, com `to <tipo>` |

Servidor mínimo (conferido: `curl /ola/ana` devolveu `{"msg": "ola ana"}`):

```ps
from jinker import Jinker, jsonify

app = Jinker(__name__)

@app.get("/ola/<nome>")
funct ola(nome) {
    return jsonify({"msg": "ola " + nome})
}

if __name__ == "main" {
    app(host="127.0.0.1", port=18431)
}
```

- O handler lê `request`, que existe de graça dentro da rota.
- O segmento `<nome>` (ou `:nome`) é injetado no parâmetro de mesmo nome quando a funct o declara.
- `return corpo, status` define o status.
- O CORS é por servidor: `cors(app, origins=[...])`.
- A referência completa está em `docs/jinker.md` e `docs/jinker/`.

Libs `.pr` instaladas nesta máquina em `~/.jinga/libs` (são do dono e vão junto para o Mac): `Controllers.pr`, `math.pr`, `pagination.pr`, `pondy.pr`, `prserver.pr`, `random.pr`, `roken.pr`, `sunotify.pr`, `swagger.pr` (documentada em `docs/swagger/swagger.md`).

### 2.19 Erros comuns que a própria linguagem aponta

| Escrito | Mensagem do motor | Certo |
|---|---|---|
| `// oi` | `'//' e divisao inteira, nao comentario — comentario e '#' (ou bloco entre tres aspas)` | `# oi` |
| `if x == 1:` | `bloco com ':' nao existe mais — use '{ }'` | `if x == 1 {` |
| `class A() {` | ``o '()' fica so na instanciacao: escreva `class A {` `` | `class A {` |
| `model M {` | `esperado '(' apos nome do model` | `model M() {` |
| `action f()` | `'action' saiu da linguagem; a funcao se declara com 'funct'` | `funct f()` |
| `} else if x {` | `esperado inicio de bloco com '{'` | `} elif x {` |
| `for i in xs` | `esperado 'each' em 'for each'` | `for each i in xs` |
| `a = b = 5` | `expressao invalida` | Uma atribuição por linha |
| `++x`, `.5`, linha começando com `+` | `expressao invalida` | `x++`, `0.5`, parênteses |
| `x === 1` | `` `===` nao existe nesta linguagem; use `==` `` | `==` |
| `x is 5` | `'is' com literal 'int' a direita — 'is' compara TIPO` | `x == 5` ou `x is int` |
| `ação = 1` | `nome so aceita letra sem acento, digito e _: 'ç'` | `acao` |
| `count = 1` | `'count' e palavra reservada ...` | Outro nome (`base` também é reservada) |
| `int x` | `declaracao de variavel exige '='` | `int x = 0` |
| `funct f(x: int)` | `no parametro o tipo vem ANTES do nome` | `funct f(int x)` |
| `int async f(x)` | `faltou 'funct' antes de 'f': int async funct f(...)` | Com `funct` |
| `print("x")` | `NameError: name 'print' is not defined` | `post("x")` |
| `post(vlaor)` | `NameError: name 'vlaor' is not defined. Did you mean: 'valor'?` | O nome certo |
| `json.parsse(...)` | `AttributeError: module 'json' has no attribute 'parsse'. Did you mean: 'parse'?` | `json.parse` |
| `{tls: True}` | `NameError: name 'tls' is not defined` | `{"tls": True}` |
| Variável criada no `if` e usada depois | `NameError: name 'dentro' is not defined` | Declarar antes do bloco |
| `for each k in d` (dict) | `TypeError: 'dict' object is not iterable` | `d.keys()` / `d.items()` |
| `"a" + 1` | `TypeError: can only concatenate str (not "int") to str` | `"a" + str(1)` ou f-string |
| `soma(1, 2, 3)` | `TypeError: soma() takes 2 positional arguments but 3 were given` | Aridade certa |
| Método sem `self` | `TypeError: método m() sem self: ...` | `funct m(self)` ou `static` |
| `c.s()` com `s` static | `funct 's' e static: chame pela Entity (Tipo.s(...))` | `C.s()` |
| `c.x` com `x` private | `acesso negado: 'x' e private de C` | Método público |
| `private x = 1` dentro de funct | `'private' aqui nao tem efeito: so vale no topo do arquivo ...` | Só no topo ou no corpo da class |
| `base()` com dois pais | `base() com mais de um pai: diga qual, base(A) ou base(B)` | `base(A).__init__(...)` |
| `from json import *` dentro de funct | `` `*` do import so vale no topo do arquivo ... `` | Nomear: `from json import a, b` |
| `import .mod` | `import com ponto na frente nao existe (o relativo e so com from) ...` | `import './mod.pr'` ou `from .mod import x` |
| `import db` | `ImportError: No module named 'db'` | `import psodbc` |
| `str funct f() { x = 1 }` | `f() devolve str e pode chegar ao fim sem return` | `return` do tipo |
| `post("t")` dentro de `int funct` | `saída de conta() esperava int, recebeu str` | Só saída do tipo |
| `try { }` sozinho | `esperado 'catch' ou 'finally' apos bloco do try` | Com `catch` ou `finally` |
| `break` fora de laço | `'break' fora de laco` | Dentro de laço |
| `"C:\pasta"` | `SyntaxWarning: sequencia de escape invalida '\p'` (roda) | `r"C:\pasta"` |

Toda mensagem vem com o quadro `em arquivo.pr, linha N` e o cursor `^^^` no ponto da causa. Todos os erros de tipo do arquivo saem de uma vez. Regra do dono: uma mensagem que não aponta a causa é bug do motor e se corrige na mensagem, no motor e no checador.

### 2.20 Onde está cada coisa

| Assunto | Arquivo |
|---|---|
| Referência da linguagem, por capítulo | `docs/linguagem/01-estrutura-lexica.md` ... `14-decoradores.md` |
| Visão única da sintaxe | `docs/LANGUAGE.md`, `docs/Jinga.md` (inclui desempenho medido com `perf stat`) |
| Uma página por builtin | `docs/builtins/<nome>/` |
| Uma página por método | `docs/str/`, `docs/list/`, `docs/dict/`, `docs/tup/`, `docs/byte/`, `docs/bytes/` |
| Uma pasta por lib | `docs/<lib>/` (índice em `docs/INDEX.md`) |
| Objetos que as libs devolvem | `docs/objetos-internos/` |
| Exceções | `docs/exceptions/exceptions.md` |
| Editor e LSP | `docs/lsp.md` |
| Depurador | `docs/debugger.md` |
| Motor | `vm/ps_lexer.c`, `vm/ps_parser.c`, `vm/ps_compiler.c` (inclui o checador de tipos), `vm/jinga_vm.c`, `vm/ps_*.c`, `vm/ps_so.c` (camada de sistema Linux, macOS e a base do Windows), `vm/ps_jit_x64.h` |

### 2.21 Pontos em que a doc antiga diverge do motor (conferido rodando)

- `docs/LANGUAGE.md`, seção Entity: diz que "os parênteses após o nome são obrigatórios". O motor **recusa** `class A() {`, e o certo é `class A {` (a `07-entity.md` está certa).
- `docs/LANGUAGE.md`, seção Tipos: diz que `int n = input()` converte. O motor recusa (`esperava int, recebeu str|Null`). O certo é `int(input())`.
- `docs/LANGUAGE.md`: diz que `try`/`finally` sem `catch` não vale e que `for each` não desempacota. Os dois funcionam hoje (`10-excecoes.md` e `05-controle-de-fluxo.md` estão certos).
- `docs/LANGUAGE.md`: diz que redeclarar com outro tipo passa. O motor recusa (`já foi declarada como int e não pode ser redeclarada como long`). Redeclarar com o mesmo tipo passa.
- `docs/INDEX.md`, tabela "Apelidos": lista `db`, `mp`, `qr`, `JSON`, `dataentity` e o plural de `request`. Todos dão `ImportError`.
- `docs/Jinga.md`, cabeçalho: diz v16.1.9, igual ao fonte (`vm/ps_versao.h`) e ao binário instalado (conferido no fim de 2026-09-28; mais cedo no mesmo dia o instalado respondia 16.1.6 e o cabeçalho dizia 16.1.7).
- `jinga.md`, seção "Aprofundar": aponta para `notas/` e `notas/CLAUDE.md`. A pasta `notas/` não existe mais (foi apagada pelo dono em 2026-09-05).

## 3. O motor (`vm/`)

O motor da Jinga (o `--version` o chama de **PSVM**) é um programa em C que vira **um executável só**. Todo o fonte dele mora em `vm/`: no commit `b180807` são 53 arquivos (52 versionados e o `ps_build.h`, gerado pelo Makefile) e 62.006 linhas (`.c`, `.h`, `.def` e `.inc`), contadas com `wc -l` em 2026-09-28. O `Makefile` compila os 20 `.c` numa única chamada do compilador. Os `.h` que carregam código (`ps_jit_x64.h`, `ps_dl.h`, `ps_tipos.h`, `ps_pgerr.h`, `ps_qr_tables.h`) entram por `#include`.

```
FONTES := vm/ps_lexer.c vm/ps_ast.c vm/ps_parser.c vm/ps_compiler.c vm/ps_pilha.c
          vm/ps_hash.c vm/ps_regex.c vm/ps_mail.c vm/ps_http.c vm/ps_qr.c vm/ps_xlsx.c
          vm/ps_db.c vm/ps_mongo.c vm/ps_jinker.c vm/ps_pkg.c vm/ps_debug.c
          vm/ps_retornos.c vm/ps_so.c vm/jinga_vm.c vm/main.c
```

Números do binário medidos neste laptop em 2026-09-28:

| Item | Valor |
|---|---|
| `./pool --version` | `Jinga 16.1.9 [PSVM] (2026-09-28) Runtime standalone` |
| Arquivo | `./pool`, na raiz do repo, com 9.066.080 bytes (igual byte a byte ao `/usr/local/bin/jinga`) |
| Dependências dinâmicas (`ldd ./pool`) | só `libm.so.6`, `libgmp.so.10` e `libc.so.6`, além do `linux-vdso` e do `ld-linux` |
| Bibliotecas estáticas (Linux, via `-Wl,-Bstatic`) | sqlite, OpenSSL (libssl/libcrypto), libpng, expat, zlib |
| Bibliotecas estáticas (macOS) | os `.a` do Homebrew para OpenSSL e libpng; sqlite, expat e zlib vêm do próprio macOS; gmp vem do Homebrew (`-lgmp`); desde `3c245e7` também os frameworks Security e CoreFoundation (raízes TLS do Keychain) |
| Carregadas só no primeiro `connect` (dlopen) | clientes do Postgres, do MariaDB/MySQL, do unixODBC e do Mongo (ver 3.13) |
| Versão | constante `PS_VERSAO` em `vm/ps_versao.h` (hoje `"16.1.9"`) |
| Data de build | `vm/ps_build.h`, gerado pelo Makefile |

No repositório, `make pool` gera `./pool`. Instalado, o mesmo binário responde por `jinga` (roda programas) e `jpkg` (gerencia pacotes). Os nomes antigos `pool` e `psl` continuam como atalhos.

---

### 3.1 Pipeline de ponta a ponta

```
fonte .pr
  -> lexer em FLUXO           vm/ps_lexer.c    os tokens nascem quando o parser pede, em blocos de 4096
  -> parser INCREMENTAL       vm/ps_parser.c   uma declaração de topo por vez; AST numa arena
  -> compilador, PASSADA 1    vm/ps_compiler.c colhe e PODA cada declaração; tipagem estática
  -> compilador, PASSADA 2    vm/ps_compiler.c lê o fonte de novo e gera o bytecode
  -> VM                       vm/jinga_vm.c    laço com goto calculado
       -> JIT x86-64          vm/ps_jit_x64.h  código de máquina por funct, na 1ª execução
```

As funções de entrada estão declaradas em `vm/ps_vm.h`:

| Função | Uso |
|---|---|
| `ps_roda_fonte(fonte, len, caminho, &erro)` | roda um `.pr` inteiro; é o caminho de `jinga arquivo.pr`, `-e` e do executável gerado por `-o` |
| `ps_verifica_fonte(...)` | lexer, parser e compilador, **sem executar nada**; é o `--check` |
| `ps_desmonta_fonte(...)` | compila como se fosse rodar e imprime o bytecode; é o `--bytecode` |
| `ps_metadata_json(FILE*)` | modelo de tipos do motor em JSON; é o `--metadata` |
| `ps_embute_deps(...)` / `ps_emb_*` | tabela de fontes embutidos do `-o` |
| `ps_debug_porta(porta)` | liga o depurador DAP |
| `ps_set_argv(...)` | alimenta o `sys.argv` |

O `import` de um módulo `.pr` passa pelo mesmo `ps_compila_fonte`, que aparece em 6 pontos de `jinga_vm.c`. Rodar, importar e `--check` dão, portanto, o mesmo veredito.

---

### 3.2 Lexer (`vm/ps_lexer.c`, `vm/ps_lexer.h`)

Depende só da libc. As regras que não são óbvias, tiradas do cabeçalho do arquivo:

- A indentação é **estrita**: só espaços (TAB é erro), sempre múltiplo de 4, e sobe no máximo um nível por vez.
- Dentro de `(`, `[` e `{` a indentação é ignorada e não sai NEWLINE.
- Uma linha que começa com `.membro` continua a anterior (encadeamento de métodos em várias linhas).
- `"""` é **comentário de bloco**, não string. String de várias linhas usa `'''`.
- `<hex>` e `<nome>` viram cor (`T_COLOR`) só se o hex tiver 3 ou 6 dígitos ou se o nome estiver na tabela. Fora disso, `<` volta a ser operador.

Tipos de token (`PSTokType`): `T_EOF`, `T_KW`, `T_IDENT`, `T_IDENT_UPPER`, `T_INT`, `T_FLO`, `T_STR`, `T_FSTRING`, `T_BOOL`, `T_NULL`, `T_COLOR`, `T_OP`, `T_NEWLINE`, `T_INDENT`, `T_DEDENT`, os seis de parêntese/chave/colchete, `T_COLON`, `T_SEMI`, `T_COMMA`, `T_DOT`, `T_AT` e `T_BYTES` (literal `b"..."`). Três só existem em modos especiais: `T_COMMENT` (modo editor), `T_ERRO` e `T_SINC` (modo de recuperação).

Modos do lexer:

| Modo | Função | Quem usa |
|---|---|---|
| normal | `ps_lexer_tokenize` | lista inteira, para ao primeiro erro |
| editor | `ps_lexer_tokenize_editor` | guarda os comentários como `T_COMMENT`; é o que o `--tokens` usa para o realce |
| recuperação | `ps_lexer_tokenize_modo(..., comentarios, recupera=1)` | segue depois do erro e junta todos em `erros`. Um `(`, `[` ou `{` sem par vira erro **no abridor**, e o lexer fecha o grupo à força com `T_SINC` |
| fluxo | `ps_lexer_fluxo` + `ps_lexer_tok` + `ps_lexer_solta_ate` | usado pelo compilador. Os tokens ficam em blocos fixos de `PS_TOK_BLOCO = 4096`, e os blocos de trás são soltos a cada declaração de topo. O vetor de tokens do arquivo inteiro nunca existe |
| sonda | `ps_lexer_sonda_grupos` | só acha os grupos sem par, soltando os blocos conforme nascem; é usado quando o parser falha ao rodar |

- **Segunda passada**: só roda no modo de recuperação e só quando a primeira achou grupo sem par. Ela mexe apenas nos grupos que a primeira apontou (os `alvos`). Fecha o grupo na linha que só pode ser começo de declaração (`funct nome`, `class Nome`, `@`, `import`...) ou na linha que volta à indentação do abridor sem começar com um fechador. Um arquivo válido nunca chega nessa regra, e por isso o `--check` não recusa código válido.
- **Avisos**: um aviso (`SyntaxWarning`) não é erro. O programa roda, o aviso vai para o stderr no formato `<arquivo>:<linha>: SyntaxWarning: ...`, e o `--check` o devolve no JSON. Exemplo de aviso: `"C:\pasta"`, onde `\p` não é escape válido.
- **f-string**: cada `{...}` é gravado com a posição **real** no fonte (`struct PSInterp`). Assim o editor enxerga as expressões de dentro da string.
- **Erro na causa, rodando**: quando o parser falha, `sintaxe_na_causa` (em `jinga_vm.c`) chama `ps_lexer_sonda_grupos`. Se o erro caiu dentro de um grupo fechado à força, a frase e a posição passam a ser as do abridor. Rodar e editor dizem a mesma coisa. Medido em 2026-09-28:

```
$ printf 'funct f(a {\n    return a\n}\npost(f(1))\n' > sint.pr && jinga sint.pr
SyntaxError: parentese '(' aberto nao foi fechado
  em sint.pr, linha 1
  | funct f(a {
  |        ^^^
(rc 2)
```

---

### 3.3 Parser e AST (`vm/ps_parser.c`, `vm/ps_parser.h`, `vm/ps_ast.c`, `vm/ps_ast.h`)

- **Descida recursiva**. A cadeia de precedência, da mais fraca para a mais forte: `or -> and -> not -> comparação -> | -> ^ -> & -> << >> -> + - -> * / % -> unário -> pós-fixo (chamada, .membro, [índice]) -> primário`.
- **Regras fixas**:
  - palavra reservada não vira nome;
  - `base` é reservada contextual: não vira nome, mas `base(...)` chama;
  - bloco aceita `{ }` ou `:` com INDENT/DEDENT, e as duas formas convivem;
  - nó que não é compilável para com erro explícito, nunca gera árvore errada em silêncio.
- **Comentário desatualizado no topo de `ps_parser.c`**: ele ainda diz que chave de dict sem aspas vira string. Isso não vale mais. Medido em 2026-09-28: `d = {nome: 1}` com `nome` não definido dá `NameError: name 'nome' is not defined` antes de rodar (rc 2). Com `str nome = "k"` antes, sai `{'k': 1}`. A chave é expressão.
- **AST** (`ps_ast.h`): union etiquetada em cima de uma **arena** (`ps_ast.c`, blocos de 64 KB, alinhados em 16). A árvore nasce no parse e morre quando a compilação termina, então não há GC nem `free` por nó. Um erro no meio do parse também não vaza memória. Os nomes dos nós (`N_PROGRAM`, `N_LITERAL`, `N_BINARY_OP`...) fazem parte do contrato do `--ast`.
- **Modos**:
  - `ps_parse` / `ps_parse_modo(toks, n, recupera)`: com `recupera`, registra cada statement que falha, pula para o próximo e segue. É o que o editor usa para ter a árvore inteira de um arquivo pela metade.
  - `ps_parse_lista`: as expressões dentro de cada f-string viram nós filhos do literal, com linha e coluna reais.
  - `ps_parse_fonte`: lê e parseia sem o vetor de tokens inteiro.
- **Parser incremental**: `ps_parse_inc_abre`, `ps_parse_inc_proxima`, `ps_parse_inc_resultado`, `ps_parse_inc_lexer` e `ps_parse_inc_fecha` entregam uma declaração de topo por vez. A anterior morre quando a seguinte é pedida.
- **Profundidade**: não existe teto por número de níveis. O parser mede a **folga real da pilha do C** com `ps_pilha_apertada()` (`vm/ps_pilha.c`). A margem é 1/8 do tamanho da pilha, limitada entre 16 KB e 512 KB, e desde `ec711c7` nunca é menor que **dois degraus reais** medidos rodando (o maior salto da pilha entre duas conferências seguidas). Motivo, no comentário do arquivo: o gcc põe 12 KB no quadro do laço da VM e o clang põe 23 KB (medido com `-fstack-usage`); com a margem fixa, o clang da Apple estourava a página de guarda da fibra antes do `RecursionError`.

---

### 3.4 Compilador em duas passadas (`vm/ps_compiler.c`, `vm/ps_compiler.h`)

O compilador resolve duas coisas em tempo de compilação, e elas saem do caminho quente:

- **Nome vira índice**: `n` dentro de uma funct vira `locals[0]`, sem busca por string.
- **Tabela de protótipos plana**: `MAKE_FUNCTION` leva o índice do protótipo.

O compilador **não conhece** `Value`, `Obj` nem o GC. Ele emite constantes neutras (`PSConst`: `K_NULL`, `K_BOOL`, `K_INT`, `K_FLO`, `K_STR`, `K_BIGINT`, `K_BYTES`), e a VM as converte ao carregar. Uma funct aceita no máximo `PS_MAX_PARAMS = 256` parâmetros fixos (o `self` conta).

**`ps_compila_fonte(fonte, len, recupera, resolvedor, &fc)`** existe desde 2026-09-24:

1. **Primeira passada**: lê o arquivo uma declaração de topo por vez (parser incremental sobre o lexer em fluxo). De cada declaração guarda só o que **outras** declarações precisam (cabeçalhos, campos e fatos dos corpos que o checador consulta) numa arena `keep`. Esse é o passo da **poda** (`poda()`, `passada1_stmt`). Com erro de sintaxe, o compilador para de colher, mas o parser segue até o fim no modo de recuperação para a lista de erros sair completa.
2. **Segunda passada**: abre o parser de novo sobre o mesmo fonte, gera o código de cada declaração (`passada2_stmt`) e a solta.
3. **Gêmeos**: um mapa por posição no fonte liga o nó podado ao nó da segunda passada (`c.gemeos`). Só vale para nós **deste** arquivo; nó de módulo importado nunca passa por `gemeo_de`.
4. Na VM, `proto_assume` assume os vetores do `PSProto`. A tabela de linhas tem uma entrada por instrução (`linhas[ip/2]`).

Medido quando entrou (nota da memória, 2026-09-24): com 200 mil functs, a memória caiu de 612 MB para 353 MB e o tempo de 16 s para 5,9 s; por linha, de 3,1 KB para 1,7 KB. Mudanças no lexer, no parser ou no compilador são conferidas assim: `--ast` e `--check` byte a byte em 23.742 arquivos `.pr` (hash da saída, binário instalado contra o novo), mais `make oom` na VPS.

**Fusões e especializações** que o compilador emite:

| Função | O que gera |
|---|---|
| `funde_salto_falso` | `JF_LT`, `JF_GT`, `JF_LE`, `JF_GE`, `JF_EQ`, `JF_NE`: comparação e salto numa instrução, em `if`, `while` e lado esquerdo do `and` |
| `funde_incremento` | `INC/DEC_GLOBAL_INT` e `INC/DEC_LOCAL_INT` para `x++`, `x--`, `x += 1` e `x -= 1` em variável **declarada** `int`. É uma instrução no lugar de sete, e o estouro de 64 bits dá o mesmo erro do `COERCE_DECL` |
| `for each i in range(...)` | `RANGE_PREPARA` decodifica início, fim e passo uma vez só, e o `ITER_RANGE` de cada volta vira só aritmética |

---

### 3.5 Checador estático (tipagem) (`vm/ps_compiler.c`, funções `tp_*`)

A conferência de tipos mora no próprio compilador e roda no mesmo funil de rodar, importar e `--check`. Para cada pergunta "este valor serve neste lugar?" há três respostas:

| Resposta | Efeito |
|---|---|
| `T_SIM` | sabido e certo: nada é emitido para conferir rodando |
| `T_NAO` | sabido e errado: erro de compilação (`terro`). A compilação **segue** para achar os outros erros, e a lista sai inteira, ordenada pelo fonte |
| `T_TALVEZ` | o tipo só se sabe rodando (`json.parse`, `d["k"]`): a conferência é emitida no lugar (`OP_COERCE_DECL` / `OP_CONFERE_TIPO`) |

Regras gravadas no código:

- **Tipo escrito é da variável**: `str s = "a"` seguido de `s = 42` é erro. O compilador emite `COERCE_DECL` antes de **toda** escrita no nome: reatribuição, `for each`, desempacotamento e escrita de dentro de funct aninhada.
- **Variável sem tipo escrito** fica com o tipo da primeira atribuição. Um valor de tipo desconhecido fixa "qualquer".
- **`Null`** não fixa tipo nem conflita numa variável sem tipo escrito. Variável declarada (`str s`, parâmetro tipado) não aceita `Null`.
- **Resolução de nomes**: segue a mesma ordem do `carrega_nome`/`guarda_nome_modo` (campo static da classe, módulo, `global`, local, captura, global do arquivo).
- **Retorno dos nativos**: vem de `vm/ps_retornos.c`, cuja tabela é **gerada** por `scripts/mede_retornos.pr` em `vm/retornos_medidos.inc` (574 entradas, ordenadas para busca binária). Exemplo: `DbConnection.cursor -> DbCursor`. Três formatos: `"str"`, `"dict|Null"` e `"*"`.
- **"Did you mean"**: `ps_nativo_sugestao` dá a mesma sugestão antes de rodar e rodando.
- **Erro na causa**: a regra acusa na declaração e depois recupera, sintetizando o que faltou para não repetir o erro em cada uso. O modelo está em `tp_self_dos_metodos` (método sem `self`).
- **Resultado**: com erro de tipo, o programa **não roda**. Todos os erros saem no stderr, e o código de saída é 2.

Medido em 2026-09-28:

```
$ printf 'int x = "a"\n' > erro.pr && jinga erro.pr
AttributedValueError: variável x esperava int, recebeu str
  em erro.pr, linha 1
  | int x = "a"
  |         ^^^
(rc 2)
```

**Tabela de tipos** (`vm/ps_tipos.def`, única, incluída pelo parser, pelo compilador, pela VM e pelo `--metadata`). O formato é `PS_TIPO(SUFIXO, "nome", aceita_fn, decl, expr, count, model)`. São 13 tipos: `str`, `int`, `flo`, `bool`, `list`, `dict`, `tup`, `type`, `char`, `PoolFile`, `Object`, `long`, `byte`. São 12 apelidos (`PS_APELIDO`): `string`, `String`, `integer`, `Integer`, `tuple`, `Tuple`, `dictionary`, `Dictionary`, `json`, `JSON`, `object` e `Long`. **A ordem é o código gravado no bytecode**, então tipo novo entra só no fim.

---

### 3.6 Bytecode e opcodes (`vm/ps_opcodes.def`)

- **Lista única**: `ps_compiler.c` (que emite) e `jinga_vm.c` (que executa) incluem o mesmo `.def`, cada um definindo `PS_OP` do seu jeito. O desmontador usa a terceira coluna.
- **Formato**: uma linha por opcode.
  ```
  PS_OP(NOME, NUMERO, "TEXTO DO DESMONTADOR")
  PS_OP(LOAD_CONST, 0, "LOAD_CONST")
  PS_OP(LOAD_BASE_MEMBRO,   114, "LOAD_BASE_MEMBRO")
  ```
- **Quantidade**: **115 opcodes**, numerados de 0 a 114 sem buraco (conferido em 2026-09-28). Só dois têm texto diferente do nome: `IS` sai como `IS_OP` e `IN` como `IN_OP`.
- **Numeração**: o número é explícito porque está gravado em todo bytecode já emitido. Opcode novo entra **no fim**, com o próximo número livre, e nenhum número se reaproveita. `scripts/audita_c.pr` reprova qualquer `OP_X = <número>` escrito fora do `.def`.
- **Instrução**: cada instrução tem dois `int32`, `code[ip]` com o opcode e `code[ip+1]` com o argumento, e o `ip` anda de 2 em 2.

Os 115, na ordem do `.def`:

```
LOAD_CONST LOAD_LOCAL STORE_LOCAL LOAD_GLOBAL STORE_GLOBAL ADD SUB MUL DIV MOD NEG LT GT LE GE
EQ NE JUMP JUMP_IF_FALSE POP_TOP CALL RETURN MAKE_FUNCTION BIT_OR BIT_XOR BIT_AND LSHIFT RSHIFT
BIT_NOT HALT BUILD_LIST BUILD_DICT INDEX_GET INDEX_SET ITER_NEXT DUP BUILD_STR BUILD_TUPLE SLICE
JUMP_IF_SET LOAD_NAME STORE_NAME CALL_KW SETUP_TRY POP_TRY RAISE PUSH_ERR_TYPE JUMP_IF_TRUE LEN
HAS_KEY MAKE_CLASS GET_MEMBER SET_MEMBER LOAD_SELF CALL_BASE DUP2 IMPORT_MOD IS IN LOAD_TIPO COUNT
COUNT_PARES CHECK_NONNULL MAKE_MODEL UNPACK YIELD CLOSE_SE_TEM NOT TO_BOOL COERCE_DECL COERCE_RET
RERAISE SKIP_IF_IMPORT MAKE_ENUM CLEAR_LOCAL CLEAR_GLOBAL AWAIT LOAD_BASE_INIT MAKE_CELL CELL_GET
CELL_SET LOAD_UPVAL STORE_UPVAL MAKE_CLOSURE CELL_GET_NAME CELL_SET_NAME ITER_RANGE IMPORT_FROM POW
FLOORDIV SWAP ROT3 EXC_CASA DECORA LIST_EXTEND DICT_MERGE CALL_EX LOAD_METODO SET_METODO
IMPORT_FROM_ESTRELA JUMP_SE_UNSET CONFERE_TIPO ENGOLE JF_LT JF_GT JF_LE JF_GE JF_EQ JF_NE
INC_GLOBAL_INT DEC_GLOBAL_INT INC_LOCAL_INT DEC_LOCAL_INT RANGE_PREPARA LOAD_BASE_MEMBRO
```

Saída real de `jinga --bytecode` (2026-09-28) para `int n = 0` / `while n < 3 { n++ }` / `post(n)`:

```
== proto 0 <module> nlocals=0 nparams=0
     0  LOAD_CONST       0       ; 0
     2  STORE_GLOBAL     0       ; n
     4  LOAD_GLOBAL      0       ; n
     6  LOAD_CONST       1       ; 3
     8  JF_LT            14      ; -> 14
    10  INC_GLOBAL_INT   0       ; n
    12  JUMP             4       ; -> 4
    14  LOAD_GLOBAL      1       ; post
    16  LOAD_GLOBAL      0       ; n
    18  CALL             1
    20  POP_TOP          0
    22  HALT             0
```

---

### 3.7 A VM (`vm/jinga_vm.c`, 31.085 linhas)

**Valor** (`struct Value_`): etiqueta mais união, 16 bytes (etiqueta em +0, união em +8). As etiquetas (`VType`) são `V_NULL`, `V_BOOL`, `V_INT` (int64 nativo, sem alocação), `V_FLOAT` (double, sem alocação), `V_FUNC` (índice de protótipo), `V_NATIVE` (builtin em C), `V_UNSET` (slot nunca escrito, diferente de `Null`), `V_TIPO` (referência de tipo, imediata) e `V_OBJ` (objeto sob o GC).

**Objetos**: são 51 tipos (`ObjType`, de `OBJ_STRING` a `OBJ_CLOSURE`, com sentinela `OBJ__COUNT`):

- linguagem: string, list, tuple, dict, class, instance, bound, método nativo, módulo, nativa, model, enum, gerador, arquivo, módulo `.pr`, bytes, célula, closure, future, bigint (GMP), regex compilado;
- bibliotecas: conexão e cursor sqlite; mail; response HTTP; QR; manpu; DB; Mongo; PoolFile; os 11 objetos do jinker (`JINKER`, `JCORS`, `JREG`, `JRESP`, `JREQ`, `JPROXY`, `JUPLOAD`, `JSOCKNS`, `JEMIT`, `JCHAN`, `JCHST`); conexão WS cliente; socket; processo.

**Limites** (constantes no fonte):

| Constante | Valor | O que é |
|---|---|---|
| `MAX_FRAMES` | 65536 | frames de chamada no programa principal |
| `STACK_SIZE` | 65536 | pilha de valores |
| `LOCALS_SIZE` | 262144 | pool de locais |
| `MAX_HANDLERS` | 256 | `try` ativos por frame de base |
| `PS_STR_MAX` | 256 MB | string construída por largura (`zfill`, `ljust`...); acima vira `MemoryError` |
| `GC_INICIAL` | 1 MB | primeiro limiar de coleta |

Um estouro é **erro explícito**, nunca escrita fora do vetor. A recursão profunda é barrada pela folga da pilha do C (`ps_pilha`), não por contagem.

**Despacho por goto calculado**:

- Com GCC ou clang (`__GNUC__`) e sem `-DPS_SEM_GOTO`, `PS_GOTO` fica ligado. Cada opcode tem um rótulo `L_<NOME>` (macro `PS_CASE(x)`, que põe o rótulo **e** o `case`). Com `-DPS_SEM_GOTO`, o mesmo corpo volta a compilar como `switch`.
- Há três tabelas indexadas pelo número do opcode:
  - `TAB`: o despacho normal;
  - `TAB_DBG`: tudo cai em `L_dbg`, que chama `dbg_passo`. Com o depurador desligado, o `if` dele não fica no caminho de cada instrução;
  - `TAB_UMA`: tudo cai em `L_uma`, que entra no código de máquina (ver 3.8).
- **Despacho replicado**: o tratador quente busca a próxima instrução e salta ele mesmo (macro `PROXIMA()`), então cada sítio de salto indireto tem o próprio histórico no preditor.
- `vm_executa_base` leva `__attribute__((optimize("no-crossjumping")))` só no GCC. `no-gcse` **piora** (medido: +45% de instruções). O Makefile passa `--param=max-goto-duplication-insns=64` quando o compilador não é clang. Com o padrão (8), 52 sítios de despacho viravam 2 e o interpretador ficava 16% mais lento em instruções.

**Chamada**: `chama_proto` decide entre gerador, fibra e frame. O binding de argumentos fica em `liga_args` (receptor, `@static`, aridade, `*args`, `**kwarg`, tipo declarado) e é o mesmo para chamada vinda do bytecode ou do C (handler do jinker, `map`, decorador, fibra async).

**Códigos de saída** (`reporta` em `main.c`):

| rc | Situação |
|---|---|
| 0 | sucesso |
| 1 | erro em tempo de execução (traceback no stderr); tarefa `async` que quebrou sem ninguém aguardar (`PS_ERRO_TAREFA`); `--check` com erro |
| 2 | `SyntaxError` ou erro da tipagem estática (o programa não rodou) |
| 3 | `NotImplementedError` (nó que o compilador não emite) |
| 4 | `MemoryError` |
| 64 | uso errado da linha de comando |
| 65 | `-o`: módulo não achado ou não compila; nada é gerado |
| 66 | arquivo `.pr` não abre |
| 70 | `-o`: o binário não acha o próprio executável, ou o payload embutido está corrompido |
| 73 | `-o`: escrita incompleta ou sem permissão de execução |

`sys.exit(n)` mantém o `n`.

---

### 3.8 JIT x86-64 (`vm/ps_jit_x64.h`)

É incluído **uma vez** por `jinga_vm.c`, antes do laço. Fica ligado quando `PS_GOTO` está ativo, a arquitetura é `__x86_64__` e não há `-DPS_SEM_JIT` (macro `PS_JIT`). O MacBook Pro 2019 (i9-9880H) é x86-64, então o JIT vale lá também. O comentário do job `mac` em `.github/workflows/build.yml` diz o mesmo: "Mac Intel ... mesma arquitetura, mesmo JIT".

**Desenho**:

- **Compilação preguiçosa**: uma funct vira código de máquina na primeira vez que roda, pelo `JIT_TENTA` na entrada do frame de base, no fim de `chama_proto` e no `RETURN`. Funct nunca chamada não custa nada. O estado por proto fica em `Proto.nativo`, `Proto.nativo_ip[]` (retomada em qualquer `ip`) e `Proto.jit_estado` (0 = ainda não tentou, 1 = tem nativo, 2 = recusado, não tenta de novo).
- **Gabaritos em linha** (opcodes que o nativo executa): `LOAD/STORE_LOCAL`, `LOAD/STORE_GLOBAL`, `LOAD/STORE_NAME`, `LOAD_CONST`, `CLEAR_LOCAL/GLOBAL`, `DUP`, `POP_TOP`, `JUMP` (o recuo confere o GC), `JUMP_IF_FALSE/TRUE`, `JF_*`, `LT`/`GT`/`LE`/`GE`/`EQ`/`NE`, `ADD`/`SUB`/`MUL`, `INC/DEC_*_INT`, `COERCE_DECL`, `NOT`, `TO_BOOL`, `ITER_RANGE`, `CALL` e `RETURN`.
- **Ilha**: todo o resto, e todo caminho rápido que desiste (operando que não é int, estouro, global não definido, coleta de lixo devida), sai do nativo com o `ip` da instrução. O interpretador executa **só aquela instrução** com o tratador de sempre e, pela `TAB_UMA`/`L_uma`, volta para o nativo na seguinte. Erro, chamada, retorno, `try`, gerador, fibra, import e depurador continuam no interpretador, com as mesmas frases. Nenhuma regra é duplicada.
- **Conferir antes de mexer**: quando o caminho rápido desiste, a pilha está exatamente como o interpretador espera para refazer a instrução.
- **Registradores** (System V): `r12` = vm, `r13` = base da pilha de valores, `r14` = locals + lbase, `r15` = `JitCtx`, `rbx` = próximo slot livre da pilha (ponteiro). Cópias de `Value` vão em duas metades de 8 bytes.
- **Chamada direta** (`jit_chama`): só para funct nativa, sem gerador, async ou static, com aridade exata, sem padrão/varargs/kwargs e dentro da folga da pilha. O chamado roda no mesmo `JitCtx` com `direto = 1`. Se ele cai numa ilha, o interpretador **adota** o frame que voltou (fp, lbase, sp, locals_top, ip, proto, closure, nargs).
- **Heurística**: um proto em que mais de **15%** das instruções sempre saem para ilha (dentro dos laços; sem laço, no proto inteiro) **não compila** e fica interpretado.
- **Memória executável**: `ps_mem_exec_nova` (camada de sistema) faz `mmap` RW, copia o código e dá `mprotect` para leitura e execução, nunca escrita e execução ao mesmo tempo. Se o sistema recusar, o programa segue interpretado e nunca aborta.
- **Depurador**: com `--debug` ativo, `JIT_TENTA` não compila nada e tudo roda no interpretador.

**Como desligar e diagnosticar**:

| Controle | Efeito |
|---|---|
| `JINGA_JIT=0` | desliga o código de máquina (lido uma vez, em `jit_ligado()`) |
| `jinga --sem-jit ...` | igual a `JINGA_JIT=0`: o `main` faz `setenv("JINGA_JIT","0")` e tira a flag do argv; vale em qualquer posição |
| `JINGA_JIT_LOG=1` | escreve no stderr cada proto compilado |
| `JINGA_JIT_FALHA=1` | simula a recusa do `mmap`/`mprotect`; o programa segue interpretado |
| `JINGA_GC_LOG=1` | escreve no stderr cada coleta de lixo (`[gc] coleta N`); com e sem JIT o número tem de ser o mesmo |
| `-DPS_SEM_JIT` | compila o binário sem o JIT |

```
$ JINGA_JIT_LOG=1 jinga ex.pr
[jit] <module>: 12 instrucoes, 615 bytes
3
$ jinga --sem-jit ex.pr
3
```

`make check` roda a suíte duas vezes, `./testar` e `JINGA_JIT=0 ./testar`: o interpretador é a referência e os dois têm que concordar em tudo. O `build.yml` do macOS também roda `./testar && JINGA_JIT=0 ./testar`.

Números medidos com `perf stat` neste laptop, registrados na nota `jinga-rapida-fase1` (wall-clock aqui varia 2x por causa da temperatura; a comparação vale só em instruções e ciclos):

| Caso | Antes | Fase 1 (interpretador) | Com JIT |
|---|---|---|---|
| laço tipado 1e7 (instruções / ciclos) | 5,19G / 1,65G | 1,12G / 0,46G | 0,33G / 0,087G (8,7 ciclos por volta) |
| fib(27) | 605M / 203M | 349M / 116M | 253M / 63M (com chamada direta) |
| fib(30), ciclos | 549M (interpretado) | | 267M |

O laço `int n = 0; while n < 1e9 { n++ }` levou 2,74 s com `-o` e 17,5 s com `--sem-jit`. `str_hash` preguiçoso levou um caso de texto de 19,6 s para 0,31 s.

Ainda não feito: builtin chamado a partir do nativo continua sendo ilha, float não tem gabarito em linha, e membro/coleção vão por helper.

---

### 3.9 Coletor de lixo (GC)

- **Tipo**: mark-and-sweep que para o mundo, com lista ligada de todos os objetos (`vm->objetos`).
- **Pontos seguros**: o GC só dispara em `PONTO_SEGURO()`. Alocar nunca coleta, só soma em `vm->alocado`. Os pontos seguros são o `JUMP` de recuo (a volta de todo `while`/`for each`) e a entrada de cada `CALL`. Neles a pilha está consistente, e o frame corrente publica `gc_fp` e `gc_cl` para o closure em execução ser raiz.
- **Limiar**: depois de cada coleta, `proximo_gc = alocado * 2`, com mínimo `GC_INICIAL` (1 MB).
- **Raízes**, em `gc_coleta` e só ali: globais; pilha viva `[0, sp)`; locais `[0, locals_top)`; singletons e requisição corrente do jinker (`jk_cors`, `jk_proxy`, `jk_req`, `jk_app`); módulos `.pr` importados (`mods_ps`); conexões WebSocket abertas (`ws_vivos`); closures dos frames em execução; fibras suspensas (`fib_marca_gc`); constantes dos protótipos.
- **Marcação sem recursão em C**: pilha cinza (`empilha_cinza` / `percorre_cinzas`).
- **Tabela por tipo** (`GC_INFO[OBJ__COUNT]`): cada `ObjType` declara:
  - o tracer: `GC_LEAF` (sem filhos), `GC_ONE` (um `Value` num offset) ou `GC_FN` (tracer próprio);
  - `tam` (tamanho fixo, descontado de `vm->alocado`);
  - `fin` (finalizador, que libera os buffers C).
  O sweep é `vm->alocado -= gi->tam; if (gi->fin) gi->fin(vm, o); free(o);`.
- **Validação no boot**: `gc_valida_tabela` **aborta** se algum tipo ficar sem tracer ou sem `tam`/`fin`. Um `ObjType` novo é obrigado a declarar os três.
- **Buffers temporários**: `SBUF_AUTO` e `TXTBUF_AUTO` (`__attribute__((cleanup))`) liberam sozinhos; são 100 ocorrências em `jinga_vm.c`. A regra da casa: buffer temporário local é `SBUF_AUTO`/`TXTBUF_AUTO`, nunca `free` manual. Posse transferida zera o `.b` na hora. Um `SBuf` **não** é terminado em NUL: entrega-se sempre com tamanho, ou NUL-terminado explicitamente.
- **Arena por requisição** no jinker: path, query, headers, nome e valor saem de um bloco bump de 4 KB (`PSJkReq.ar`), soltos de uma vez por `ps_jk_req_solta`. O corpo (até 64 MB) é um `malloc` à parte.
- **Invariantes**: `PS_ASSERT`/`PS_ASSERT_MSG` (`vm/ps_assert.h`) conferem `sp`, `locals_top` e `gc_fp` no início da coleta e o tipo de cada objeto marcado. Só valem no build `-DPS_DEBUG` (`make check-debug`). No binário normal custam zero.

---

### 3.10 Fibras, pool de threads e laço de eventos

**Fibras** (green-threads, em `jinga_vm.c`):

- Cada requisição HTTP, cada handler de WebSocket e cada `async funct` roda numa fibra: `FIB_HTTP`, `FIB_WS` e `FIB_ASYNC`.
- Uma fibra tem pilha do C própria e vetores próprios de valores, locais e frames.
- **Troca de contexto em assembly** (`ps_fctx_swap`): no x86-64, salva só `rbp`, `rbx`, `r12` a `r15` e `rsp`, sem nenhuma syscall. As instruções são as mesmas no Linux e no macOS (mesma ABI System V); muda só a forma do objeto (o Mach-O põe `_` na frente do nome e não tem `.type`/`.size`). Em outras arquiteturas, o fallback é `ucontext`.

| Constante | Valor |
|---|---|
| `FIB_CSTACK` | 256 KB de pilha do C por fibra (o comentário de `ps_pilha.h` ainda fala em 128 KB; o valor em vigor é 256 KB) |
| `FIB_STACK` | 2048 valores |
| `FIB_LOCALS` | 4096 valores |
| `FIB_FRAMES` | 512 frames |
| `FIB_HARD` | 8192 fibras concorrentes (rede de segurança; o pool cresce sob demanda) |

- **Pilha com página de guarda**: `ps_pilha_fibra_nova` faz `mmap` com uma página `PROT_NONE` no endereço baixo. Um estouro vira falha no ponto exato, em vez de escrita silenciosa no heap. O mapeamento é reservado e só as páginas tocadas viram RAM.
- **Back-pressure**: com o pool cheio, a conexão é **enfileirada** (`http_drena_fila`), nunca servida inline.
- **Uma thread só**: a VM tem uma única thread. Não há corrida nem GC concorrente.

**Offload para o pool de threads** (`fib_offload`):

- A chamada C bloqueante vai para uma das `POOL_THREADS = 8` threads (`pool_garante` / `pool_submete` / `pool_worker`). A fibra cede para o escalonador e volta quando a thread sinaliza o **aviso** (`PsAviso`, da camada de sistema).
- A thread toca só dado C (handle do driver, buffers), **nunca** o heap da VM.
- Regra de GC: nenhum objeto da VM é criado antes de um `fib_offload` na mesma função.
- Fora de fibra e sem tarefa async viva, a chamada roda inline.
- No programa principal **com** tarefa async viva, a I/O vai para o pool e o principal roda as tarefas enquanto espera.

Tudo que passa por `fib_offload` hoje:

| Área | Funções |
|---|---|
| banco SQL | `db_conn_offload`, `db_exec_offload` |
| Mongo | `mg_conn_off`, `mg_find_off`, `mg_ins_off`, `mg_upd_off`, `mg_rm_off`, `mg_cnt_off` |
| SMTP | `smtp_conn_off`, `smtp_login_off`, `smtp_send_off` |
| IMAP | `imap_conn_off`, `imap_login_off`, `imap_sel_off`, `imap_search_off`, `imap_fetch_off` |
| HTTP cliente | `req_http_offload` |
| WebSocket cliente | `ws_conn_offload`, `ws_send_off`, `ws_recv_off`, `ws_frame_off`, `ws_req_off` |
| sockets | `sk_acc_off`, `sk_conn_off`, `sk_dns_off`, `sk_send_off`, `sk_recv_off` |
| processos | `espera_proc_off`, `drena_proc_off`, `proc_ler_off`, `proc_espera_off` |

`sleep()` dentro de fibra cede por timer (`fib_espera_ms`). O SQLite roda inline.

**Laço de eventos**:

- `g_jk_epfd` é criado por `ps_ev_cria()`. No Linux é `epoll`, no macOS é `kqueue` (ver 3.11).
- Cada fd registrado aponta para uma estrutura cujo primeiro campo é o tipo do observador:
  - `EPW_HTTP`, `EPW_WS`: conexão;
  - `EPW_LHTTP`, `EPW_LWS`: socket de escuta;
  - `EPW_FIBWAIT`: fibra esperando offload;
  - `EPW_MORTO`: conexão que morreu no meio da volta; o enterro é adiado para não ler memória liberada no mesmo lote de eventos;
  - `EPW_PRINCIPAL`: I/O do programa principal.
- **async/await**: `async funct` vira fibra `FIB_ASYNC` mais um objeto future (`OBJ_FUTURO`). `await` e `gather` dirigem o escalonador (`sched_roda`), no topo do programa e dentro de handler.
- **Tarefa com erro que ninguém aguardou**: `avisa_futures_com_erro` imprime o erro no fim, e o processo sai com **rc 1** (`PS_ERRO_TAREFA`), mesmo depois de `exit()`.

Medidas registradas nas notas `roadmap-jinker-async` e `gc-finalizer-refactor` (agosto de 2026):

- rota `/slow` (`sleep 0.1`): de 10 para 197 requisições por segundo;
- 16 mil conexões ociosas ocupam cerca de 16 MB (cerca de 275 B por conexão, porque o buffer de leitura é solto na ociosidade por `ps_jk_conn_solta_buf`);
- `pg_sleep(0.2)` com 10 concorrentes: 0,42 s;
- a troca em assembly levou a rota `/fast` com 1 worker de cerca de 6 mil para cerca de 12 mil requisições por segundo.

---

### 3.11 Camada de sistema (`vm/ps_so.h`, `vm/ps_so.c`)

Existe desde o commit `529c518`. O motor não chama mais `epoll`, `eventfd` nem `/proc` direto: tudo que muda de um sistema para outro fica atrás desta API pequena. `PS_SO_LINUX`, `PS_SO_MAC` e `PS_SO_WINDOWS` são escolhidos pelo compilador, e `PS_SO_NOME` vale `"linux"`, `"darwin"` ou `"windows"`.

| Área | API | Linux | macOS | Windows (desde `3c245e7`) |
|---|---|---|---|---|
| eventos (por nível, só leitura) | `ps_ev_cria`, `ps_ev_fecha`, `ps_ev_poe`, `ps_ev_arma`, `ps_ev_tira`, `ps_ev_espera` (devolve `PsEvento{ptr, leu, hup}`) | `epoll_create1(EPOLL_CLOEXEC)`; `ler = 0` equivale ao antigo `EPOLL_CTL_MOD` com eventos 0 | `kqueue()` com `EVFILT_READ` sem `EV_CLEAR` (por nível); `ler = 0` vira `EV_DISABLE`; até 256 eventos por espera. O kqueue não é herdado no `fork`, e cada worker do jinker cria o seu | `WSAPoll` sobre uma tabela de fds |
| aviso entre threads | `ps_aviso_cria`, `ps_aviso_sinaliza`, `ps_aviso_espera`, `ps_aviso_fecha` (`PsAviso{le, escreve}`) | `eventfd(0, EFD_CLOEXEC)`, com `le` e `escreve` sendo o mesmo fd | `pipe()` com `FD_CLOEXEC` nas duas pontas | par de sockets conectado em 127.0.0.1 (o `WSAPoll` só espera socket) |
| pilha de fibra | `ps_pilha_fibra_nova`, `ps_pilha_fibra_solta` | `mmap` mais página de guarda `PROT_NONE` | igual | a pilha é do próprio sistema (`CreateFiberEx`, pelo comentário de `ps_so.c`); aqui fica só uma página de registro (`VirtualAlloc`) |
| memória do JIT | `ps_mem_exec_nova`, `ps_mem_exec_solta` | `mmap` RW, `memcpy` e `mprotect(PROT_READ\|PROT_EXEC)` | igual | `VirtualAlloc` RW, `memcpy`, `VirtualProtect(PAGE_EXECUTE_READ)` e `FlushInstructionCache` |
| resolução de nomes (módulo `sockets`) | `ps_host_por_nome`, `ps_host_por_ip4` | `gethostbyname_r` / `gethostbyaddr_r` | `gethostbyname` / `gethostbyaddr` (resultado por thread) | `gethostbyname` / `gethostbyaddr` do Winsock |
| próprio executável (`-o`, `sys.executable`, reload do jinker) | `ps_meu_executavel` | `readlink("/proc/self/exe")` | `_NSGetExecutablePath` mais `realpath` | `GetModuleFileNameA` |
| raízes TLS (cliente HTTPS e mail) | `ps_so_tls_cas` (desde `3c245e7`) | as do OpenSSL (`SSL_CTX_set_default_verify_paths`) | as do OpenSSL mais as âncoras do Keychain (`SecTrustCopyAnchorCertificates`) | as do OpenSSL mais o repositório `ROOT` do Windows (`CertOpenSystemStoreA`) |
| socket (desde `3c245e7`) | `ps_so_inicia`, `ps_sock_fecha`, `ps_sock_le`, `ps_sock_escreve`, `ps_sock_nao_bloqueia`, `ps_sock_timeout`, `ps_sock_espera_leitura`, `ps_sock_errno` | `close`, `read`, `poll` e o `errno` de sempre | igual ao Linux | `closesocket` e o erro do Winsock traduzido pro `errno` |

- **Windows**: o ramo existe desde `3c245e7`, compilado cruzado com MinGW contra o MSYS2; a suíte ainda não roda lá (9.10). O `vm/ps_win.h` (126 linhas no commit) dá os nomes que o Windows chama diferente: `mkdir`, `realpath`, `setenv`, `strndup`, `memmem`, `strcasestr`, `sysconf`, `gmtime_r`/`localtime_r`.
- `sys.platform()` é decidido na compilação e devolve `linux`, `darwin` ou `windows`. Medido aqui: `jinga -e 'import sys; post(sys.platform())'` imprime `linux`.
- **Ajustes de macOS fora de `ps_so`**:
  - `ps_jinker.c` define `MSG_NOSIGNAL` como 0 (o processo inteiro já ignora `SIGPIPE`);
  - `ps_db.c` e `ps_mongo.c` têm as listas de `.dylib` (ver 3.13);
  - o Makefile tem um ramo `Darwin`: Homebrew, clang da Apple, `.a` do OpenSSL e da libpng, e os frameworks Security e CoreFoundation.

---

### 3.12 jinker: servidor HTTP/1.1 e WebSocket

O jinker tem duas camadas:

- **`vm/ps_jinker.c` / `.h`**: o transporte. Socket cru mais OpenSSL, protocolo escrito à mão, **sem conhecer a VM**. Lê e escreve bytes e estrutura requisições.
- **Camada jinker em `jinga_vm.c`**: rotas, handlers, CORS, salas e o laço. Os objetos são `OBJ_JINKER`, `JCORS`, `JREG`, `JRESP`, `JREQ`, `JPROXY`, `JUPLOAD`, `JSOCKNS`, `JEMIT`, `JCHAN` e `JCHST`.

Comportamento gravado no código:

- **Subida**: `app(debug=, host=, port=, reload=, workers=)` (os parâmetros de `jk_obj_callable`).
- **Multi-processo (prefork)**: com `workers > 1` (limitado a 256), o processo faz `fork` de N filhos que dividem o socket HTTP, e o kernel balanceia o `accept`. Cada worker tem a própria VM. O pai só supervisiona e derruba os filhos no Ctrl+C/TERM. Cada filho recria o próprio pool de threads, porque o `fork` copia só a thread que chamou.
- **WebSocket** num socket separado, na **porta + 1**, aberto só pelo worker 0. Salas e broadcast ficam num processo só.
- **Globais injetadas**: `request` e `channel` são postos direto no slot global na subida do servidor.
- **Keep-alive**: a leitura é bufferizada por conexão, porque um `recv` pode trazer o fim de uma requisição e o começo da outra.
- **Corpo** de no máximo 64 MB (`JK_CORPO_MAX`).
- **Requisição torta**: uma requisição malformada recebe **400** e uma com `Transfer-Encoding` recebe **501**, nunca "fechar calado" (RFC 9112, contra *request smuggling*).
- **HEAD**: uma rota de GET responde HEAD sozinha, com os mesmos headers (inclusive `Content-Length`) e sem corpo. O mecanismo é `PSJkConn.sem_corpo`.
- **Resposta**: sempre com `Content-Length`, `Server: Jinker` e `Date`.
- **WebSocket (RFC 6455)**: handshake com `Sec-WebSocket-Accept` (SHA-1 da OpenSSL mais base64), ping respondido com pong, e um cliente (`ps_jk_ws_conecta`) com frames mascarados.
- **Multipart** (`ps_jk_multipart`), tabela de MIME por extensão e decodificação de URL (no path, `+` não vira espaço; na query, vira).
- **TLS**: `ps_jk_tls_ctx` com cert e chave. `ps_jk_tls_autogera` gera um certificado autoassinado (CN=localhost, RSA 2048, 365 dias).
- **CORS**: `cors(app, ..., local=False)` é o modo estrito. Só a lista `origins` vale, localhost fora da lista recebe 403, e pedido **sem** `Origin` recebe 403, tanto no HTTP quanto no handshake do WebSocket. A lista de `origins` vale no handshake do WS. Sem `cors()`, fica como sempre foi: HTTP com `*` e WS aceitando qualquer origem. O 403 de origem sai **sem** `Access-Control-Allow-Origin`. `jk_is_local` só aceita `127.` seguido de dígito ou ponto (`127.evil.com` não é local), e `[::1]:porta` é local.
- **PoolIp**: rate limit por IP numa janela de 60 s, mais ban em dias.
- **Auto-reload** (desenvolvimento, só com 1 worker): vigia o mtime do `.pr` de entrada e re-executa o processo quando ele muda.

---

### 3.13 Drivers de banco carregados por `dlopen` no primeiro `connect` (`vm/ps_dl.h`)

**Por que existe**: o binário era ligado aos clientes do Postgres, do MySQL, do ODBC e do Mongo, e o sistema carregava todos eles antes da primeira linha de qualquer programa, junto com o que eles arrastam (Kerberos, LDAP, gnutls, SASL: 36 bibliotecas). Medido com callgrind, 90% do trabalho de um `pool --version` era o carregador dinâmico relocando símbolos dessas bibliotecas, e mais 8% o construtor do gnutls. Um programa nem abria numa máquina sem o cliente do Mongo.

**Como funciona**:

1. Cada driver declara, numa macro X, **todas** as funções que chama da biblioteca: `PG_FUNCS`, `MY_FUNCS` e `OD_FUNCS` em `ps_db.c`, `MG_FUNCS` em `ps_mongo.c`.
2. `PS_DL_PONTEIRO(f)` cria um ponteiro `dl_f` com o tipo da declaração do cabeçalho. O cabeçalho continua incluído; só a ligação sai.
3. Um `#define PQexec dl_PQexec` troca o nome pelo ponteiro, e o código continua escrevendo `PQexec(...)`.
4. `PS_DL_CARREGA` roda **uma vez**, protegido por mutex (o `connect` pode rodar na thread do pool). Faz `dlopen(RTLD_NOW | RTLD_LOCAL)` no primeiro nome da lista que o sistema achar e resolve todas as funções com `dlsym`. No Windows (desde `3c245e7`) quem faz esse papel é `LoadLibraryA`/`GetProcAddress`. O estado fica guardado: 0 = nunca tentou, 1 = carregou, -1 = falhou e não tenta de novo.
5. Na falha, a frase é: `<driver> precisa da biblioteca <nome>, que nao foi encontrada (<motivo>)`, ou `falta a funcao X`.

Nomes procurados, na ordem:

| Driver | Linux | macOS | Windows (desde `3c245e7`) |
|---|---|---|---|
| Postgres | `libpq.so.5`, `libpq.so` | `libpq.5.dylib`, `@executable_path/../lib/jinga/libpq.5.dylib`, `/usr/local/opt/libpq/lib/libpq.5.dylib`, `/opt/homebrew/opt/libpq/lib/libpq.5.dylib` | `libpq.dll` |
| MariaDB/MySQL | `libmariadb.so.3`, `libmysqlclient.so.21`, `libmariadb.so` | `libmariadb.3.dylib`, `@executable_path/../lib/jinga/libmariadb.3.dylib`, `/usr/local/lib/mariadb/...`, `/opt/homebrew/lib/mariadb/...`, `/usr/local/opt/mariadb-connector-c/lib/mariadb/...`, `/opt/homebrew/opt/mariadb-connector-c/lib/mariadb/...` | `libmariadb.dll` |
| ODBC (mssql) | `libodbc.so.2`, `libodbc.so` | `libodbc.2.dylib`, `@executable_path/../lib/jinga/libodbc.2.dylib`, `/usr/local/lib/libodbc.2.dylib`, `/opt/homebrew/lib/libodbc.2.dylib` | `odbc32.dll` |
| Mongo | `libmongoc-1.0.so.0`, `libmongoc-1.0.so` | `libmongoc-1.0.0.dylib`, `@executable_path/../lib/jinga/...`, `/usr/local/lib/...`, `/opt/homebrew/lib/...`, `/usr/local/opt/mongo-c-driver@1/lib/...`, `/opt/homebrew/opt/mongo-c-driver@1/lib/...` | `libmongoc2.dll` (driver 2.x, o do MSYS2), `libmongoc-1.0.dll` |

- `/usr/local` é o Homebrew do Mac Intel. `@executable_path/../lib/jinga/` é o pacote gerado por `build_bundle.sh`.
- O **sqlite** vem dentro do binário, e um programa que usa só sqlite não carrega nenhum driver.
- **Postgres**: um erro vira classe de exceção pelo SQLSTATE, com a tabela `vm/ps_pgerr.h` (252 entradas, todas filhas de `DatabaseError`; por exemplo, `catch (UniqueViolation e)`).
- **Placeholders**: passam por um varredor só (`sql_varre_placeholders`). O sqlite e o MySQL usam `?`, o Postgres usa `%s`.
- **Mongo**: a ponte é JSON. `Value -> JSON -> BSON` na entrada, `BSON -> JSON relaxado -> Value` na saída, e o `_id` é removido.
- **Nota**: a seção "Guia de deploy" do `jinga.md` ainda lista libpq, mysqlclient e odbc como embutidos no ELF. O `Makefile` e o `ldd ./pool` (só libm, libgmp e libc) mostram que não estão: eles vêm por `dlopen`.

---

### 3.14 Outros módulos em C dentro do motor

| Módulo | Arquivo | O que faz |
|---|---|---|
| cliente HTTP/HTTPS | `ps_http.c/.h` | socket mais OpenSSL, sem libcurl. **Verifica** o certificado (`SSL_VERIFY_PEER` mais `SSL_set1_host`) contra as raízes do OpenSSL e as do sistema (`ps_so_tls_cas`, desde `3c245e7`); lê o corpo por Content-Length, chunked ou até fechar; segue 3xx; tem teto de corpo, timeout e modo de fluxo para arquivo (`nbaixado`) |
| SMTP, IMAP e MIME | `ps_mail.c/.h` | uma conexão para os dois protocolos. O TLS **verifica** a cadeia e o hostname, com as mesmas raízes (`ps_so_tls_cas`); só `PS_MAIL_TLS_INSEGURO=1` desliga. O comentário do `ps_mail.h` ainda diz "SEM verificação"; o código em `liga_tls` verifica |
| hash | `ps_hash.c/.h` | SHA-256/384/512 (RFC 6234), HMAC (RFC 2104), PBKDF2-HMAC-SHA256 (RFC 8018), base64 e base64 urlsafe (RFC 4648), sem OpenSSL; no Windows o sorteio vem de `BCryptGenRandom` |
| regex | `ps_regex.c/.h` | backtracking recursivo com codepoint (não byte). Tem classes, quantificadores gulosos e preguiçosos, grupos, `\1`..`\9`, `(?P<n>...)`, `(?i)`, `(?m)`, `(?s)`, `(?i:...)`, lookahead e lookbehind de largura fixa, e `\b`, `\B`, `\A`, `\Z`. Recusa com erro: `(?>...)`, `(?(id)...)`, `(?#...)` e `\D`/`\W`/`\S` dentro de `[...]` |
| QR Code | `ps_qr.c/.h`, `ps_qr_tables.h` | ISO/IEC 18004 em modo byte, versões 1 a 40, níveis L/M/Q/H, Reed-Solomon, escolha de máscara pelas 4 regras de penalidade; o PNG sai pela libpng |
| xlsx | `ps_xlsx.c/.h` | ZIP escrito à mão sobre a zlib mais XML com expat |
| pacotes | `ps_pkg.c/.h` | `jpkg`: `~/.jinga` (ou `$JINGA_HOME`, ou `$POOLSCRIPT_HOME`) com `commands/`, `libs/` e `bin/` (shims; no Windows, `.cmd` e PATH por `setx`); `installed.json` é reescrito a partir do disco; `config.json` guarda o `registry_url`; download por HTTPS com sha256 opcional. Uma `~/.poolscript` antiga é movida para `~/.jinga` com aviso |
| inteiro grande | `ps_gmp_min.h` | a interface `mpz` da GMP declarada à mão (símbolos `__gmpz_*`), ligada a `libgmp.so.10` no Linux e a `-lgmp` do Homebrew no macOS |

---

### 3.15 Depurador (`jinga --debug <porta> <arquivo.pr> [args]`)

- O motor fala o **Debug Adapter Protocol** direto. `vm/ps_debug.c` faz o enquadramento `Content-Length: N\r\n\r\nJSON`, lendo byte a byte de propósito. A cola com frames e variáveis fica em `jinga_vm.c` (`dbg_passo`, `dbg_serve`).
- Escuta **só em `127.0.0.1:<porta>`** e **não executa a primeira instrução** antes de o editor conectar e terminar o aperto de mão.
- Usa socket e não stdin/stdout porque o `post()` do programa quebraria o enquadramento. A saída do programa continua no stdout dele.
- **O que faz**: breakpoint por arquivo e linha (o caminho é comparado pelo fim); `next`, `stepIn`, `stepOut`, `continue` e `pause`, de linha em linha; pilha de chamadas; variáveis **vivas no ponto** (o compilador grava a faixa de bytecode de cada nome em `PSVarDbg`, porque os slots são reaproveitados); parada automática na exceção não capturada, com o frame ainda montado.
- **Gráfico de execução**: arestas linha a linha em que aresta repetida vira contador, com teto de 4096 arestas distintas. É o pedido `jingaGrafico`, extensão própria ao DAP, que continua respondendo depois do `terminated`.
- **Recusa com motivo**: `evaluate`, `setVariable`, breakpoint condicional ou por contagem, logpoint e `exceptionInfo`. As fibras aparecem como uma thread só.
- **JIT**: com o depurador ativo, tudo roda no interpretador.
- **Teste**: `teste/depurador.pr` (dentro do `make check`) fala DAP com o motor. Os alvos `teste/alvo_debug.pr` e `teste/alvo_debug_erro.pr` têm os números de linha como parte do teste.
- **VS Code**: F5, sem `launch.json`. A documentação completa está em `docs/debugger.md`.

---

### 3.16 Flags do binário

Saída literal de `./pool --help` em 2026-09-28:

```
Jinga 16.1.9 — PSVM (VM em C, runtime standalone)

Uso:
  jinga arquivo.pr           Roda um arquivo
  jinga arquivo.pr -o nome   Gera um executavel que roda sozinho (sem o fonte
                             e sem a jinga instalada)
  jinga -e "<codigo>"        Roda codigo inline (uma linha)
  jinga build                Roda todos os .pr da pasta atual
  jinga --check [arq.pr]     So analisa (nao roda); JSON com o erro. Sem
                             arquivo, le da entrada padrao
  jinga --check --path <arq> Confere o fonte da entrada padrao COMO SE fosse
                             esse arquivo (acha os modulos vizinhos)
  jinga //doc                Mostra a URL da especificacao
  jinga --contexto L:C [arq] O que o cursor toca (pro editor); sem arquivo, stdin
  jinga --tokens [arq]       Tokens do lexer em JSON (pro realce); sem arquivo, stdin
  jinga --ast [arq]          A arvore do parser em JSON (pro editor); sem arquivo, stdin
  jinga --bytecode [arq]     O bytecode que o compilador gera, uma instrucao por linha
  jinga --sem-jit ...        Roda so no interpretador, sem gerar codigo de maquina
  jinga --utf16 ...          Com --tokens/--ast/--contexto/--check: colunas em UTF-16
  jinga --version / -V       Mostra a versao
  jinga --help / -h          Mostra esta ajuda

Pacotes (lib e comando .pr):
  jpkg install <arq.pr>          Instala (o arquivo decide via #!lib / #!cmd)
  jpkg install <arq.pr> -asLib   Forca lib importavel (import nome)
  jpkg install <nome>            Busca <nome> no registry configurado
  jpkg uninstall <nome>          Remove (acha sozinho: comando ou lib)
  jpkg uninstall <nome> -asLib   Forca a categoria lib
  jpkg list                      Lista comandos e libs instalados
  jpkg registry set-url <url>    Configura o indice de pacotes
  jpkg registry show             Mostra o registry configurado

`pool` e `psl` sao os nomes antigos dos mesmos comandos e continuam valendo.

Libs internas: json, date, regex, os, sys, dotenv, jwt, hash, bytes, sqlite3, mail,
  request, qrcode, manpu, psodbc, jinker, sockets, datasentity

Docs: https://github.com/kleberDevion/poolscript
```

O que o `main.c` aceita além do que o `--help` lista:

| Comando | O que faz |
|---|---|
| `jinga --debug <porta> <arquivo.pr> [args]` | depurador DAP (3.15); a porta tem que estar entre 1 e 65535 |
| `jinga --metadata` | modelo de tipos em JSON (3.17) |
| `jinga compile <arq.pr> -o <saida>` | mesmo efeito de `jinga arq.pr -o saida` |
| `jinga repl` | imprime que o REPL ainda não está no binário C e sai com 64 |
| `-v` | igual a `--version` / `-V` |
| `help` | igual a `--help` / `-h` |
| `check`, `tokens`, `ast`, `bytecode`, `contexto`, `metadata`, `debug` | as mesmas flags sem os dois traços |
| `install`, `uninstall`, `list`, `registry` direto no `jinga` | o mesmo código do `jpkg` (`ps_pkg.c`) |
| `--utf16` e `--sem-jit` | valem em qualquer posição e saem do argv antes do resto |

Variáveis de ambiente lidas pelo motor:

| Variável | Efeito |
|---|---|
| `JINGA_JIT=0` | desliga o JIT |
| `JINGA_JIT_LOG=1` | registra cada proto compilado |
| `JINGA_JIT_FALHA=1` | simula a recusa da memória executável |
| `JINGA_GC_LOG=1` | escreve no stderr cada coleta de lixo |
| `PS_HASH_SEED=<n>` | fixa a semente do hash de dict. O padrão é sorteada de `/dev/urandom` a cada processo |
| `PS_MAIL_TLS_INSEGURO=1` | desliga a verificação de certificado no mail (servidor de teste) |
| `JINGA_HOME` (ou `POOLSCRIPT_HOME`) | pasta de pacotes e libs; o padrão é `~/.jinga` |
| `QR_DUMP` | despeja os codewords do QR no stderr (diagnóstico) |
| `PATH`, `TMPDIR`, `HOME` | lidas pelos módulos `os` e `jpkg` |

---

### 3.17 Comandos de editor: `--check`, `--ast`, `--tokens`, `--contexto`, `--metadata`, `--bytecode`

Nenhum deles executa código do usuário. Todos leem do arquivo ou do stdin, e `--utf16` troca a contagem de colunas para UTF-16. Saídas reais de 2026-09-28:

**`--check [arq]`**:

- Passa pelo mesmo `ps_verifica_fonte` (lexer, parser, compilador e tipagem estática).
- `--check --path <arq>` lê o buffer do stdin como se fosse aquele arquivo, e assim os `import` vizinhos resolvem.
- Cada erro traz `l2`/`c2` (fim do trecho): no parser é o fim do token, no lexer é o grupo sem par (abridor + 1), na tipagem é o fim do nó.
- Os avisos do lexer também vão no JSON.
- rc é 0 sem erro e 1 com erro.

```
$ jinga --check ex.pr
{"ok":true}
$ jinga --check erro.pr
{"ok":false,"tipo":"AttributedValueError","msg":"variável x esperava int, recebeu str","linha":1,"coluna":9,"erros":[{"tipo":"AttributedValueError","msg":"variável x esperava int, recebeu str","linha":1,"coluna":9,"l2":1,"c2":12}]}
$ echo 'int y = 1' | jinga --check
{"ok":true}
```

**`--tokens [arq]`**: modo editor do lexer, com comentários. Os campos são `t` (tipo), `l`, `c`, `n` (tamanho), `l2`, `c2` e `v` (texto).

```
{"tokens":[{"t":"KW","l":1,"c":1,"n":3,"l2":1,"c2":4,"v":"int"},{"t":"IDENT","l":1,"c":5,"n":1,"l2":1,"c2":6,"v":"n"},...
```

**`--ast [arq]`**: a árvore em JSON, feita no modo de recuperação, então sai inteira mesmo com erro. Os campos são `k` (nó), `l`, `c`, `l2`, `c2`, `texto`, `texto2`, `a`/`b`/`c` (filhos) e `lista`.

```
{"ok":true,"arvore":{"k":"Program","l":1,"c":1,"lista":[{"k":"VarDecl","l":1,"c":5,"l2":1,"c2":10,"texto":"n","texto2":"int","a":{"k":"Literal",...
```

**`--contexto L:C [arq]`**: pergunta ao lexer o que o cursor toca. Os contextos são `topo`, `membro` (com `receptor` e `tipo`), `nenhum`, `decorador`, `import` e `erro`.

```
$ printf 'import os\nos.\n' > ctx.pr && jinga --contexto 2:4 ctx.pr
{"contexto":"membro","parcial":"","receptor":"os"}
```

**`--metadata`**: sai das próprias tabelas do motor (`MODULOS[]`, `METODOS_*`, `ps_tipos.def`, `retornos_medidos.inc`). As chaves de topo são `modulos`, `acesso`, `tipos`, `builtins`, `excecoes`, `tipos_nomes`, `tipos_valor`, `tipos_declaraveis`, `keywords`, `tipos_apelidos` e `modulos_sem_import` (`["Parsing"]`). Medido: 63.945 bytes, 256 entradas com `"kind"`. É o que o servidor LSP e `scripts/audita_doc.pr` consomem; `teste/confere_metadata.pr` confere no `make check`.

**`--bytecode [arq]`**: o desmontador (`ps_desmonta`), com uma linha por instrução no formato `ip  NOME  arg  ; significado` (exemplo em 3.6). O cabeçalho de cada proto mostra `nlocals`, `nparams`, `gerador` e `async`.

---

### 3.18 `-o`: executável que roda sozinho

`jinga programa.pr -o saida` (ou `jinga compile programa.pr -o saida`) gera uma **cópia deste mesmo binário** com o programa grudado no fim. Não há compilador de C no meio.

```
[ binário da jinga ][ payload ][ tamanho do payload: 16 dígitos ][ MAGIA: 10 bytes ]
```

- **Payload v2** (magia `PSPOOLEXE2`): leva o principal **e todo `.pr` que ele alcança por import**, transitivamente, inclusive libs instaladas. A resolução é feita na compilação exatamente como o import resolve rodando. Estrutura: `[16 dígitos: n entradas]`, e cada entrada é `[16][textual][16][real][16][fonte]`. A entrada 0 é o cabeçalho (caminho do principal, realpath, pasta de libs da máquina que compilou), a 1 é o principal e as seguintes são os módulos.
- **Payload v1** (`PSPOOLEXE1`, um fonte só): um executável gerado antes continua rodando.
- **Conferência antes de gerar**: o principal e cada módulo passam por `ps_verifica_fonte` (o mesmo `--check`). Módulo que não se acha ou não compila dá rc 65 e nada é gerado.
- **Na partida**: o binário lê o próprio arquivo (`ps_meu_executavel`). Se achar a magia, roda o embutido e ignora os subcomandos, e todos os argumentos vão para `sys.argv`. O `import` consulta a tabela de embutidos (`ps_emb_busca`) antes do disco, e o traceback lê a linha do fonte embutido.
- **Recompilar a partir de um executável já gerado** corta o payload anterior antes de grudar o novo.
- O arquivo nasce com permissão 0755.

Medido em 2026-09-28 (binário 16.1.9):

```
$ echo 'post("oi")' > ex2.pr && jinga ex2.pr -o ex2bin
gerado: ex2bin
$ ls -l ex2bin      # 9.066.683 bytes (o binário de 9.066.080 + o payload)
$ ./ex2bin
oi
```

O `-o` embute **fonte**, e o executável compila e roda na partida. O JIT atua do mesmo jeito que em `jinga arquivo.pr`.

---

### 3.19 Variantes de build do motor

| Macro ou alvo | Efeito |
|---|---|
| `-DPS_SEM_GOTO` | despacho por `switch` em vez de goto calculado (também desliga o JIT) |
| `-DPS_SEM_JIT` | binário sem código de máquina |
| `-DPS_DEBUG` (`make pool-debug`, `make check-debug`) | liga `PS_ASSERT`: a suíte inteira com as invariantes valendo, em `-O1 -g` |
| `make pool-asan` / `make check-asan` | ASan, UBSan e LeakSanitizer na suíte inteira (`detect_leaks=1`) |
| `make pool-oom` / `make oom` | `--wrap=malloc,calloc,realloc,strdup` no ligador; falha a i-ésima alocação de cada programa de `teste/oom_varre.pr`. Passar significa morrer limpo. **Roda na VPS**: neste laptop o alvo morre por falta de memória |
| `make pool-fuzz` / `make fuzz` | fuzzer do clang (`-fsanitize=fuzzer`) só no front-end (`--check`); os achados ficam em `teste/fuzz_achados/`, e o `make check` os re-executa |
| `make unidade` | teste de unidade em C dos módulos puros (pilha, hash, regex, ast, xlsx) |
| `./rebuild_vm.sh [--check] [--install]` | `make limpa` mais `make pool` (rebuild limpo, contra timestamp de granularidade de segundo), mais a suíte e a instalação opcionais |

---

### 3.20 Tabela arquivo -> papel (`vm/`, linhas por `wc -l` no commit `b180807`)

| Arquivo | Linhas | Papel |
|---|---:|---|
| `main.c` | 1742 | o executável `jinga`: CLI, `--check`/`--tokens`/`--ast`/`--contexto`/`--bytecode`/`--metadata`/`--debug`, `-e`, `build`, `-o` (payload, `ps_payload`, `emb_carrega`, `cmd_compila`), `reporta` (traceback e códigos de saída), subcomandos de pacote |
| `jinga_vm.c` | 31085 | a VM: valores, objetos, GC, laço de despacho, chamadas, exceções, geradores, closures, fibras, escalonador async, pool de threads, stdlib inteira (builtins, métodos de str/list/dict/bytes/PoolFile, módulos json, date, regex, os, sys, dotenv, jwt, hash, bytes, sqlite3, mail, request, qrcode, manpu, psodbc, jinker, sockets, datasentity), camada jinker, depurador (cola DAP), import e embutidos, `--metadata`, `ps_roda_fonte` |
| `ps_jit_x64.h` | 705 | JIT x86-64: codificador de instruções, gabaritos, `jit_chama`, heurística dos 15%, `JINGA_JIT*` |
| `ps_lexer.c` | 1597 | lexer: modos normal, editor, recuperação, fluxo e sonda; segunda passada; avisos; interpolações de f-string |
| `ps_lexer.h` | 222 | tipos de token, `PSToken`, `PSTokenList`, API do lexer |
| `ps_parser.c` | 4262 | parser por descida recursiva, modo de recuperação, parser incremental |
| `ps_parser.h` | 82 | `PSParseResult` e API do parser, incluindo `ps_parse_inc_*` |
| `ps_ast.c` | 180 | arena (blocos de 64 KB, alinhamento 16) e construtores de nó |
| `ps_ast.h` | 216 | tipos de nó (`N_*`) e estrutura `PSNode` |
| `ps_compiler.c` | 7327 | compilador AST -> bytecode em duas passadas, tipagem estática (`tp_*`), fusões, desmontador |
| `ps_compiler.h` | 346 | `PSConst`, `PSProto`, `PSPrograma`, `PSErroTipo`, `ps_compila_fonte`, `PS_MAX_PARAMS` |
| `ps_opcodes.def` | 263 | a lista única de opcodes: 115 opcodes, `PS_OP(NOME, NUMERO, "TEXTO")` |
| `ps_tipos.def` | 49 | tabela única de tipos (13) e apelidos (12) |
| `ps_tipos.h` | 73 | consulta à tabela de tipos para parser, compilador e `--metadata` |
| `ps_retornos.c` | 55 | busca binária no tipo de retorno dos nativos |
| `ps_retornos.h` | 52 | API de retornos, membros nativos, "Did you mean" (`ps_nativo_sugestao`) |
| `retornos_medidos.inc` | 584 | **gerado** por `scripts/mede_retornos.pr` (574 entradas); não se edita à mão |
| `ps_vm.h` | 159 | API pública da VM: `ps_roda_fonte`, `PSErroExec`, embutidos do `-o`, `ps_verifica_fonte`, `ps_metadata_json`, `ps_debug_porta` |
| `ps_so.h` | 223 | camada de sistema: API de eventos, aviso, memória, nomes, caminho, sockets (`ps_sock_*`) e raízes TLS (`ps_so_tls_cas`) |
| `ps_so.c` | 653 | implementações: Linux (epoll, eventfd, `/proc`), macOS (kqueue, pipe, `_NSGetExecutablePath`, Keychain) e Windows (WSAPoll, par de sockets no loopback, VirtualAlloc/VirtualProtect, `GetModuleFileNameA`, repositório `ROOT`) |
| `ps_win.h` | 126 | só no build do Windows: os nomes que o Windows chama diferente (`mkdir`, `realpath`, `setenv`, `strndup`, `memmem`, `strcasestr`, `sysconf`, `gmtime_r`/`localtime_r`) |
| `ps_dl.h` | 99 | carga preguiçosa de biblioteca: `PS_DL_PONTEIRO`, `PS_DL_RESOLVE`, `PS_DL_CARREGA`, `ps_dl_abre` (`dlopen` ou, no Windows, `LoadLibraryA`) |
| `ps_db.c` | 704 | fachada SQL: sqlite, Postgres, MySQL/MariaDB e ODBC no mesmo molde, varredor de placeholders, listas de `dlopen` (e de DLL no Windows) |
| `ps_db.h` | 67 | `PSDbDriver`, `PSCel`, `PSDbRes`, API de conexão |
| `ps_pgerr.h` | 268 | SQLSTATE -> nome da exceção do Postgres (252 entradas, filhas de `DatabaseError`) |
| `ps_mongo.c` | 288 | Mongo via libmongoc/libbson (por `dlopen`), com ponte JSON |
| `ps_mongo.h` | 39 | API do Mongo (find, insert, update, remove, count) |
| `ps_jinker.c` | 1053 | transporte do jinker: HTTP/1.1, WebSocket (servidor e cliente), multipart, MIME, TLS, arena por requisição |
| `ps_jinker.h` | 144 | API do transporte (`PSJkConn`, `PSJkReq`, códigos `PSJK_*`) |
| `ps_http.c` | 489 | cliente HTTP/HTTPS sem libcurl |
| `ps_http.h` | 60 | `PSHttpResp` e `ps_http_request` |
| `ps_mail.c` | 1030 | SMTP, IMAP e MIME sobre socket mais OpenSSL |
| `ps_mail.h` | 61 | API de mail (o comentário sobre TLS está desatualizado; ver 3.14) |
| `ps_hash.c` | 468 | SHA-2, HMAC, PBKDF2 e base64 |
| `ps_hash.h` | 57 | API de hash |
| `ps_regex.c` | 1064 | motor de regex por backtracking |
| `ps_regex.h` | 61 | API e escopo suportado do regex |
| `ps_qr.c` | 589 | gerador de QR Code e PNG |
| `ps_qr.h` | 32 | API do QR |
| `ps_qr_tables.h` | 3620 | tabelas do padrão QR, geradas por script |
| `ps_xlsx.c` | 533 | leitura e escrita de xlsx (ZIP sobre zlib, mais expat) |
| `ps_xlsx.h` | 42 | `PSGrade` e API do xlsx |
| `ps_pkg.c` | 652 | gerenciador de pacotes (`jpkg`) |
| `ps_pkg.h` | 26 | API do `jpkg` (`PS_PKG_AUTO`/`CMD`/`LIB`) |
| `ps_debug.c` | 97 | enquadramento do DAP (leitura byte a byte, escrita com retomada) |
| `ps_debug.h` | 30 | API do transporte DAP |
| `ps_pilha.c` | 120 | medição de folga da pilha do C (por thread; margem de 1/8, entre 16 KB e 512 KB, e nunca menos que dois degraus reais medidos) |
| `ps_pilha.h` | 50 | API de folga: `ps_pilha_marca`, `ps_pilha_apertada`, leitura e reposição para troca de fibra |
| `ps_assert.h` | 88 | `PS_ASSERT`/`PS_ASSERT_MSG`, só com `-DPS_DEBUG` |
| `ps_ext.h` | 57 | a extensão da linguagem (`.pr`); `.ps`, `.psl` e `.p` só são reconhecidas para dizer o conserto |
| `ps_gmp_min.h` | 97 | interface mínima da GMP declarada à mão |
| `ps_versao.h` | 18 | `PS_VERSAO` (fonte única; regra base 100: `8.2.99 -> 8.3.0`) |
| `ps_build.h` | 2 | **gerado** pelo Makefile, fora do git: `PS_BUILD_DATA` (data da compilação, mostrada no `--version`) |

A árvore de trabalho tem, além disso, mudança sem commit em vários arquivos de `vm/` (o Windows em andamento, lista e horário em 9.10); os números acima são os do commit.

## 4. Editor

### 4.1 Como o suporte a editor funciona

Um servidor de linguagem só atende os três editores (VS Code, IntelliJ, Neovim) e qualquer outro cliente LSP. Ele é o `jinga-lsp`, um wrapper de shell que executa `node <prefixo>/share/jinga/lsp/server.js --stdio`.

| peça | onde fica | papel |
|---|---|---|
| `editor/vscode/server.js` | repositório; instalado em `/usr/local/share/jinga/lsp/` | o protocolo LSP |
| `editor/vscode/analise.js` | idem (o `server.js` faz `require('./analise.js')`) | a análise |
| dependências npm do servidor | `editor/vscode/node_modules` (não versionado; vem de `npm install`) | copiadas pelo `make install` pra `share/jinga/lsp/node_modules` |
| `jinga-lsp` / `poolscript-lsp` | `/usr/local/bin` | o atalho que os editores chamam (o segundo é o nome antigo) |
| `extension.js` | só na vsix | cliente do VS Code, que sobe o servidor |

De onde vem cada resposta. Nada fica escrito à mão no servidor:

| camada | fonte |
|---|---|
| estrutura (escopo, Entity, import) | `jinga --ast`, a árvore do parser |
| módulos, tipos, métodos | `jinga --metadata`, lido das tabelas da VM |
| string, comentário, contagem de parêntese | `jinga --tokens`, o lexer |
| diagnóstico | `jinga --check` (com `--path` pro buffer não salvo) |
| prosa de hover e completion | `docs/<escopo>/<nome>/<nome>.md` |

Regras dele que valem pro servidor:
- **Nenhuma expressão regular nos `.js`.** Regex ali vira uma segunda gramática, e esse era o defeito.
- **O editor nunca copia regra do motor.** Quando o servidor precisa de uma lista, o certo é criar o campo no C, publicar no `--metadata` e o servidor ler de lá. Exemplo: a tabela `MODULOS_SEM_IMPORT`.
- **Mexeu na doc ou no LSP:** roda o teste com o `./pool` do repositório e instala. Recarregar a janela não basta, é preciso derrubar o processo antigo do servidor (4.2).

**Onde o servidor acha a doc** (`raizDoc()` em `server.js`), na ordem:
1. `dirname(realpath(jinga))/../share/jinga/docs`
2. `.../share/poolscript/docs`
3. `<base>/docs`
4. `docs` do diretório atual

Com o `jinga` instalado em `/usr/local/bin`, a doc lida é `/usr/local/share/jinga/docs`. Com `./pool`, é a `docs/` do repositório.

Comandos do binário que existem pra servir o editor (tirados de `docs/lsp.md` e do `jinga --help`):

| comando | devolve |
|---|---|
| `jinga --metadata` | módulos, tipos e métodos, das tabelas da VM |
| `jinga --tokens [arq]` | `{"tokens":[...],"erros":[...]}` com posição, tamanho, `l2`/`c2` |
| `jinga --ast [arq]` | a árvore do parser em JSON, com `erros` (recupera e segue) |
| `jinga --contexto L:C [arq]` | o que o cursor toca (`topo`, `membro`, `import`, `texto`, `comentario`...) |
| `jinga --check [arq]` | todos os erros em JSON, com `linha`/`coluna`/`l2`/`c2` |
| `jinga --check --path <arq>` | o mesmo pra um buffer da entrada padrão que vale como aquele arquivo |
| `jinga --bytecode [arq]` | o bytecode, uma instrução por linha |
| `--utf16` | junto dos quatro primeiros: colunas na contagem UTF-16 do protocolo |
| `jinga --debug <porta> arq.pr` | o motor falando DAP (usado pelo depurador da extensão) |

Sem arquivo, esses comandos leem o fonte da entrada padrão.

### 4.2 Instalar e atualizar o servidor

```bash
cd ~/poolscript-lang
(cd editor/vscode && npm install --omit=dev --no-audit --no-fund)   # num clone novo: sem isto o install copia o servidor sem dependências
make pool
sudo make install
```

O `make install` põe em `PREFIXO=/usr/local`:
- `bin/jinga`, `bin/jpkg`, `bin/pool` e `bin/psl` (o mesmo binário);
- `bin/jinga-lsp` e `bin/poolscript-lsp`;
- `share/jinga/lsp/` (`server.js`, `analise.js` e os `node_modules` do servidor);
- `share/jinga/docs/` (todas as páginas `.md`).

No Linux ele também tenta o tipo MIME. Sem root, cai no `install-icone`. No macOS esse passo é pulado (`ifneq ($(SO),Darwin)`).

O servidor precisa do `node` no PATH, e o wrapper sai com erro se não houver. Nesta máquina o node é o v18.19.1. O servidor roda nele, mas empacotar a vsix exige 22 ou mais (4.3).

**Depois de todo `make install`, derrubar o servidor que está rodando.** O editor segue com o processo antigo, e ele vê o comportamento velho. O cliente sobe o novo em segundos:

```bash
for p in $(pgrep -f "^node /usr/local/share/jinga/lsp/server.js"); do kill $p; done
```

Nunca use `pkill -f` com o padrão escrito no próprio comando: ele mata o shell que o roda.

**Testar o servidor:**

```bash
node editor/vscode/teste_servidor.js ./pool
```

Com `./pool`, mede a doc do repositório. Com `pool` ou `jinga` do PATH, mede a cópia instalada, e uma doc instalada velha faz hover "falhar" sem defeito no código. Pra provar a cópia instalada: primeiro `sudo make install` (ou `sudo ./instalar.sh`), depois o teste com `jinga`. O `make check` roda este teste e PULA, dizendo que pulou, sem `node` ou sem `editor/vscode/node_modules`.

### 4.3 VS Code

Extensão **Jinga**:

| item | valor |
|---|---|
| fonte | `editor/vscode/` |
| id instalado | `klebersantanadeoliveira.jinga-8.6.8` (em `~/.vscode/extensions/`) |
| manifesto | `name` `jinga`, `publisher` `KleberSantanadeOliveira`, `version` 8.6.8 |
| versão | própria, separada da linguagem (4.3 e 5.15) |
| linguagem | id `jinga`, extensão `.pr`, gramática `source.jinga` |
| ativação | `onLanguage:jinga`, `onDebugResolve:jinga` |

Comandos da extensão:
- `jinga.rodar` (Rodar arquivo), no atalho `ctrl+f5`. No macOS o VS Code lê `ctrl` como a tecla Control mesmo.
- `jinga.depurar` (Depurar arquivo).
- `jinga.grafico` (Gráfico de execução).

Depurador: tipo `jinga`, com os campos `programa`, `pararNaEntrada`, `args` e `cwd`. A extensão chama o motor com `--debug <porta>`, e o teste `teste/depurador.pr` cobre esse caminho.

Temas embutidos: "Jinga One Dark" e "Jinga C# Dark". O tema que ele usa no dia a dia é o **Ariake Dark** (`wart.ariake-dark-0.2.2`, do marketplace).

Configurações:
- `jinga.jinga`: caminho do binário (vazio = o do PATH). Com `"/caminho/do/repo/pool"`, o servidor roda direto do repositório, sem instalar.
- `jinga.lsp.ativo`: `false` desliga o servidor e deixa só o realce.
- Por padrão, pra `[jinga]`: `editor.wordBasedSuggestions: off`, `editor.suggest.showWords: false`, tab 4 com espaços.

**Qual servidor sobe** (`jingaLsp()` em `extension.js`): o primeiro `jinga-lsp` executável, e em cada pasta também o `poolscript-lsp`, procurando nesta ordem:
1. o PATH do editor;
2. `/usr/local/bin` e `/usr/bin`;
3. `~/.local/bin` e `~/bin`;
4. `~/.jinga/bin` e `~/.poolscript/bin`.

Só sem nenhum deles a extensão usa o `server.js` embutido na vsix, que é cópia de reserva e envelhece. Um VS Code aberto pelo menu não herda o PATH do shell, por isso a lista fixa. A primeira linha do painel **Saída -> Jinga** diz qual subiu: `servidor: /usr/local/bin/jinga-lsp` ou `servidor: embutido (...)`.

**Empacotar e instalar a vsix:**

```bash
make vsix            # gera editor/vscode/jinga-<versao>.vsix; exige Node >= 22
make instala-vsix    # gera e roda: code --install-extension <vsix> --force
```

- O alvo `vsix` confere a versão do node antes e sai 1 abaixo da 22. O empacotador que o `npx` baixa não roda mais no 18. Nesta máquina (node v18.19.1) ele recusa. Pra apontar outro node: `PATH=/caminho/do/node22/bin:$PATH make vsix`.
- A vsix só precisa ser refeita quando muda o cliente (`extension.js`), a gramática ou os temas. O servidor vem do `make install`.
- Depois de trocar a extensão: `Ctrl+Shift+P` (no Mac `Cmd+Shift+P`) -> *Developer: Reload Window*.
- O `make instala-vsix` usa o comando `code`. No Mac ele entra no PATH pela paleta do VS Code, em *Shell Command: Install 'code' command in PATH*.
- Hoje não existe nenhum `.vsix` gerado no repositório. Pra levar a extensão pro Mac sem Node 22, copiar a pasta instalada `~/.vscode/extensions/klebersantanadeoliveira.jinga-8.6.8` pra mesma pasta no Mac funciona. O cliente é JavaScript puro.

Coisas da máquina atual que não estão no repositório:
- A extensão **JinkerTls** (`klebersantanadeoliveira.jinkertls-1.0.1`, "JinkerTls Language Support") não tem fonte neste repositório. A cópia instalada está em `~/.vscode/extensions/klebersantanadeoliveira.jinkertls-1.0.1`.
- O `~/.config/Code/User/settings.json` ainda tem um bloco `"[poolscript]"` do nome antigo da linguagem. Ele não vale pra nada: o id é `jinga`, e a extensão já traz os padrões certos.

### 4.4 Neovim

`vim` e `vi` desta máquina abrem o Neovim (v0.9.5), via `/etc/alternatives`. A config que vale é `~/.config/nvim/init.lua`. Um `~/.vimrc` não é lido por nada, então não se cria. Os swap files ficam em `~/.local/state/nvim/swap/`.

```bash
make nvim     # instala em ~/.config/nvim (NVIM_CFG=... troca o destino)
```

O alvo:
- instala `editor/nvim/colors/ariake-dark.lua` e `editor/nvim/syntax/jinga.vim`;
- remove o `syntax/poolscript.vim` antigo;
- se o `init.lua` da máquina for diferente do `editor/nvim/jinga.lua`, guarda uma cópia `init.lua.bak-<AAAAMMDD-HHMMSS>` e instala o novo. Nesta máquina já existem `init.lua.bak-20260901`, `init.lua.bak-20260917-223757` e `init.lua.bak-antes-do-conserto`.

O que a config entrega (modelo de edição pedido por ele em 2026-08-28, porque os comandos de uma tecla do modo normal apagavam coisa sem avisar):
- fundo transparente, e o arquivo abre em inserção;
- `Ctrl+S` salva, `Ctrl+Z` sai e volta da edição, `Esc` não faz nada;
- `()`, `[]` e `{}` fecham sozinhos; `)` anda por cima do fechamento; `<BS>` num par vazio apaga os dois;
- o menu de sugestão abre sozinho depois de `.` ou de duas letras, e `<C-Space>` força; `completeopt` `noselect,noinsert`;
- `K` é hover, `gd` vai pra definição, `[d` e `]d` navegam o diagnóstico;
- `.pr` declarado como filetype `jinga`. O Neovim traz `.pr` como SDL de fábrica.

**Servidor no Neovim** (`jinga.lua`): usa `jinga-lsp` se for executável, senão `poolscript-lsp`, senão `node ~/poolscript-lang/editor/vscode/server.js --stdio`. Por isso o clone no Mac vai em `~/poolscript-lang`. Sem nenhum dos três, avisa pra rodar `sudo make install`.

**Teste:** `editor/nvim/teste_nvim.sh [caminho-do-jinga]`. O padrão é o `jinga` do PATH. Ele sobe um Neovim de verdade com os arquivos do repositório e confere filetype, syntax, pares, anexo do LSP e completion. PULA sem `nvim`, e entra no `make check`. Pra testar teclas por pty, use `script -qec` e mande as teclas depois do editor subir. Tecla entregue antes é engolida e dá falso negativo.

No Mac o `/usr/bin/vim` é o do sistema. Pra `vim` abrir o Neovim, o caminho é um link em `/usr/local/bin`, que vem antes de `/usr/bin` no PATH padrão do macOS: `brew install neovim`, depois `ln -s "$(brew --prefix)/bin/nvim" /usr/local/bin/vim`. Numa Homebrew em `/usr/local`, que é o caso do Mac Intel, o `nvim` já está lá.

### 4.5 IntelliJ (JetBrains)

Nesta máquina:
- IntelliJ IDEA Ultimate 2026.2 em `/opt/idea-IU-262.8665.337`;
- plugins em `~/.local/share/JetBrains/IntelliJIdea2026.2/`, entre eles `jinga-icons` e `lsp4ij`;
- JDK: `javac 21.0.12` em `/usr/bin/javac`.

O plugin fica em `editor/intellij/`:

| peça | caminho | o que é |
|---|---|---|
| fonte Java | `editor/intellij/plugin/src/jinga/icons/*`, `.../lsp/*` | ícone do `.pr`, indentação no Enter, auto-fechamento com type-over, fábrica do servidor |
| stubs | `editor/intellij/plugin/stubs/` | só as assinaturas usadas; saem do jar (o `build.sh` reprova se vazar `com/` ou `org/`) |
| descritor | `plugin/resources/META-INF/plugin.xml` | id `jinga.icons`, nome "Jinga IDEA", versão 1.2.0, `since-build 231` (IDEA 2023.1, Java 17) |
| LSP | `plugin/resources/META-INF/jinga-lsp4ij.xml` | declara o servidor `jinga` (padrão `*.pr`, languageId `jinga`) |
| realce | `editor/intellij/bundle/Jinga.tmbundle` | cópia da gramática da vsix; vai dentro do jar e é registrada no boot |

```bash
make intellij
```

O alvo faz, na ordem:
1. `bundle/atualiza.sh` copia `editor/vscode/syntaxes/jinga.tmLanguage.json` pro bundle;
2. `plugin/build.sh` compila com `javac --release 17` e gera `editor/intellij/plugin/dist/jinga-icons.jar` (`JAVAC=<caminho>` e `ALVO=<versão>` trocam o compilador e o alvo);
3. instala em `$(IJ_HOME)/IntelliJIdea*` ou `IdeaIC*`, na subpasta `jinga-icons/lib/`, com `IJ_HOME` padrão `~/.local/share/JetBrains`;
4. roda `plugin/teste_intellij.sh`.

Depois é preciso **reiniciar o IDEA**, porque o plugin só carrega no boot.

Requisitos no IDEA: o plugin **LSP4IJ** (marketplace), o plugin TextMate (vem com o IDEA) e o `node` na máquina. O plugin procura o `jinga-lsp` em caminho absoluto, com a mesma lista de pastas do VS Code, e passa o caminho do `jinga` em `initializationOptions`. Um servidor "Jinga" (ou "PoolScript") cadastrado à mão em *Settings -> Languages & Frameworks -> Language Servers* tem que ser apagado: sobem dois servidores, e o cadastro antigo mapeava `*.ps`. O plugin avisa num balão.

Pra ver se o servidor respondeu: *View -> Tool Windows -> Language Servers -> Jinga -> Traces*, com *Trace: verbose* na aba *Debug*.

**No Mac** o `IJ_HOME` padrão é caminho do Linux. O alvo não acha instalação, imprime o caminho do jar e manda instalar pelo menu. O caminho no Mac é:

```bash
editor/intellij/plugin/build.sh      # precisa de um JDK (javac)
# IDEA: Settings > Plugins > engrenagem > Install Plugin from Disk > editor/intellij/plugin/dist/jinga-icons.jar
```

O `teste_intellij.sh` procura o IDEA em `$IDEA_HOME` e, sem ele, onde cada sistema o instala: no Linux `/opt/idea*` ou o Toolbox; no macOS o `.app` (`/Applications`, `~/Applications` ou o Toolbox antigo), com o java em `jbr/Contents/Home/bin`; no Windows (MSYS2) `C:\Program Files\JetBrains` ou o Toolbox em `%LOCALAPPDATA%`. O LSP4IJ vem da pasta de plugins do sistema (`~/.local/share/JetBrains/*/`, `~/Library/Application Support/JetBrains/*/plugins/`, `%APPDATA%\JetBrains\*\plugins\`) ou de `LSP4IJ_LIB`. Fora disso ele PULA e diz quantas checagens não rodaram. No Windows o `jinga-lsp` esperado segue a regra do próprio sistema (a ordem da `PATHEXT`, e a pasta que o `instalar.ps1` põe no PATH do usuário).

### 4.6 Helix

Em `~/.config/helix/languages.toml`:

```toml
[language-server.jinga]
command = "node"
args = ["/caminho/do/repo/editor/vscode/server.js"]

[[language]]
name = "jinga"
file-types = ["pr"]
language-servers = ["jinga"]
```

### 4.7 Realce, tema e logo

- **Realce.** As listas de tipos, builtins e exceções da gramática (`editor/vscode/syntaxes/jinga.tmLanguage.json`) e do syntax do Neovim (`editor/nvim/syntax/jinga.vim`) são geradas do `jinga --metadata`:
  - `./pool scripts/gera_realce.pr` reescreve os dois arquivos;
  - `./pool scripts/gera_realce.pr --portao` reprova se divergirem, e entra no `make check`;
  - o mesmo portão reprova escopo da gramática sem cor nos temas.
- **Logo** (o "J"). Sai de uma geometria só, desenhada em Jinga:

  ```bash
  make logo        # ./pool scripts/gera_logo.pr
  ```

  Arquivos gerados: `dados/icones/text-jinga.svg`, `editor/vscode/images/icon.png` (256 px), `editor/vscode/images/arquivo.png` (128 px) e `editor/intellij/plugin/resources/icons/jinga_pr.png` (16 px, e `@2x` de 32 px).
- **Tipo MIME do desktop** (Linux): `sudo make install-mime` registra `text/jinga` (`dados/zz-jinga.xml`, `text-jinga.svg`). `make install-icone` troca só a logo em `~/.local/share/icons`, sem root. No macOS esses alvos não se aplicam.

### 4.8 Editor no Mac, em ordem

1. Node 22 ou mais (instalador oficial, pacote macOS x64, ou `brew install node`).
2. No repositório: `(cd editor/vscode && npm install --omit=dev --no-audit --no-fund)`, depois `make pool` e `sudo make install` (ou o instalador, ver 5.14).
3. VS Code: `make instala-vsix` (com o comando `code` no PATH), ou a cópia da pasta da extensão. Depois instalar o tema Ariake Dark pelo marketplace e trazer a JinkerTls (4.3).
4. Neovim: `brew install neovim`, depois `make nvim`.
5. IntelliJ: instalar o LSP4IJ pelo marketplace, rodar `editor/intellij/plugin/build.sh` e usar *Install Plugin from Disk* com o jar.
6. Conferir: `node editor/vscode/teste_servidor.js ./pool` e `editor/nvim/teste_nvim.sh`.

## 5. Build, testes, portões, VPS, CI, release e versão

### 5.1 Estado de agora (medido em 2026-09-28)

| item | valor |
|---|---|
| versão da linguagem | **16.1.9** (`vm/ps_versao.h`, bump no commit `3c245e7`); `main` e `origin/main` em `b180807` |
| `./pool --version` | `Jinga 16.1.9 [PSVM] (2026-09-28) Runtime standalone` |
| versão da extensão do VS Code | 8.6.8 |
| binário `./pool` | 9.066.080 bytes |
| esta máquina | Linux Mint 22.3, Intel i3-1115G4, 4 CPUs, 7,5 GiB de RAM, glibc 2.39, `perf_event_paranoid` = 4 |
| `gh` desta máquina | 2.101.0, com o token salvo **inválido** ("The token in keyring is invalid"); no Mac é preciso `gh auth login` |
| repositório | `origin` = `https://github.com/kleberDevion/poolscript-lang`, que redireciona pro canônico `https://github.com/kleberDevion/poolscript`; em URL nova, escrever o canônico |
| identidade do commit | `~/.gitconfig` existe desde 2026-09-27 (73 bytes) com `kleberDevion <klebersantanadeoliveira07@gmail.com>`; sem ele, `git -c user.name=kleberDevion -c user.email=klebersantanadeoliveira07@gmail.com commit ...`. Sempre com o trailer `Co-Authored-By: Regressão <noreply@anthropic.com>` |

O Makefile, os fontes e o binário ficam todos na raiz do repositório: `make <alvo>` de lá, sem `-f`.

### 5.2 Dependências de compilação

**Linux (Debian/Ubuntu)**, a lista do `instalar.sh`:

```bash
sudo apt-get install -y build-essential git pkg-config nodejs npm \
  libsqlite3-dev libpq-dev unixodbc-dev libssl-dev libpng-dev \
  libexpat1-dev zlib1g-dev libmariadb-dev libmongoc-dev libbson-dev libgmp-dev
```

Pra `make check` completo também entram: `clang` (fuzz), `lcov` (cobertura), `valgrind` (memcheck), `nvim` e `node`.

**macOS Intel** (Homebrew em `/usr/local`). A lista do `scripts/compila.pr` e do `build.yml`:

```bash
xcode-select --install
brew install openssl@3 libpng gmp libpq mariadb-connector-c unixodbc mongo-c-driver@1 coreutils
brew install gcc        # o analisa e o avisos do make check exigem o gcc de verdade; sem ele os dois saem 1 e o check reprova
brew install lcov gh neovim node
```

- O `build.yml` e o `compila.pr` registram que o Homebrew não tem mais pacote pronto pro Mac Intel, então esses pacotes compilam do fonte e a primeira vez demora.
- A anotação do CI de 2026-09-28 avisa: `mongo-c-driver@1` está marcado como descontinuado no Homebrew e será desligado em 2027-04-01.
- `coreutils` dá o `gtimeout`, que o Makefile acha sozinho (`TIMEOUT := timeout ou gtimeout`). Os drivers `.pr` chamam `timeout` direto (5.17), então no Mac entra no PATH o `gnubin`:

  ```bash
  export PATH="$(brew --prefix coreutils)/libexec/gnubin:$PATH"
  ```

Como o Makefile muda por sistema (`SO := $(shell uname -s)`):
- **Linux:** gcc, com `-Wduplicated-branches` e `--param=max-goto-duplication-insns=64` (sem esse parâmetro o laço da VM fica 16% mais lento em instruções, medido com perf stat). Sqlite, TLS, PNG, expat e zlib entram estáticos (`-Wl,-Bstatic`).
- **Darwin:** clang da Apple, sem os dois parâmetros acima. TLS e PNG entram pelos `.a` do Homebrew; sqlite, expat e zlib vêm do macOS; desde `3c245e7` liga também os frameworks Security e CoreFoundation.
- **Os dois:** os clientes de banco (Postgres, MySQL, ODBC, Mongo) não são ligados. Cada driver abre a biblioteca no primeiro `connect` (`vm/ps_dl.h`), e os cabeçalhos entram só pra compilar.

### 5.3 Compilar

```bash
make pool                 # o binário ./pool (o alvo continua se chamando pool)
./pool --version
./rebuild_vm.sh           # rebuild LIMPO: make limpa + make pool
./rebuild_vm.sh --check   # rebuild + make check
./rebuild_vm.sh --install # rebuild + make check + sudo make install
```

- O `pool` depende dos `.c`, de todos os `vm/*.h`, de `vm/ps_versao.h`, `vm/ps_build.h`, `vm/retornos_medidos.inc` e do próprio Makefile. Um bump ou mudança de flag força o relink.
- `vm/ps_build.h` é gerado a cada `make` com a data da compilação, e só é trocado quando muda. Ele fica fora do git. É o que faz o `--version` distinguir dois binários da mesma versão.
- `vm/retornos_medidos.inc` é gerado por `scripts/mede_retornos.pr` e fica versionado.
- `vm/ps_opcodes.def` é a fonte única dos opcodes. O `scripts/audita_c.pr` reprova opcode numerado fora dele.
- Todo `.pr` novo ou editado passa por `./pool --check arquivo.pr` antes de salvar.

### 5.4 Todos os alvos do Makefile

| alvo | o que faz |
|---|---|
| `pool` | compila o binário standalone `./pool` |
| `vm/ps_build.h` | gera o cabeçalho com a data da compilação (roda sozinho a cada `make`) |
| `bundle` | `make pool` + `./build_bundle.sh`: o pacote portátil em `dist/` |
| `editor/vscode/node_modules` | `npm install --omit=dev --no-audit --no-fund` na pasta da extensão |
| `logo` | `./pool scripts/gera_logo.pr`: SVG e PNG da logo |
| `vsix` | empacota `editor/vscode/jinga-<versao>.vsix` (Node >= 22) |
| `instala-vsix` | `vsix` + `code --install-extension ... --force` |
| `nvim` | instala tema, syntax e `init.lua` em `~/.config/nvim` (backup datado se diferia) |
| `intellij` | sincroniza a gramática, compila o jar, instala no IDEA e roda o teste do plugin |
| `install` | instala `jinga`, `jpkg`, `pool`, `psl`, `jinga-lsp`, `poolscript-lsp`, o servidor e a doc em `PREFIXO` (padrão `/usr/local`) |
| `install-mime` | tipo MIME `text/jinga` + ícone em `DADOS` (padrão `/usr/share`, pede root) |
| `install-icone` | só a logo em `~/.local/share/icons` (sem root) |
| `desinstala` | remove binários, `share/jinga`, `share/poolscript` e o MIME |
| `verifica` | `ldd ./pool`, símbolos de Python (deve ser vazio) e tamanho; usa `ldd`/`nm -uD`, que são do Linux |
| `limpa` | apaga `./pool` |
| `testar` | compila o runner `./testar` (`teste/ps_teste.c` + `teste/casos_*.c`) |
| `pool-asan` | binário com `-fsanitize=address,undefined` |
| `analisa` | `gcc -fanalyzer` arquivo por arquivo, com `ulimit -v` de `ANALISA_MB` (padrão 2000); `vm/jinga_vm.c` fica fora salvo com `ANALISA_TUDO=1`; os falsos positivos conferidos ficam em `ANALISA_ACEITOS` (o `nb` de `ps_xlsx.c`, o `strdup` de `ps_db.c` e, no Windows, o descritor do `ps_w_socket` em `ps_win.h`); onde o sistema recusa o teto (o macOS recusa o `ulimit -v`) a saída diz e mostra o limite que vale |
| `memcheck` | `valgrind --leak-check=full ... ./pool $(PS)` num script só (`make memcheck PS=teste/e2e/ws.pr`) |
| `bench` | `nice -n 19 ./pool teste/bench.pr $(BENCH)`; `BENCH=--grava` adota a referência, `BENCH=--portao` reprova se piorou |
| `avisos` | recompila com `-Wlogical-op -Wshadow -Wformat-truncation=$(AVISOS_NIVEL)` (padrão 1) e reprova se houver aviso — e também se algum fonte não compilar com o gcc usado (`NAO COMPILOU`), que antes passava como "nenhum aviso" |
| `check` | o portão inteiro (5.6) |
| `check-e2e` | `./pool teste/e2e_roda.pr $(E2E)`: todos os e2e, inclusive os que precisam de serviço |
| `check-e2e-local` | os e2e sem serviço externo (`arquivo sqlite socket pkg mail_local jinker manpu qrcode c.pr d.pr f.pr` — o driver filtra por pedaço do nome, e `c` sozinho pegava também o par jinker, o socket e o qrcode), cada um com `E2E_TIMEOUT` (padrão 300 s, acima do pior caso do par jinker dentro do driver), dizendo quais caíram; filtro que não casa nenhum script sai 2 (um script renomeado não vira "e2e: ok" calado) |
| `unidade` | compila e roda `teste/unidade.c` (os módulos puros chamados direto em C) a CADA chamada (`.PHONY`): o binário que sobrava de uma rodada que falhou dava "is up to date" e o `make check` passava sem rodar nada; no Windows liga também `-lbcrypt` e sai `unidade.exe` |
| `pool-oom` | binário com `--wrap=malloc,calloc,realloc,strdup` (`teste/ps_oom.c`) |
| `oom` | `./pool teste/oom_varre.pr`: falha a i-ésima alocação de cada programa |
| `pool-fuzz` | binário do fuzzer do clang (`-fsanitize=fuzzer,address,undefined`) sobre o front-end; o alvo traz os caminhos de cabeçalho do Linux |
| `semeia` | `./pool teste/fuzz_semeia.pr`: semeia `teste/fuzz_corpus` |
| `fuzz` | fuzza por `FUZZ_T` segundos (padrão 120); achados vão pra `teste/fuzz_achados/` (versionado) |
| `propriedade` | `./pool teste/propriedade.pr $(PROP_N) $(PROP_SEMENTE)` (padrão 200 e 1) |
| `leis` | `nice -n 19 ./pool teste/leis.pr $(LEIS_N) $(LEIS_SEMENTE)` (padrão 300 e 1) |
| `check-asan` | `PS_POOL=pool-asan ASAN_OPTIONS=detect_leaks=1:abort_on_error=0 ./testar` |
| `pool-debug` | binário `-O1 -g -DPS_DEBUG` (invariantes da VM ligadas) |
| `check-debug` | `PS_POOL=pool-debug ./testar` |
| `cobertura` | compila `cob/pool` com `--coverage -O0`, roda suíte, unidade, drivers e e2e (inclusive `db mongo mail ws` quando os serviços existem), gera `cob/html/index.html` e termina no portão `teste/cobertura_portao.pr`; a saída de cada `lcov` fica em `cob/lcov_*.txt`, e o fim dela aparece quando ele falha (antes ia pra `/dev/null` e o CI só mostrava "Error 1") |

Variáveis que se passam na linha de comando: `CC`, `CFLAGS`, `PREFIXO`, `DADOS`, `DADOS_USUARIO`, `NVIM_CFG`, `IJ_HOME`, `ANALISA_MB`, `ANALISA_TUDO`, `PS`, `BENCH`, `AVISOS_NIVEL`, `E2E`, `E2E_TIMEOUT`, `FUZZ_T`, `PROP_N`, `PROP_SEMENTE`, `LEIS_N`, `LEIS_SEMENTE`.

### 5.5 A suíte em C: `./testar`

```bash
make testar
./testar                 # tudo
./testar <grupo>         # só um grupo (o filtro casa o nome do grupo OU o nome do caso)
./testar -v              # mostra também os que passaram
JINGA_JIT=0 ./testar     # a mesma suíte sem o código de máquina (o interpretador é a referência)
PS_POOL=pool-asan ./testar   # troca o binário testado (o ./pool também precisa existir)
```

- O runner roda o `./pool` num subprocesso por caso, num diretório próprio, com 20 s por caso. Travou é FALHA.
- A VM morrer por sinal vira `a VM MORREU com sinal N (<nome>)`. Desde 16.1.8 o nome vem do `strsignal`: o 10 é SIGUSR1 no Linux e SIGBUS no macOS.
- Toda falha é gravada também em `teste/ultima_falha.txt` (fora do git), com fonte, rc e saídas.
- Nenhum caso casar com o filtro sai 2 ("isso é falha, não sucesso").
- Um caso de `pendentes` que passa sai como `JA FUNCIONA`, que é falha: ele tem que ser movido pro grupo de regressão.
- Desde `3c245e7` um caso pode valer só num sistema (campo `so` do caso em `teste/ps_teste.h`). Nos outros sistemas o `testar` lista quantos e quais não rodaram, e eles nunca contam como passou.

Os grupos, e quantos casos o `./testar <grupo>` rodou nesta máquina em 2026-09-28. O número pode incluir caso de outro grupo cujo nome contém a palavra:

| grupo | arquivo | o que é | casos |
|---|---|---|---|
| `crash` | `teste/casos_crash.c` | o que matava a VM (segfault, SIGFPE, OOM) | 36 |
| `inteiros` | `teste/casos_inteiros.c` | inteiro grande (bignum) | 31 |
| `erros` | `teste/casos_erros.c` | quando a VM tem que reclamar e quando tem que deixar passar | 45 |
| `linguagem` | `teste/casos_linguagem.c` | semântica: escopo, laço, UTF-8, coleção, arquivo, import | 834 |
| `pendentes` | `teste/casos_pendentes.c` | fila de trabalho, hoje vazia; os antigos achados ficaram como regressão | 28 |
| `cobertura` | `teste/casos_cobertura.c` | corpus antigo + matriz de interação de features | 256 |
| `diferencial` | `teste/casos_diferencial.c` | o corpus inteiro com a saída de hoje; gerado por `teste/geradores/diferencial.pr` | 1114 |
| `equivalencia` | `teste/casos_equivalencia.c` | a mesma computação de várias formas, que têm que concordar | 144 |
| `oraculo` | `teste/casos_oraculo.c` | receptor x método x argumentos; gerado por `teste/geradores/oraculo.pr` | 5903 |
| `robustez` | `teste/casos_robustez.c` | aridade errada em cada método; gerado por `teste/geradores/robustez.pr` | 13 |
| `libs` | `teste/casos_libs.c` | bibliotecas que rodam offline | 144 |

Com `JINGA_JIT=0`, o grupo `linguagem` rodou os mesmos 834 casos, todos passando. Na VPS, em 2026-09-25 sobre `3d82c5b`: ASan 8491/8491 e OOM em 6382 pontos sem morte.

Os geradores ficam em `teste/geradores/`: `diferencial.pr`, `oraculo.pr`, `robustez.pr`, `equivalencia.pr` e `para_chaves.pr`. Arquivo gerado não se edita à mão: regera.

### 5.6 `make check`: o portão, na ordem

Ele depende de `pool` e `testar`, e executa:

1. `./testar`
2. `JINGA_JIT=0 ./testar`
3. `make unidade`
4. `./pool teste/confere_metadata.pr`
5. `./pool teste/confere_assercoes.pr`
6. `./pool teste/confere_duplicados.pr`
7. `./pool scripts/audita_doc.pr`
8. `./pool teste/confere_cobertura_doc.pr`
9. `./pool scripts/conserta_barra_doc.pr --portao`
10. `./pool teste/cobre_stdlib.pr`
11. `./pool scripts/audita_c.pr`
12. `./pool scripts/audita_exemplos_doc.pr`
13. `./pool teste/fuzz_replay.pr`
14. `./pool teste/depurador.pr`
15. `./pool teste/exemplos_roda.pr`
16. `make leis`
17. `./pool teste/cli_roda.pr`
18. `./pool teste/sockets_roda.pr`
19. `./pool teste/jinker_roda.pr`
20. `./pool teste/mongo_roda.pr` (PULA sem `mongod`)
21. `node editor/vscode/teste_servidor.js ./pool` (PULA sem `node` ou sem `node_modules`)
22. `./pool scripts/gera_realce.pr --portao`
23. `editor/nvim/teste_nvim.sh` (PULA sem `nvim`)
24. `editor/intellij/plugin/teste_intellij.sh` (PULA sem IDEA)
25. `make analisa`
26. `make avisos`
27. `make check-debug`
28. `make check-e2e-local`

No fim ele lista o que fica fora do portão: `check-e2e`, `check-asan` (o Makefile anuncia ~4 min), `oom` (~5 min), `fuzz` e `cobertura`.

**Nesta máquina o `make check` inteiro é morto pelo OOM killer** (exit 137, três vezes em 2026-09-04). Aqui ele roda em lotes, um alvo pesado por comando, sempre com `timeout` e `nice`:

```bash
timeout 900 nice -n 19 ./testar
timeout 900 env JINGA_JIT=0 nice -n 19 ./testar
timeout 600 nice -n 19 ./pool teste/confere_metadata.pr
# ... um .pr do portão por comando ...
for a in arquivo sqlite socket pkg mail_local jinker; do timeout 300 nice -n 19 ./pool teste/e2e_roda.pr $a; done
for a in manpu qrcode c.pr d.pr f.pr;                 do timeout 300 nice -n 19 ./pool teste/e2e_roda.pr $a; done
```

O `make check` inteiro, o `check-asan`, o `oom` e o e2e completo rodam na VPS (5.11).

### 5.7 Fora do portão

```bash
make check-e2e                 # banco/mail/socket/mongo com serviço de verdade
make check-e2e E2E=jinker      # só o que casa com o filtro
make check-asan                # a suíte sob ASan+UBSan+LeakSanitizer (vazamento reprova)
make oom                       # falha de alocação ponto a ponto; NÃO roda nesta máquina (mata por memória): VPS
make fuzz FUZZ_T=1800          # fuzzer; o que achar vai pra teste/fuzz_achados/ e o check replaya
make cobertura                 # linha E ramo por arquivo, com catraca
make propriedade PROP_N=400 PROP_SEMENTE=$(date +%j)
make leis LEIS_N=5000 LEIS_SEMENTE=$(date +%j)
make analisa ANALISA_TUDO=1 ANALISA_MB=12000   # só em máquina que aguente
make avisos AVISOS_NIVEL=2     # revisão; nível 2 acusa snprintf com precisão explícita
make bench                     # compara com teste/bench_base.txt
make memcheck PS=teste/e2e/ws.pr
```

Serviços do e2e (credenciais de teste, as mesmas do CI):

| serviço | como o teste espera |
|---|---|
| PostgreSQL | `127.0.0.1:5432`, usuário `ps_teste`, senha `ps_teste`, banco `ps_teste` (`teste/e2e/db.pr`) |
| MySQL/MariaDB | `127.0.0.1:3306`, usuário `ps_teste`, senha `ps_teste`, banco `ps_teste`; no CI a senha de root é `ps_teste_raiz` |
| MongoDB | `mongod --dbpath /tmp/ps_mongo_db --port 27099 --bind_ip 127.0.0.1`, banco `ps_teste` (`teste/e2e/mongo.pr`); o `teste/mongo_roda.pr` sobe o próprio `mongod` em `/tmp` (no Windows, na pasta `ps_mongo_teste` da pasta corrente) |
| mail real | `MAIL_SMTP`, `MAIL_IMAP`, `MAIL_USER` e `MAIL_PASS` no ambiente (`teste/e2e/mail.pr`); sem eles, PULOU |
| mail local | `teste/e2e/mail_local.pr` sobe `teste/smtp_falso.py` e `teste/imap_falso.py`; precisa de `openssl` e `python3`, senão PULOU |

Regra dele pra ferramenta de teste: faltou banco, binário ou pacote pra testar, instala (na VPS por apt oficial ou Docker) sem pedir. Dado dele fica intocado, e o serviço fica desligado fora da rodada.

**Cobertura.** Na VPS com os bancos de pé, em 2026-09-21: linhas 83,7%, funções 90,5%, ramos 60,1%. Sem os bancos, o portão acusa `ps_db.c` e `ps_mongo.c`, e isso é PULOU, não regressão.
- Piso por arquivo em `teste/cobertura_base.txt` (linha%, ramo%, ramos cobertos, ramos totais). Exemplo: `jinga_vm.c 79.8 55.0 10990 19984`.
- `./pool teste/bench_cobertura.pr` ordena por ramos descobertos e mostra a coluna `fn=0` (funções nunca executadas). `--fn` lista essas funções, `--só-mede` não grava histórico.
- Cobertura da doc: `teste/confere_cobertura_doc.pr` já mede "o membro está documentado em algum lugar do escopo" (página, linha de tabela ou título de seção), não mais "um arquivo por membro". O piso gravado em `teste/cobertura_doc_base.txt` é **522**. `--lista` mostra o que falta, `--grava` regrava a base.

### 5.8 Drivers de teste (`teste/`)

| arquivo | uma linha |
|---|---|
| `alvo_debug.pr`, `alvo_debug_laco.pr`, `alvo_debug_erro.pr` | programas-alvo do depurador; as linhas são referenciadas por número, então mexer quebra o teste de propósito |
| `bench.pr` | benchmark do laço de despacho, em rodízio; `make bench` |
| `bench_cobertura.pr` | benchmark de cobertura arquivo a arquivo, ordenado por ramos descobertos, com `fn=0` |
| `book.pr` | arquivo avulso sem cabeçalho; nenhum alvo nem script o chama |
| `carga.pr` | carga no jinker (`--so-http`) e comparação de motor (`--so-motor`) |
| `cli_roda.pr` | CLI e `jpkg`, com registry servido pelo próprio jinker em loopback (260 checagens em 2026-09-28) |
| `cobertura_portao.pr` | catraca de cobertura por arquivo: só sobe |
| `cobre_stdlib.pr` | chama a stdlib que nenhum teste chamava (109 checagens em 2026-09-28) |
| `confere_assercoes.pr` | reprova caso que confere menos do que o runner sabe conferir |
| `confere_cobertura_doc.pr` | catraca de membros documentados |
| `confere_duplicados.pr` | reprova dois casos com o mesmo fonte |
| `confere_metadata.pr` | confere o `--metadata` chamando cada método de verdade |
| `depurador.pr` | fala DAP com o motor (breakpoint, pilha, variáveis, passos, gráfico) |
| `e2e_roda.pr` | orquestra os e2e por papel: par (servidor + cliente), sozinho, sem par |
| `exemplos_roda.pr` | roda os `examples/`; os que bloqueiam só passam por `--check`; `mail_demo`, `request`, `push_get` e `swagger` podem dar PULOU |
| `fuzz_replay.pr` | replaya `teste/fuzz_achados/` (hoje `crash-parser-profundo`) |
| `fuzz_semeia.pr` | semeia o corpus do fuzzer com os programas válidos do repositório |
| `jinker_alvo.pr`, `jinker_alvo_tls.pr`, `jinker_alvo_estrito.pr` | servidores-alvo do `jinker_roda.pr`: HTTP, HTTPS e origem estrita (`cors ... local=False`); saem por `/__parar` pra a cobertura contar |
| `jinker_roda.pr` | o servidor jinker a fundo, em loopback |
| `leis.pr` | leis da linguagem sobre valores sorteados, com encolhimento do contraexemplo |
| `mongo_roda.pr` | sobe `mongod` próprio em `/tmp` e testa o driver; PULOU sem o binário |
| `oom_varre.pr` | varredura de falha de alocação (usa `pool-oom`) |
| `propriedade.pr` | teste metamórfico com entrada sorteada |
| `sockets_roda.pr` | lib de sockets inteira em loopback |
| `e2e/*.pr` | `arquivo`, `sqlite`, `socket`, `pkg`, `mail_local`, `jinker_srv` + `jinker_cli` + `jinker_bruto`, `manpu`, `qrcode`, `c`, `d`, `f`, `db`, `mongo`, `mail`, `ws` (servidor sem par, é relatado e não rodado) |
| `unidade.c` | unidade em C dos módulos puros (pilha, hash, regex, ast, xlsx) |
| `ps_oom.c`, `ps_fuzz.c` | ganchos do `pool-oom` e do `pool-fuzz` |

A lib `swagger` do usuário mora em `~/.jinga/libs/swagger.pr`, não no repositório. Sem ela no Mac, o `exemplos_roda.pr` dá PULOU no `swagger.pr` e o `audita_exemplos_doc.pr` lista o bloco de `docs/swagger` como módulo de exemplo. Nenhum dos dois reprova.

### 5.9 Scripts (`scripts/`)

Todos em Jinga, rodados com `./pool scripts/X.pr` (ou `jinga scripts/X.pr`). Os scripts `.py` antigos da doc não existem mais.

| script | uma linha |
|---|---|
| `acha_chaves_nuas.pr` | acha chave de dict sem aspas em `.pr` e na doc, pelos tokens do motor |
| `audita_c.pr` | padrões de C que o compilador não pega (`x = realloc(x,...)`, objeto do GC de `malloc`, opcode fora do `.def`); `--lista` |
| `audita_doc.pr` | título de cada página contra a assinatura do `--metadata`; `--tudo` |
| `audita_exemplos_doc.pr` | todo bloco `ps` ou `jinga` da doc pelo `--check`; `ps contexto` vai na frente dos blocos seguintes; `--lista` |
| `bump.pr` | sobe a versão da linguagem ou da extensão (5.15) |
| `casos_para_chaves.pr` | converte o fonte dos casos em C do estilo `:` pro `{ }`, mantendo a expectativa |
| `ci_anota.sh` | roda um passo do CI e publica as falhas como anotação `::error::` (5.12) |
| `compila.pr` | compila, testa e empacota pra linux, mac ou windows (5.13) |
| `conserta_barra_doc.pr` | `//` usado como comentário na doc vira `#`; `--aplica`, `--portao` |
| `converte_funct.pr` | `action`/`reaction` -> `funct`, pelo lexer, conferindo o fluxo de tokens |
| `converte_tudo.pr` | converte `.pr` de `:` pra `{ }` e prova que a saída não mudou; `--grava` |
| `docs_para_chaves.pr` | converte os blocos de código de um `.md` de `:` pra `{ }` |
| `fortalece_casos.pr` | acrescenta stdout e rc medidos a caso que conferia só o stderr; nunca regrava regressão |
| `gera_bytes_docs.pr` | gera as páginas de método do tipo `byte` rodando cada exemplo; `--confere` |
| `gera_logo.pr` | a logo em SVG e PNG, com rasterizador próprio |
| `gera_opcodes.pr` | migração única que gerou `vm/ps_opcodes.def` |
| `gera_realce.pr` | listas do realce do VS Code e do Neovim a partir do `--metadata`; `--portao` |
| `mede_retornos.pr` | mede o tipo de retorno de cada nativo chamando-o e grava `vm/retornos_medidos.inc`; `--confere` |
| `migra_classe.pr` | `class Nome()` -> `class Nome`, `base(x)` -> `base().__init__(x)`; `--aplica`, `--libs`, `--confere` |
| `migra_pr.pr` | `.ps`, `.psl` e `.p` -> `.pr` e as referências; `--aplica`, `--so-renome`, `--sem`, `--libs` |
| `para_chaves.pr` | um fonte do estilo `:` pro `{ }`, pelo lexer |
| `regrava_dif.pr` | regrava a expectativa de casos nomeados; só quando o comportamento mudou por decisão, caso a caso |
| `remove_grafia_antiga.pr` | `action`/`reaction` -> `funct` como palavra, em texto qualquer |

`bump_version.pr`, na raiz, é o antigo: ainda lista `poolscript.md`, que não existe mais, e arrasta a versão da extensão junto. O que se usa é `scripts/bump.pr`.

### 5.10 Regras de execução e de medição

- **Nada de falso verde.** Etapa que não rodou diz PULOU e o motivo. Não se põe skip silencioso, nem gambiarra que transforme erro em sucesso. Não se congela como "esperada" uma saída que não foi conferida.
- **Medir todas as variantes.** Nunca afirmar "o motor faz/não faz X" sem ter rodado X. Toda classe de vetor entra na suíte como lista (laço), não como um ou dois exemplos. Regra escrita em dois lugares do motor é bug latente: se unifica.
- **Medir a categoria antes de corrigir.** Gera a matriz, classifica, mostra a lista, corrige em lote e prova de novo com a matriz. A matriz combinatória da época do interpretador em Python (`tests/matriz_casos.py`) não existe mais. O que ficou dela são os geradores de `teste/geradores/`.
- **Scripts de apoio só em Jinga** (`.pr`, rodados no `./pool`). Python no repositório só nos servidores falsos `teste/smtp_falso.py` e `teste/imap_falso.py`.
- **Doc, exemplos e notas mudam no mesmo commit do comportamento**, com `make check` inteiro. Número vai medido, nunca de memória.
- **Nesta máquina** (i3-1115G4, 7,5 GiB, uns 2,4 a 3,5 GiB livres com o desktop aberto):
  - um alvo pesado por comando; `&&` e `;` encadeados contam como vários;
  - `timeout` em todo comando, `nice -n 19` no que é pesado;
  - `uptime` antes; com load acima de 8, espera;
  - `ulimit -v` em ferramenta de análise; o `-fanalyzer` no `jinga_vm.c` passa de 6 GB e derrubou a sessão;
  - matar os processos que o teste subiu (servidor LSP, jinker);
  - E2E com editor real só na validação final.
- **Performance se mede só com `perf stat`** (instruções e ciclos). O tempo de parede aqui varia 2x entre execuções iguais (o processador esquenta e baixa o clock). Com a senha do sudo desta máquina, `dormenis`:

  ```bash
  printf 'dormenis\n' | sudo -S -p '' perf stat -e instructions,cycles ./pool arquivo.pr
  ```

  O `perf` é do kernel Linux e não existe no macOS. No Mac, a medição de instruções e ciclos se faz num Linux (a VPS; conferir lá com `perf --version`). Os números de referência (fase 1 e código de máquina) estão na tabela de 3.8.
- **Código de máquina (JIT x86-64, `vm/ps_jit_x64.h`).** Os controles (`JINGA_JIT=0`, `--sem-jit`, `JINGA_JIT_LOG=1`, `JINGA_JIT_FALHA=1`) estão em 3.8; o `make check` roda a suíte com e sem ele.
- **No Mac** nada disso foi medido. A regra dos lotes nasceu desta máquina. Na primeira rodada do `make check` no Mac (8 núcleos, 16 GB) se vê se ele aguenta inteiro. Até lá vale o roteiro da VPS.

### 5.11 VPS `tecsol-vps`

Ordem dele desde 2026-09-20: build e suíte pesada rodam na VPS, e depois de cada rodada não sobra pasta, log nem cache lá.

| item | valor |
|---|---|
| acesso | `ssh -o BatchMode=yes tecsol-vps`: alias do `~/.ssh/config`, `HostName 10.8.0.1` (endereço pela VPN), `User root`, `IdentityFile ~/.ssh/vps_tecsol_hostinger`, `IdentitiesOnly yes` |
| sem VPN | `tecsol-vps-pub` (`HostName 187.127.9.180`, mesma chave); só funciona enquanto a porta 22 pública estiver aberta. Em 2026-09-28 não conectou (`Connection timed out`): a 22 pública está fechada no firewall |
| máquina | Ubuntu 24.04, 4 núcleos, 16 GB, node 22 e npm; dependências do `ci.yml` + `pkg-config` + `lcov` instaladas em 2026-09-20; sem Neovim e sem IDEA (esses portões PULAM lá) |
| usuário da suíte | **`nobody`**, nunca root (`runuser -u nobody`, com `HOME` dentro da pasta da rodada) |
| pasta | `/tmp/ps_ck_<nome>` (a nota da memória usa `/tmp/ps-lote-N`), com log fora dela |

Levar pro Mac `~/.ssh/config` e as chaves `~/.ssh/vps_tecsol_hostinger` (+ `.pub`). O conteúdo da chave não entra neste documento.

Por que nunca como root: o caso `os.move("o.txt", "/dev/full")` faz `rename()` por cima do nó de dispositivo, que root consegue, e o transforma em arquivo comum. Aí 11 casos de disco cheio (ENOSPC) passam a falhar. Se só essa família falhar, restaurar com `rm -f /dev/full && mknod -m 666 /dev/full c 1 7`.

Roteiro:

```bash
L=ps_ck_$(date +%m%d%H%M)
REPO=~/poolscript-lang

# 1. o /tmp de antes, pra saber o que a rodada criou
ssh -o BatchMode=yes tecsol-vps 'ls /tmp > /tmp/.ps-antes'

# 2. a árvore de trabalho, sem .git, dependências e binários
rsync -a --exclude=.git --exclude=node_modules --exclude=dist --exclude=cob \
  --exclude=/pool --exclude=/pool-asan --exclude=/pool-debug --exclude=/pool-oom \
  --exclude=/pool-fuzz --exclude=/testar --exclude=/unidade \
  "$REPO"/ tecsol-vps:/tmp/$L/

# 3. o roteiro da rodada, dentro da pasta (HOME lá dentro: o cache do npm sai junto)
cat > /tmp/roda_$L.sh <<'EOF'
cd "$(dirname "$0")"
export HOME="$PWD/home"; mkdir -p "$HOME"
(cd editor/vscode && npm install --omit=dev --no-audit --no-fund)
for alvo in "make pool testar" "make check" "make check-asan" "make oom"; do
  echo "== $alvo"; $alvo; echo "rc=$?"
done
EOF
scp /tmp/roda_$L.sh tecsol-vps:/tmp/$L/roda.sh && rm -f /tmp/roda_$L.sh

# 4. dono nobody e execução desacoplada (o ssh pela VPN cai em rodada longa)
ssh tecsol-vps "chown -R nobody:nogroup /tmp/$L && nohup setsid runuser -u nobody -- bash /tmp/$L/roda.sh > /tmp/$L.log 2>&1 < /dev/null &"

# 5. acompanhar
ssh tecsol-vps "tail -n 20 /tmp/$L.log; grep -n 'rc=' /tmp/$L.log"

# 6. LIMPEZA TOTAL: ver o que a rodada deixou em /tmp e apagar tudo
ssh tecsol-vps "comm -13 /tmp/.ps-antes <(ls /tmp)"
ssh tecsol-vps "rm -rf /tmp/$L /tmp/$L.log /tmp/ps_* /tmp/.ps-antes"
```

Depois do passo 6, apagar também o que o `comm` listou e que a rodada criou (as suítes deixam `/tmp/ps_*`, e o e2e deixa as pastas do `mongod`). Não se instala nada na VPS além do que o CI instala sem avisar.

**Reproduzir o CI.** O runner do GitHub não é uma Ubuntu limpa: a imagem `ubuntu-24.04` traz PostgreSQL e `libpq-dev` 18 do repositório do PostgreSQL. Em 20 a 22/09 isso quebrou o link, quando o libpq ainda era ligado estático. Hoje os clientes de banco não são mais ligados. Conferir mudança de `ci.yml` ou de dependência de link continua sendo:
1. subir na VPS um container `ubuntu:24.04` com a preparação da imagem do GitHub;
2. rodar o passo do yml com o texto exato, sob `bash -eo pipefail`, e a suíte como usuário comum;
3. no fim, apagar pasta, container e imagem.

### 5.12 CI no GitHub

**`.github/workflows/ci.yml`** roda em push na `main`, em pull request e todo dia às 03:00 UTC:

| job | quando | o que roda |
|---|---|---|
| `portao` | todo push e PR | apt, `make pool testar`, `make check` |
| `propriedade` | todo push e PR | `make pool`; `make propriedade PROP_N=400 PROP_SEMENTE=$(date +%j)`; `make leis LEIS_N=5000 LEIS_SEMENTE=$(date +%j)`; `make leis LEIS_N=300 LEIS_SEMENTE=$(date +%j)` |
| `robustez` | só no agendado | `make check-asan`, `make oom`, `make analisa ANALISA_TUDO=1 ANALISA_MB=12000`, `make avisos AVISOS_NIVEL=2` (`continue-on-error`), `make fuzz FUZZ_T=1800`, `make bench BENCH=--portao`, `./pool teste/fuzz_replay.pr`, artefato `fuzz-achados` |
| `cobertura` | todo push | serviços `postgres:16`, `mariadb:10.11` e `mongo:7` (27099), com as credenciais de 5.7; `make cobertura`; artefato `cobertura-html` |

**`.github/workflows/build.yml`** roda em push na `main` e nas branches `porte-*`, em tag `v*`, e sob pedido (`workflow_dispatch` com `alvo` = `linux | mac | windows | todos`):

| job | máquina | passos |
|---|---|---|
| `linux` | `ubuntu-24.04` | apt; `make pool testar`; `./testar && JINGA_JIT=0 ./testar`; `npm install` + `./build_bundle.sh`; artefato `jinga-linux-x86_64` (`dist/jinga-portable.tar.gz`) |
| `mac` | `macos-15-intel` (Mac Intel, a arquitetura do MacBook Pro 2019), timeout de 240 min | `brew install openssl@3 libpng gmp libpq mariadb-connector-c unixodbc mongo-c-driver@1 coreutils`; cada passo via `scripts/ci_anota.sh`: `make pool testar`, versão, `./testar`, `JINGA_JIT=0 ./testar` (passo próprio, `if: always()`), `npm install` + `make check`, `./build_bundle.sh`; artefato `jinga-macos-x86_64` |
| `release` | `ubuntu-24.04`, só em tag `v*`, depois de `linux` e `mac` | baixa os dois artefatos e roda `gh release create "$GITHUB_REF_NAME" pacotes/* --title "Jinga $GITHUB_REF_NAME"` |

O `build.yml` não tem job de Windows. `alvo=windows` não roda nada, e o `compila.pr windows` falha no download do artefato.

**`scripts/ci_anota.sh`**:

```bash
scripts/ci_anota.sh make pool testar
```

Ele roda o comando e, se falhar, publica uma anotação `::error title=passo falhou::` com todas as linhas `FALHOU|MORREU|TRAVOU|JA FUNCIONA|error:|Error N` (5 linhas de contexto cada, até 400 linhas) e as últimas 25 linhas da saída. O motivo: o log do job só se baixa com admin do repositório (medido, 403 "Must have admin rights to Repository"), e as anotações de repositório público qualquer um lê pela API. Com o repositório privado (desde 2026-09-28), a leitura das anotações também pede o `gh` logado. O `ci.yml` não usa o `ci_anota.sh`, então falha nele só se lê pelo log, com o `gh` logado.

**Consultar sem login:**

```bash
curl -s "https://api.github.com/repos/kleberDevion/poolscript/actions/runs?per_page=6"   # execuções
curl -s "https://api.github.com/repos/kleberDevion/poolscript/actions/runs/<run_id>/jobs" # jobs e passos
curl -s "https://api.github.com/repos/kleberDevion/poolscript/check-runs/<job_id>/annotations"
```

Desde que o repositório ficou privado (2026-09-28), essas três chamadas sem login respondem 404 (medido às 23h11: `actions/runs` e `check-runs/.../annotations` deram 404, e o `raw.githubusercontent.com/.../instalar.sh` também). Com o `gh` logado, a mesma leitura sai por `gh api "repos/kleberDevion/poolscript/actions/runs?per_page=6"`.

Com login: `gh run list`, `gh run view <id> --log-failed`, `gh run watch <id> --exit-status`. A página é https://github.com/kleberDevion/poolscript/actions.

**Estado em 2026-09-28:**
- Commit `4aedf8b`, `build`: `linux` verde; `mac` vermelho no passo da suíte. A anotação mostrou duas falhas:
  - `crash`: "recursao via map dentro de async e RecursionError na pilha menor da fibra", com a VM morta por sinal 10, que no macOS é SIGBUS;
  - `linguagem`: "close() acusa o que nao conseguiu gravar", com `PermissionError ... '/dev/full'`.
- Commit `4aedf8b`, `ci`: `portao` vermelho no `make check` (exit 2, sem detalhe fora do log); `cobertura` e `propriedade` verdes.
- O commit `ec711c7` (16.1.8) ataca a primeira falha:
  - a folga da pilha passa a ser pelo menos dois degraus reais medidos em execução, porque o clang põe 23 KB no quadro do laço da VM contra 12 KB do gcc;
  - separa a suíte sem JIT num passo próprio;
  - faz a anotação juntar todas as falhas.
- Os workflows do `ec711c7` terminaram em falha: `build` (run 36506350266) com 8 falhas na suíte do Mac, e `ci` (run 36506350332) no `make check` e no `make cobertura` (9.10).
- Commit `b180807` (último da `main`, código de `3c245e7` mais um exemplo), lido pela API no fim do dia:
  - `build` (run 36507856713): `linux` verde; `mac` vermelho nos dois passos da suíte, com `compila` e `versao` verdes e `portao inteiro`, `empacota` e `release` pulados. As anotações: "processo filho nao herda os descritores abertos do motor" (saída esperada 4, obtida 5) nos dois passos, e num deles também "pool de fibra cheio é erro limpo, nao morte" (TRAVOU, passou de 20 s). Totais: 8536 casos, 8535 e 8534 passaram. Os 6 casos do `/dev/full` aparecem listados como de outro sistema;
  - `ci` (run 36507856867): `portão` vermelho no `make check` (a anotação só diz exit code 2), `cobertura` vermelho no `make cobertura`, `propriedade` verde, `ASan/UBSan` pulado.
- Contagem pela API: o workflow `ci` tem 139 execuções e nenhuma verde; o `build` tem 13 e nenhuma verde (em todas o job `linux` passou e o `mac` reprovou).

### 5.13 `scripts/compila.pr`

```bash
jinga scripts/compila.pr                 # o sistema desta máquina
jinga scripts/compila.pr linux|mac|windows|todos
jinga scripts/compila.pr mac --instala   # e instala (só no próprio sistema)
jinga scripts/compila.pr mac --portao    # roda o make check inteiro no lugar da suíte
```

Antes de haver `jinga` instalado, o mesmo roda com `./pool scripts/compila.pr ...`.

**No próprio sistema:**
1. confere as dependências. No Mac roda `brew list --versions` e `brew install` do que faltar; no Windows, `pacman -S --needed` no MSYS2 UCRT64; no Linux, se o build falhar, imprime a linha do `apt`;
2. `make pool testar` e `./pool --version`;
3. `./testar` e `JINGA_JIT=0 ./testar`, ou `make check` com `--portao`;
4. `./build_bundle.sh`;
5. com `--instala`: no Mac `cd dist/jinga-macos-x86_64 && sudo ./instalar.sh`, senão `sudo make install`; e mostra `jinga --version`.

**Pra outro sistema:** exige o `gh` logado e a branch no GitHub.
1. `gh workflow run build.yml --ref <branch> -f alvo=<sistema>`;
2. espera 8 s e acha a execução com `gh run list`;
3. `gh run watch <id> --exit-status`;
4. `gh run download <id> -n jinga-<sistema>-x86_64 -D dist`.

### 5.14 Pacotes, instalador e release

**`build_bundle.sh`** (ou `make bundle`) exige `./pool` compilado.

No **Linux** gera `dist/jinga-portable/` e `dist/jinga-portable.tar.gz`, com:
- `jinga.bin`;
- as `.so` de que ele depende, menos o núcleo do glibc (libc, libm, libpthread, libdl, librt, libresolv e o loader);
- os clientes de banco `libpq.so.5`, `libmariadb.so.3`, `libodbc.so.2` e `libmongoc-1.0.so.0` com o que eles arrastam; se faltar um na máquina, o script para;
- o wrapper `jinga` (e `pool`) com `LD_LIBRARY_PATH`;
- `lsp/`, `docs/`, `dados/` (MIME e ícone), `instalar.sh` e `LEIAME.txt`.

A máquina de destino precisa ser x86-64 com **glibc >= 2.38** (Debian 13, Ubuntu 24.04+). Abaixo disso, o instalador compila.

No **macOS** gera `dist/jinga-macos-x86_64/` e `dist/jinga-macos-x86_64.tar.gz`, com:
- `bin/jinga` e o link `bin/jpkg`;
- `lib/jinga/*.dylib`: as que não são do sistema, mais libpq, mariadb, libodbc e libmongoc do Homebrew. Os caminhos são reescritos com `install_name_tool` pra `@executable_path/../lib/jinga/...` e `@loader_path/...`, e tudo é assinado de novo ad hoc (`codesign -s - -f`);
- `share/jinga/lsp`, `share/jinga/docs`, `instalar.sh` e `LEIAME.txt`.

O pacote roda sem Homebrew na máquina de destino.

**`instalar.sh`:**

```bash
curl -fsSL https://raw.githubusercontent.com/kleberDevion/poolscript/main/instalar.sh | sudo bash
sudo ./instalar.sh                 # de dentro do repositório ou do pacote: instala o que está ao lado
sudo ./instalar.sh /opt/jinga      # outro prefixo
sudo ./instalar.sh --remover
PACOTE=http://<maquina>:8080/jinga-portable.tar.gz sudo -E bash instalar.sh   # pacote de outra origem
```

**Com o repositório privado**, a primeira linha (o `curl` do `raw.githubusercontent.com`) e o download do release que o `instalar.sh` faz sozinho respondem 404: os dois são sem login. No Mac o caminho é clonar (com o `gh auth login` feito) e rodar de dentro do repositório, que instala o que está ao lado sem baixar nada.

No **Linux**:
1. sem binário ao lado, baixa `releases/latest/download/jinga-portable.tar.gz`; senão o nome antigo `pool-portable.tar.gz`; senão clona, instala as dependências por apt e compila;
2. instala o binário (wrapper + `jinga.bin` + `lib/jinga`, ou o `pool` compilado) e os links `jpkg`, `pool` e `psl`;
3. instala o LSP (todos os `.js` menos `extension.js` e `teste_servidor.js`, mais os `node_modules`) e instala o node por apt se faltar;
4. remove instalações antigas do PATH (inclusive o `~/.local/bin` do `SUDO_USER` e as pastas do Windows no WSL) e os `alias` dos arquivos de shell, guardando uma cópia `.antes-da-jinga`;
5. instala a doc em `share/jinga/docs`, o MIME e o ícone.

No **macOS**:
1. sem pacote ao lado, baixa `releases/latest/download/jinga-macos-x86_64.tar.gz`;
2. com `bin/jinga` ao lado, copia `bin`, `lib/jinga` e `share/jinga`; com `./pool` do repositório, instala o compilado (ligado ao Homebrew daqui), os `.js`, os `node_modules` e a doc;
3. cria os links `jpkg`, `pool` e `psl` e os wrappers `jinga-lsp` e `poolscript-lsp`.

**Releases publicados hoje** (conferido pela API em 2026-09-28):
- `v8.4.3` (2026-09-08, o `latest`, asset `pool-portable.tar.gz`);
- `v8.2.54` e `v8.2.53` (asset `pool-portable.tar.gz`);
- `Linux_standalone_ELF_8_2_31`;
- `v8_2_17`.

Consequências:
- No Linux, o `curl ... | sudo bash` instala hoje a **8.4.3**, pelo nome antigo do pacote.
- No Mac, ele para com "não há pacote do macOS publicado" e manda compilar do fonte.
- Um release com pacote velho é pior que nenhum, e mexer no empacotamento pede republicar.

**Publicar um release novo** (o `latest/download` leva alguns minutos de CDN pra trocar; a URL por tag responde na hora):

```bash
git tag v16.1.9 && git push origin v16.1.9     # build.yml: linux + mac + release com os dois pacotes
# alternativa só Linux, à mão:
make bundle && gh release create v16.1.9 dist/jinga-portable.tar.gz --title v16.1.9
```

O job `release` só roda se `linux` e `mac` passarem.

### 5.15 Versão e bump

- **Fonte única:** `#define PS_VERSAO "16.1.9"` em `vm/ps_versao.h`. O comentário do header descreve a numeração em base 100: os dígitos 2 e 3 vão de 0 a 99 e, ao chegar em 100, zeram e somam 1 à esquerda (8.2.99 -> 8.3.0, 8.99.99 -> 9.0.0).
- O `scripts/bump.pr` soma 1 na casa pedida e não faz essa virada sozinho. Em X.Y.99, passar a versão exata.

```bash
jinga scripts/bump.pr                 # mostra as versões e onde a da linguagem aparece
jinga scripts/bump.pr patch           # 16.1.9 -> 16.1.10
jinga scripts/bump.pr minor           # 16.1.9 -> 16.2.0
jinga scripts/bump.pr major           # 16.1.9 -> 17.0.0
jinga scripts/bump.pr 16.2.0          # exatamente essa
jinga scripts/bump.pr ext patch       # só a extensão (patch|minor|major|X.Y.Z)
```

- Arquivos da linguagem que o bump troca: `vm/ps_versao.h`, `jinga.md`, `docs/Jinga.md`, `docs/os/cmd/cmd.md` e `docs/os/run/run.md`.
- A extensão tem versão própria em `editor/vscode/package.json` e `package-lock.json`, hoje 8.6.8. Ela só sobe quando gramática, tema ou servidor da vsix mudam. Depois: `make vsix && make instala-vsix`.
- **Regra dele: toda alteração leva bump, no mesmo commit.** Ele usa o `--version` pra saber se o binário do PATH tem a mudança. Sem bump, roda o velho e conclui que não foi feito.
- **Qual casa subir**, decidido pelo conteúdo e nunca pra agradar:
  - `patch`: correção sem mudar comportamento válido;
  - `minor`: novidade compatível;
  - `major`: quebra código existente. A classe sem `()` foi 16.0.0.
- Rótulo errado se corrige com um commit de versão dizendo o porquê.
- Depois do bump é obrigatório `make pool`, senão o binário segue respondendo a versão antiga, e depois instalar.

### 5.16 Sequência de uma alteração, do começo ao commit

```bash
./pool --check arquivo.pr                    # todo .pr tocado
make pool && ./testar <grupo>                # o pontual primeiro
jinga scripts/bump.pr patch|minor|major      # pelo conteúdo
make pool                                    # a versão nova no binário
make check                                   # na VPS; aqui, em lotes (5.6)
sudo make install                            # binário, servidor LSP e doc instalados
for p in $(pgrep -f "^node /usr/local/share/jinga/lsp/server.js"); do kill $p; done
node editor/vscode/teste_servidor.js ./pool  # se mexeu no editor ou na doc
git -c user.name=kleberDevion -c user.email=klebersantanadeoliveira07@gmail.com commit
```

A mensagem do commit leva o trailer `Co-Authored-By: Regressão <noreply@anthropic.com>`. Doc, exemplos, gramática, vsix e teste diferencial da mudança entram no mesmo commit.

### 5.17 No Mac: primeira compilação e o que já se sabe

```bash
xcode-select --install
# Homebrew (no Intel ele fica em /usr/local)
brew install openssl@3 libpng gmp libpq mariadb-connector-c unixodbc mongo-c-driver@1 coreutils gcc lcov gh neovim node
export PATH="$(brew --prefix coreutils)/libexec/gnubin:$PATH"   # põe no ~/.zshrc
git clone https://github.com/kleberDevion/poolscript.git ~/poolscript-lang
cd ~/poolscript-lang
make pool testar
./pool --version
./testar
JINGA_JIT=0 ./testar
(cd editor/vscode && npm install --omit=dev --no-audit --no-fund)
sudo make install                   # ou: ./pool scripts/compila.pr mac --instala (pacote + instalar.sh)
gh auth login
```

Com o repositório privado, o `git clone` por HTTPS pede login: faça `gh auth login` e `gh auth setup-git` antes do clone, ou clone por SSH (`git clone git@github.com:kleberDevion/poolscript.git ~/poolscript-lang`, com a chave `github_tecsol` da 7.5; o acesso por SSH foi conferido em 2026-09-28 com `git ls-remote`).

O que o código e o CI já mostram do macOS:
- **`/dev/full` não existe no macOS.** Ocorrências nos casos: `teste/casos_crash.c` (7 linhas), `teste/casos_diferencial.c` (7) e `teste/casos_linguagem.c` (4), além de `teste/ps_oom.c`. Em `4aedf8b` pelo menos "close() acusa o que nao conseguiu gravar" falhou por isso. Desde `3c245e7` os 6 casos que dependem dele valem só no Linux; no Mac o `testar` lista os 6 e não os conta como passou.
- **`setsid` e `timeout` saíram do `make check` no 16.2.3.** Os drivers do portão sobem e derrubam processo pelo `Process` do `os.run(..., capture="live")` e contam o prazo no relógio, sem shell (o `os.cmd` do Windows é o cmd.exe). Continuam chamando, fora do `make check`: `scripts/mede_retornos.pr` (os dois), `teste/propriedade.pr` e `teste/oom_varre.pr` (`timeout`; rodam só no Linux, pelo `ci.yml`), `teste/geradores/diferencial.pr`, `scripts/converte_tudo.pr`, `fortalece_casos.pr` e `regrava_dif.pr` (`timeout`). O `editor/nvim/teste_nvim.sh` usa `timeout` ou `gtimeout` (o nome do coreutils do Homebrew) e diz quando não acha nenhum.
- **SIGBUS na recursão dentro de fibra:** corrigido em `ec711c7` (16.1.8). Nos CIs seguintes (`ec711c7`, `3c245e7`, `b180807`) esse caso não aparece mais entre as falhas do Mac (5.12 e 9.10).
- **`make analisa` e `make avisos`** pedem o gcc do Homebrew (`gcc-<N>` em `$(brew --prefix)/bin`).
- **Alvos presos ao Linux:** `make verifica` (`ldd`, `nm -uD`), `pool-fuzz` (caminhos `/usr/include/...`), `install-mime` e `install-icone` (pulados no Darwin) e o `IJ_HOME` do `make intellij` (4.5).
- **`perf` não existe no macOS** (5.10).
- **O JIT é x86-64**, a mesma arquitetura do i9-9880H, e roda no Mac Intel. Em `4aedf8b` a suíte com JIT no `macos-15-intel` só acusou as falhas listadas acima.

## 6. Trilha de dev e pendências

### 6.1 Onde a trilha mora e como se usa

- O repositório não tem arquivo de roadmap. A trilha é a nota `memory/trilha-de-dev.md`, que existe pra você não ter que lembrar o assistente.
- Pendências de lotes específicos ficam em notas próprias: `revisao-model-pendente.md`, `roadmap-self-hosting.md`, `backlog-pendente.md` e `memory/*-sobre-guzer.md`.
- O plano em andamento é `~/.claude/plans/steady-weaving-pine.md`.
- Cada item da trilha vira um lote com plano próprio, nunca "de brinde" dentro de outro lote.
- Pendência de comportamento do motor entra em `teste/casos_pendentes.c`: o caso é escrito com o comportamento certo e o campo `pendente = 1`. Enquanto o motor não acertar, o caso falha. Quando acertar, o runner acusa "JA FUNCIONA" e manda tirar o caso de lá. Hoje a fila está vazia: nenhum caso tem o campo ligado, e o cabeçalho do arquivo diz isso.

### 6.2 Estado medido em 2026-09-28 (fim do dia)

- Versão da linguagem **16.1.9** no repo (`vm/ps_versao.h`, bump no commit `3c245e7`) e no binário instalado:

```
$ ./pool --version
Jinga 16.1.9 [PSVM] (2026-09-28) Runtime standalone
$ jinga --version
Jinga 16.1.9 [PSVM] (2026-09-28) Runtime standalone
```

- Extensão do editor: **8.6.8** (`editor/vscode/package.json`).
- `main` em `b180807`, igual a `origin/main`. A árvore de trabalho tem mudança sem commit em vários arquivos de `vm/` (o Windows em andamento; lista e horário em 9.10).
- GitHub Actions, lido pela API (`api.github.com/repos/kleberDevion/poolscript/actions/runs`) quando o repositório ainda era público. Privado, essa leitura pede o `gh` logado:
  - Workflow `build`, commit b180807 (run 36507856713):
    - job `linux`: verde (compila, suíte com JIT ligado e desligado, empacota);
    - job `mac`: dependências, compilação e `--version` verdes; cai nos dois passos da suíte (detalhe em 5.12 e 9.10); os passos `portao inteiro` e `empacota` foram pulados, e o job `release` também.
    - Mais cedo, no commit 4aedf8b, o job `mac` tinha caído na suíte com 2 falhas (6.3).
  - Workflow `ci`: **139 execuções registradas, nenhuma verde**. O `build` tem 13, nenhuma verde.
    - No último push (b180807, run 36507856867), o job `portão (suíte, doc, padrões em C, analisador)` falha no `make check` com exit 2, e o `cobertura (catraca por arquivo)` falha no `make cobertura`.
    - `teste de propriedade (semente do dia)` saiu verde.
    - `ASan/UBSan, falha de alocação e fuzz` foi pulado.
    - No 4aedf8b, o `cobertura` tinha saído verde.
    - O log só se baixa logado com permissão de administrador. Do shell do assistente, o `gh` responde 401 porque não lê o token do chaveiro.
- `node --version` neste laptop: `v18.19.1`.

### 6.3 Em andamento: Jinga no macOS e no Windows

Plano de 2026-09-28 (`steady-weaving-pine.md`). Decisões suas:

- **um motor só**: o mesmo C, compilado pelo clang ou gcc de cada sistema;
- o JIT continua gerando x86-64 (Mac Intel e Windows x64 são x86-64);
- LLVM entra só como compilador (clang), não como segundo motor;
- ordem: "tudo neste plano, nada pela metade".

| Parte | O que é | Estado |
|---|---|---|
| 1 | Camada de sistema `vm/ps_so.h` + `vm/ps_so.c`: eventos, aviso entre threads, pilha de fibra com guarda, memória executável do JIT, o próprio executável, resolução de nomes | **Feita pra Linux e macOS**. Linux usa epoll + eventfd + mmap + `/proc/self/exe`; macOS usa kqueue + pipe + `_NSGetExecutablePath` (529c518, 16.1.5; DNS do sockets em c669a78, 16.1.7). Linux ficou inalterado: 8547 casos (JIT ligado e desligado), jinker 173, sockets 29, cli 260, `perf stat` na faixa do 16.1.4. Backend Windows: em 4aedf8b só existiam as macros `PS_SO_WINDOWS` e `sys.platform()` devolvendo `"windows"`; desde `3c245e7` (16.1.9) o ramo existe (WSAPoll, par de sockets no loopback, VirtualAlloc/VirtualProtect, `GetModuleFileNameA`, API de socket e raízes TLS do sistema; ver 3.11). |
| 2 | macOS completo + ferramenta de build | Makefile com ramo Darwin. Pacote `dist/jinga-macos-x86_64` feito pelo `build_bundle.sh`, com as dylibs em `@executable_path`/`@loader_path` e assinatura ad hoc. `instalar.sh` com ramo Darwin. `.github/workflows/build.yml` com Linux e `macos-15-intel`, publicando release em tag `v*` (6980c40, 16.1.6). `scripts/compila.pr` (`linux\|mac\|windows\|todos`, `--instala`, `--portao`) e `scripts/ci_anota.sh`, que publica as últimas 80 linhas do passo que falha como anotação legível sem login (89e32d7). Depois: pilha do clang (`ec711c7`, 16.1.8), casos só do Linux e TLS pelo Keychain (`3c245e7`, 16.1.9). **Falta**: suíte do Mac verde, `make check` no Mac, empacotar no CI, e no seu Mac `jinga scripts/compila.pr mac --instala` + o LSP no VS Code. Estado atual em 9.10. |
| 3 | Este md (`JINGA-TUDO.md`, raiz; no `.gitignore` em 4aedf8b, versionado desde que o repositório é privado) | Montado em 2026-09-28. Atualizar no fim com o Windows. |
| 4 | Windows nativo (MSYS2 UCRT64, gcc ou clang; MSVC não, por causa do goto computado) | **Começou em `3c245e7`** (primeira parte, compilando cruzado com MinGW); o resto segue sem commit na árvore de trabalho (9.10). |
| 5 | Release com os três artefatos: `jinga-portable.tar.gz`, `jinga-macos-x86_64.tar.gz`, `jinga-windows-x86_64.zip` | Não começou. |

As 2 falhas da suíte no macOS (anotação do job `mac`, commit 4aedf8b). A anotação termina no grupo `linguagem`; os grupos seguintes e o resumo da rodada não aparecem nela.

1. Grupo `crash`, caso "recursao via map dentro de async e RecursionError na pilha menor da fibra": a VM morreu com sinal 10, que no macOS é SIGBUS.
2. Grupo `linguagem`, caso "close() acusa o que nao conseguiu gravar": o caso abre `/dev/full`, que é dispositivo do Linux. No macOS sai `PermissionError: [Errno 13]` no lugar do esperado `OSError: [Errno 28] No space left on device`.

Depois disso: `ec711c7` corrigiu a primeira; `3c245e7` passou os 6 casos do `/dev/full` a valer só no Linux e reescreveu os 2 casos de processo filho que olhavam o `/proc`. No `b180807` o Mac ainda reprova em "processo filho nao herda os descritores abertos do motor" (5.12 e 9.10).

Regra do plano pra esse tipo de caso: diferença real do sistema ganha a expectativa própria daquele sistema, e nenhum caso é pulado.

O que a Parte 4 (Windows) exige, pelo plano:

- backends da camada de sistema: eventos com WSAPoll e socket de despertar; Fibers; VirtualAlloc/VirtualProtect; CreateProcess e ConPTY; Winsock (`closesocket`, `ioctlsocket`, timeout em DWORD ms, par por loopback); Console API com VT; `GetModuleFileNameW`; `USERPROFILE`; `BCryptGenRandom`; certificados do repositório do Windows; `memmem`/`strcasestr` próprios;
- JIT Win64: os 3 pontos de ABI em `vm/ps_jit_x64.h` passam a usar rcx/rdx/r8, 32 bytes de sombra, rdi/rsi preservados;
- gmp: conversão int64 por `mpz_import`/`mpz_export`, porque `long` tem 32 bits no Windows;
- jinker com workers: processo filho com o socket duplicado (`WSADuplicateSocket`) no lugar do `fork`; reload por CreateProcess;
- bancos: `LoadLibrary` de `libpq.dll`, `libmariadb.dll`, `odbc32.dll`, `libmongoc-1.0.dll`, com as DLLs dentro do zip;
- editor: `jinga-lsp.cmd`; `extension.js` e o plugin do IntelliJ procurando `.cmd`/`.exe` e `%USERPROFILE%`; `server.js` com URI `file:///C:/...`;
- suíte: `teste/ps_teste.c` com CreateProcess. Bug do porte se corrige; diferença real do SO (`cmd.exe`, `\r\n`) ganha a expectativa do Windows;
- `instalar.ps1` e o job `windows-latest` no `build.yml`. Nenhum dos dois existe hoje.

Verificação pedida no plano, a cada parte:

- Linux: `make check`, `check-asan` e `oom` na VPS, mais `perf stat` do laço e do `--check` contra o 16.1.4.
- Mac: job com `make check` inteiro.
- Windows: suíte inteira, e `jinga.exe -o` gerando um `.exe` que roda.
- Versão: **minor** por parte entregue, com bump, install e reinício do servidor do editor em cada commit.

### 6.4 Trilha de dev: a ordem que você deu (2026-09-26)

Ordem: **8 -> 5 -> 1 -> 3**.

1. **Item 8: bcrypt na lib `hash`** (anotado em 2026-09-22).
   - Hoje `hash.crypt`/`hash.check` fazem base64(sal[32] + PBKDF2-SHA256 com 310000 iterações). Não há bcrypt no motor, nem Blowfish/EksBlowfish.
   - O lote: bcrypt em C sem lib externa, formato `$2b$<custo>$<22 sal><31 hash>` compatível com hash gerado em outro lugar; doc em `docs/hash/`; teste com vetores conhecidos.
   - São **design seu**, perguntados no plano: o nome da função, o parâmetro de custo, e se o `check` reconhece os dois formatos pelo prefixo `$2`.
2. **Item 5: PoolIp do jinker persistindo os IPs bloqueados entre reinícios** (2026-09-19).
   - Você disse: "o jinker poolip não tem nenhum sistema de hash no qual guarda os ips após restart". Hoje contagem e bloqueio vivem só em memória (`oauth={"poolip": true, "rate": N, "bloq": dias}`, doc em `docs/objetos-internos/PoolIp.md`).
   - O desenho é **perguntado a você no plano**: onde guardar, formato, expiração pelos `bloq` dias, o que a doc promete.
3. **Item 1: callbacks da IUP** (a lib gráfica em C da PUC-Rio, 2026-09-15).
   - Opcodes e encanamento pra uma `funct` Jinga servir de callback das funções da IUP, com o C chamando de volta a VM.
   - A doc da IUP fica com você ("eu me viro na docs deles"). O motor fica com o assistente.
4. **Item 3: o resto do caminho quente.**
   - Feito: goto calculado, código de máquina x86-64, chamada direta nativo->nativo.
   - Falta: cache de resolução de método por sítio de chamada; campo de instância sem passar por dict (`self.n` gasta dois `acha_slot` por volta, 7,6% + 6,3% do perfil de `p.mais()`).
   - Do JIT, ainda não feito: builtin chamado de código nativo segue como "ilha"; float em linha; membro e coleção passando por helper.
   - O lote entra com número antes e depois (`perf stat`), nunca "ficou mais rápido".

Itens da trilha já fechados: 2 (Pyrite) foi superado pelo rename pra Jinga; 4 (`public`/`private`), 6 (IntelliJ) e 7 (regressão do LSP) estão feitos. Detalhes em 6.6.

### 6.5 A fazer, sem posição dada por você na fila

- **Revisão adversarial do `model valida dados`** (commit 384d27c, revisão de 2026-09-20).
  - Você mandou guardar e corrigir depois, e não se pergunta de novo se é pra corrigir: é o próximo lote do `model`.
  - Resultado completo, com repro e saída de cada achado: `~/.claude/projects/-home-kleberdevion-poolscript-lang/06928479-1e0f-496c-858d-80747c7afcb6/subagents/workflows/wf_1ce76a2c-1d5/journal.jsonl`. Esse arquivo vai junto pro Mac.
  - Em 2026-09-28, `acha_model_por_nome` segue em `vm/jinga_vm.c:9753` e o `"%.60s.%.60s"` em `vm/jinga_vm.c:9839`. Os achados 1 e 4 seguem abertos.
  - Os 14 confirmados:
    1. [alta] Model aninhado é resolvido só pelo NOME na tabela única da VM: um `E` do programa principal substitui o `E` do módulo dentro do model importado.
    2. [alta] Literais int de `in`/`not_in`/`min`/`max` passam por double. Acima de 2^53 o valor gravado é outro, e INT64_MAX vira lixo.
    3. [alta] `int(length=N)`: flo >= 2^63 e INT64_MIN passam com 1 dígito.
    4. [alta] Caminho do campo no 422 sai corrompido com prefixo acima de 60 caracteres: some nível do meio, some `[1]`, sai `..`.
    5. [média] `from m import *` liga o model como valor mas não como tipo de campo (`e: E` dá "tipo desconhecido").
    6. [média] NUL dentro do literal de regex ou `in`: a compilação valida um valor e o runtime usa outro.
    7. [média] Regex que estoura o teto de passos vira "nao casa com o padrao", mesmo pra texto que casa.
    8. [média] `num_texto` com `%g`: "no maximo 10, veio 10" pra 10.000001; "veio 1.23457e+18".
    9. [baixa] `of=list/tup/char/Object` diz "tipo desconhecido" pra tipo conhecido.
    10. [baixa] Erro de `length` em tipo errado aponta a coluna do campo, não a do parâmetro.
    11. [baixa] `min=+1` é recusado e `min=-1` é aceito.
    12. [baixa] Model recursivo: a partir de 35 dicts aninhados, dado válido dá False ou 422 "aninhamento fundo demais", limite não documentado.
    13. [baixa] `valor_curto` corta em 100 bytes e parte o UTF-8.
    14. [baixa] Lista do `in` no 422 cortada em cerca de 200 bytes, com vírgula pendurada e sem `]`.
  - Os 13 sem verificação se medem antes de corrigir:
    - regex com NUL: o `--check` aprova e o programa falha com "sem memoria";
    - `==` contra model com campo vindo de import que ainda não rodou devolve False;
    - regex com mais de 120 caracteres sai cortado no 422;
    - hover do model: `min=undefined` pra 1e999, `max=2.0` vira `max=2`, `length=0` some, `Null` no `in` vira `…`;
    - testes que faltam: "nao pode ser Null", `()` vazio e vírgula final, model importado como tipo de campo, `in=[x]` com variável, e o comentário do bloco 17 do `teste_servidor.js`.
- **Self-hosting** (compromisso seu de 2026-08-24).
  - Lexer, parser e avaliador da Jinga escritos em Jinga. Toda limitação encontrada no caminho é corrigida no motor, nunca contornada.
  - Ordem: lexer; parser -> AST; avaliador (expressões, statements, funções e escopo, Entity e decoradores); por fim, diferencial rodando os `.pr` da suíte.
  - Você disse "depois": começa quando você mandar.
- **API de interface gráfica** (nota `memory/*-sobre-guzer.md`).
  - É conversa, não tarefa aprovada.
  - A guzer foi removida inteira em 2026-09-05 (fa08661, por ordem sua). Se o assunto voltar, é do zero e decisão sua.
  - O que você reclamou dela vale pra qualquer UI futura: contêiner com tamanho fixo que não usava a largura da janela; redimensionar não gerava evento; teste que só conferia `type()` e nunca media geometria.
- **Transpilador**: ideia sua, e combinamos que vem por último.
- **`swagger.pr` fica fora do repositório**: só existe em `~/.jinga/libs/swagger.pr`, e o pacote e o instalador não o levam. Na troca de máquina, copiar `~/.jinga/libs` inteiro. A lib é sua.
- **`docs/builtins/load/load.md`** diz "Retorno: bool", e o motor devolve `dict` (medido em 2026-09-28: `type(load(...))` dá `dict`). A nota `cobertura-benchmark-e-doc` registra a divergência como não corrigida por decisão sua.
- **IntelliJ**: ação sua registrada em 2026-09-19. Apagar o cadastro manual "PoolScript" em Settings > Language Servers, reiniciar o IDEA e conferir na aba Traces. A nota não registra se foi feito.
- **`make vsix` exige Node >= 22**. Este laptop tem Node 18.19.1, e a VPS tem Node 22.
- **Achado desta varredura** (2026-09-28, uma linha, sem mexer): variável criada dentro de `if` e usada depois do bloco passa no `jinga --check` (`{"ok":true}`) e, rodando, dá `NameError: name 'y' is not defined`.

### 6.6 Feito recente (2026-09-15 a 2026-09-28)

| Data | Versão | Commit | O que |
|---|---|---|---|
| 09-28 | - | b180807 | `examples/notify.pr` (3 linhas a mais) |
| 09-28 | 16.1.9 | 3c245e7 | Windows (base) e macOS: sockets, TLS e bibliotecas pela camada de sistema; `vm/ps_win.h`; casos só do Linux (campo `so`) |
| 09-28 | 16.2.0 | ver `git log` | Windows nativo completo no código (processos, ConPTY, fibras, JIT Win64, jinker sem fork, gmp int64, pacote, `instalar.ps1`, job `windows`); vetores da fibra sob demanda (923 MB -> 243 MB); este md versionado (repositório privado) |
| 09-29 | 16.2.1 | dd368eb | Windows compila no CI (gcc 16 e cabeçalhos novos do UCRT64: `strndup`, `__USE_MINGW_ANSI_STDIO`) |
| 09-29 | 16.2.2 | ver `git log` | As 20 falhas do 1º run do Windows (8516/8536): `_isatty` do NUL, casos com sintaxe de `sh` no `os.cmd`, `/dev/full`, url do sqlite, pty; bugs achados no caminho: `os.rmdir(force)` seguia link e apagava o destino, `os.copy` apagava link, `os.move` dizia ok sem apagar a origem, `rename` do Windows não substituía, `--check` dava MemoryError na borda do aninhamento; membro ausente numa chamada diz "method" e sugere a primeira palavra (`send` -> `send_linux`) |
| 09-30 | 16.2.3 | ver `git log` | O `make check` inteiro portado pra macOS e Windows (os drivers sobem e derrubam processo pelo `Process`, sem shell; pastas de trabalho relativas); o CI Linux (`ci.yml`) vermelho desde 26/09 (atalho do `jpkg` testava o `jinga` instalado, fibra pequena demais sob ASan, erro do `lcov` escondido); motor no Windows: caminho `C:\` absoluto, separador `\` (`vm/ps_caminho.h`), `realpath` que não resolvia link (o static do jinker servia arquivo de fora), erro do Winsock no connect com prazo, aspas pro metacaractere do `cmd.exe` no `os.run`; `delete()` que não apagava dizia True; o F5 do VS Code não depurava (a sonda da extensão gastava a única conexão); `attach(pasta)` com o mesmo erro nos 3 sistemas; verdes falsos fechados em `unidade`, `audita_c`, `audita_exemplos_doc`, `conserta_barra_doc`, `e2e_roda` e `leis` |
| 09-30 | 16.2.4 | ver `git log` | O que o CI do 16.2.3 achou: a nota do `private` no traceback saía com lixo da pilha em todo erro de runtime (as três entradas do motor limpavam o erro campo a campo e esqueciam a nota; no Linux a pilha vinha zerada por acaso, no Mac não); o mongo no Windows não achava as `bson_*` (o `GetProcAddress` só olha a própria DLL; agora procura também nas que ela importa, como o `dlsym`); `make cobertura` com contador atômico (as threads do pool deixavam contador negativo e o `lcov` recusava a medição) |
| 10-01 | 16.2.5 | ver `git log` | O que o CI do 16.2.4 achou: no macOS a conexão aceita pelo jinker herdava o não-bloqueante da escuta (o BSD herda, o Linux não; só o Windows desligava), e a leitura do pedido do WebSocket e o handshake do TLS, feitos na thread do pool, voltavam EAGAIN e fechavam a conexão (10 falhas do `jinker_roda`); o `-fanalyzer` do gcc 16 (Windows) acusa um falso "leak of 'nv'" no lexer (aceito com o motivo); o `make analisa ANALISA_TUDO=1` do noturno nunca rodou desde 26/08 (um `;` sozinho na receita), e com a receita certa o teto de 12000 MB passava da RAM do runner privado (7,9 GB) e o derrubava: agora 6000; o fuzzer não compilava desde que o `ps_verifica_fonte` ganhou os avisos |
| 10-05 | 16.2.6 | ver `git log` | Etapa 0 do plano "JIT nunca pior que o interpretador": a chamada direta entre functs nativas (`jit_chama`) passa a ser ponto seguro do GC, como a entrada do CALL no interpretador. Uma recursão que aloca não coletava no código de máquina: `g(22)` com uma lista por chamada ia a 2045 page-faults, contra 609 agora (98,6 M -> 82,9 M instruções). A conferência custa 3 instruções por chamada (fib +0,74%, chamada +0,57%). `JINGA_GC_LOG=1` diz cada coleta no stderr, e o teste exige o mesmo número de coletas com e sem JIT (sem a correção: 1 contra 2) |
| 10-05 | 16.2.7 | ver `git log` | Regras do front-end: (1) só `True`, `False` e `Null`: `true`, `false`, `null`, `None` e `none` são nomes comuns, e usar um deles dá NameError antes de rodar com a dica pelo "Did you mean" (`True`/`False`/`Null` entraram nos candidatos); migrador `scripts/migra_literais.pr` (por token; `.pr`, blocos da doc e programas dos casos em C; 1465 trocas no repositório); mensagens e assinaturas do motor que mostravam a forma antiga (`pty=true`, `keepends=false`, `slice indices must be integers or None`, `nao pode ser null`, o 422 do model) passaram pra nova; (2) `return` e `yield` fora de funct são SyntaxError antes de rodar (o `return` no topo encerrava o programa calado, com rc 0 e `--check` ok); (3) `json.parse` sem `import json` passava no `--check` (o `json` também é nome de tipo) e quebrava rodando: agora é NameError no check; `tipos_apelidos` do `--metadata` sai do `ps_tipos.def` (a cópia digitada esquecia `json`, `JSON`, `object`, `Long`); (4) editor: os 27 builtins sem página de palavra-chave (`len`, `range`, `sum`…), `json`, `PoolFile` e `byte` diziam "rodando dá NameError" no hover, e `str`/`list`/`dict`/`tup` saíam como `class`: agora o builtin mostra a assinatura e a página, o tipo primitivo diz `tipo primitivo` e o tipo do motor `tipo nativo`; `class` só a classe escrita em Jinga |
| 10-05 | 16.2.8 | ver `git log` | `static Null funct main()` dava "name 'static' is not defined": o `Null` não era aceito como tipo de retorno e a linha se partia calada em três comandos (`static`, `Null` e a funct sem tipo), com o erro longe da causa. (1) `Null funct` é a funct que não devolve valor (decisão sua): `return` seco ou `return Null`, o fim devolve `Null`, devolver valor é erro ("retorno de f() esperava Null, recebeu int", antes de rodar ou rodando), e o `post` de qualquer coisa vale nela; junta com `static`, `async`, `private` e método. (2) Dois comandos na mesma linha exigem `;`: sem ele é SyntaxError no começo do segundo, em vez de partir a linha calado. Vale também entre membros de classe (campo depois de campo passava). 30 pontos colados em 4 arquivos ganharam `;`. (3) O erro cai na CAUSA, nunca num `;` que não conserta: a sonda achou 13 regressões da primeira versão da regra e todas viraram caso. Nome solto seguido de comando (`retrun x`, `whlie i < 3 {`, `const int x = 1`) é NameError no nome, com as palavras da linguagem entre os candidatos do "Did you mean"; `Conta c` sem valor é "declaracao de variavel exige '='"; `else`/`elif`/`catch`/`finally`/`case`/`each` fora do lugar dizem a construção a que pertencem (e o `else` depois do guard diz que o guard não tem `else`); `x if c` sem `else` é "faltou 'else' no ternario"; `int|str u = 1` é "tipo uniao nao vale na declaracao", no `|`; `Null` como tipo de variável, parâmetro ou campo é "'Null' so e tipo no retorno de funct". Um só lugar lê a cabeça de funct (`le_cabeca`): dois tipos de retorno (`Null int funct`, o `Null` sumia calado), modificador repetido (`static static`, `public private`) e modificador digitado errado (`pirvate` -> 'private', `funtc` -> 'funct') são erro no segundo/no errado; o "faltou 'funct'" conhece todos os prefixos (`static Null f()`, `nonnull f(x)`, `Ponto f()`), não só `async` e tipo da tabela; decorador e método na mesma linha dentro da classe voltaram a ser uma declaração; `str private name = n` (modificador depois do tipo) vale dentro do construtor. O "Did you mean" passou a contar letras invertidas como uma edição (`riase` dava 'case', `fro` dava 'flo') e a cobrar o teto pela palavra mais curta (`Conta` sugeria 'not'). (4) O arquivo dele com `while x < 10.000.000.000` e o `{` na linha de baixo: o número com ponto de milhar virava `10.000` + `.000` ("esperado nome do membro apos '.'"), e o `{` órfão do cabeçalho quebrado virava DICIONÁRIO, com "faltou ':' no dicionario" em cima de `x++;`. Agora o lexer diz "numero nao usa '.' como separador de milhar: escreva 10000000000 (ou 10_000_000_000)" e marca o lugar (T_SINC), e a recuperação do parser pula o bloco junto com o cabeçalho quebrado: um erro só, no número |
| 09-28 | 16.1.8 | ec711c7 | Folga da pilha medida rodando (o clang põe o dobro de pilha no laço da VM); nome do sinal pelo `strsignal` no runner |
| 09-28 | 16.1.7 | c669a78 | macOS: resolução de nomes do módulo `sockets` pela camada de sistema |
| 09-28 | 16.1.6 | 6980c40 | Pacote macOS, `instalar.sh` com ramo Darwin, `build.yml` (Linux + macOS Intel) |
| 09-28 | - | 89e32d7 | `scripts/compila.pr` e `scripts/ci_anota.sh` |
| 09-28 | 16.1.5 | 529c518 | Camada de sistema `vm/ps_so`: o motor deixa de chamar epoll, eventfd e `/proc` direto |
| 09-28 | 16.1.4 | b51dfd7 | NameError antes de rodar sugere o nome parecido ("Did you mean"). Regra geral, sem tabela. |
| 09-28 | - | a110fa4, 4aedf8b | Sobra `path/docs` do exemplo swagger sai do repo; `JINGA-TUDO.md` no `.gitignore` |
| 09-26 | 16.1.0-16.1.3 | 14d677d, db9d839, 1ab6c92 | `private`/`public` em qualquer declaração do módulo; acesso a private acusado antes de rodar, com a declaração no traceback (trilha 4) |
| 09-26 | 16.1.1 | a738d8f | Nome legado `action` sai do motor e do editor: o nó é `FunctDecl` |
| 09-26 | 16.0.1-16.0.3 | 612dabe, 641afbb, 7dea716 | Editor: `type` fora do completion; escopo inteiro dentro de chamada junto com os `nome=` (trilha 7c); enum no hover |
| 09-26 | 16.0.0 | 262f2cb | Major pela quebra de compatibilidade da classe sem `()` |
| 09-26 | 15.95.2-15.95.4 | a7f574e, e810f58, be63174 | Editor tipa toda variável pela expressão; hover rico (classe do valor, Entity do arquivo, módulo) |
| 09-26 | 15.95.1 | 2a695d3 | Logo no desktop: arco do J corrigido |
| 09-26 | 15.95.0 | 5b929cc | Classe sem `()`; `base(Pai).membro`; `s[i]` linear (cache de posição) |
| 09-25 | 15.94.3-15.94.4 | 3d82c5b, 9c1f4be | Chamada direta entre functs em código de máquina (fib(30): 549 M -> 267 M ciclos); depurador |
| 09-24 | 15.94.0-15.94.2 | 03d4769, 8b0409e, 8db868c | JIT x86-64 na 1ª execução de cada funct (laço tipado a 8,7 ciclos por volta) |
| 09-24 | 15.93.1-15.93.6 | 6b372a5 .. 4f2988c | Fase 1: `jinga --bytecode`, goto calculado, laço tipado de 12 pra 5 instruções, hash de string preguiçoso (texto 19,6 s -> 0,31 s) |
| 09-24 | 15.93.0 | 066eff7 | `cors(app, ..., local=False)`, `Origin` valendo no WebSocket |
| 09-24 | 15.92.0-15.92.1 | 0d01921, 92c0ec4 | A linguagem passa a se chamar Jinga; logo gerada em Jinga |
| 09-24 | 15.91.33 | b72a0dc | Compila direto do fonte em 2 passadas (200 mil functs: 612 MB -> 353 MB, 16 s -> 5,9 s) |
| 09-23 | 15.91.29-15.91.32 | 12583b8, 53256ee, c891d53, 3e52b52 | Servidor não morre mais pelo epoll; `--utf16`; parêntese esquecido não engole o arquivo; partida 5x mais rápida |
| 09-22 | 15.91.22-15.91.28 | 016279b, 1d7fa74, 7460735, 4884403, 4888584, 6b3330d, c6800c7 | Nenhum emoji no repo e `mail` sem tabela de provedores; VS Code sobe o LSP instalado (trilha 7a); CI com o libpq certo; comandos de editor; `os.run` com terminal; WebSocket não congela; f-string vira árvore |
| 09-21 | 15.91.21 | 7d734a9 | Cobertura volta a rodar. Na VPS, com os bancos: linhas 83,7%, funções 90,5%, ramos 60,1%. |
| 09-20 | 15.91.15-15.91.20 | e4534fc, 5e1c75c, ee4b23b, 5713b17, 384d27c, 8a0aad5 | IntelliJ declara o servidor LSP (trilha 6); `cors(app, ...)`; roteador; `static` visível na classe; `model` valida dados; "Did you mean" em membro |
| 09-19 | 15.91.10-15.91.14 | d9fd855, 6c044ab, 81b734e, 24185bd, 6d91e0c | Chave de dict é expressão; `-o` leva os módulos; `fromAddress`/`toAddress`; tipagem regra 5 (assinatura de `.pr` importado); `/<id>` é um segmento |
| 09-16/17 | 15.91.5-15.91.9 | ab7825c, 2ac2e67, 17e3409, fc1bd8c, bf7e8fb | Tipagem estática F0-F2; método sem `self` acusado na declaração |
| 09-16 | - | 88f4943 | Extensão `.pr`, com migrador |

O roadmap que você aprovou em 2026-09-05 está fechado nos quatro itens:

- `assert` (`docs/builtins/assert/assert.md`);
- cookie e sessão no jinker (d68b166, doc em b79fc94);
- `model=` na rota com 422 (3d73a3f);
- depurador DAP com gráfico de execução (d1d3416, `docs/debugger.md` seção 5).

A métrica de cobertura da doc passou a medir "documentado em algum lugar" em vez de "um arquivo por membro" (05ec920). A base atual está em `teste/cobertura_doc_base.txt`: 522.

### 6.7 Decisões tomadas: não se propõem de novo

**Linguagem**

- I10: `is` como no Python.
- I11: `//` é divisão inteira, junto com `**` e `pow`; comentário só com `#`.
- I18: path param injetado no argumento do handler quando o dev quiser.
- I19: ficam `sqlite3`, `manpu`, `psodbc`, `qrcode` e `request`. Saem os apelidos `db`, `mp`, `qr`, `sqlite` e o plural de `request`.
- I22: `True`, `False` e `Null` saem com inicial maiúscula (medido: `True False Null`).
- Null é igual só a Null (I1/I2/I3, 28/08).
- `__name__` vale `"main"` no programa principal. O guard só é reconhecido com o literal `"main"`.
- Tipagem estática, aprovada em 2026-09-16:
  - tudo que tem tipo é conferido antes de rodar;
  - variável sem tipo fica com o tipo da 1ª atribuição;
  - funct tipada só devolve e só dá saída do seu tipo (medido: `post(1)` em `str funct` dá AttributedValueError). `int funct` com erro vira 500 e sem return vira 0; `bool funct` com erro vira False e sem return vira True;
  - chamada conferida, inclusive de `.pr` importado;
  - nome nunca ligado dá NameError antes de rodar;
  - `--check` lista todos os erros, e programa com erro de tipo não roda (rc 2).
- Sem conversão implícita. `char c = 64` e `long y = 5` ficam como estão.
- Chave do literal de dict é expressão.
- Apelidos de tipo (`String`) **não** viram chamáveis. Conversão é só `str(x)`, `int(x)` e afins.
- `class X {` sem `()`, e `()` vazio é erro. `model X() {` fica com `()`.
  - `base()` com um pai só e `base(Pai)` com qualquer ancestral alcançam tudo do pai.
  - `base()` com dois ou mais pais é erro.
  - `base(Pai).x = v` é erro.
  - A forma antiga `base(x)` é erro que aponta a nova.
- `static` (campo e método, próprio ou herdado) é alcançado pelo nome solto de dentro da classe. A keyword `here` foi rejeitada.
- `private`/`public` valem em qualquer declaração do módulo. `private` fora do topo ou de Entity é erro.
- `dict.value()` existe, e método ganha de chave (`d.value` é o método).
- `regex.match` casa a string inteira. `regex.compile` devolve `Pattern`.
- PoolFile: `save`, `move`, `copy` e `delete` são **métodos do objeto arquivo**, não de lib.
- Lib nova: você escreve o design em `.pr` primeiro, e o assistente implementa só quando você mandar. O design de áudio (`vct.ps`) não existe mais no disco: pedir o atual se o assunto voltar.
- jwt só com HMAC (HS). Token com `alg` `none` ou `RS256` é recusado (decisão de escopo sua, 2026-07-30).

**jinker**

- `cors(app, ...)`: o servidor é o primeiro argumento, que é variádico; sem app é TypeError; a config é por servidor.
- `cors(..., local=False)` liga o modo estrito: só a lista vale, e pedido sem `Origin` recebe 403 no HTTP e no WebSocket. **Sem `cors()`, fica como era.**
- Multi-processo com salas WS entre processos precisa de backplane e não fecha 100%. Não se começa.
- O modelo de concorrência é fibras cooperativas + offload da I/O em thread. Uma trava global foi descartada.

**Nome, comandos e distribuição**

- A linguagem é Jinga.
  - Comandos: `jinga` (roda), `jpkg` (pacotes), `jinga-lsp`.
  - Nomes antigos continuam como compatibilidade e **nunca se removem**: `pool`, `psl`, `poolscript-lsp`, `~/.poolscript` (migrado pra `~/.jinga` pelo `jpkg`), `POOLSCRIPT_HOME`, `import poolscript.libs.X`.
  - A extensão é só `.pr`.
  - O repo segue `kleberDevion/poolscript`, e a pasta local segue `~/poolscript-lang`.
- Um motor só pros três sistemas. clang é compilador, não segundo motor. O JIT segue x86-64. MSVC não entra.
- O módulo `sockets` expõe só as constantes que o sistema oferece.
- No Windows, nenhum caso da suíte é pulado.
- Versão: base 100, rótulo pelo conteúdo, extensão com versão própria.

**Doc e testes**

- Não fragmentar doc em `.md` por membro pequeno.

### 6.8 O que as notas antigas dizem e não vale mais

- O interpretador em Python não existe mais: `src/poolscript/`, `tests/test_*.py`, `pyproject.toml` e o `bump_version.py` em Python sumiram. Com eles caem a "paridade interp x VM", a matriz diferencial de 2026-08-24 (1048 divergências em 2198 casos) e o gap do caret de coluna entre os dois motores. Hoje a VM em C é testada direto (`./testar`, `teste/casos_*.c`).
- `notas/` foi apagado por você em 2026-09-05: `notas/LIMITACOES.md`, `notas/DECISOES-API.md` e `notas/CLAUDE.md` não existem. As decisões sobreviveram em `memory/decisoes-api-ja-respondidas.md`.
- A guzer foi removida (fa08661). O `import guzer` responde ImportError.
- Pyrite foi superado pelo rename pra Jinga.
- "Fase 2 (JIT x86-64) pendente", no índice `MEMORY.md`: o JIT v1 entrou em 15.94.0 e a chamada direta em 15.94.3.
- "Funct tipada fica como tá" (2026-09-09): superado pela tipagem estática. A regra está ativa (medido).
- "Chave de dict sem aspas é texto": superado em d9fd855.
- "`sys.argv` não traz o nome do script": medido o contrário, `argv[0]` é o caminho do script.
- "Indexar `s[i]` em laço é O(n²)": corrigido em 5b929cc.
- "Doc em 131/514 membros com uma página por membro": a métrica mudou (base 522).
- "Sem `~/.gitconfig`": existe desde 2026-09-27.
- `vm/poolscript_vm.c` agora é `vm/jinga_vm.c`. Os binds pro Python (`ps_*_bind.c`) não existem mais.
- `rebuild_vm.sh` segue no repo como rebuild limpo (`make -s limpa` + `make`). Não há mais extensão Python.
- A senha do sudo `kl3ber` (nota `vm-c-port-status`, 2026-08-22) não vale mais: a atual é a da seção 7.1.

## 7. Máquina, contas e credenciais

Tudo abaixo foi levantado em 2026-09-28 lendo os arquivos e rodando comandos nesta máquina. Chave privada e token aparecem só pelo caminho. O conteúdo fica nos arquivos. Senha de teste e a senha do sudo deste laptop estão escritas porque ele pediu.

### 7.1 Este laptop (o que sai)

| item | valor medido |
|---|---|
| Sistema | Linux Mint 22.3 (Zena), pacotes da base Ubuntu 24.04 |
| Kernel | 7.0.0-30-generic, x86_64 |
| glibc | 2.39 (`Ubuntu GLIBC 2.39-0ubuntu8.8`) |
| CPU | Intel Core i3-1115G4 @ 3,00 GHz, `nproc` = 4 |
| RAM | 7,5 GiB (`free -h`) |
| Disco | NVMe de 234 GB, 118 GB usados (54%) |
| Usuário | `kleberdevion`, home `/home/kleberdevion` |
| Senha do sudo | `dormenis` (conferida com `sudo -S` em 2026-09-28; a nota de 2026-08-22 registra `kl3ber`, que não vale mais) |

A regra da "máquina sensível" nasceu nesta máquina (nota `maquina-sensivel-drivers.md`). O load chegou a 21 com o IDEA indexando. O `make check` inteiro foi morto pelo OOM killer 3 vezes em 2026-09-04 (exit 137). Por isso as rodadas usam `timeout` e `nice -n 19`, um job pesado por vez, e a suíte pesada vai pra VPS (7.6).

### 7.2 O Mac (o que entra)

Dados que ele passou:

| item | valor |
|---|---|
| Modelo | MacBook Pro 15" 2019 |
| CPU | Intel Core i9-9880H (Coffee Lake), 8 núcleos / 16 threads, 2,3 GHz base, turbo até 4,8 GHz, L3 de 16 MB |
| RAM | 16 GB DDR4 2400 MHz, soldada |
| Disco | SSD PCIe de 512 GB, soldado |
| Vídeo | AMD Radeon Pro 560X 4 GB GDDR5 + Intel UHD Graphics 630 |
| Segurança | chip Apple T2 |
| Sistema | macOS 15.7.4 |
| Arquitetura | x86_64 (Intel) |

O que muda por causa do Mac:
- O `instalar.sh` baixa no macOS o pacote `jinga-macos-x86_64.tar.gz` (linha 65), que é a arquitetura deste Mac.
- No macOS a home fica em `/Users/<usuário>`, e não em `/home/kleberdevion`. Todo caminho absoluto com `/home/kleberdevion` quebra lá (lista em 8.6).
- O shell padrão do macOS é o zsh. O PATH que aqui está em `~/.bashrc` e `~/.profile` vai para o `~/.zshrc`.
- A pasta do assistente leva o caminho do projeto no nome. Aqui ela é `~/.claude/projects/-home-kleberdevion-poolscript-lang/`: dentro dela ficam as sessões salvas (os `.jsonl`) e a memória (`memory/`, com 75 arquivos, e `MEMORY.md` é o índice). No Mac, com o repo em `/Users/<usuário>/poolscript-lang`, o nome passa a ser `-Users-<usuário>-poolscript-lang`. Com a cópia inteira do `~/.claude` levada pro Mac, basta renomear:
  ```bash
  mv ~/.claude/projects/-home-kleberdevion-poolscript-lang ~/.claude/projects/-Users-<usuário>-poolscript-lang
  ```
  Aí `claude --resume` de dentro do repositório lista as sessões antigas, e a memória carrega sozinha.

### 7.3 Contas

| conta | identificador | onde entra |
|---|---|---|
| GitHub (pessoal) | `kleberDevion` | repo da Jinga e projetos pessoais |
| E-mail da conta | `kleberdevion@proton.me` | é também o comentário da chave `github_tecsol` |
| Identidade dos commits | `kleberDevion <klebersantanadeoliveira07@gmail.com>` | `~/.gitconfig` |
| Organização da Tecsol no GitHub | `Tecsol-Engenharia` | repos `egn-tecsol`, `tecsol-link`, `lp_avaliacao`, `lp_vendedores`, `st-tc`, `tp-app` |
| Outros donos de repo usados aqui | `davifcouto` (`tecsolgeral`), `pablogineli` (`cdntv-api-central`) | ver seção 8 |
| Publisher do VS Code Marketplace | `KleberSantanadeOliveira` (campo `publisher` de `editor/vscode/package.json`) | extensão `jinga` |
| Nome do assistente pra ele | "Regressão", trailer `Co-Authored-By: Regressão <noreply@anthropic.com>` | commits (nota `user-nome-regressao.md`) |

### 7.4 Git e GitHub

- `~/.gitconfig` tem só `[user]` com `name = kleberDevion` e `email = klebersantanadeoliveira07@gmail.com`. Não há `credential.helper` nem `insteadOf`, nem no `~/.gitconfig` nem no `.git/config` do repo.
- Quando o `~/.gitconfig` some (aconteceu em 2026-09-19), o commit sai com:
  ```bash
  git -c user.name=kleberDevion -c user.email=klebersantanadeoliveira07@gmail.com commit ...
  ```
- `gh` 2.101.0: a conta `kleberDevion` está ativa, mas o `gh auth status` responde `The token in keyring is invalid`. Em `~/.config/gh/config.yml` está `git_protocol: https`. Em `~/.config/gh/hosts.yml` está `user: kleberDevion`, e o token fica no keyring.
- O SSH do GitHub funciona. `ssh -T git@github.com` respondeu "Hi kleberDevion! You've successfully authenticated" em 2026-09-28, com a chave `~/.ssh/github_tecsol`.
- O remote do repo da Jinga é `origin https://github.com/kleberDevion/poolscript-lang`, que redireciona para o canônico `https://github.com/kleberDevion/poolscript` (**privado** desde 2026-09-28, branch `main`). Privado, tudo que lia o GitHub sem login para de funcionar: o `curl` do `instalar.sh`, o download do release e a leitura das anotações do CI pela API. Sem helper configurado, push por HTTPS depende do `gh auth`. Push por SSH exige o remote na forma SSH.
- Não há `~/.git-credentials`, `~/.netrc`, `~/.npmrc`, `~/.docker/config.json`, `~/.pgpass`, `~/.my.cnf` nem `~/.vsce`. Portanto não há token de npm, de registry do docker nem de publicação da extensão salvo nesta máquina.
- O `~/.gnupg` existe, mas `gpg --list-secret-keys` volta vazio. Não há chave GPG.

No Mac:
```bash
git config --global user.name kleberDevion
git config --global user.email klebersantanadeoliveira07@gmail.com
gh auth login -h github.com
ssh -T git@github.com
# push por SSH no repo da Jinga:
git remote set-url origin git@github.com:kleberDevion/poolscript.git
```

### 7.5 SSH

`~/.ssh/config` completo (não tem segredo):
```
Host tecsol-vps
    HostName 10.8.0.1
    User root
    IdentityFile ~/.ssh/vps_tecsol_hostinger
    IdentitiesOnly yes

# break-glass: IP publico direto (so funciona enquanto a 22 publica nao estiver fechada)
Host tecsol-vps-pub
    HostName 187.127.9.180
    User root
    IdentityFile ~/.ssh/vps_tecsol_hostinger
    IdentitiesOnly yes

Host github.com
    HostName github.com
    User git
    IdentityFile ~/.ssh/github_tecsol
    IdentitiesOnly yes
```

| arquivo | tipo | comentário da chave | uso |
|---|---|---|---|
| `~/.ssh/github_tecsol` (+ `.pub`) | ED25519 | `kleberdevion@proton.me` | GitHub (`Host github.com`) |
| `~/.ssh/vps_tecsol_hostinger` (+ `.pub`) | ED25519 | `claude-code-session` | VPS (`tecsol-vps` e `tecsol-vps-pub`) |
| `~/.ssh/id_ed25519` (+ `.pub`) | ED25519 | `cudegrilo` | nenhum `Host` do config aponta pra ela; é a chave padrão do ssh |
| `~/.ssh/known_hosts`, `known_hosts.old` | hosts conhecidos | — | evita a pergunta de confiança na primeira conexão |

Permissões aqui: a pasta tem `700`, as chaves privadas e o config têm `600`, e os `.pub` têm `644`.

No Mac:
```bash
mkdir -p ~/.ssh && chmod 700 ~/.ssh
# copiar: config, github_tecsol(.pub), vps_tecsol_hostinger(.pub), id_ed25519(.pub), known_hosts
chmod 600 ~/.ssh/config ~/.ssh/github_tecsol ~/.ssh/vps_tecsol_hostinger ~/.ssh/id_ed25519
chmod 644 ~/.ssh/*.pub
ssh -T git@github.com
```

### 7.6 VPN (WireGuard) e VPS `tecsol-vps`

**VPN**
- Conexão do NetworkManager `wg0`, tipo `wireguard`. Está ativa na interface `wg0` com `10.8.0.2/32` e tem `autoconnect = no`. Depois de reiniciar, sobe com `nmcli connection up wg0`.
- Arquivos: `~/.wg-tecsol/wg0.conf` e `~/.wg-tecsol/priv.key`. O `wg0.conf` já leva a `PrivateKey` dentro. Os campos são `Address`, `PrivateKey`, `PublicKey`, `Endpoint`, `AllowedIPs` e `PersistentKeepalive`.
- Valores sem segredo: `Address = 10.8.0.2/32`, `Endpoint = 187.127.9.180:51820`, `AllowedIPs = 10.8.0.1/32` (só a VPS passa pelo túnel), `PersistentKeepalive = 25`.
- `/etc/wireguard` só abre com sudo. `/etc/openvpn/client` e `/etc/openvpn/server` estão vazios: o OpenVPN não é usado, embora o `openvpn.service` esteja habilitado.
- No Mac: instale o app WireGuard (App Store), use "Import Tunnel(s) from File", escolha o `wg0.conf` e ative. Para testar: `ssh -o BatchMode=yes tecsol-vps true`.

**VPS**
- `ssh tecsol-vps` entra como root pelo túnel, em 10.8.0.1 (conferido em 2026-09-28: rc 0).
- `ssh tecsol-vps-pub` aponta pro IP público 187.127.9.180, porta 22 (o break-glass do config). Em 2026-09-28 não conectou (`Connection timed out`): a 22 pública está fechada no firewall (9.7).
- Medido em 2026-09-28: Ubuntu 24.04.4 LTS, 4 núcleos, 15 GiB de RAM, node v22.23.2. O usuário `ubuntu` tem uid 1000 e está no grupo `sudo`. `/dev/full` é nó de caractere `1, 7`.
- A mesma VPS roda os serviços da Tecsol. O `~/projetos/tecsol-logs.py` aponta pra 187.127.9.180, e o mapa de segredos da Tecsol está em `~/projetos/backups/TECSOL-TUDO.md`.

**Uso pra Jinga (nota `suite-na-vps.md`, desde 2026-09-20)**

Build, suíte inteira, ASan, `make oom` e e2e rodam na VPS. Localmente fica só o leve: `make pool`, `--check`, casos isolados e LSP pontual. O roteiro:

1. Na VPS, antes de tudo: `ls /tmp > /tmp/.ps-antes`.
2. Subir a árvore sem `.git`, `node_modules`, `dist`, `cob` e binários (`pool`, `pool-asan`, `pool-debug`, `pool-oom`, `testar`, `unidade`):
   ```bash
   tar czf - --exclude=.git --exclude=node_modules --exclude=dist --exclude=cob . \
     | ssh tecsol-vps 'mkdir -p /tmp/ps_ck_1 && tar xzf - -C /tmp/ps_ck_1'
   ```
3. Rodar como `nobody` (`chown -R nobody:nogroup` na pasta, `HOME` dentro dela, `runuser -u nobody`), nunca como root. Usar `nohup setsid` e deixar o log fora da pasta, porque o ssh pela VPN cai em rodada longa. Alvos: `make pool testar`, `npm ci` em `editor/vscode` (portão do LSP), `make check`, `make check-asan`, `make oom`.
4. Limpar no mesmo script:
   - apagar a pasta (`/tmp/ps_ck_*` ou `/tmp/ps-lote-N`);
   - apagar o que apareceu em `/tmp` desde a lista `.ps-antes` (`/tmp/ps_cli_teste`, `/tmp/ps_lsp_t...`);
   - `npm cache clean --force` e `rm -rf ~/.npm/_logs`.

   A regra é resíduo zero.

A pegadinha do `/dev/full` quando a suíte roda como root, e o conserto, estão em 5.11.

Além do que o `ci.yml` instala, a VPS também recebeu `pkg-config`, `lcov`, `postgresql-server-dev-16`, `libkrb5-dev` e `libmariadb-dev`. Neovim e IDEA não estão na VPS, então os portões deles pulam lá.

### 7.7 Bancos e serviços de teste (credenciais de teste)

| recurso | onde | host:porta | usuário | senha | banco |
|---|---|---|---|---|---|
| PostgreSQL | `teste/e2e/db.pr`; CI `postgres:16`; local: serviço `postgresql` 16.15 habilitado e ativo | 127.0.0.1:5432 | `ps_teste` | `ps_teste` | `ps_teste` |
| MariaDB/MySQL | `teste/e2e/db.pr`; CI `mariadb:10.11` (root: `ps_teste_raiz`); local: serviço `mariadb` 10.11.14 habilitado e ativo | 127.0.0.1:3306 | `ps_teste` | `ps_teste` | `ps_teste` |
| MongoDB (e2e) | `teste/e2e/mongo.pr`; CI `mongo:7` com `27099:27017` | 127.0.0.1:27099 | sem autenticação | — | `ps_teste` |
| MongoDB (autônomo) | `teste/mongo_roda.pr` sobe o próprio `mongod` em `/tmp/ps_mongo_teste` (no Windows, `ps_mongo_teste` na pasta corrente) | 127.0.0.1:28961 | sem autenticação | — | `ps_teste` |
| SMTP falso | `teste/e2e/mail_local.pr` + `teste/smtp_falso.py`, STARTTLS com certificado auto-assinado em `/tmp/ps_smtp_cert.pem` | 127.0.0.1, porta dada pelo sistema | `teste@local` | `segredo123` | — |
| Gmail real | `teste/e2e/mail.pr`, variáveis `MAIL_SMTP`, `MAIL_IMAP`, `MAIL_USER`, `MAIL_PASS` (senha de app) | — | — | — | sem as variáveis o teste diz PULOU |
| Isca do jinker | `teste/jinker_alvo.pr` grava `estatico/.env` com `SENHA=nao`; o servidor não pode servir esse arquivo | — | — | — | — |

Outros fatos sobre os bancos:
- Conferido em 2026-09-28: o login `ps_teste`/`ps_teste` funciona localmente no psql e no cliente mariadb.
- `mongod` 8.0.31 está instalado com o serviço desabilitado. O `mongosh` não está instalado.
- `PS_MAIL_TLS_INSEGURO=1` aceita certificado não confiável. O `mail_local.pr` prova que sem essa variável a conexão falha.

Criar usuário e banco (do `teste/e2e/LEIAME.md`, forma Linux):
```bash
sudo -u postgres psql -c "CREATE USER ps_teste WITH PASSWORD 'ps_teste';" \
                     -c "CREATE DATABASE ps_teste OWNER ps_teste;"
sudo mysql -e "CREATE DATABASE ps_teste;
               CREATE USER 'ps_teste'@'localhost' IDENTIFIED BY 'ps_teste';
               GRANT ALL ON ps_teste.* TO 'ps_teste'@'localhost';"
```

Mongo do e2e (LEIAME):
```bash
mkdir -p /tmp/ps_mongo_db
mongod --dbpath /tmp/ps_mongo_db --port 27099 --bind_ip 127.0.0.1 --quiet &
./pool teste/e2e/mongo.pr
pkill -f "port 27099"
```

No Mac, com as mesmas imagens, variáveis e portas do job `cobertura` do `ci.yml`:
```bash
docker run -d --name ps_pg -e POSTGRES_USER=ps_teste -e POSTGRES_PASSWORD=ps_teste \
  -e POSTGRES_DB=ps_teste -p 5432:5432 postgres:16
docker run -d --name ps_maria -e MARIADB_ROOT_PASSWORD=ps_teste_raiz -e MARIADB_DATABASE=ps_teste \
  -e MARIADB_USER=ps_teste -e MARIADB_PASSWORD=ps_teste -p 3306:3306 mariadb:10.11
docker run -d --name ps_mongo -p 27099:27017 mongo:7
make check-e2e                 # todos os e2e
make check-e2e E2E=jinker      # só o que casa com o filtro
```

### 7.8 Outros arquivos com segredo (só o caminho)

| caminho | o que guarda |
|---|---|
| `~/.ssh/github_tecsol`, `~/.ssh/vps_tecsol_hostinger`, `~/.ssh/id_ed25519` | chaves privadas SSH |
| `~/.wg-tecsol/wg0.conf`, `~/.wg-tecsol/priv.key` | chave privada do WireGuard |
| `~/.config/gh/hosts.yml` | conta do gh; o token fica no keyring (inválido) |
| `~/.jinga-ide/contas-git.json`, `~/.jinga-ide/git-credenciais` | contas de Git da Jinga IDE (token) |
| `~/.jinga-ide/sessao.json` | porta, token e raiz da sessão do backend da Jinga IDE |
| `~/.jinga/libs/pondy.pr` | chave de API da Groq escrita no fonte, no cabeçalho `Authorization` das linhas 15, 42, 94, 116 e 135 |
| `~/.claude.json`, `~/.claude/` | login e configuração do Claude Code, e a memória do assistente |
| `~/.codex`, `~/.kimi-code`, `~/.continue`, `~/.copilot` | configuração e login de outros assistentes |
| `~/.thunderbird` | perfil de e-mail |
| `~/projetos/backups/TECSOL-TUDO.md` | mapa de onde fica cada senha, token e chave da Tecsol, sem os valores |
| `.env` de projeto | `~/projetos/api-auth/.env`, `mail-sender/.env`, `painel/.env`, `To-do/.env`, `site-tecsol/.env`, `tecsol-site/.env`, `tecsol-api/api/.env`, `tecsol-one/.env.local`, `Fuxlin-mail/fluxlin.env`, `exercicios/jinga/.env`, `libs/pagination/.env`, `megalink/front-de-planos-youtv/.env.local` |
| TLS de projeto | `~/projetos/painel/api/service/.config/key.jinkerTls.key` e `certificate.jinkerTls`. O `api-auth` espera `auth/cert.pem` e `auth/key.pem`, mas a pasta `api/auth/` está vazia. O `mail-sender` aponta pra `/opt/tecsol/server/mail-verify/api/.config/{cert,key}.pem` |

### 7.9 Jinga instalada nesta máquina

| caminho | o que é |
|---|---|
| `/usr/local/bin/jinga`, `jpkg`, `pool`, `psl` | o mesmo binário, 9.066.080 bytes, `Jinga 16.1.9 [PSVM] (2026-09-28)` (medido no fim do dia; igual byte a byte ao `./pool` do repo) |
| `/usr/local/bin/jinga-lsp`, `poolscript-lsp` | script sh de 248 bytes: `exec node /usr/local/share/jinga/lsp/server.js --stdio`; sem node ele sai com erro |
| `/usr/local/bin/pool.pyinstaller.bak` | 50 MB, de 2026-07-28, sobra do binário antigo |
| `/usr/local/share/jinga/docs` | a doc que o hover do LSP lê (a instalada, não a do repo) |
| `/usr/local/share/jinga/lsp` | o servidor LSP (`server.js`) |
| `~/.jinga/libs/` | 9 libs dele (8.4) |
| `~/.jinga/installed.json` | as 9 libs com `"source": "local"`, instaladas em `2026-09-28T21:55:25+00:00` |
| `~/.jinga/bin/`, `~/.jinga/commands/` | vazios |
| `/opt/jinga-ide`, `/usr/bin/jingging` | pacote deb `jinga-ide` 0.1.0 (8.2) |
| `~/.jinga-ide/` | estado da Jinga IDE: `backend/`, `backend.log`, `sessao.json`, `contas-git.json`, `git-credenciais`, `ultimo-projeto` (vale `/home/kleberdevion/projetos`), `tmp/` |

PATH: `export PATH="/home/kleberdevion/.jinga/bin:$PATH"` está na linha 31 do `~/.bashrc` e na linha 3 do `~/.profile`. No Mac, a linha vai para o `~/.zshrc` com `/Users/<usuário>/.jinga/bin`.

Instalação de lib (`jpkg help`):
```bash
jpkg install <arq.pr>          # o arquivo decide via #!lib / #!cmd
jpkg install <arq.pr> -asLib   # força lib importável
jpkg list
```

### 7.10 Editores

**VS Code 1.137.0**
- Extensões dele: `klebersantanadeoliveira.jinga-8.6.8` (fonte em `editor/vscode`, `name: jinga`, `version: 8.6.8`) e `klebersantanadeoliveira.jinkertls-1.0.1` ("JinkerTls Language Support").
- A extensão é cliente LSP e sobe o `jinga-lsp` instalado. Depois de reinstalar a Jinga, é preciso reiniciar o processo do servidor (item 7 da trilha).
- `~/.config/Code/User/settings.json`, trechos da linguagem:
  ```json
  "[poolscript]": {
      "editor.wordBasedSuggestions": "off",
      "editor.snippetSuggestions": "none"
  },
  "github.copilot.enable": { "poolscript": false }
  ```
  O resto: tema `GitHub Dark`, fonte `Fira Code` com ligaduras, terminal `bash`.
- No Mac, o arquivo fica em `~/Library/Application Support/Code/User/settings.json`.
- Empacotar a vsix exige Node 22 ou mais novo. O node do sistema aqui é 18.19.1, e o `make vsix` avisa isso. O `~/.nvm` tem v16.20.2 e v24.18.0.

**Neovim 0.9.5**
- `vim` e `vi` apontam para o `nvim` via `/etc/alternatives`.
- `~/.config/nvim/init.lua` tem 316 linhas e não usa gerenciador de plugin:
  - tema `colors/ariake-dark.lua` e `syntax/poolscript.vim`;
  - `vim.filetype.add({ extension = { pr = "poolscript" } })`;
  - sobe o `poolscript-lsp` do PATH, com reserva em `~/poolscript-lang/editor/vscode/server.js` via node;
  - `K` mostra o hover e `gd` vai à definição;
  - `Ctrl+S` salva e `Ctrl+Z` alterna o modo de edição.
- Cópias antigas na mesma pasta: `init.lua.bak-20260901`, `init.lua.bak-20260917-223757` e `init.lua.bak-antes-do-conserto`. O repo também tem `editor/nvim`.

**IntelliJ IDEA Ultimate 2026.2** (`/opt/idea-IU-262.8665.337`)
- Configuração em `~/.config/JetBrains/IntelliJIdea2026.2`. Plugins em `~/.local/share/JetBrains/IntelliJIdea2026.2`. Não existe instalação IdeaIC nesta máquina.
- Plugins da linguagem:
  - `lsp4ij`;
  - `jinga-icons` (id `jinga.icons`, nome "Jinga IDEA", versão 1.2.0). É o plugin do repo, em `editor/intellij/plugin`: `build.sh` gera `dist/jinga-icons.jar`, e ele declara o servidor sozinho para `*.pr`. O repo também tem `editor/intellij/bundle/Jinga.tmbundle` com `atualiza.sh`.
- `options/UserDefinedLanguageServerSettings.xml` ainda tem o cadastro manual "PoolScript": `commandLine = /usr/local/bin/poolscript-lsp`, `workingDir = /home`, padrões `*.ps`, `*.psl`, `*.p`. Apagar esse cadastro em Settings > Language Servers é pendência DELE (item 6 da trilha). O arquivo também tem a sobra de um modelo "Smart Data Lake Language Server".
- No Mac, a configuração fica em `~/Library/Application Support/JetBrains/IntelliJIdea2026.2`.

### 7.11 Ferramentas medidas nesta máquina

| ferramenta | versão |
|---|---|
| gcc | 13.3.0 |
| clang | 18.1.3 |
| GNU Make | 4.3 |
| perf | 7.0.12 |
| git | 2.43.0 |
| OpenSSL | 3.0.13 |
| Python | 3.12.3 (só nos servidores falsos `teste/smtp_falso.py`/`imap_falso.py` e para inspecionar a stdlib Python) |
| node / npm | v18.19.1 (`/usr/bin/node`) / 9.2.0; `~/.nvm` com v16.20.2 e v24.18.0 |
| gh | 2.101.0 |
| Docker | 29.8.1 (serviço habilitado e ativo) |
| PostgreSQL | 16.15, cliente e servidor (serviço habilitado e ativo em 127.0.0.1:5432) |
| MariaDB | 10.11.14 (serviço habilitado e ativo em 127.0.0.1:3306) |
| mongod | 8.0.31 (serviço desabilitado); `mongosh` ausente |
| OpenJDK | 21.0.12 |
| Maven | 3.8.7 |
| VS Code | 1.137.0 |
| Neovim | 0.9.5 |

### 7.12 Ordem pra montar as credenciais no Mac

1. Copiar os arquivos da 7.5 para `~/.ssh` e aplicar os `chmod`. Testar com `ssh -T git@github.com`.
2. Importar `~/.wg-tecsol/wg0.conf` no app WireGuard. Testar com `ssh -o BatchMode=yes tecsol-vps true`.
3. `gh auth login -h github.com` e `git config --global` com nome e e-mail (7.4).
4. Antes de desligar este laptop, copiar `~/.jinga/libs/` inteiro: `Controllers.pr`, `pondy.pr`, `roken.pr` e `swagger.pr` só existem ali (8.4).
5. Com a Jinga instalada, reinstalar as libs com `jpkg install` (8.4) e pôr o PATH no `~/.zshrc`.
6. Copiar `~/.config/nvim/`, os trechos do `settings.json` do VS Code e o `options/` do IntelliJ.
7. Subir os bancos de teste (7.7).
8. Levar os `.env`, os certificados e `~/.jinga-ide/contas-git.json` (7.8), ou refazer os tokens nos serviços.

## 8. Seus projetos

Regra dele: os projetos dele são **relatados, nunca alterados** pelo assistente. As palavras dele, na nota `design-de-api-e-do-usuario.md`: "para de mexer fora do projeto", "para de dizer que tem erro onde não tem". Quando ele relata um erro, a causa se busca no motor, sem abrir os `.pr` dele para achar culpa. Tudo abaixo foi levantado em 2026-09-28 lendo o README ou o arquivo principal de cada pasta.

### 8.1 Onde fica cada coisa

- `~/projetos` é um repo git próprio: `origin https://github.com/kleberDevion/projetos.git`, branch `master`, último commit de 2026-09-28 ("CORE: Correções e mudanças de projeto"), 17 entradas sem commit.
  - Rastreia conteúdo de verdade só em `asaas/` (537 arquivos), `Fuxlin-mail/` (25), `To-do/` (5) e `tecsol-logs.py`.
  - `mail-sender`, `painel`, `psp-payment`, `site-tecsol`, `tecsol-api`, `tecsol-link`, `tecsol-lp-avaliacao`, `tecsol-lp-vendedores`, `tecsol-one`, `tecsol-site` e `tp-app` entram só como ponteiro de repositório (gitlink, modo 160000).
  - `algoritmos`, `api-auth`, `backups`, `exercicios`, `exercicios-java`, `jingaIDE`, `libs` e `megalink` não estão rastreados.
- `~/jingaIDE`, `~/exercicios` e `~/megalink` não existem na home. As pastas ficam em `~/projetos/jingaIDE`, `~/projetos/exercicios` e `~/projetos/megalink`.

### 8.2 Projetos da Jinga e pessoais

| pasta (`~/projetos/...`) | o que é | linguagem | git em 2026-09-28 |
|---|---|---|---|
| `jingaIDE` | IDE no estilo IntelliJ pra Jinga. Backend em Jinga (`backend/servidor.pr`, servidor jinker com árvore, tokens, diagnóstico, AST, busca, metadata, contexto, doc, execução e ponte DAP por WebSocket). Front desktop em Kotlin/JVM. Completion via `jinga --metadata`, `--contexto` e `--ast`; doc rápida lida de `/usr/local/share/jinga/docs`. Pacote `jinga-ide_0.1.0_amd64.deb` (traz o JetBrains Runtime 25 e exige `jinga` 15.94.3 ou mais novo), instalado aqui em `/opt/jinga-ide` com o comando `jingging` | Jinga + Kotlin | `kleberDevion/jingaIDE`, `main`, **62 arquivos alterados sem commit** |
| `libs` | fontes das libs Jinga dele: `math/`, `pagination/`, `prserver/`, `random/`, `stereotypes/`, `sunotfy/`, `lixo/` (comaly, dirs, groq, jsDB, langs, ret) e `enviroment/` vazia | Jinga | `kleberDevion/libs`, `master`, commit único "init: Bibliotecas da linguagem Jinga/PoolScript", 1 alterado (`sunotfy/sunotify.pr`) |
| `api-auth` | API jinker (`api/main.ps`) de token por e-mail: `POST /api/v1/jinkeble/send-new-token/<email>` e `/validar_token_djabo/<token>`; `oauth` com `poolip`, `tls`, `rate: 300`, `bloq: 31`; pasta `smtp/` com o mailer | Jinga (`.ps` antigo) | sem git próprio, fora do repo projetos |
| `mail-sender` | API jinker de sessão por e-mail: `POST /api/v3/session/token-email` e `/token-validar`, com models `RouteNewToken` e `RouteValidarToken`; `tls`, `rate: 2000`, `bloq: 400`, `poolip` | Jinga | `kleberDevion/exu`, `main`, 6 alterados |
| `painel` | API jinker do painel de operadores (`/api/v1/`): healthcheck, login (`/loginpanel:form`, model `LoginPAINEL`), cadastro (`/sing-inpanel`), tokens, ocorrências da VPS, relatório mensal por e-mail. `runner.pr` imprime o banner e sobe `warningPanel.Routesapp` com `HOST`/`PORT` do ambiente (porta padrão 1002) | Jinga | `kleberDevion/painel`, `main`, **1 commit sem push** |
| `To-do` | API jinker de tarefas: `POST /manager/services/v0/new-task:specs` com model `newtask`; `frontend/` vazio | Jinga | dentro do repo projetos |
| `Fuxlin-mail` | "servidor de routing de pacotes de e-mail" (`spec.md`, versão 000.000.01). `server/main.pr` com `route_prefix='/smtp/spi/router/'`, health e `/compose:payload`; pastas auth, inbox, planos, smtp, usuários; `correio/` com `especificacao.html` e `schema.sql` | Jinga | dentro do repo projetos. `Fuxlin-mail/.git` é um arquivo de 12 bytes que o git recusa ("invalid gitfile format") |
| `psp-payment` | saque das comissões dos parceiros Tecsol: lê `comissoes` do Postgres do egn-tecsol, grava em `saques` e paga por PIX via Asaas. Versão C# (`dotnet run`, sobe em `http://127.0.0.1:5090`) e versão Jinga em `pool/` (`banco.pr`, `guarda.pr`, `main.pr`, `pix.pr`, `schema.sql`, binário `psp-pool`) | C# + Jinga | git **sem remoto**, `master`, 6 alterados: o histórico só existe neste disco |
| `algoritmos` | exercícios de algoritmo: `bubble.ps`, `fg.ps`, `kl.ps`, `entendendoAlgoritmos/01SearchBinary.ps` (busca binária), `replay/` (8 `.ps`) | Jinga (`.ps` antigo) | não rastreado |
| `exercicios` | `boomer/` (7 `.ps` + `status.txt`), `jinga/000jinga.pr` (classe `Bank` com `psodbc` e `dotenv`), `python/` e `desafios/` (exercícios em Python), `db.db` | Jinga + Python | não rastreado |
| `exercicios-java` | pasta vazia | — | não rastreado |
| `asaas` | referência da API do Asaas baixada em 2026-09-01: 213 contratos em 34 grupos, spec OpenAPI completa (486 schemas), ferramentas `tools/asaas.py` e `gen-contracts.py`, vídeo do tutorial de Sandbox | docs + Python | dentro do repo projetos |
| `tecsol-logs.py` | painel gráfico da VPS da Tecsol: logs do journald de cada serviço por SSH, varredura de portas, descoberta de rotas nos domínios da Tecsol, edição de arquivo remoto com backup antes de gravar, monitor com notificação e webhook; alvo travado em 187.127.9.180 e `*.tecsolengenharia.com.br` | Python | dentro do repo projetos |
| `backups` | `TECSOL-TUDO.md` (35.826 bytes, "Tecsol: tudo o que sei", 28/09/2026 às 20h, só onde está cada segredo, sem valores); `tecsol-bundles/` (git bundle de `egn-tecsol`, `lp_avaliacao`, `lp_vendedores`, `st-tc` com 104 MB, `tecsolgeral`, `tecsolgeral-davi`, `tecsol-link`, `tp-app`, mais `.espelhos/`); `tecsol-trabalho-20260928/` (`memoria-claude/`, `org/`, `workflows/`, cópia do TECSOL-TUDO.md); `tp-app-arquivos-locais/` (planilha `.xlsm` e `.claude/`) | — | não rastreado |

Fora de `~/projetos`:
- `~/README-perfil.md`: README do perfil do GitHub ("Brasileiro. Escrevo em português. Criei uma linguagem de programação."), que aponta para `kleberDevion/poolscript`.

### 8.3 Trabalho: Tecsol e Megalink

| pasta (`~/projetos/...`) | o que é | git em 2026-09-28 |
|---|---|---|
| `tecsol-api` | "Tecsol, API + CRM": API REST em Python + SQLite (`api/`, com sync de vendas e seed), CRM web em TypeScript (`crm/`), `api-java/` (Java, `pom.xml`), `deploy/deploy.sh`, `scripts/`, `DOCUMENTACAO.md` | `Tecsol-Engenharia/egn-tecsol` (SSH), branch `appmod/java-upgrade-20260815170433`, **139 alterados** |
| `tecsol-one` | sistema interno da Tecsol, protótipo de interface sem banco e sem login; `TECSOL-ONE.md` é a especificação; `referencia/vba/` guarda os 53 módulos VBA da planilha que ele substitui | remoto `davi` = `davifcouto/tecsolgeral`, branch `integração-api`, limpo |
| `tecsol-site` | site "Energia Solar em Linhares ES \| Tecsol Engenharia" (blog, cases, contato, dúvidas, política de privacidade) | `Tecsol-Engenharia/st-tc`, `main`, 2 commits atrás do remoto |
| `site-tecsol` | site anterior da Tecsol (páginas Blog, Cases, Contato, Dúvidas; último commit "Atualizou orçamentos") | `kleberDevion/site-tecsol` (SSH), `main`, 1 alterado, branch remoto apagado (`[gone]`) |
| `tecsol-link` | página "link na bio" do consultor comercial da Tecsol | `Tecsol-Engenharia/tecsol-link`, `main`, 1 alterado, 2 atrás |
| `tecsol-lp-avaliacao` | "Avalie a TECSOL" (páginas comercial, residencial e rural) | `Tecsol-Engenharia/lp_avaliacao`, `main`, 1 alterado, 2 atrás |
| `tecsol-lp-vendedores` | página por vendedor, cartão de visitas digital por chaveiro NFC: `parceiros.tecsolengenharia.com.br/i/{codigo}` conta o clique e redireciona com `?ref=`, e a indicação vira negócio no CRM Solarz | `Tecsol-Engenharia/lp_vendedores`, `main`, 1 alterado, 2 atrás |
| `tp-app` | "Tecsol Parceiros: Indique. Ganhe. Cresça." (dashboard, indicações, ganhos, metas, rede, cursos, materiais, meu-link); `ts.pr` é um teste da lib `sunotify` | `Tecsol-Engenharia/tp-app`, `master`, 1 alterado, 2 atrás |
| `megalink/cdntv-api-central` | backend em TypeScript (`src/server.ts`, módulos, rotas, serviços) | `pablogineli/cdntv-api-central`, branch `demandaPablo-1`, limpo |
| `megalink/central-mksolutions` | front web; o README é o de um exemplo de modelo, não do projeto | `kleberDevion/central-mksolutions`, branch `criar-planos-youtv`, limpo |
| `megalink/front-de-planos-youtv` | página com duas abas: "Solicitar teste" (teste de 15 dias do YouTV Premium por CPF) e "Plano ativo" | `kleberDevion/front-de-planos-youtv`, `main`, 12 alterados |
| `megalink/landpage_guide_install_youtvapp` | "Guia de Instalação · YOUTV Digital, MEGALINK" | `kleberDevion/landpage_guide_install_youtvapp`, `master`, limpo |
| `megalink/DADOS-METRICS` | relatório `index.html` ("Atendimento Amigalink Jul–Ago"), `atendimento-megalink-jul-ago-2026.pptx`, planilha `historico-22-09-2026 (1).xlsx` | sem git próprio |
| `megalink/site-libre-telecom` | site da Libre Telecom (`dados-branding/`, `src/`) | sem git próprio |

### 8.4 Libs dele em `~/.jinga/libs`

| lib | linhas | o que faz (lido do código) | onde mais existe |
|---|---|---|---|
| `Controllers.pr` | 17 | decorador `Controller(func)` que serializa chamadas com a trava global `lock` (espera com `sleep(0.4)`) | **só aqui** |
| `math.pr` | 23 | `public Entity math` com `static f(x, a, b)` = `a * x + b` (ValueError se `x` for nulo ou negativo), `fat(x)` (fatorial) e `square(x)` | versão diferente em `~/projetos/libs/math/math.ps` |
| `pagination.pr` | 358 | paginação para todo banco do `psodbc`: `page(url, query, page=1, per_page=20, ...)` com OFFSET e `cursor(url, query, key, ...)` com keyset; cola `LIMIT ? OFFSET ?` (ou `OFFSET/FETCH` no SQL Server) no fim da query | igual a `~/projetos/libs/pagination/pagination.pr` |
| `pondy.pr` | 161 | `public class pondy`, cliente assíncrono da API da Groq: `create` (chat), `models`, `transcription` e `translate` (áudio), `load_model` (roda modelo local por comando externo); **a chave de API está no fonte** (7.8) | **só aqui** |
| `prserver.pr` | 9 | `run(path)` sobe um `Jinker` com `static_folder=path` em `127.0.0.1:2202`, com `debug=True` | igual a `~/projetos/libs/prserver/prserver.pr` |
| `random.pr` | 124 | `public class asterisco`: `valcp` (confere dígito verificador de CPF), `randint(limit)` e `letter(limit)` a partir de `/dev/urandom` + `hash.sha256` + timestamp, `randid(webToken)` | versão diferente em `~/projetos/libs/random/random.pr` |
| `roken.pr` | 20 | `class Broken` com `_Hashing(string)`, que embaralha o texto em camadas de base64 sobre bytes em hex | **só aqui** |
| `sunotify.pr` | 25 | `class SunotiFy(title, body)` com `.send()`: notificação de desktop via `notify-send`, que é comando do Linux e não existe no macOS | igual à cópia de trabalho de `~/projetos/libs/sunotfy/sunotify.pr` |
| `swagger.pr` | 257 | gerador de OpenAPI, cliente e página Swagger a partir das rotas de um app jinker: `swagger.infos(target=...).title().description().version().authorContact().email().name().number().newDoc().bundle().metaData().SwaggerGEN()`; roda nos dois motores; doc em `docs/swagger/swagger.md` do repo; **não tem `#!lib`** na primeira linha | **só aqui** (o repo da Jinga tem só `examples/swagger.pr`) |

Reinstalar no Mac, depois de copiar a pasta para, por exemplo, `~/libs-jinga`:
```bash
cd ~/libs-jinga
for f in Controllers math pagination pondy prserver random roken sunotify; do jpkg install $f.pr; done
jpkg install swagger.pr -asLib
jpkg list
```

### 8.5 Imagem `~/Imagens/pyrite.jpeg`

- JPEG de 1254x1254, 3 componentes (sem canal alfa). É o logo de gema facetada dourada sobre fundo preto.
- Era do rename da linguagem para Pyrite (2026-09-15). Esse rename foi superado pelo rename para **Jinga** em 2026-09-24 (item 2 da trilha).
- Regra dele sobre essa imagem: a remoção do fundo preto é feita **só** com a lib `pnglib`.

### 8.6 O que só existe neste disco (copiar antes de desligar)

- `~/.jinga/libs/Controllers.pr`, `pondy.pr`, `roken.pr` e `swagger.pr` não têm cópia em outro lugar. `math.pr` e `random.pr` diferem da cópia em `~/projetos/libs`.
- `~/projetos/psp-payment` é git sem remoto.
- `~/projetos/algoritmos`, `api-auth`, `backups`, `exercicios`, `megalink/DADOS-METRICS` e `megalink/site-libre-telecom` não estão em nenhum git remoto.
- Trabalho sem commit:
  - `jingaIDE`: 62 alterados;
  - `tecsol-api`: 139;
  - `megalink/front-de-planos-youtv`: 12;
  - `mail-sender` e `psp-payment`: 6 cada;
  - `libs`, `site-tecsol`, `tecsol-link`, `tecsol-lp-avaliacao`, `tecsol-lp-vendedores` e `tp-app`: 1 cada;
  - o repo `~/projetos`: 17 entradas.
- `painel` tem 1 commit sem push.
- Os `.env`, os certificados e as contas da Jinga IDE listados em 7.8.
- `~/Imagens/pyrite.jpeg`, `~/README-perfil.md`, `~/.config/nvim/` e `~/.wg-tecsol/`.

Caminhos absolutos `/home/kleberdevion/...` que não existem no Mac, onde a home é `/Users/<usuário>`. Fica o relato; a decisão de mudar é dele:

| onde | arquivos |
|---|---|
| fontes Jinga | `painel/api` (10 arquivos: `main.pr`, `ControleOcorrencias.pr`, `__genner.pr`, `db.pr`, `UNTILS.pr`, `controller-usersingin.pr`, `controller-userlogin.pr`, `reports.pr`, `RelatorioMensal.pr`, `webhookTokens.pr`), `Fuxlin-mail/server` (3: `main.pr`, `helpers/comfirmExists.pr`, `smtp/RestMail.pr`), `To-do/api/main.pr`, `api-auth/api/main.ps`, `algoritmos/replay/down.ps` |
| outros | `tecsol-api/scripts/seguranca/run-gobuster-grande.sh`, `megalink/landpage_guide_install_youtvapp/.claude/settings.local.json`, `backups/tecsol-trabalho-20260928/org/varredura.json`, `backups/tecsol-trabalho-20260928/org/seg/ceticos.json` e `ceticos2.json`, `backups/tp-app-arquivos-locais/.claude/settings.json` |
| Jinga IDE | `~/.jinga-ide/ultimo-projeto` (vale `/home/kleberdevion/projetos`) |
| Neovim | reserva do LSP em `~/poolscript-lang/editor/vscode/server.js` (com `~`, funciona se o repo ficar em `~/poolscript-lang`) |

## 9. Mudança pro Mac, passo a passo

Este é o roteiro do dia da troca, na ordem em que as coisas acontecem. A situação abaixo foi medida em 2026-09-28:

- Repositório em `b180807`, versão 16.1.9 (`Jinga 16.1.9 [PSVM] (2026-09-28) Runtime standalone`).
- O Windows entrou no commit do 16.2.0; o que falta dele é ler e tratar o CI (9.10).
- O Mac é um MacBook Pro 2019: Intel i9-9880H (8 núcleos, 16 threads), 16 GB, SSD de 512 GB, chip T2, macOS 15.7.4.
  - É x86-64 como este laptop, então o motor e o código de máquina (JIT) são os mesmos.

Onde aparece `<usuario>`, é o nome da conta no Mac (o `whoami` responde).

### 9.1 No laptop velho: o que levar pro pendrive

| o quê | caminho | tamanho medido | observação |
|---|---|---|---|
| repositório inteiro | `~/poolscript-lang` | 413 MB | Inclui o `.git` (ramos locais `main` e `async`) e a pasta `.claude/` do repo (`settings.json` e `settings.local.json`, as permissões do assistente). Inclui também o `JINGA-TUDO.md` e o trabalho sem commit. Os dois estavam no `.gitignore` até 2026-09-28; com o repositório privado passam a ser versionados (às 23h11 a mudança do `.gitignore` ainda estava sem commit e os dois apareciam como não rastreados). |
| SSH | `~/.ssh` | 40 KB | `config`, `known_hosts`, `known_hosts.old` e três chaves com os `.pub`: `github_tecsol` (comentário `kleberdevion@proton.me`, usada no `Host github.com`), `id_ed25519` (comentário `cudegrilo`; segundo o `TECSOL-TUDO.md` entra como davifcouto no GitHub) e `vps_tecsol_hostinger` (comentário `claude-code-session`, usada em `tecsol-vps` e `tecsol-vps-pub`). As três estão carregadas no agente (`ssh-add -L`). |
| túnel WireGuard | `~/.wg-tecsol` | 12 KB | `wg0.conf` completo do peer `10.8.0.2` e `priv.key` (ver 9.7) |
| libs dele | `~/.jinga` | 80 KB | `libs/` com Controllers, math, pagination, pondy, prserver, random, roken, sunotify e swagger, mais o `installed.json`. `bin/` e `commands/` estão vazias. |
| gh | `~/.config/gh` | 12 KB | `config.yml` (`git_protocol: https`) e `hosts.yml` (usuário `kleberDevion`). O token não fica no arquivo: hoje o `gh` daqui responde `HTTP 401: Bad credentials`. No Mac é `gh auth login` novo. |
| estado do assistente | `~/.claude` e `~/.claude.json` | 1,2 GB e 60 KB | Memória e conversas em `projects/` (953 MB), os planos em `plans/` (o `steady-weaving-pine.md` é o plano deste porte) e as configurações. |
| Neovim | `~/.config/nvim` | 84 KB | `init.lua`, `colors/`, `syntax/` e os `init.lua.bak-*` |
| VS Code | `~/.config/Code/User/settings.json`, `~/.config/Code/User/snippets/` (`cpp.json`, `poolscript.json`), a lista do `code --list-extensions` (81 extensões) e `~/.vscode/extensions/klebersantanadeoliveira.jinkertls-1.0.1` | 4 KB, 12 KB, 40 KB | A jinkertls não tem fonte em nenhum outro lugar da máquina: a pasta instalada é a única cópia. |
| projetos | `~/projetos` | 7,8 GB (4,0 GB sem `node_modules`) | `~/exercicios` e `~/megalink` não existem: estão em `~/projetos/exercicios` e `~/projetos/megalink`. Também ficam aqui `jingaIDE`, `tp-app`, `libs`, `backups` (691 MB, com o `TECSOL-TUDO.md`) e os `tecsol-*`. |
| estado do jingaIDE | `~/.jinga-ide` | 132 KB | `contas-git.json`, `git-credenciais`, `sessao.json` |
| logo do rename | `~/Imagens/pyrite.jpeg` | 46.781 bytes | a logo do nome Pyrite (trilha de dev) |
| GPG | `~/.gnupg` | 20 KB | |
| identidade do git | `~/.gitconfig` | 73 bytes | `kleberDevion` / `klebersantanadeoliveira07@gmail.com` |
| shell | `~/.bashrc`, `~/.profile` | | Os dois põem `~/.jinga/bin` no PATH; o `.bashrc` carrega o nvm. No Mac servem só de referência (o shell lá é o zsh). |
| Tecsol | `~/.local/bin/tecsol-bundles.sh`, `~/.config/systemd/user/` | | Os timers de usuário da Tecsol (`TECSOL-TUDO.md`, seção 0.2) |
| pessoais | `~/Downloads` (703 MB), `~/Imagens` (16 MB), `~/.thunderbird` (19 MB), `~/.mozilla` (93 MB) | | se quiser levar |

**O pendrive tem que estar em exFAT.** O FAT32 recusa arquivo acima de 4 GB, e o exFAT não guarda permissão de arquivo Unix. Por isso tudo vai dentro de `.tar.gz`, que guarda permissão, link simbólico e data:

- Para formatar, use o app Discos do Mint (Formatar partição > exFAT).
- A senha do sudo deste laptop é `dormenis` (7.1).
- Formatar apaga o que estiver no pendrive.

```bash
P=/media/kleberdevion/<NOME-DO-PENDRIVE>     # onde o Mint monta o pendrive
cd ~
git -C ~/poolscript-lang status --short        # o que aparecer aqui só existe nesta cópia
code --list-extensions > ~/extensoes-vscode.txt
tar czf "$P/mudanca-home.tar.gz" \
    poolscript-lang .ssh .wg-tecsol .jinga .config/gh .claude .claude.json \
    .config/nvim .config/Code/User/settings.json .config/Code/User/snippets \
    .vscode/extensions/klebersantanadeoliveira.jinkertls-1.0.1 \
    .jinga-ide .gnupg .gitconfig .bashrc .profile Imagens/pyrite.jpeg \
    extensoes-vscode.txt .local/bin/tecsol-bundles.sh .config/systemd/user
tar czf "$P/mudanca-projetos.tar.gz" --exclude=node_modules projetos
# conferência: tem que dar 75 (as notas da memória da Jinga)
tar tzf "$P/mudanca-home.tar.gz" | grep -c '^\.claude/projects/-home-kleberdevion-poolscript-lang/memory/.'
sync
```

Os dois `.tar.gz` levam chave privada, o `JINGA-TUDO.md` (com credenciais) e a chave do túnel. Apague os dois do pendrive quando o Mac estiver de pé.

**Não desligue o laptop velho antes do 9.7.** O túnel da VPS tem que funcionar no Mac primeiro. Depois disso, tire o túnel deste laptop do boot (hoje o `wg-quick@wg0` está `enabled` e `active`):

```bash
sudo systemctl disable --now wg-quick@wg0
```

### 9.2 No Mac: devolver os arquivos

O pendrive aparece em `/Volumes/<NOME-DO-PENDRIVE>`.

```bash
cd ~
tar xzf /Volumes/<NOME-DO-PENDRIVE>/mudanca-home.tar.gz
tar xzf /Volumes/<NOME-DO-PENDRIVE>/mudanca-projetos.tar.gz
chmod 700 ~/.ssh ~/.wg-tecsol ~/.gnupg
chmod 600 ~/.ssh/config ~/.ssh/known_hosts ~/.ssh/known_hosts.old \
          ~/.ssh/github_tecsol ~/.ssh/id_ed25519 ~/.ssh/vps_tecsol_hostinger \
          ~/.wg-tecsol/*
chmod 644 ~/.ssh/*.pub
ssh-add --apple-use-keychain ~/.ssh/id_ed25519 ~/.ssh/github_tecsol ~/.ssh/vps_tecsol_hostinger
git -C ~/poolscript-lang status --short        # tem que mostrar o mesmo que no laptop velho
```

O disco do Mac (APFS) não diferencia maiúscula de minúscula no nome de arquivo. Medido: nenhum par de nomes colide no repo, em `~/projetos`, `~/.claude`, `~/.jinga`, `~/.ssh`, `~/.config/nvim` e `~/.wg-tecsol`.

O shell do Mac é o zsh. O PATH e o "vim é o nvim" deste laptop vão pro `~/.zshrc`. No Mac, `vim` é o Vim da Apple; aqui `vim` e `vi` são link pro `nvim`.

```bash
cat >> ~/.zshrc <<'EOF'
export PATH="$HOME/.jinga/bin:$HOME/.local/bin:$PATH"
alias vim=nvim
alias vi=nvim
EOF
```

### 9.3 A memória do assistente

A pasta de memória é nomeada pelo **caminho absoluto do projeto**: cada `/` vira `-`. Aqui o repo está em `/home/kleberdevion/poolscript-lang`, então a memória está em `~/.claude/projects/-home-kleberdevion-poolscript-lang/memory` (75 notas, `MEMORY.md` é o índice).

No Mac o repo fica em `/Users/<usuario>/poolscript-lang`, e o assistente procura em `~/.claude/projects/-Users-<usuario>-poolscript-lang/memory`. Sem a cópia abaixo, ele começa sem nenhuma das regras.

```bash
U=$(whoami)                     # o <usuario>
mkdir -p ~/.claude/projects/-Users-$U-poolscript-lang
cp -R ~/.claude/projects/-home-kleberdevion-poolscript-lang/. ~/.claude/projects/-Users-$U-poolscript-lang/
ls ~/.claude/projects/-Users-$U-poolscript-lang/memory | wc -l      # 75
```

A pasta inteira vai junto: a memória e as conversas antigas (`.jsonl`).

Outros projetos também têm memória, todos com o prefixo velho:

| pasta | notas |
|---|---|
| `-home-kleberdevion-projetos-tp-app` | 34 |
| `-home-kleberdevion` (sessões abertas na home) | 6 |
| `-home-kleberdevion-jingaIDE` | 4 |
| `-home-kleberdevion-megalink-site-libre-telecom` | 3 |
| `-home-kleberdevion-landpage-guide-install-youtvapp` | 2 |

Este laço copia todas trocando o prefixo da home velha pelo da nova. Mantendo o mesmo caminho dentro da home, os nomes batem:

```bash
H=$(printf '%s' "$HOME" | tr -c 'A-Za-z0-9' '-')    # /Users/<usuario> -> -Users-<usuario>
cd ~/.claude/projects
for d in -home-kleberdevion*; do
  novo="$H${d#-home-kleberdevion}"
  mkdir -p "./$novo"
  cp -R "./$d/." "./$novo/"
done
```

As memórias do jingaIDE, do site-libre-telecom e do landpage foram gravadas quando esses projetos estavam direto na home (`~/jingaIDE`, `~/megalink/...`). Hoje eles estão em `~/projetos/...`. Abrindo o assistente em `~/projetos/jingaIDE`, a pasta procurada é `-Users-<usuario>-projetos-jingaIDE`: copie a `memory/` pra lá do mesmo jeito.

Se o repo for pra outra pasta, o nome é esse caminho com `/` trocado por `-`. Pra conferir o nome exato, abra o assistente uma vez na pasta e rode `ls ~/.claude/projects`: a pasta que ele criou é a certa. Copie a memória pra dentro dela.

Duas coisas mudam no Mac:

- **As permissões gravadas citam `/home/kleberdevion/...`.** São 90 citações em `~/.claude/settings.json`, 86 no `.claude/settings.json` do repo e 158 no `.claude/settings.local.json`. Essas entradas deixam de casar e o assistente pede de novo.
- **O login do Claude Code fica no Keychain.** Não serve o `~/.claude/.credentials.json` copiado: faça login de novo no primeiro `claude`.

### 9.4 No Mac: ferramentas de base

1. **Ferramentas de linha de comando da Apple** (clang, make, git):
   ```bash
   xcode-select --install
   clang --version && make --version && git --version
   ```
2. **Homebrew**, o comando oficial. No Mac Intel o prefixo é `/usr/local`:
   ```bash
   /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
   brew doctor
   ```
3. **gh** e login novo:
   - Na hora do login escolha GitHub.com, HTTPS e "autenticar o git com as credenciais do GitHub", e entre pelo navegador.
   - O token vai pro Keychain.
   - O remote do repo continua `https://github.com/kleberDevion/poolscript-lang`, que redireciona pro canônico `kleberDevion/poolscript`.
   ```bash
   brew install gh
   gh auth login
   gh auth setup-git
   gh auth status
   ```
4. **Node**: instalador oficial do nodejs.org, versão LTS, "macOS Installer (.pkg)" x64. Ele instala em `/usr/local/bin/node`.
   - O `make vsix` exige Node 22 ou mais.
   - Referência deste laptop: o node do sistema é v18.19.1 e o nvm tem v24.18.0.
   - O Homebrew no Mac Intel compila o node do fonte.
   ```bash
   node -v        # v22 ou mais
   ```
5. **Claude Code**:
   ```bash
   curl -fsSL https://claude.ai/install.sh | bash
   cd ~/poolscript-lang && claude
   ```
   Pra conferir a memória, pergunte qual é o trailer do commit. A resposta certa é `Co-Authored-By: Regressão <noreply@anthropic.com>`.
6. **Fonte do editor**: o `settings.json` do VS Code usa `Fira Code`.
   ```bash
   brew install --cask font-fira-code
   ```

### 9.5 No Mac: a Jinga

O Makefile tem ramo Darwin:

- Compila com o clang da Apple.
- Pega cabeçalhos e bibliotecas por `brew --prefix`.
- Liga o OpenSSL e o libpng estáticos (os `.a` do Homebrew).
- Usa o sqlite, o expat e o zlib do próprio macOS, mais os frameworks Security e CoreFoundation.
- Os clientes de banco não são ligados: o motor abre o `.dylib` no primeiro `connect`.

1. **Dependências de compilação.** É a mesma lista do `scripts/compila.pr` e do `build.yml`.
   - No Mac Intel o Homebrew não tem mais pacote pronto e compila cada um do fonte, então a primeira vez demora.
   - Não há medição desse tempo: no runner do CI esses pacotes já vêm na imagem, e o passo leva 0,1 min.
   - O `coreutils` dá o `gtimeout`, que o Makefile usa no lugar do `timeout`.
   ```bash
   brew install openssl@3 libpng gmp libpq mariadb-connector-c unixodbc mongo-c-driver@1 coreutils
   ```
2. **Compilar e testar.** Apague primeiro os binários do Linux que vieram na cópia.
   - O tar preserva as datas, e o `vm/ps_build.h` só muda quando muda o dia.
   - Então, no mesmo dia da cópia, o `make` acha o `./pool` do Linux em dia e não recompila.
   - No runner `macos-15-intel` do CI, `make pool testar` levou 0,4 min e o `./testar` 1,4 min (run 36506350266).
   ```bash
   cd ~/poolscript-lang
   rm -f pool pool-asan pool-debug pool-oom testar unidade
   (cd editor/vscode && npm install --omit=dev --no-audit --no-fund)
   make pool testar
   ./pool --version
   ./pool -e 'import sys; post(sys.platform())'       # darwin
   ./testar
   JINGA_JIT=0 ./testar
   ```
3. **Instalar pela ferramenta**, rodando o `./pool` recém-compilado. Com a `jinga` já instalada, o comando do dia a dia vira `jinga scripts/compila.pr mac --instala`.
   ```bash
   ./pool scripts/compila.pr mac --instala
   ```
   O que ela faz, pela ordem:
   - confere cada pacote com `brew list --versions` e instala os que faltam;
   - roda `make pool testar`, o `./testar` e o `JINGA_JIT=0 ./testar`;
   - monta o pacote com `./build_bundle.sh` em `dist/jinga-macos-x86_64/` e `dist/jinga-macos-x86_64.tar.gz`. O pacote tem o `bin/jinga` e as `.dylib` de fora do sistema em `lib/jinga/`, com caminhos `@executable_path/../lib/jinga/...`, reassinadas ad hoc;
   - instala com `cd dist/jinga-macos-x86_64 && sudo ./instalar.sh`. O sudo pede a senha do Mac no terminal.

   O resultado fica em `/usr/local`:
   - `bin/jinga`;
   - `jpkg`, `pool` e `psl` como link pro `jinga`;
   - `jinga-lsp` e `poolscript-lsp`;
   - `lib/jinga/` e `share/jinga/lsp` + `share/jinga/docs`.

   **Se a suíte reprova, a ferramenta para em "a suite reprovou" e não instala.** Até a suíte do Mac fechar verde (ver 9.10), a instalação é pelo instalador, de dentro do repo. O ramo Darwin do `instalar.sh` instala o `./pool` compilado nesta máquina, o LSP (com o `node_modules`) e a doc:
   ```bash
   cd ~/poolscript-lang && sudo ./instalar.sh
   ```
   O `sudo make install` também tem ramo Darwin (sem o MIME, que é do Linux).
4. **O dia a dia depois de cada mudança.** A regra do bump continua: toda mudança sobe a versão no mesmo commit, depois `make pool` e instalar.
   - Depois de instalar, derrube o servidor LSP antigo e o editor sobe o novo.
   - Nunca use `pkill -f` com o padrão na mesma linha: ele mata o próprio shell.
   ```bash
   make pool && sudo ./instalar.sh
   kill $(pgrep -f "^node /usr/local/share/jinga/lsp/server.js")
   ```
5. **O portão inteiro no Mac** (`make check` ou `compila.pr mac --portao`) precisa também do gcc de verdade, porque os alvos `analisa` e `avisos` são do gcc. Sem ele, o Makefile para com `no macOS: brew install gcc`.
   ```bash
   brew install gcc neovim
   ```
6. **Bancos de teste no Mac**: é opcional, porque o e2e pesado roda na VPS (seção 5). O usuário, a senha e o banco de teste são `ps_teste` nos dois servidores (`teste/e2e/db.pr`). O Mongo sobe à parte na porta 27099. O `teste/mongo_roda.pr` do `make check` sobe o próprio `mongod` em `/tmp` e pula se não houver o binário.
   ```bash
   brew install postgresql@16 mariadb
   brew services start postgresql@16
   brew services start mariadb
   "$(brew --prefix postgresql@16)/bin/psql" -d postgres \
       -c "CREATE USER ps_teste WITH PASSWORD 'ps_teste';" \
       -c "CREATE DATABASE ps_teste OWNER ps_teste;"
   sudo mysql -e "CREATE DATABASE ps_teste;
                  CREATE USER 'ps_teste'@'localhost' IDENTIFIED BY 'ps_teste';
                  GRANT ALL ON ps_teste.* TO 'ps_teste'@'localhost';"
   brew tap mongodb/brew && brew install mongodb-community
   mkdir -p /tmp/ps_mongo_db
   mongod --dbpath /tmp/ps_mongo_db --port 27099 --bind_ip 127.0.0.1 --quiet &
   make check-e2e
   ```
7. **Gatekeeper.** Binário ou pacote baixado pelo navegador vem com a marca de quarentena, e o macOS recusa rodar. O `curl` do `instalar.sh` e o `gh run download` não põem essa marca.
   ```bash
   xattr -d com.apple.quarantine <arquivo>
   xattr -dr com.apple.quarantine ~/Downloads/jinga-macos-x86_64      # a pasta inteira
   ```
   Ainda não existe release com o pacote do Mac: o último release é o `v8.4.3` (2026-09-08), anterior ao porte. O `instalar.sh` rodado fora do repo no Mac responde "não há pacote do macOS publicado. Compile do fonte". Com o repositório privado, o `curl` do `instalar.sh` e o download de release sem login respondem 404 (5.14). O pacote do CI se baixa com `gh run download <id> -n jinga-macos-x86_64 -D dist`.

### 9.6 No Mac: editores

**VS Code**

1. Baixe de code.visualstudio.com a versão pra Mac com chip Intel e arraste pra Aplicativos.
2. Ponha o `code` no PATH: Cmd+Shift+P > "Shell Command: Install 'code' command in PATH".
3. Configurações e snippets. O tar os devolveu em `~/.config/Code`, e o VS Code do Mac lê de outro lugar:
   ```bash
   D="$HOME/Library/Application Support/Code/User"
   mkdir -p "$D/snippets"
   cp ~/.config/Code/User/settings.json "$D/"
   cp ~/.config/Code/User/snippets/* "$D/snippets/"
   ```
4. Extensões: as da loja pela lista. As duas dele vêm à parte.
   ```bash
   grep -v '^klebersantanadeoliveira\.' ~/extensoes-vscode.txt | xargs -n1 code --install-extension
   ```
5. A extensão Jinga sai do repo. O `make instala-vsix` gera `editor/vscode/jinga-<versao>.vsix` (versão 8.6.8 da extensão) e instala; precisa do Node 22 ou mais. Depois, Cmd+Shift+P > "Developer: Reload Window".
   ```bash
   cd ~/poolscript-lang && make instala-vsix
   ```
6. A jinkertls já voltou pra `~/.vscode/extensions/` (a mesma pasta no Mac). Se ela não aparecer na lista de extensões, empacote com o mesmo empacotador do `make vsix` e instale:
   ```bash
   cd ~/.vscode/extensions/klebersantanadeoliveira.jinkertls-1.0.1
   npx --yes @vscode/vsce package --allow-missing-repository --skip-license -o /tmp/jinkertls.vsix
   code --install-extension /tmp/jinkertls.vsix
   ```
7. Confira no painel Saída > Jinga. A primeira linha diz qual servidor subiu, e tem que ser `servidor: /usr/local/bin/jinga-lsp`. A extensão procura em `/usr/local/bin` mesmo quando o VS Code não herda o PATH do shell.

**IntelliJ**

1. Baixe o IntelliJ IDEA pra Mac Intel (`.dmg`) em jetbrains.com. Aqui está instalado o Ultimate 2026.2 (`/opt/idea-IU-262.8665.337`).
2. Settings > Plugins > Marketplace > **LSP4IJ**. É o cliente LSP do IDEA a quem o plugin da Jinga entrega o `jinga-lsp`.
3. Compile o plugin.
   - O `build.sh` usa o `javac` do PATH ou o que `JAVAC=` apontar.
   - No macOS, o `javac` do PATH (`/usr/bin`) só pede pra instalar um Java. Aponte o do runtime que vem dentro do IDEA.
   ```bash
   cd ~/poolscript-lang
   JAVAC="/Applications/IntelliJ IDEA.app/Contents/jbr/Contents/Home/bin/javac" make intellij
   ```
   O alvo procura o IDEA em `~/.local/share/JetBrains`, que é o lugar do Linux. No Mac ele diz "nao achei instalacao do IntelliJ" e deixa o jar pronto.
4. Instale pelo disco: Settings > Plugins > engrenagem > Install Plugin from Disk > `~/poolscript-lang/editor/intellij/plugin/dist/jinga-icons.jar`. Reinicie o IDEA (o plugin só carrega no boot).
5. Se existir um servidor "Jinga" ou "PoolScript" cadastrado à mão em Settings > Languages & Frameworks > Language Servers, apague: senão sobem dois.
6. Pra conferir: View > Tool Windows > Language Servers > Jinga. O plugin passa ao servidor um PATH reforçado com `/usr/local/bin`, então ele acha o `jinga-lsp` e o `node` mesmo com o IDEA aberto pelo Dock.

**Neovim**

```bash
brew install neovim
cd ~/poolscript-lang && make nvim
```

O `make nvim` instala em `~/.config/nvim` (o mesmo caminho no Mac): `init.lua`, `colors/ariake-dark.lua` e `syntax/jinga.vim`. Se o `init.lua` que veio no tar for diferente, ele guarda uma cópia `init.lua.bak-<data>` antes de trocar. `K` mostra o hover e `gd` vai pra definição.

### 9.7 VPN e VPS

- A VPS de produção é Hostinger KVM `srv1549888.hstgr.cloud`, IPv4 `187.127.9.180`.
- O `ssh tecsol-vps` (root, chave `~/.ssh/vps_tecsol_hostinger`) vai pra `10.8.0.1` e só funciona com o túnel WireGuard de pé. A porta 22 pública está fechada no firewall (conferido em 2026-09-28: `ssh tecsol-vps-pub` deu `Connection timed out`).
- O túnel escuta em `51820/udp`.

**Medido em 2026-09-28 (`ssh tecsol-vps 'wg show wg0 allowed-ips'`):**

- A VPS tem um peer só: chave pública `f69Yh6bCSggto0qD7m2zzaCzL+zw5z5uo1HTg5YkWUA=`, com `10.8.0.2/32`.
- Essa é exatamente a chave pública de `~/.wg-tecsol/priv.key` (`wg pubkey < ~/.wg-tecsol/priv.key`).
- O `~/.wg-tecsol/wg0.conf` é o arquivo completo desse peer: `Address = 10.8.0.2/32`, a chave privada, `Endpoint = 187.127.9.180:51820`, `AllowedIPs = 10.8.0.1/32`, `PersistentKeepalive = 25`.
- O `TECSOL-TUDO.md` (seção 0.1) diz que a chave está selada no TPM e não migra. A cópia em `~/.wg-tecsol` migra.

**Passo 1: o túnel no Mac com o arquivo copiado.**

```bash
brew install wireguard-tools
sudo mkdir -p /etc/wireguard
sudo cp ~/.wg-tecsol/wg0.conf /etc/wireguard/wg0.conf
sudo chmod 600 /etc/wireguard/wg0.conf
sudo wg-quick up wg0
ssh tecsol-vps true && echo "tunel ok"
```

- No Mac o `wg-quick` não é serviço: depois de reiniciar, rode `sudo wg-quick up wg0` de novo.
- A outra forma é o app WireGuard da App Store: "Import Tunnel(s) from File" > `~/.wg-tecsol/wg0.conf` > Activate.
- Não suba o túnel no Mac e no laptop velho ao mesmo tempo. É a mesma chave, e a VPS fica com o endereço de quem falou por último.
- Com o Mac funcionando, faça o `disable --now` do 9.1 no laptop velho.

**Passo 2: chave nova só do Mac (peer `10.8.0.3`, o procedimento do `TECSOL-TUDO.md`).** Com o túnel do passo 1 de pé:

```bash
umask 077
wg genkey | tee ~/wg-mac.key | wg pubkey > ~/wg-mac.pub
PUB=$(cat ~/wg-mac.pub)
ssh tecsol-vps "wg set wg0 peer $PUB allowed-ips 10.8.0.3/32 && printf '\n[Peer]\nPublicKey = %s\nAllowedIPs = 10.8.0.3/32\n' '$PUB' >> /etc/wireguard/wg0.conf"
```

1. Edite `/etc/wireguard/wg0.conf` no Mac: `Address = 10.8.0.3/32` e `PrivateKey =` com o conteúdo de `~/wg-mac.key`.
2. Suba o túnel com a chave nova:
   ```bash
   sudo wg-quick down wg0 && sudo wg-quick up wg0
   ssh tecsol-vps true && echo "tunel ok"
   ```
3. Tire o peer velho da VPS:
   ```bash
   ssh tecsol-vps "wg set wg0 peer f69Yh6bCSggto0qD7m2zzaCzL+zw5z5uo1HTg5YkWUA= remove"
   ```
4. Apague na VPS o bloco `[Peer]` do `10.8.0.2` em `/etc/wireguard/wg0.conf`.
5. Apague no Mac o `~/wg-mac.key` e a pasta `~/.wg-tecsol`.

Na VPS, só o `wg0.conf` cita o IP do laptop: firewall, sshd, Postgres e nginx não dependem dele (`TECSOL-TUDO.md`, conferido em 28/09).

**Se o túnel não subir**, entre pelo console web da Hostinger (hPanel > VPS > Console), que não passa pelo firewall. Lá dentro:

- rode `wg set wg0 peer <PUB_NOVO> allowed-ips 10.8.0.3/32`;
- ou rode `systemctl stop tecsol-firewall`, que abre a 22 pública, e depois religue.

**A VPS continua sendo onde roda a suíte pesada do Linux.** É o roteiro da seção 5:

- ssh;
- pasta em `/tmp`;
- a suíte como `nobody`;
- `nohup setsid`;
- limpeza total depois.

Os timers da Tecsol (`~/.config/systemd/user`) não sobem no Mac, porque o macOS não tem systemd. A VM do 9.8 roda eles com os comandos do `TECSOL-TUDO.md`.

### 9.8 Linux numa VirtualBox, pra continuar testando o build do Linux

1. Em virtualbox.org, baixe a versão "macOS / Intel hosts" e instale.
   - Na primeira vez o macOS bloqueia a extensão da Oracle.
   - Libere em Ajustes do Sistema > Privacidade e Segurança > Permitir, e reinicie.
2. Crie a VM com o Ubuntu 24.04 LTS, o mesmo do runner `ubuntu-24.04` do CI e da VPS. A configuração: 4 núcleos e 8 GB (metade do Mac), disco de 40 GB, rede em NAT.
3. Dentro da VM, as dependências são as do `compila.pr` e do `build.yml`, mais o git e o node. O `compila.pr linux --instala`:
   - compila e roda a suíte com o JIT ligado e desligado;
   - monta o `dist/jinga-portable.tar.gz` (que exige `libpq.so.5`, `libmariadb.so.3`, `libodbc.so.2` e `libmongoc-1.0.so.0`, todas trazidas pelos `-dev`);
   - faz o `sudo make install` com o MIME e o ícone do `.pr`, que o macOS não tem.
   ```bash
   sudo apt-get update
   sudo apt-get install -y git gcc make libsqlite3-dev libpq-dev libmariadb-dev unixodbc-dev \
       libssl-dev libpng-dev libexpat1-dev zlib1g-dev libgmp-dev libmongoc-dev libbson-dev nodejs npm
   git clone https://github.com/kleberDevion/poolscript.git ~/poolscript-lang
   cd ~/poolscript-lang
   (cd editor/vscode && npm install --omit=dev --no-audit --no-fund)
   make pool testar
   ./pool scripts/compila.pr linux --instala
   ```
   Com o repositório privado, o clone dentro da VM precisa de login (`gh auth login` e `gh auth setup-git` na VM) ou de uma chave SSH cadastrada no GitHub, com a URL `git@github.com:kleberDevion/poolscript.git`.
4. Trabalho sem commit do Mac chega na VM por um ramo `porte-*`. O `build.yml` também roda o CI em push nesses ramos.
   ```bash
   # no Mac:
   git push origin HEAD:porte-<nome>
   # na VM:
   git fetch && git checkout porte-<nome>
   ```

### 9.9 Conferência final

```bash
jinga --version                                  # Jinga 16.1.9 [PSVM] (<data da compilacao>) Runtime standalone
jinga -e 'import sys; post(sys.platform())'      # darwin
which jinga jpkg jinga-lsp                       # /usr/local/bin/...
cd ~/poolscript-lang
./testar                                         # a última linha tem que dizer "0 falharam"
JINGA_JIT=0 ./testar
node editor/vscode/teste_servidor.js ./pool      # o servidor LSP dirigido como o editor faz
git status --short                               # o mesmo do laptop velho
gh auth status
ssh tecsol-vps true && echo "tunel ok"
ls ~/.claude/projects/-Users-$(whoami)-poolscript-lang/memory | wc -l    # 75
```

- O `./testar` do Mac lista no fim "6 caso(s) valem so em outro sistema (este e darwin) e nao rodaram". São os casos do `/dev/full`, que só o Linux tem, e não contam como passou.
- No editor:
  - abra um `.pr` no VS Code;
  - pare o mouse em cima de `post`: o hover mostra a página da doc;
  - digite `regex.`: a lista de completion abre;
  - Saída > Jinga mostra `servidor: /usr/local/bin/jinga-lsp`.
  - O mesmo `.pr` no `nvim` (`K` pro hover) e no IDEA.
- No assistente: `cd ~/poolscript-lang && claude` e pergunte o trailer do commit. A resposta tem que ser `Co-Authored-By: Regressão <noreply@anthropic.com>`.

### 9.10 O que ainda está em andamento no porte

O plano é o `~/.claude/plans/steady-weaving-pine.md`, com um motor só (o mesmo C) e o JIT x86-64 nos três sistemas.

| parte | estado |
|---|---|
| 1. Camada de sistema (`vm/ps_so.h`, `vm/ps_so.c`) | feita (`529c518`): o motor não chama mais epoll, eventfd e `/proc` direto |
| 2. macOS | feito: pacote, instalador e workflow (`6980c40`), sockets pela camada (`c669a78`), pilha do clang (`ec711c7`), casos só do Linux e TLS pelo Keychain (`3c245e7`) |
| 3. `scripts/compila.pr` | feito (`89e32d7`) |
| 4. Este md | `JINGA-TUDO.md`, raiz; no `.gitignore` em `4aedf8b`, versionado desde que o repositório é privado |
| 5. Windows nativo | código completo (16.2.0); no 1º run do CI de verdade (16.2.1) a suíte deu 8516/8536, e o 16.2.2 trata as 20; o 16.2.3 porta o `make check` inteiro; a suíte fecha 8545/8545 no CI; o 16.2.4 trata o que o `make check` do 16.2.3 achou; conferir o CI do 16.2.4 |
| 6. Release com os três pacotes | o `build.yml` já publica os três numa tag `v*`; nenhuma tag criada |

**macOS**

- **A suíte ainda não fechou verde no CI.**
  - No job `mac` (runner `macos-15-intel`), `compila` e `versao` passam.
  - No `ec711c7` o `./testar` deu `8542 casos: 8534 passaram, 8 falharam`, com e sem JIT: 6 casos do `/dev/full` e 2 de processo filho que olhavam o `/proc`.
  - O `3c245e7` tratou os 8: os 2 foram reescritos pelo que o filho faz, e os 6 valem só no Linux e são listados, nunca contados.
  - O CI desse commit no ramo `porte-mac` terminou em falha (run 36507821136). As anotações do job `mac` dizem `8536 casos: 8535 passaram, 1 falharam`, nos dois passos (com e sem JIT). O que falha é "processo filho nao herda os descritores abertos do motor" (grupo `libs`, `teste/casos_libs.c:1121`): o caso conta `ls /dev/fd | wc -l` no filho, espera 4 e no macOS sai 5. É um dos 2 casos reescritos.
  - No `b180807` (`main`, run 36507856713), o mesmo caso falha nos dois passos, e num deles também "pool de fibra cheio é erro limpo, nao morte" (grupo `crash`, `teste/casos_crash.c:166`) travou (passou de 20 s). Totais: 8535 e 8534 de 8536.
  - **No 16.2.0 os dois foram tratados.** O caso dos descritores contava as linhas do `ls /dev/fd`, e o `ls` do macOS abre um descritor a mais (o da pasta de volta): media o `ls`, não o motor. Agora ele usa uma testemunha, o número de um socket aberto no motor, que não pode existir no filho, e o 2, que tem que existir. O do pool de fibra: cada fibra tinha ~128 KB residentes (o `calloc` dos três vetores), e 9000 tarefas davam 923 MB de RSS no Linux (medido). Os vetores passaram pra memória mapeada que só vira RAM quando é tocada: 243 MB e 0,37 s (antes 1,09 s), e o `perf stat` de um lote de tarefas async foi de 334,8 M pra 332,7 M instruções. Que isso era o estouro de tempo do Mac é a hipótese; a prova é o próximo CI do Mac.
- **O `make check` inteiro nunca rodou até o fim no Mac.** O passo "portao inteiro" ficou `skipped` em todos os runs, porque a suíte antes dele reprovou. O que já se sabe lendo o código:
  - `analisa` e `avisos` exigem `brew install gcc`;
  - `editor/nvim/teste_nvim.sh` chamava `timeout` direto; desde o 16.2.3 usa `timeout` ou `gtimeout`;
  - `editor/intellij/plugin/teste_intellij.sh` procurava o IDEA só onde o Linux o instala; desde o 16.2.3 conhece o `.app` do macOS e as pastas do Windows (no CI não há IDEA, e ele diz PULOU com a contagem);
  - o `make intellij` não conhece a pasta de plugins do Mac e cai na instalação pelo disco.
- **Nenhum release tem o pacote do Mac.** O último release é o `v8.4.3` (2026-09-08). O job `release` do `build.yml` roda em tag `v*` e precisa do `linux` e do `mac` verdes. Hoje ele publica dois pacotes: `jinga-portable.tar.gz` e `jinga-macos-x86_64.tar.gz`.
- **O `mongo-c-driver@1` sai do Homebrew.** O CI avisa que ele está descontinuado e será desativado em 2027-04-01. O build do Mac depende dele (`BREW_MG` no Makefile, `libmongoc-1.0.0.dylib` no `build_bundle.sh`).
- **O workflow `ci` do Linux também reprovou no `ec711c7`**, nos passos `make check` e `make cobertura` (run 36506350332). No `b180807` (run 36507856867) repetiu nos mesmos dois passos; a anotação só diz "exit code 2", porque o `ci.yml` não usa o `ci_anota.sh`. O motivo se lê com `gh run view 36507856867 --log-failed` (precisa do `gh` logado com admin do repositório).

**Windows nativo** (MSYS2 UCRT64, gcc ou clang; o MSVC não serve por causa do goto computado)

- **Feito no `3c245e7`**, primeira parte, compilando cruzado com MinGW contra o MSYS2:
  - `vm/ps_win.h` (126 linhas);
  - na camada de sistema: eventos por WSAPoll, aviso entre threads por par de sockets no loopback, VirtualAlloc/VirtualProtect, GetModuleFileName, raízes TLS do repositório ROOT do Windows e a API de socket (erro do Winsock traduzido pro `errno`);
  - `ps_http`, `ps_mail`, `ps_jinker`, `ps_pkg` (`.cmd` e `setx`), `ps_dl` (LoadLibrary), `ps_db`/`ps_mongo` (nomes das DLLs), `ps_pilha` e `ps_hash` (BCryptGenRandom) compilam pro Windows e continuam iguais no Linux.
- **Feito no commit do 16.2.0** (todos os `vm/*.c` compilam sem aviso com o MinGW contra o MSYS2; no Linux a suíte inteira, com e sem JIT, cli 260, sockets 29, jinker 173, depurador 26, unidade 253, LSP 244 e os portões da doc passam):
  - fibras por `CreateFiberEx`/`SwitchToFiber`; terminal pela Console API com VT;
  - processos por `CreateProcessW` com a lista de handles herdados (o `closefrom` do Linux) e pseudoconsole (ConPTY) pro `pty=True`, com uma thread que fecha o console quando o filho sai (senão a saída nunca chega ao fim); `kill` encerra com o código `-sinal`; `os.cmd` pelo `cmd.exe /d /s /c`; `os.code` acha `code.cmd`;
  - a ABI Win64 no JIT (`vm/ps_jit_x64.h`: argumentos em rcx/rdx/r8, rsi/rdi salvos, 32 bytes de sombra);
  - jinker sem fork: cada worker é o programa rodado de novo e recebe o socket por `WSADuplicateSocketW` (por um cano, porque precisa do pid do filho); o reload vira supervisor (o processo que o terminal espera roda o programa de novo a cada pedido do filho);
  - gmp: int64 por `mpz_import`/`mpz_export`; e os `long` que carregavam int da linguagem (`read(n)`, `sys.stdin.read(n)`, skip/limit/count do mongo, `Max-Age` do cookie, o PoolIp) viraram `int64_t`;
  - o `jpkg` e o `import` leem a mesma pasta do usuário (`USERPROFILE` no Windows; o motor lia `HOME`, que no Windows só existe dentro do MSYS2);
  - o `SO_REUSEADDR` não é ligado no Windows (lá ele deixa outro servidor escutar na mesma porta);
  - o depurador fala pelo socket com a API de socket da camada (no Windows o socket não é descritor de arquivo);
  - partida: Winsock, console em UTF-8 com VT, saída sem trocar `\n` por `\r\n`; o `jinga.exe` tem manifesto com `activeCodePage` UTF-8 (argv, ambiente e nomes de arquivo com acento) e pilha de 8 MB; `-o` completa `.exe`;
  - Makefile com ramo MSYS2 (`-static`: o `jinga.exe` só pede DLL do Windows); `teste/ps_teste.c` com `CreateProcess`, threads nos canos e tempo máximo por `WaitForSingleObject`; job `windows` no `build.yml` (MSYS2 UCRT64: compila, suíte com e sem JIT, `make check`, pacote e o `instalar.ps1` de verdade numa pasta temporária); `build_bundle.sh` gera `jinga-windows-x86_64.zip` (com as DLLs dos bancos e os `.cmd`); `instalar.ps1` (em `%LOCALAPPDATA%\Programs\Jinga`, sem administrador, põe o `bin` no PATH do usuário); `compila.pr` roda tudo pelo `sh` do MSYS2;
  - editor: `server.js` acha o motor sem `sh` e monta URI de arquivo pelo `pathToFileURL`; `extension.js` acha o `jinga-lsp.cmd` e sobe o `server.js` instalado ao lado dele; o terminal do "Rodar" no Windows é o PowerShell; o plugin do IntelliJ procura `.cmd`/`.exe` em `%LOCALAPPDATA%\Programs\Jinga\bin`. A extensão subiu pra 8.6.9; a `.vsix` se gera com Node 22 (`make vsix`), que o Homebrew do Mac tem;
  - doc: `jinga.md` (instalação no macOS e no Windows, o que muda no Windows), `docs/lsp.md`, e as páginas de `os.cmd`, `os.run`, `Process`, `sockets` e `Jinker`.
- **CI de verdade (16.2.1, run 36516748809):** Windows compilou e rodou; suíte 8516/8536. As 20 falhas foram diagnosticadas por um workflow (6 grupos, cada um com verificador adversário) e tratadas no 16.2.2: 1 bug do porte (`_isatty` diz "terminal" pro NUL; agora `ps_e_terminal` pergunta ao console), e casos que dependiam de detalhe acidental reescritos testando a mesma propriedade (aspas simples do `sh` no `os.cmd` -> `os.run` com lista; `/dev/full` -> só Linux + irmãos portáveis "pai que é arquivo"; `startswith("/")` -> medir que o caminho não depende do cwd; pty -> ler até `""`). Mac: suíte verde com e sem JIT; o `make check` caía no caso dos 30 mil parênteses porque o make 3.81 da Apple sobe a pilha dos filhos (64 MB) e 30 mil níveis cabem — o caso usa agora PS_PARSE_PROF_MAX + 1.
- **Conferir aqui sem Windows:** `scratchpad/win_check.sh` (sintaxe com os cabeçalhos do UCRT64 extraídos do repo.msys2.org, `-std=gnu2x`) e `scratchpad/win_liga.sh` (liga o `jinga.exe` estático). Os pacotes ficam em `scratchpad/ucrt/` (some com a sessão; baixar de novo pelo `ucrt64.db`).
- **CI do 16.2.2 / `ffb55a8`:** Mac passou as duas suítes e o `make check` até o `cobre_stdlib` (constantes de socket que o Mac não tem); Windows passou as duas suítes e caiu na `unidade` (faltava `-lbcrypt`). O `ci.yml` (Linux) estava vermelho desde 26/09.
- **16.2.3:** todo o `make check` portado (cada harness por um agente, revisado por outro, medido na VPS original x novo), mais os bugs do motor no Windows achados no caminho (ver a linha do histórico). Conferido antes do push: compilação do Windows limpa com o sysroot UCRT64 (`win_check.sh`), e o `make check` inteiro na VPS como `nobody`, com o PATH sem o `jinga` instalado.
- **CI do 16.2.3 / `cd8d31a`:** Mac e Windows passaram as duas suítes (8545/8545, com e sem JIT). No `make check`, o Mac caiu no `cli_roda` (1 de 260: a nota do `private` com lixo da pilha, bug do motor nos 3 sistemas que só o Mac mostrou; o valgrind acusava no Linux também). O Windows passou tudo até o `jinker` e caiu no `mongo_roda`: o driver 2.x do MSYS2 é `libmongoc2.dll` + `libbson2.dll`, e o `GetProcAddress` não procura na dependência. No Linux, o portão do `ci.yml` passou; a cobertura caiu no `lcov` ("negative count", contador sem trava com threads).
- **16.2.4:** as três causas tratadas (ver a linha do histórico). Conferido antes do push:
  - o `make check` inteiro na VPS como `nobody`;
  - o `make cobertura` na VPS: o `lcov` aceita a medição. A catraca acusa só o `ps_db.c` (209 -> 177 ramos cobertos), porque lá o e2e de banco diz PULOU: a 5432 da VPS é do PostgreSQL de produção. O job do CI sobe os bancos como serviço;
  - no `win_check.sh`, com as flags do `avisos` (`-Wshadow -Wlogical-op -Wformat-truncation=1`), os 20 fontes saem limpos;
  - o `-fanalyzer` no código do Windows acha só os 3 falsos positivos do `ANALISA_ACEITOS`.

  O `ps_so.c` não tem piso no `teste/cobertura_base.txt`, e os pisos que sobem no CI se perdem: a catraca regrava o arquivo só na cópia do job.

  Ainda não rodaram no CI: no Windows, o LSP, `gera_realce`, nvim, IntelliJ, `analisa`, `avisos`, `check-debug` e `check-e2e-local`; no Mac, tudo depois do `cli_roda`.
- **CI do 16.2.4 / `0cd5c25`:** Linux inteiro verde, inclusive a cobertura com os bancos de pé. O Mac passou o `cli_roda` (260) e caiu em 10 checagens do `jinker` (WebSocket e HTTPS), que rodavam no Mac pela primeira vez. O Windows passou o mongo (28), o LSP (240) e caiu no `analisa` (gcc 16, falso positivo no lexer).
- **Diagnóstico do Mac** (ramo temporário `diag-mac`, job no `macos-15-intel`): sem crash, servidor vivo; o handshake cru dava 101, 403 e 404, mas o `ws_connect` do motor caía no handshake e o HTTPS fechava depois do ClientHello. Com o `accept` bloqueante, o `jinker_roda` deu 173/173 no Mac.
- **Noturno (`robustez` do `ci.yml`), lido em 2026-10-01:** vermelho desde 26/08 sem ninguém ver o porquê. Com a receita do `analisa` certa, cada passo que nunca rodava mostrou o seu defeito: o teto do analisador maior que o runner, o fuzzer sem compilar, e o `bench --portao` acusando piora de 150% a 440%.
- **O bench acusa uma piora real do JIT, e a base NÃO foi regravada.** Contando instruções (`perf stat`) na mesma máquina, o binário da base (15.93.6) contra o atual:
  - sem JIT, o interpretador está igual à base (diferença de até 2%);
  - com JIT, `aritmetica` e `laco_tipado` caem 70%, `fib` 25% e `chamada` 13%;
  - também com JIT, ficam PIORES que o próprio interpretador: `lista_indice` +22%, `dict` +12%, `metodo` +8%, `objeto` +5%, `closure` +5% e `gc` +3%.
  - No `lista_indice`, o `jit_roda` leva 47% das instruções, e `nome_do_tipo_valor` e `strcmp` aparecem no laço quente.
  - A razão contra a régua mistura os dois efeitos (a régua é o `aritmetica`, que o JIT acelerou), mas o portão reprovou certo. Regravar esconderia a piora; o conserto é no JIT, uma tarefa com plano próprio.
- **CI do 16.2.5 / `49f5c39`:** verde nos 3 sistemas, com o `make check` inteiro no Mac e no Windows.
- **Conserto do JIT:** plano aprovado em 2026-10-02, com 13 etapas (Etapa 0 no 16.2.6). Objetivo: o código de máquina nunca fica mais caro que o interpretador no mesmo programa, e o portão do bench passa a comparar cada caso com ele mesmo (JIT contra interpretador, e os dois contra um binário de referência), por contagem de instruções. O noturno fica vermelho no `bench` até a Etapa 2.
- **Falta**: as Etapas 1 a 12 do conserto do JIT; o release `v*`.
- **Portão do Windows, pelo plano:**
  - a suíte inteira no job `windows-latest`/MSYS2;
  - cada falha separada em bug do porte (corrige) ou diferença real do sistema (`cmd.exe` no lugar de `sh`, `\r\n`), que ganha a expectativa do Windows;
  - nenhum caso pulado;
  - o `jinga.exe -o` gerando `.exe` que roda.
- **Depois**, o release com três pacotes: `jinga-portable.tar.gz`, `jinga-macos-x86_64.tar.gz` e `jinga-windows-x86_64.zip`. A seção 9 deste md ganha o passo a passo do Windows.
