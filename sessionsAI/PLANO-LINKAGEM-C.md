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
| 6 | sintaxe da lib externa | `import` da lib, **vale para qualquer lib C**: cabeçalho (`import <stdio.h>`) ou binário (`import c "libfoo.so"`, `"libfoo.dll"`, `"libfoo.dylib"`, `"libfoo"`). O `.decl` guarda a assinatura; o importador de `.h` no build (`cc -E`) gera o `.decl`. Fechar na fatia 2a. |
| 7 | **lista de libc** (2026-11-17) | **Não existe.** O acesso ao C é **total**: qualquer função de qualquer lib C (`.h`, `.so`, `.dll`, `.dylib`), sem catálogo de nomes que a linguagem imponha. O que **não** é opcional é o `import` da **lib**: para chamar, o programa diz de onde vem. A tabela do C1 (`c_padrao.decl`, 735 nomes) é a **assinatura** desses nomes (o tipo), não uma licença de uso. |
| 8 | **`import` de lib** (2026-11-17) | **Obrigatório, para qualquer lib C** — `.h`, `.so`, `.dll`, `.dylib`. Sem o import da lib, o nome não existe: `NameError`, como hoje. O import dá a lib ao programa (carrega o binário e/ou as assinaturas). |

### 2.2. O que muda na tabela do C1 (abre a 1b)

A regra, em duas linhas: **assinatura do C = sempre; lib = só com `import`**.

| hoje | depois |
| --- | --- |
| achar a função **só** pelo nome na tabela; o resto é `NameError` | a tabela dá o **tipo** do nome (rápido, para o `--check`); a função é resolvida no símbolo (`dlsym`) quando a chamada roda |
| `import <stdio.h>` obrigatório | **continua obrigatório** — é o que carrega a lib. O que muda é que a lista da tabela deixa de ser o limite: qualquer nome da lib importada vale |
| nome fora da tabela: `NameError` | com o cabeçalho importado e sem tipo na tabela, o programa **escreve a declaração** (`ps_c_le_decl`) e chama; o tipo escrito vale para o `--check` |
| `ps_c_especie == PSC_NOME_FORA` recusa (`setjmp.h`, `vprintf`, `va_list`) | nada é recusado por estar fora da tabela. `vprintf` chama com `va_list` declarado; `setjmp`/`longjmp` chama (o risco é do programa, e a doc diz) |
| macros (`isnan`, `INT64_C`, `NULL`, `EOF`) | valor de macro **só pela tabela** (o C não expõe macro no `.so`): o que não está lá tem que ser escrito pelo programa. Limite do C, não da Jinga |

O que **não** muda: a tabela continua gerada e conferida contra os cabeçalhos, porque é ela que dá tipo e aridade ao `--check`. Regra para quem for mexer: a tabela **dá assinatura**; o `import` **é que libera a lib**. Faltar na tabela não impede chamar; faltar o `import` impede.

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
2. **[em andamento]** Rodar `make check` completo e confirmar rc=0. A
   primeira rodada achou um bug real do motor: erro de tipo ESTÁTICO dentro
   de `try { } catch (TypeError e)` impedia o programa de rodar, e o catch
   nunca via (o `r.save(7)` do `jinker_roda.pr`). O conserto está em
   `vm/ps_compiler.c` — o `terro` pergunta à pilha de `try` se o catch pega a
   classe; se pega, o erro fica pro motor, com a mesma frase — e o caso fixo
   entrou em `teste/casos_erros.c`.
3. **[feito no `1d5213b`]** Commitar a 20.0.49. A instalação no PATH fica
   para quando o dono quiser (`sudo make install`); o `./pool --version` é
   a conferência desta árvore.
4. Criar o ramo `obra/c2` a partir do main.

**Portão:** `make check` verde; suíte 10398/10398 com e sem JIT; versões iguais.

### Fase 1 — portar o C1 para o main (fatias)

Cada fatia é um commit que compila e passa nos portões; nada de merge.

1. **1a — runtime do C**: `vm/ps_c.c`, `vm/ps_c.h`, `vm/ps_c_padrao.inc`,
   `vm/c_padrao.decl`, `libffi`, unidade C11 e testes de tipo/layout. Sem
   sintaxe nova ainda.
2. **1b — linguagem**: o `import` do **cabeçalho obrigatório** (decisão 8)
   libera a lib; a tabela dá o tipo, e o nome sem tipo na tabela recebe a
   declaração escrita no programa; tipos do C em variável; `--check`,
   `--metadata` e editor.
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

### Fase 5 — performance da linguagem (visão do dono, 2026-11-17)

Depois da linkagem e dos generics. A régua é o `make bench` e o
`teste/bench.pr`: medir antes, mudar, medir de novo. O alvo declarado por ele
é **programas complexos e longos competitivos com Go e Rust**. Sem plano
ainda; abre quando a Fase 4 fechar, com o estado medido do bench no dia.

### Fase 6 — as libs em Jinga, fora do binário (visão do dono, 2026-11-17)

**Reescrever as libs em Jinga e mandá-las para `kleberDevion/libs`**, para
diminuir o peso do binário na compilação. Hoje o `pool` liga mail, http, qr,
db, mongo, jinker, pkg, debug, so, unicode e a tabela do C num executável
só; quem usa `post()` paga o resto. A direção é a mesma que já moveu os
clientes de banco e a `libffi` para o `dlopen` no primeiro uso (`vm/ps_dl.h`):
o que é lib sai do motor.

O caminho que a Fase 2 abre: a lib em Jinga é um pacote `jpkg` que declara o
que usa e é compilado junto com o programa, sem libffi. Antes disso, decidir
onde o código da lib vive (repo separado), como o motor o carrega sem embutir
o binário, e como o instalador sabe o que instalar. Mede-se pelo binário
(tamanho, tempo de partida) e pelo bench — nunca por impressão.

Ver `sessionsAI/VISAO-2026-11-17.md`, com o registro da ordem dele.

## 4. Portões e critérios de pronto

### 4.0. Quem diagnostica o C é o `cc`/`clang` (decisão dele, 2026-11-17)

**A Jinga não reimplementa diagnóstico de C.** O compilador C do sistema já é
o oráculo, e é ele que decide:

- tipo errado numa chamada do C, aridade, conversão, o layout de `struct`;
- segfault, ponteiro inválido, uso depois de `free`, estouro de buffer;
- os avisos e os endereços do C11 (o `unidade_c11.c` é exatamente isso).

O papel da Jinga: **repassar** a declaração ao `cc`/`clang` (o importador de
`.h` no build: `cc -E` → `.decl`; e o oráculo dos 13 casos que roda o par
C/Jinga), **mostrar a frase dele** quando a chamada falha, e não inventar
regra própria onde a do C vale. O `--check` só confere o que a Jinga sabe sem
rodar (tipo conhecido, aridade, o formato do `printf`); o que depende do C é
do C. Nada de reescrever analisador semântico de C em `.pr`.

Em prática, na 1d/1e: o tratador de queda mostra a linha da Jinga e repassa
ao `cc` a função e os tipos para ele apontar o erro; o `audita_c` e o oráculo
do `cc` são de **build**, não portões que repetem a análise do compilador.

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

Depois da 1a, a 1b entra com o `import <stdio.h>` (cabeçalho, a lib do
sistema) e o `--check`; o `import` de `.so`/`.dll` de terceiros entra na
Fase 2, com o mesmo obrigatório.

## 7. Aprovação

Aprovado pelo dono em 2026-11-17: fatias no `obra/c2`, C1 como 21.0.0,
`.decl` com importador de `.h` no build, `T[]` como lista com item conferido,
primeiro lote Fase 0 + 1a. `types.pr` fica em `examples/` e é pulado com
motivo. Nenhuma decisão aberta.

Aprovado também, no mesmo dia, a **decisão 7**: lista de libc para limitar o
que o programa alcança não faz sentido; o acesso é total. A tabela do C1 é
ajuda de tipo, não porteiro.
