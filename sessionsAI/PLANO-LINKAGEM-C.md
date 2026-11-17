# Plano de retomada — linkagem C (2026-11-17)

> **Modo de plano.** Aprovado em 2026-11-17. A ordem é a do `CLAUDE.md`: lote
> pequeno, plano antes, aprovação dele, depois o código. A primeira entrega é
> a **Fase 0 + fatia 1a** (seção 6).

## 0. Estado de partida

- `main` está em **20.0.49** (commit `1d5213b`, as correções da catraca).
- A catraca `teste/portoes_conhecidas.txt` está **vazia**. O `make check`
  chega até a exceção conhecida de `examples/types.pr` e para ali; os alvos
  depois dele não rodaram nesta rodada. É o esboço do item 16, que o dono
  decidiu manter em `examples/`, e o `teste/exemplos_roda.pr` o executa como
  exemplo. Na Fase 0 o arquivo é reescrito no desenho da seção 2.1 e passa a
  ser pulado com motivo (lista `PROPOSTA`).
- O C1 do Plano 1 está no ramo **`origin/obra/c1`** (`b60e4a3`), parado desde
  o 17.0.19. Ele **não está no main** e não foi mergeado.
- O C1 foi construído sobre uma base de três versões atrás. O main já mudou
  parser, compilador, VM, removeu o `ps_xlsx.c` e fechou dezenas de itens.
  **Merge direto do `obra/c1` não é o caminho**: ele reintroduz comportamento
  do 17.x. O caminho é portar por fatias, com teste em cada uma.

## 1. O que o C1 já entrega

| fatia | commit | o que já existe |
|---|---|---|
| passo 1 | `c22fd9a` | tipos do C e chamada pela libffi (`vm/ps_c.c`) |
| passo 2 | `2a8d39e` | tabela da biblioteca padrão: 13 cabeçalhos, 735 nomes, 300 funções; `c_padrao.decl` → `ps_c_padrao.inc`; conferida contra os cabeçalhos do sistema |
| fatia 1 | `a7bf818` | função do C pelo nome em todo arquivo, sem `import`; constantes; `NULL`; ponteiro opaco (`FILE *`); `str()` do texto do C; `errno`; `--check` e erros |
| fatia 2 | `220e7e5`/`8175c09` | ponteiro e memória: `int *p`, `*p`, `p[i]`, `&x`, `sizeof`, cast, `const` |
| fatia 3 | `7a5b958` | array, `struct`, `union`, `typedef`, `{...}`, `->`, `offsetof`; o programa declara os próprios tipos |
| fatia 4 | `52e0d14` | callbacks (`qsort`), `atexit`/`exit`, `errno` escrito, tratador de queda com a linha |
| fatia 5a | `2831313` | `--check` do C, editor (hover, completion, assinatura), `--metadata` com a chave `c`, realce |
| fatia 5b | `b60e4a3` | doc do C **parcial** (o ramo parou no meio) |
| fatia 5c | — | oráculo do `cc` (um par C/Jinga por função) **não feito** |

O que o C1 **não** tem: `.so`/`.dll` arbitrária; catálogo de libs externas;
`int[]`/`str[]` da linguagem (o C1 tem array do C, `int v[6]`); generics;
a doc completa; a tabela do Windows; a rodada na VPS; o merge como 17.1.0.

## 2. Decisões tomadas (2026-11-17)

| # | decisão | escolha |
|---|---|---|
| 1 | `examples/types.pr` | O `deftype class` sai; fica só `type nome = tipo`, o limitado `type nome = tipo(min=..., max=...)`, o `type enum Nome { membro = tipo; }` e a classe com `@literal`. O arquivo continua em `examples/`, entra na lista `PROPOSTA` do `exemplos_roda.pr` e é pulado com motivo. |
| 2 | como trazer o C1 | Portar por fatias num ramo novo (`obra/c2`) a partir do main, com teste e commit por fatia. |
| 3 | versionamento | C1 = **21.0.0** (a fatia 1a abre a linha); libs externas = **21.1.0**; `T[]`/tipos = **21.2.0**; generics = **22.0.0**. |
| 4 | assinatura de lib externa | Arquivo `.decl` + carga do `.so`; importador de `.h` em tempo de build (`cc -E` → `.decl`), nunca preprocessador em runtime. |
| 5 | `T[]` | Lista de hoje com o tipo do item conferido, crescendo com `addEnd`; decidir o que `type()` devolve. |
| 6 | sintaxe da lib externa | `import c "libfoo.so"` + declarações no `.decl`; fechar na fatia 2a. |

### 2.1. O desenho do item 16 (tipos)

Só estas formas, decididas pelo dono:

```ps
type int16 = int                                  # alias
type Idade = int(min=0, max=150)                  # limitado

type enum Faixa {                                 # enum de tipos
    idade = int(min=18, max=120);
}
Faixa.idade user_age = 23                         # o membro é o tipo

type class Dinheiro {                             # classe que recebe literal
    int centavos = 0
    @literal
    funct __init__(self, flo reais) { ... }
}
Dinheiro preco = 10.50
```

Fora da faixa é erro, antes de rodar quando o valor é escrito e rodando quando
não é. `@literal` pega o tipo do parâmetro do construtor e o passa pra classe;
sem a marca continua a regra de hoje (sem conversão implícita). Nada de
`deftype class`, `callme` ou `Literals`.

## 3. Fases

### Fase 0 — fechar a base (20.0.49)

1. **[feito]** Reescrever `examples/types.pr` no desenho da seção 2.1 e
   pulá-lo no `exemplos_roda.pr` com motivo (lista `PROPOSTA`).
2. **[feito, 2026-11-17]** Rodar `make check` completo e confirmar rc=0. A
   primeira rodada achou um bug real do motor: erro de tipo ESTÁTICO dentro
   de `try { } catch (TypeError e)` impedia o programa de rodar, e o catch
   nunca via (o `r.save(7)` do `jinker_roda.pr`). O conserto está em
   `vm/ps_compiler.c` — o `terro` pergunta à pilha de `try` se o catch pega a
   classe; se pega, o erro fica pro motor, com a mesma frase — e o caso fixo
   entrou em `teste/casos_erros.c`.
   Medido no codespace
   (`symmetrical-space-goldfish-r4qqr6jxxppvcpvg7`, `gh codespace ssh`), o
   único lugar onde o `check` inteiro cabe: suíte 10402/10402 com e sem JIT,
   unidade 249, P1/P2/P3/P4 e os demais portões com 0 falhas conhecidas,
   depurador 26, exemplos 11 rodados + 1 `PROPOSTA` (`types.pr`), analisa,
   e2e local ok — **RC=0**. O P3 é a parte longa (702 s, 1334 blocos,
   10611 hovers).
3. **[feito no `1d5213b`]** Commitar a 20.0.49. A instalação no PATH fica
   para quando o dono quiser (`sudo make install`); o `./pool --version` é
   a conferência desta árvore. No codespace o PATH estava em 20.0.48 com a
   árvore em 20.0.50: `make install` alinhou as duas antes do `check`.
4. **[feito, 2026-11-17]** Criar o ramo `obra/c2` a partir do main
   (`a5d92cd`).

**Portão:** `make check` verde; suíte 10398/10398 com e sem JIT; versões iguais.

> **Onde rodar (2026-11-17):** na máquina local o `check` passa de 120 s e o
> terminal corta. Rode no codespace pelo `gh`:
> `gh codespace ssh -c symmetrical-space-goldfish-r4qqr6jxxppvcpvg7 -- 'cd /workspaces/poolscript && make check'`.
> Primeiro `git pull`, `sudo make install` e o `npm install` do editor.
> A suíte medida no 20.0.50 (após a Fase 0, item 1) é 10402/10402; o número
> da seção 6 é o de 20.0.49.

### Fase 1 — portar o C1 para o main (fatias)

Cada fatia é um commit que compila e passa nos portões; nada de merge.

1. **1a — runtime do C**: `vm/ps_c.c`, `vm/ps_c.h`, `vm/ps_c_padrao.inc`,
   `vm/c_padrao.decl`, `libffi`, unidade C11 e testes de tipo/layout. Sem
   sintaxe nova ainda.
2. **1b — linguagem**: `import <stdio.h>`, nomes e constantes do C, tipos do
   C em variável, `--check`, `--metadata` e editor.
3. **1c — ponteiro e memória**: `*p`, `p[i]`, `&x`, `sizeof`, cast, `const`,
   array, `struct`, `union`, `typedef`, `{...}`, `->`, `offsetof`.
4. **1d — callbacks e fim de programa**: `qsort`/`bsearch`, `atexit`,
   `exit`, `errno`, tratador de queda com linha.
5. **1e — doc e oráculo**: `docs/c/` (13 cabeçalhos), capítulo 15, `audita_c`,
   exemplos novos e o oráculo do `cc` (fatias 5b/5c que faltaram).

**Portão de cada fatia:** unidade C11; `casos_c`; suíte com e sem JIT;
`make check`; P1–P4; `audita_c`; sem regressão de bench (C1 mediu ±0,5%).
Depois da 1e: macOS, Windows/MSYS2 e VPS (check, asan, oom).

### Fase 2 — libs C externas (`.h`/`.so`)

1. **2a — registro de libs**: cada lib tem nome, caminho, lista de símbolos e
   tipos; `dlopen`/`LoadLibrary` e `dlsym`/`GetProcAddress` por lib, com erro
   claro quando falta a lib ou o símbolo.
2. **2b — assinaturas**: o `.decl` é a fonte; o importador de `.h` roda no
   build (`cc -E`) e gera `.decl`. Sem preprocessador no runtime.
3. **2c — catálogo**: OpenSSL (`libssl`/`libcrypto`), zlib e o que o projeto
   já usa; cada lib com exemplo e teste de chamada real.
4. **2d — `sockets` + OpenSSL**: expor as funções do C e escrever o exemplo
   de conexão TLS; o Dev monta a `conn` SSL no programa, como combinado.
   Se a sintaxe ficar boa, uma `sockets` fina por cima; o wrapper não esconde
   o C.

**Portão:** chamada real a uma `.so`; erro de lib/símbolo; `sizeof`/`offsetof`
conferidos com o `cc`; nenhum custo para quem não importa C; doc no mesmo
commit.

### Fase 3 — tipos da linguagem

1. **`T[]`** (item 13): parser, checador, VM, `--metadata`, editor e doc.
2. **enums**: o C1 traz o enum do C como inteiro; o enum da Jinga já existe.
   Fechar a convivência dos dois e o `type()`.
3. **`struct`/`typedef`**: o C1 já tem o do C; decidir se a Jinga ganha um
   `struct` próprio ou se o do C é a forma.
4. **tipo limitado** (item 16): `type`, `type enum` e `@literal`, conforme a
   seção 2.1, separado do `T[]`.

**Portão:** P1–P4, suíte, editor, doc; cada tipo novo com erro antes de rodar
quando o valor é escrito e rodando quando não é.

### Fase 4 — generics

Depois de `T[]` e do sistema de tipos estável. Decidir a sintaxe (Java/C# ou
outra), se o tipo é apagado ou reificado, e se aceita limite. Portão: suíte,
P1–P4, editor, doc e bench.

## 4. Portões e critérios de pronto

- **Todo commit**: `make check`; `./testar` com e sem JIT; `teste/unidade`;
  `casos_c`; `audita_c`; P1–P4.
- **C ABI**: `sizeof`, `alignof` e `offsetof` contra o `cc` do sistema;
  tabela por plataforma.
- **Cross-platform**: Linux (esta máquina), macOS, Windows/MSYS2; VPS para
  `check`, `check-asan` e `oom`.
- **Custo**: os 13 casos do bench dentro de ±0,5%; programa que não usa C não
  paga libffi, tabela nem relocação.
- **Segurança**: erro antes de rodar quando o tipo se sabe; queda do C com
  linha da Jinga; ownership de memória documentado; C é inseguro por
  natureza, e a doc diz isso.
- **Doc**: mesma entrega do código; exemplo que roda no P1.

## 5. Riscos

- **Conflito de três versões**: parser/compilador/VM do main mudaram muito
  desde o 17.0.19; por isso fatia, não merge.
- **libffi**: dependência nova; no Linux/macOS é aberta na primeira chamada,
  no Windows é estática. O pacote e o instalador precisam saber.
- **Cabeçalhos reais**: macros, `#if`, atributos e `inline` não são C puro de
  declaração; o `.decl` + importador de build evita carregar isso no runtime.
- **ABI**: `long` 4/8, alinhamento de struct, variádicas e convenção de
  chamada mudam por sistema; a tabela medida é a régua.
- **Memória**: `&x` de variável da Jinga, GC e `free` do C; ponteiro para
  valor que o GC move é armadilha. O C1 já fixa `&x` numa caixa; revisar.
- **Generics**: se entrar antes do `T[]` e da ABI, vira retrabalho.

## 6. Primeira entrega aprovada

**Fase 0 + fatia 1a**, nesta ordem:

1. reescrever `examples/types.pr` no desenho da seção 2.1 e pulá-lo no
   `exemplos_roda.pr` com motivo;
2. rodar `make check` completo e confirmar rc=0;
3. commitar a 20.0.49;
4. criar `obra/c2`;
5. portar `ps_c.c/h`, `ps_c_padrao.inc`, `c_padrao.decl` e a unidade C11,
   sem sintaxe nova, com bump **21.0.0**;
6. rodar unidade C11 e `make check`.

Depois da 1a, a 1b entra com o `import <stdio.h>` e o `--check`.

## 7. Aprovação

Aprovado pelo dono em 2026-11-17: fatias no `obra/c2`, C1 como 21.0.0,
`.decl` com importador de `.h` no build, `T[]` como lista com item conferido,
primeiro lote Fase 0 + 1a. `types.pr` fica em `examples/` e é pulado com
motivo. Nenhuma decisão aberta.
