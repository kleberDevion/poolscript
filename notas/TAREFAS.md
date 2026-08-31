# Tarefas abertas — consolidado das auditorias de 26 a 28/08

Uma lista só, das quatro auditorias, com a coluna que faltava em todas elas:
**o portão cobra?** Item fechado que nenhum comando reprova não está fechado —
está funcionando por sorte até alguém mexer perto.

Ordem: dívida de cobertura primeiro (é a raiz de tudo que escapou), depois o
resto por retorno sobre esforço.

Legenda da coluna PORTÃO:
- `check` — `make check` reprova se quebrar (é o gate inteiro, roda na CI também)
- `noite` — só nos alvos caros (`asan`, `oom`, `fuzz`, `analisa ANALISA_TUDO=1`)
- `nada` — **nada reprova**; quebra em silêncio

---

## 0. A dívida de cobertura — PAGA em 28/08

O ramo — que é o número que não engana — passou o da suíte em Python; linha e
função ficaram a menos de um ponto dela. A referência histórica está em
`AUDITORIA-ENGENHARIA.md` §9 (medida em `db0bcb6`, 2026-08-24).

**O número de HOJE não está escrito aqui, de propósito.** Ele sai de
`make cobertura`, arquivo por arquivo, do pior pro melhor, com a distância pra
100% — e com catraca que reprova quem cair.

Estava escrito, e envelheceu: a tabela que ficava neste ponto dizia
`ps_mail.c 16,9%` e `ps_guzer.c 22,3%` muito depois de serem 56,9% e 70,4%, e
esses números velhos foram citados como se fossem o estado atual. Número medido
em prosa não tem quem o atualize; comando tem.

---

## 0b. Como estava (mantido pra não repetir o raciocínio errado)

A suíte em Python (`db0bcb6`, 2026-08-24) cobria 53,3% de ramo e a de C, na
época, 41,4% — e a diferença não era código novo sem teste: era teste que
existia e se perdeu na migração de in-process pra fork/exec. (Esses dois são
números HISTÓRICOS, de uma medição datada; o estado de hoje sai do
`make cobertura`.) Medição e método em
`AUDITORIA-ENGENHARIA.md` §9.

O que já mudou: `make cobertura` agora mede o portão INTEIRO (suíte + drivers
`.ps` + e2e local, não só o `testar`), e `teste/cobertura_portao.ps` virou
CATRACA — quando um arquivo sobe, o piso é regravado na hora; cair reprova.
A dívida por arquivo aparece em todo relatório, com quanto falta.

| # | arquivo | falta de ramo pra meta | por quê |
|---|---|---|---|
| 0.1 | `ps_pkg.c` | 41,8 | `psl` inteiro sem teste local |
| 0.2 | `ps_mongo.c` | 40,8 | precisa de serviço; hoje só no `check-e2e` |
| 0.3 | `ps_jinker.c` | 37,1 | e2e local cobre o caminho feliz; falta protocolo torto |
| 0.4 | `main.c` | 32,9 | flags de CLI (`--tokens`, `--contexto`, `--metadata`) sem caso |
| 0.5 | `ps_hash.c` | 23,0 | módulo PURO — não tem desculpa. Candidato a 100% |
| 0.6 | `ps_regex.c` | 20,3 | módulo PURO — idem. Corpus do PCRE serve |
| 0.7 | `ps_db.c` | 12,4 | sqlite é local e não precisa de serviço |
| 0.8 | `ps_http.c` | 10,6 | loopback resolve |
| 0.9 | `poolscript_vm.c` | 5,0 | ramos de erro de alocação |
| 0.10 | `ps_xlsx.c`, `ps_compiler.c` | < 2 | quase lá |

**O teto estrutural**: metade dos ramos de um módulo em C é tratamento de erro —
`realloc` que devolveu NULL, buffer curto, socket fechado no meio do header,
UTF-8 truncado. **Nada disso é alcançável escrevendo `.ps`**, então a suíte
inteira, do jeito que está, tem teto por volta de 60% de ramo. Pra passar disso
faltam duas coisas:

- **0.11** teste chamando as funções em C direto (`teste/casos_libs.c` começou);
  o análogo é o `_testcapi` do CPython.
- **0.12** injeção de falha contando cobertura — o `teste/ps_oom.c` já faz
  `--wrap`, mas roda fora da medição.
- **0.13** a suíte em Python volta como ARQUIVO no repositório (não "está no
  `db0bcb6`"), religada nos `vm/ps_*_bind.c`, que continuam compilando.

---

## 1. Fechado, e o portão cobra

| item | onde | portão |
|---|---|---|
| §3.1 teto do parser (30 mil parênteses) | `casos_crash.c` | `check` |
| §3.1 aninhamento legítimo (500 parênteses, 200 níveis) | `casos_crash.c` | `check` |
| §3.2 teto no texto de estrutura funda | `casos_crash.c` | `check` |
| §3.2 página de guarda da fibra | `casos_crash.c` (via `async`) | `check` |
| §3.3 ciclo que fecha acima de 256 níveis | `casos_crash.c` | `check` |
| §4.1 invariantes `PS_ASSERT` | `make check-debug`, dentro do `check` | `check` |
| §4.3 e2e sem serviço | `make check-e2e-local`, dentro do `check` | `check` |
| §4.9 exemplos da doc compilam | `audita_exemplos_doc.ps` | `check` |
| §4.11 opcodes: 3 listas → `vm/ps_opcodes.def` | build quebra se sumir | `check` |
| regex `[\s\S]`, `[a\D]`, `[^\S]` | `casos_linguagem.c` (5 casos) | `check` |
| replay de achado de fuzz | `teste/fuzz_replay.ps` | `check` |
| §2.2 cobertura por arquivo | catraca, agora em todo push | `check` |

## 2. Fechado, mas NADA cobra — precisa de caso

| # | item | como cobrar |
|---|---|---|
| 2.1 | §3.4 mail com `SSL_VERIFY_PEER` + `SSL_set1_host` | servidor TLS de teste local com cert auto-assinado; hoje só `check-e2e` |
| 2.2 | §3.5 máscara do WS vinda de `ps_random_bytes` | teste em C chamando a função direto |
| 2.3 | §3.6 semente de hash por processo | teste em C: dois processos, `hash_str` da mesma chave, valores diferentes |
| 2.4 | §4.11 nada impede a lista de opcodes de se duplicar de novo | regra no `scripts/audita_c.ps`: `OP_X = <número>` fora do `.def` reprova |

## 2c. Cursor de banco fechado — DECIDIDO (28/08)

Era: `cur.close()` só liberava o statement, e um `execute()` seguinte preparava
outro e funcionava. Divergia do DB-API (PEP 249), onde operar cursor fechado é
`ProgrammingError`.

Decisão do dono: **fechado é fechado**. `execute`, `fetchall`, `fetchone` e
`fetchmany` recusam depois do `close()`. `close()` duas vezes continua válido —
nenhum DB-API trata isso como erro.

O tipo é `DatabaseError`, não `ProgrammingError`: o segundo não existe nesta
linguagem, e criar tipo de exceção novo é decisão de API à parte.

As quatro operações, e não só o `execute`: um `fetchall` depois do `close()`
lendo o resultado velho entrega dado obsoleto como se fosse atual, que é pior
que erro.

**Como isto foi pego, que é o ponto:** o teste anterior travava o comportamento
ANTIGO com o nome explícito "cursor fechado REABRE (diverge do DB-API)". Não
era falso verde — não fingia que levantava — e virou armadilha: quando o motor
mudou, ele reprovou e obrigou a atualizar teste e registro juntos. Travar
comportamento conhecido-como-errado, com o nome dizendo isso, é melhor que
deixar o caso de fora.

---

## 2b. Lacuna de API esperando decisão dele

**`bytes` é o único tipo embutido sem `.len()`.** `str`, `list`, `dict` e `tup`
têm; `bytes` só responde ao `len(x)` builtin. Nada em `docs/bytes/` diz que a
ausência é proposital, e a divergência que ESTÁ documentada é o contrário —
`.len()` como método é decisão de projeto da linguagem.

Custou seis testes: `teste/jinker_roda.ps` escrevia `pedaco.len()` sobre o que
o `recv` devolve, o `catch` engolia o "membro inexistente", e toda checagem de
protocolo cru voltava string vazia. Seis falhas com a causa invisível.

Não mexi porque método de tipo embutido é API, e API é dele. Se `.len()` entrar
em `bytes`, entra na tabela `METODOS_BYTES` do `poolscript_vm.c` e ganha página
em `docs/bytes/len/len.md` — o `audita_doc.ps` cobra as duas coisas.

---

## 3. Aberto, do original

| # | item | origem | portão hoje |
|---|---|---|---|
| ~~3.1~~ | **FEITO** — LSP passou a usar `XDG_RUNTIME_DIR` (`drwx------`); sem ele, `/tmp/ps_lsp-<uid>` | ENG §3.7 | `check` (18/18) |
| 3.2 | pacotes sem versão nem lockfile | ENG §3.8 | nada |
| 3.3 | clang-tidy / CodeQL | ENG §4.4 | nada |
| 3.4 | `detect_leaks=0` no `check-asan`; sem TSan; sem `__sanitizer_start_switch_fiber` (mais grave agora que a pilha virou `mmap`) | ENG §4.5 | noite |
| ~~3.5~~ | **FEITO** — `teste/bench.ps` + `make bench`; compara RAZÃO, não ms. Achou o O(n²) da concatenação | ENG §4.6 | noturno (`--portao`) |
| 3.6 | matriz de CI (um SO só) e `_Static_assert` no GMP | ENG §4.7 | nada |
| ~~3.7~~ | **FEITO** — compila de verdade e reprova; 2 avisos reais corrigidos (`EAGAIN\|\|EWOULDBLOCK`, `cl` sombreando o closure) | ENG §4.8 | `check` |
| 3.8 | `math`, `random`, CSPRNG: ferramenta de usuário | ENG §4.10 | nada |
| 3.9 | LICENSE (libmysqlclient é GPLv2 e entra ESTÁTICO — contamina a distribuição, decisão dele), SECURITY.md, CONTRIBUTING.md, tags coerentes, SBOM | ENG §4.12 | nada |
| 3.10 | código morto da era CPython (`ps_*_bind.c`, `#ifdef PS_MODULO_PYTHON`) | ENG §4.13 | — |
| 3.11 | catálogo de ilogismos I1..I23 (só I17 fechado) | ENG §5 | nada |

**Sobre 3.10:** está aberto DE PROPÓSITO. Eu removi e desfiz: a ponte CPython é
exatamente o que a suíte antiga usa pra alcançar `ps_db`, `ps_jinker`, `ps_http`
e `ps_hash`. Só sai depois que 0.13 estiver de pé e provar que não precisa.

---

## 3b. LF puro nos headers — RESOLVIDO (28/08)

Era: o jinker só reconhecia `\r\n\r\n`, então requisição com `\n` sozinho ficava
pedindo bytes que nunca vinham e a conexão ficava presa até o timeout — recurso
segurado por lixo.

Agora: o `\n\n` é reconhecido **só pra recusar** — 400 e fecha. Não aceita nada
a mais (aceitar LF puro é vetor de request smuggling, porque um intermediário
na frente corta a requisição num ponto diferente do nosso; o nginx recusa pelo
mesmo motivo) e não paga o timeout. É estritamente mais seguro que esperar.

A busca por `\n\n` só roda quando o `\r\n\r\n` NÃO foi achado, então corpo
legítimo que contenha `\n\n` continua passando — há caso pros dois em
`teste/e2e/jinker_bruto.ps`.

---

## 4. Erros meus que estão na lista porque precisam ser desfeitos

| # | o que eu fiz | o que tem que virar |
|---|---|---|
| 4.1 | `PS_CICLO_MAX 256` respondendo "é ciclo" por profundidade | **FEITO**: virou medição de folga (`ps_pilha_apertada`). O vetor de visitados voltou a fazer só o que ele faz — detectar ciclo. Estrutura de 300, 5000 e 200 mil níveis agora IMPRIME |
| 4.2 | `PS_PARSE_PROF_MAX 2000` no parser: contagem, não medição | **FEITO**: quem decide é `ps_pilha_apertada()`. Expressão de 5000 níveis, que o teto de 2000 recusava, agora COMPILA; o limite real medido fica por volta de 9600 e se adapta ao `ulimit -s`. O contador ficou como cinto de segurança (100 mil), pro caso de alguém esquecer de marcar a base |
| 4.3 | baseline de cobertura gravado na medição degradada — piso no fundo do buraco | **feito**: virou catraca, e a meta da suíte Python entrou como dívida visível |
| 4.4 | itens fechados sem nada que os cobre | **feito** pra o que dava caso; o que falta está no §2 acima |

Os dois primeiros eram decisão de design da linguagem, e eu os pus sem
perguntar, cada um com um comentário explicando por que era sensato —
que é o que fez passarem.
