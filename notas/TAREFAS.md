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

## 0. A dívida de cobertura (raiz)

A suíte em Python (`db0bcb6`, 2026-08-24) cobria **53,3% de ramo**; a de hoje
cobre menos. Não é código novo sem teste: é teste que existia e se perdeu na
migração de in-process pra fork/exec. Medição e método em
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

## 3. Aberto, do original

| # | item | origem | portão hoje |
|---|---|---|---|
| 3.1 | `/tmp/ps_lsp` criado 0755 | ENG §3.7 | nada |
| 3.2 | pacotes sem versão nem lockfile | ENG §3.8 | nada |
| 3.3 | clang-tidy / CodeQL | ENG §4.4 | nada |
| 3.4 | `detect_leaks=0` no `check-asan`; sem TSan; sem `__sanitizer_start_switch_fiber` (mais grave agora que a pilha virou `mmap`) | ENG §4.5 | noite |
| 3.5 | zero benchmark (a era Python tinha `bench_async.sh`) | ENG §4.6 | nada |
| 3.6 | matriz de CI (um SO só) e `_Static_assert` no GMP | ENG §4.7 | nada |
| 3.7 | `avisos` ainda com `-fsyntax-only` e sem `-Werror` — mesmo defeito que já foi corrigido no `analisa` | ENG §4.8 | nada |
| 3.8 | `math`, `random`, CSPRNG: ferramenta de usuário | ENG §4.10 | nada |
| 3.9 | LICENSE (libmysqlclient é GPLv2 e entra ESTÁTICO — contamina a distribuição, decisão dele), SECURITY.md, CONTRIBUTING.md, tags coerentes, SBOM | ENG §4.12 | nada |
| 3.10 | código morto da era CPython (`ps_*_bind.c`, `#ifdef PS_MODULO_PYTHON`) | ENG §4.13 | — |
| 3.11 | catálogo de ilogismos I1..I23 (só I17 fechado) | ENG §5 | nada |

**Sobre 3.10:** está aberto DE PROPÓSITO. Eu removi e desfiz: a ponte CPython é
exatamente o que a suíte antiga usa pra alcançar `ps_db`, `ps_jinker`, `ps_http`
e `ps_hash`. Só sai depois que 0.13 estiver de pé e provar que não precisa.

---

## 4. Erros meus que estão na lista porque precisam ser desfeitos

| # | o que eu fiz | o que tem que virar |
|---|---|---|
| 4.1 | `PS_CICLO_MAX 256`: a função responde "é ciclo" ao passar de 256 níveis. Lista de 300 níveis, sem ciclo nenhum, é reportada como recursiva — **resposta errada, não proteção** | pilha de visitados crescendo no heap; a função responde só o que ela sabe |
| 4.2 | `PS_PARSE_PROF_MAX 2000`: teto na linguagem disfarçado de robustez. Contar nível não mede pilha — 2000 níveis de `(((` gastam muito menos que 2000 de expressão com chamada | medir folga real de pilha (`getrlimit` + endereço de local), ou tirar a recursão do laço unário/parênteses |
| 4.3 | baseline de cobertura gravado na medição degradada — piso no fundo do buraco | **feito**: virou catraca, e a meta da suíte Python entrou como dívida visível |
| 4.4 | itens fechados sem nada que os cobre | **feito** pra o que dava caso; o que falta está no §2 acima |

Os dois primeiros são decisão de design da linguagem e são dele. Eu pus os dois
sem perguntar, e cada um veio com um comentário explicando por que era sensato —
que é o que fez passarem.
