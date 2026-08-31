# Onde eu parei — 2026-08-27

Estado da árvore neste minuto, pra quem pegar daqui.

## Commitado

- `febff9d` — recursão sem teto: os três SIGSEGV da AUDITORIA-ENGENHARIA.
- `6ca7def` — teto/guarda/invariantes/TLS/semente + portão. Detalhe no corpo do
  commit; o que ele fechou do §8 foi: 1, 2, 3, 4, 5, 6, 7, 9.

## NÃO commitado (working tree)

```
 M Makefile                       # `make check` chama fuzz_replay; comentário no alvo fuzz
 M notas/AUDITORIA-ENGENHARIA.md  # (só marcações minhas de leitura)
 M vm/poolscript_vm.c             # enum de opcodes -> #include "ps_opcodes.def"
 M vm/ps_compiler.c               # idem + ps_op_nome gerado pelo mesmo .def
 M vm/ps_regex.c                  # classe negada dentro de []  (ver abaixo)
?? vm/ps_opcodes.def              # a lista única dos 87 opcodes
?? scripts/gera_opcodes.ps        # o script que fez a migração e a provou
```

Tudo isso compila e passa: **7852/7852** com o binário reconstruído agora.

### 1. Opcodes: três listas viraram uma (§8.8, parte)

Os 87 opcodes estavam escritos à mão em `ps_compiler.c` (enum), em
`poolscript_vm.c` (enum idêntico) e num terceiro switch (`ps_op_nome`). Nada no
build conferia que batiam — número trocado num lado não dá erro de compilação,
dá bytecode que executa outra instrução.

`scripts/gera_opcodes.ps` leu as três, **provou que batiam hoje** (nenhum
defeito latente: 87 opcodes, 0..86, sem buraco e sem repetido) e escreveu
`vm/ps_opcodes.def`. Os dois `.c` agora fazem:

```c
enum {
#define PS_OP(nome, num, texto) OP_##nome = (num),
#include "ps_opcodes.def"
#undef PS_OP
    OP__ULTIMO
};
```

e o desmontador vira `case OP_##nome: return texto;` sobre o mesmo arquivo.
Os comentários que explicavam cada família (closure, CLEAR_LOCAL, LOAD_BASE_INIT,
ITER_RANGE, o IS_OP/IN_OP) foram REPOSTOS dentro do `.def` — não se perderam.
O gerador se recusa a sobrescrever o `.def` (`--forca` para insistir), porque
daqui pra frente a fonte é ele, editado à mão: **opcode novo entra no fim, com o
próximo número livre; número nenhum se reaproveita** (está gravado em bytecode
já emitido).

FALTA: a regra em `scripts/audita_c.ps` que reprova `OP_X = <número>` escrito
fora do `.def`. Sem ela nada impede a lista de se duplicar de novo. É o único
pedaço deste item que não fiz.

### 2. Regex: `[\s\S]`, `[a\D]`, `[^\S]` (buraco da linguagem, achado usando a própria linguagem)

`vm/ps_regex.c` RECUSAVA classe negada dentro de `[]`:

    regex: classe negada (\D \W \S) dentro de [] nao suportada

`[\s\S]` é o "qualquer coisa, inclusive \n" que todo mundo escreve — bati nisso
escrevendo o `gera_opcodes.ps` em PoolScript. Não contornei: agora a negação é
MATERIALIZADA na hora da união (`classe_uniao_negada`) — ASCII bit a bit, e
acima de 127 andando pelos buracos entre as faixas de `sub`, com qsort porque o
cálculo precisa delas ordenadas.

Conferido contra o Python, os três casos:

| padrão | entrada | saída |
|---|---|---|
| `regex.sub("/\*[\s\S]*?\*/", "-", "a/* x\ny */b")` | | `a-b` |
| `regex.findall("[a\D]+", "ab12cd")` | | `['ab', 'cd']` |
| `regex.findall("[^\S]", "a b")` | | `[' ']` |

FALTA: entrar em `notas/LIMITACOES.md` (era limitação da linguagem, a regra é
anotar E corrigir) e ganhar caso na suíte diferencial.

## O que eu removi e DESFIZ

Removi os 3 `vm/ps_*_bind.c`, os 2 blocos `#ifdef PS_MODULO_PYTHON` de
`poolscript_vm.c` e o cabeçalho do `ps_vm.h` — 258 + 569 linhas — porque o §8.8
pedia e porque nenhum alvo do `Makefile` nem o `rebuild_vm.sh` compilavam
aquilo.

**Foi errado no momento** e está TUDO restaurado (`git checkout`, nada chegou a
commit): a suíte antiga era in-process, e a ponte CPython é justamente o que
tirava `ps_db.c`, `ps_jinker.c`, `ps_http.c` e `ps_hash.c` dos 0%. Derrubar a
ponte enquanto a estrada está sendo refeita é besteira. Só apagar depois que a
suíte nova estiver de pé e provar que não precisa.

## Cobertura — o furo é meu, e é o item aberto mais importante

Ele mediu com o mesmo método nos dois lados (worktree em `db0bcb6`, `-O0 -g
--coverage`, pytest, lcov) e o número é este:

|          | Python (db0bcb6) | C (hoje) |
|----------|------------------|----------|
| casos    | 3066 testes, 113 s | 7852 casos, 73 s |
| linhas   | 78,8%            | 55,0%    |
| funções  | 83,8%            | 54,9%    |
| ramos    | 53,3%            | 41,4%    |

Por arquivo (linha/ramo), Python → C:

    main.c            70.8/52.6 -> 22.7/16.6
    poolscript_vm.c   82.0/53.4 -> 53.3/39.8
    ps_db.c           71.5/48.9 ->  0.0/ 0.0
    ps_jinker.c       74.0/52.1 ->  0.0/ 0.0
    ps_mongo.c        81.7/40.8 ->  0.0/ 0.0
    ps_pkg.c          61.2/41.8 ->  0.0/ 0.0
    ps_http.c         80.5/64.2 -> 13.3/ 6.4
    ps_hash.c         95.4/76.9 -> 41.5/37.7
    ps_regex.c        85.3/64.9 -> 62.2/45.0
    ps_parser.c       81.4/54.4 -> 93.6/67.1   (subiu — efeito dos geradores)
    ps_lexer.c        68.2/59.3 -> 75.6/67.5   (subiu)

A causa é estrutural, e é dele o diagnóstico: a suíte antiga rodava
**in-process** (`from poolscript import ...`), alcançando banco, jinker, http e
pkgmgr com mock e loopback. A de hoje é 100% fork/exec do `./pool`, e tudo que
precisa de serviço foi empurrado pro `teste/e2e/`, que até ontem não entrava em
portão nenhum. Os cinco arquivos em 0% **não são código novo sem teste — é teste
que existia e se perdeu na migração**. E 7852 casos cobrem menos que 3066 porque
6891 deles (oráculo + diferencial) são snapshot de expressão passando pelo mesmo
caminho.

### O erro que eu cometi aqui

Eu criei `teste/cobertura_base.txt` (o portão por arquivo, commitado em
`6ca7def`) gravando **a medição de hoje** — ou seja, congelei o número
DEGRADADO como piso. Ele avisou antes de eu terminar: *"vou auditar denovo e vai
ter os mesmos erros de cobertura"*. Estava certo.

**O que tem que ser feito com esse arquivo:** o piso não pode ser a medição de
hoje, tem que ser a coluna Python da tabela acima. Ou o baseline vira a meta
(cada arquivo com o número histórico e a data em comentário, reprovando até
alcançar), ou ele fica como está mas com um SEGUNDO arquivo de meta que o
portão também cobra. O que não pode é o piso ser o buraco.

### E `make cobertura` mede menos do que roda

Confira antes de qualquer coisa: o alvo `cobertura` roda `./testar`, mas o que
tira db/jinker/guzer do zero é o `make check-e2e-local` (10 scripts, entrou em
`6ca7def`). Se a medição não incluir o e2e, o baseline nasce zerado de novo por
construção.

## §8 da AUDITORIA-ENGENHARIA — placar

- 1..7, 9 — **feitos** (nos dois commits).
- 8 — **metade**: opcodes unificados (falta a regra no `audita_c.ps`); a
  remoção da era Python está DESFEITA de propósito, ver acima.
- 10 — LICENSE: o impedimento do `libmysqlclient` GPLv2 estático saiu em 31/08
  (MariaDB Connector/C, LGPL 2.1, dinâmico). Falta só escolher a licença. (isso
  contamina a distribuição do binário e precisa de decisão dele), SECURITY.md,
  CONTRIBUTING.md, tags coerentes, SBOM.
- §5, os ilogismos I1..I23 — **não começados**.
- §4.8 — `avisos` usa `-fsyntax-only` (mesmo defeito que já foi corrigido no
  `analisa`) e não tem `-Werror`.
- §4.5 — faltam as anotações de fibra pro ASan, TSan, e o `detect_leaks=0` do
  `check-asan` precisa sair.

## Coisas que valem pra qualquer um que continue

- Máquina com 7,7 GB e ~3,5 GB livres. `gcc -fanalyzer` no `poolscript_vm.c`
  derrubou a sessão inteira DUAS vezes. Sempre `nice -n 19`, um pesado por vez,
  e o arquivo grande fica fora do `analisa` por padrão.
- A suíte inteira em lote de ~6 arquivos; de uma vez o spawn do pool estoura.
- `pkill -f <padrão>` mata o próprio shell (o padrão casa a linha de comando).
  Use `pkill -x -f`, ou deixe o `e2e_roda.ps` fazer.
- `l.sort()` não recebe `key=` — é o contrato dele, documentado. Não é bug.
- `or` no fim da linha, fora de parênteses, NÃO continua a expressão.
