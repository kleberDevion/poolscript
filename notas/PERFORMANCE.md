# Performance da VM — pesquisa dele, confrontada com o que o motor faz hoje

Anotado em 2026-08-28, a partir da pesquisa que ele trouxe. A máquina alvo é a
dele: **Intel i3-1115G4, 2 núcleos / 4 threads, L1 ICache de 32 KB**.

**Fora do escopo:** o trecho da pesquisa sobre interpretação (árvore de nós,
sobrecarga de interpretar em vez de compilar) não se aplica — o motor JÁ
compila pra bytecode em array plano, a AST morre no compilador e nada dela
participa da execução. O que sobra dali de útil é só o que fala do LAÇO DE
DESPACHO do bytecode, que é assunto de qualquer VM, interpretada ou não.

A ordem aqui é deliberada: **medir primeiro**. Cada item abaixo diz o que a VM
faz HOJE, verificado no código, e o que a pesquisa recomenda. Nenhum deles
entra sem `perf` mostrando que aquele é o gargalo — otimização escolhida por
intuição é o mesmo erro dos tetos por contagem, com outra roupa.

---

## 0. Medir antes (pré-requisito de tudo)

```
perf record -g ./pool script.ps
perf report
perf stat -e branch-misses,frontend_retired.l1i_miss,cycles,instructions ./pool script.ps
```

O que o perfil decide:

| sinal | gargalo | item que ataca |
|---|---|---|
| `frontend_bound` alto, `l1i_miss` alto | o laço de dispatch não cabe na L1 | §3 |
| `branch-misses` alto | previsão do salto indireto único | §1 |
| tempo em `gc_coleta`/`marca_obj` | GC | §5 |
| tempo em `malloc`/`free` | alocação por instrução | §6 |

**Não existe benchmark no projeto** (é o §4.6 da AUDITORIA-ENGENHARIA, ainda
aberto). A era Python tinha `bench_async.sh` e `bench_webhook.sh`; hoje não há
nada, então nem dá pra dizer se uma mudança melhorou. Isso vem ANTES de
qualquer item desta lista.

---

## 1. Dispatch: `switch` → computed goto

**Hoje:** `switch (o)` em `poolscript_vm.c:18175`. O gcc gera uma tabela de
salto, mas com **um único salto indireto** pra todos os opcodes — o preditor do
i3 vê sempre o mesmo endereço de branch e erra quase sempre que o opcode muda.

**A mudança:** `goto *tabela[op]` no fim de CADA caso. Isso dá um salto
indireto POR OPCODE, e o preditor passa a aprender os pares comuns
(`LOAD_LOCAL`→`ADD`, `ADD`→`STORE_LOCAL`). É o que CPython faz desde o 3.11 e o
que Lua sempre fez.

**A favor aqui:** `vm/ps_opcodes.def` já existe. A tabela sai do mesmo X-macro
que o enum, então não há terceira lista pra dessincronizar:

```c
static void *TAB[] = {
#define PS_OP(nome, num, texto) [num] = &&L_##nome,
#include "ps_opcodes.def"
#undef PS_OP
};
```

Custo: é extensão GNU (`&&label`). O projeto já usa `__thread`,
`__builtin_frame_address` e `-fanalyzer`, então não é novidade — mas precisa de
`#ifdef __GNUC__` com o `switch` como alternativa, senão trava o motor a um
compilador só.

## 2. Bytecode contíguo — **assunto encerrado**

`PSProto.code` é `int32_t *` com pares `[opcode, arg]` num array plano
(`ps_compiler.h:43`). Sem árvore de nós na execução. Nada a fazer.

O único resíduo, e só se o `perf` pedir: os pares são dois `int32_t` (8 bytes por
instrução). Compactar pra 4 bytes (opcode em 8 bits + arg em 24) dobra a
densidade da ICache/DCache de bytecode. Mas 24 bits limitam o arg a 16 milhões,
o que muda formato de bytecode — não vale sem número do `perf`.

## 3. O laço cabe na L1 ICache? — **quase certamente NÃO**

O `switch` vai da linha 18151 à 20656: **~2500 linhas de C** num corpo só. A
L1 ICache do i3 é de 32 KB. Não tem como caber, e a pesquisa é explícita: se o
laço estoura a ICache, o ganho de qualquer outra otimização some.

**A saída conhecida:** separar os opcodes QUENTES (aritmética, comparação,
load/store local, salto, chamada) num laço enxuto, e mandar os frios
(`MAKE_CLASS`, `IMPORT_MOD`, `MAKE_MODEL`, tudo de I/O) pra funções
`noinline` chamadas de fora. O laço quente encolhe pra alguns KB e passa a
caber.

Isto é o que dá o maior ganho antes do JIT, e é o mais barato de fazer — não
muda semântica nenhuma, só move código.

## 4. `ip`/`sp` em registrador

**Hoje:** `ip`, `sp`, `stack` e `locals` já são variáveis locais do `roda()`
(não campos do `VM` relidos a cada instrução), o que é metade do caminho. O
`vm->alocado > vm->proximo_gc` no topo do laço, esse sim, lê a heap TODA
iteração — dá pra checar só nos opcodes que alocam.

## 5. GC

**Hoje:** mark-sweep, com `marca_obj` recursivo. Duas coisas conhecidas:

- **recursão no marcador** — a mesma família dos estouros já corrigidos; uma
  estrutura funda pode empilhar tão fundo quanto ela for. Vale a mesma medição
  de folga (`ps_pilha_apertada`) ou uma pilha explícita de marcação.
- **stalls**: num i3 uma coleta que pare vários ms é perceptível. O caminho
  usual é geracional (a maioria dos objetos morre jovem) ou incremental.

A pesquisa sugere evitar GC por escopo estático. **Não se aplica**: a linguagem
tem closure e estrutura que escapa do escopo — sem GC ela deixa de ser o que é.
Bump allocator por request já existe no jinker (arena por requisição).

## 6. Pool de valores pequenos

**Hoje:** não existe cache de inteiros pequenos. `V_INT` é imediato dentro do
`Value` (não aloca), então este item vale menos aqui que no CPython — o que
aloca é string, list e dict. Um pool de `PSString` curtas seria o equivalente
útil, e o alvo natural é concatenação em laço.

## 7. JIT / AOT

A pesquisa põe 10–25× no JIT, e é verdade — e também é o item mais caro do
arquivo inteiro, por ordens de grandeza. A linguagem é **dinâmica** (`Value`
com tag em tempo de execução), então seria JIT com type specialization, não
AOT.

Não antes de §1 e §3, que são baratos e cujo ganho é medível numa tarde. E não
antes de existir benchmark: sem número, não há como saber se o JIT ganhou.

Detalhe da pesquisa que vale guardar: `mmap` com `PROT_EXEC` pode ser negado
por SELinux/AppArmor. Se um dia houver JIT, precisa de caminho alternativo
(interpretar) quando o `mprotect` falhar — nunca abortar.

## 8. Governor da CPU

Não é mudança de código, é da medição: em `powersave` o número do `perf` varia
com a temperatura e não serve pra comparar.

```
cpupower frequency-set -g performance     # enquanto mede
```

Anotar no benchmark qual governor estava ativo, senão duas medições não são
comparáveis.

---

## Ordem sugerida

1. **benchmark** no repositório (§4.6 da auditoria) — sem isso nada aqui é
   verificável;
2. `perf record` num script representativo, pra escolher com dado;
3. **§3** (encolher o laço quente) — maior ganho, sem mudar semântica;
4. **§1** (computed goto pelo `.def` que já existe);
5. §4 (checagem de GC só onde aloca), §5 (marcador não recursivo);
6. §7 (JIT) só depois que 3 e 4 estiverem medidos.
