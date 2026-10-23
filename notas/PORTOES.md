# Os portões: a linguagem inteira conferida a cada commit

Decisão do Kleber (2026-10-17), depois do `forAll`: um defeito que a doc
ensina e o `--check` recusa, achado por ele e não pela suíte. A cobertura não
pode depender de agente, de limite de uso nem de alguém lembrar de olhar. Os
portões são mecânicos, rodam no `make check` e cobrem a superfície
documentada da linguagem inteira: sintaxe, semântica, checador, motor com e
sem JIT, libs e editor.

O modelo é o das linguagens consolidadas. O Rust testa todo bloco da doc
(`cargo test --doc`): bloco é código até dizer o contrário. O Python roda os
exemplos da doc pelo `doctest`. A conformidade é por cláusula da
especificação, como o Test262 do JavaScript. E o compilador é testado por
programas gerados da gramática, comparando duas formas de rodar (Csmith,
YARPGen).

## A regra

- Cada portão entra no `make check`.
- O que ele acha vira item de `notas/CORRECAO-BASE.md`, com o programa, a
  promessa da doc e a gravidade.
- O portão guarda as falhas conhecidas em `teste/portoes_conhecidas.txt`,
  cada uma apontando o item aberto (`scripts/portoes_catraca.pr`). A chave é a
  página e a impressão do bloco (os 12 primeiros dígitos do SHA-256 do texto
  dele): mudou o bloco, a falha volta a ser olhada. A falha que só acontece
  num sistema leva o nome dele (`darwin`, `linux`, `windows`). Falha nova
  deixa o portão vermelho. Item consertado que continua no arquivo também
  deixa vermelho, o que força a lista a andar. Linha mal escrita no arquivo
  reprova. Não há pulo calado.

## P1: a doc inteira é teste

- Toda cerca diz o que é: ```ps, ```ps contexto, ```ps sem-rodar: motivo,
  ```saida, ```erro, ```entrada, ```assinatura, ```texto, ```bash, ```json.
  Cerca sem marca reprova (`scripts/audita_exemplos_doc.pr`); o
  `scripts/marca_cercas.pr` propõe a marca e troca só a linha da cerca.
- Portão `check`: todo ```ps passa no `--check`, com os ```ps contexto da
  página na frente (`scripts/audita_exemplos_doc.pr`, que aceita uma página:
  menos de 1 s).
- Portão `roda`: todo ```ps roda duas vezes, com o JIT e sem
  (`scripts/confere_saida_doc.pr`). Com ```saida, o stdout tem que ser aquele;
  com ```erro, o stderr contém aquele trecho; sem ```erro, sai com 0 (o erro
  que o bloco anuncia em comentário, `# Tipo: frase`, vale como esperado);
  sem ```saida, as duas rodadas imprimem o mesmo, e a saída que muda sozinha
  (hora, sorteio) só não se compara.
- O contexto entra só no bloco que usa um nome dele: o bloco que passa no
  `--check` sozinho roda sozinho, como quem lê vai rodar, e o número das
  linhas nas mensagens não anda. Nesse caso o contexto roda antes, na mesma
  pasta, pra criar o que a página pressupõe (um arquivo, uma tabela).
- Cada rodada é isolada, porque o exemplo faz o que a doc ensina (apaga
  arquivo, abre porta, manda sinal, dispara processo; o exemplo do
  `docs/os/run/run.md` executa `rm -rf ~` de propósito, pra mostrar o perigo
  do `os.cmd`): pasta própria e vazia, que é o HOME dele; ambiente limpo; no
  macOS o `sandbox-exec` (escrita só na pasta, rede só no loopback e fora das
  portas de serviço, como a 8080, sinal só pros próprios filhos); no Linux o
  `bwrap` (disco só de leitura fora da pasta, rede e processos num espaço
  próprio); prazo por rodada. **Nenhum exemplo roda sem o isolamento**: no
  Linux sem o `bwrap` e no macOS sem o `sandbox-exec`, o portão para vermelho
  dizendo o que falta; no Windows, que não tem isolador, o portão `roda` não
  roda nenhum exemplo, diz isso e sai com 0 (decisão do Kleber, 2026-10-23).
  O portão `check` roda lá.
- Cada rodada roda num grupo de processos próprio (o `os.run` sem `capture`
  faz `setsid`), e o grupo inteiro morre no prazo e no fim: o servidor que o
  exemplo abre não fica órfão escutando na porta. O grupo morre pelo
  `kill -9 -- -<grupo>`, a forma do POSIX: o número negativo vem depois do
  `--`, e o sinal vai no 1º argumento (com `-s KILL`, o procps toma um grupo
  pequeno, como o 83, por sinal). Sem o `--`, o `kill` do procps (Ubuntu
  24.04) lê só o 1º dígito do número: em 2026-10-22, com o portão rodando
  como root na VPS, `kill -9 -1426999` virou `kill(-1, 9)` e matou todos os
  processos da máquina (caíram o Postgres, o e-mail e as APIs). Por isso,
  antes do 1º exemplo, o portão prova com o sinal 0 que o `kill` da máquina
  acerta o grupo: o grupo 1000000000, que não existe em sistema nenhum, tem
  que dar "No such process" com esse número, e um `sleep` de verdade tem que
  ser achado e morto. Se a prova falha, nenhum exemplo roda.
- Bloco que precisa de serviço de fora (servidor que fica escutando, banco,
  e-mail, rede) roda só no `--check` e leva a marca ```ps sem-rodar: motivo.
  O relatório lista cada um, com o motivo.
- Fragmento é o bloco com `...` no lugar de código, que o `--check` recusa:
  ele sai listado e não roda. O `...` num comentário ou num texto
  (`conn.execute(...)`) não faz fragmento; o bloco é conferido e roda como
  os outros.
- ```saida rc=N (ou ```erro rc=N) diz o código de saída que o exemplo
  devolve de propósito (o `sys.exit(3)`).
- O bloco que só para por importar um módulo de exemplo (`import mymod`,
  `from '../pacote/modulo.pr' import x`) sai numa lista, como no portão
  `check`.
- No portão `roda`, a falha de um bloco que já está na catraca do `check`
  (não compila) conta como conhecida.

Medido na entrada (2026-10-17): 1.773 blocos em 467 páginas; 836 sem marca,
marcados (659 ```ps, 140 ```assinatura, 28 ```texto, 5 ```bash, 2 ```json, 1
```erro); dos 1.223 ```ps conferidos, 358 recusados, em 145 páginas, quase
todos por contexto não escrito. Com o contexto da `channel.md`, o portão pegou
sozinho o `app.channel(forAll=msg)` recusado.

Medido em 2026-10-18, com o contexto de cada página escrito e as marcas
erradas da primeira passada consertadas (assinatura com `->`, árvore de
pastas, tabela de operadores marcadas como programa):

| portão | blocos | falhas | situação |
|---|---|---|---|
| `check` | 1.335 | 41 | todas itens abertos da lista; portão verde com a catraca |
| `roda` | 1.185 rodados, 298 com a saída conferida | 228 na primeira rodada | em classificação |

Fechado em 2026-10-22, medido no Mac:

| portão | blocos | de fora (com o motivo) | falhas conhecidas | situação |
|---|---|---|---|---|
| `check` | 1.368 conferidos | 24 fragmentos | 41, todas itens abertos | verde |
| `roda` | 1.116 rodados com JIT e sem, 306 com a saída conferida | 94 `sem-rodar`, 24 fragmentos, 17 de módulo de exemplo | 27, todas itens abertos (5 só fora do Linux) | verde |

Conferido no Linux em 2026-10-23, na VPS (`make check` dentro de
`unshare --pid`, com o `bwrap`): os mesmos blocos, os dois portões verdes. O
`roda` conta 63 falhas conhecidas contra 68 no Mac (a conta junta as do
`check`): as 5 que só falham fora do Linux não falham lá.

## P2: o checador e o motor concordam (a fazer)

Cada membro do `--metadata`, chamado com cada parâmetro da doc e do metadata,
por posição e por nome, com o receptor tipado (o `--check` decide) e com o
receptor `Object` (o motor decide). As páginas de membro que se chama trazem a
assinatura no título. Todo programa da suíte e da doc: o que o `--check`
aprova não quebra rodando com erro que ele promete pegar.

## P3: o editor e o motor concordam (a fazer)

O diagnóstico do servidor do editor é o do `--check` em todo bloco da doc; o
hover nunca diz "nome não definido" num programa que o `--check` aprova; o
completion depois de `x.` lista o que o motor dá ao tipo de `x`.

## P4: a gramática escrita e o gerador (a fazer)

A gramática em EBNF, tirada do parser e conferida contra ele
(`docs/linguagem/gramatica.md`), e um gerador de programas tipados a partir
dela, com os oráculos: JIT e sem JIT idênticos, checador e motor de acordo,
nada cai nem trava, e o Python onde a doc diz "como no Python".

## A sonda com agentes

Fica para o que só se acha rodando programa de verdade além do que a doc
mostra (`notas/SONDA-METODO.md`). Os portões vêm primeiro, porque não dependem
de limite de uso.
