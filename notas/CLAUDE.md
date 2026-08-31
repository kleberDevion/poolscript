# PoolScript

## Versionamento

O projeto **não usa semver**. Os dígitos 2 e 3 vão de `0` a `99`; ao chegar em
100 eles zeram e somam 1 no dígito à esquerda (base 100):

```
8.2.18  -> 8.2.19
8.2.99  -> 8.3.0     (100 nunca aparece)
8.99.99 -> 9.0.0
```

A versão da **linguagem** tem uma fonte só, e é constante de C:

- `vm/ps_versao.h` — `#define PS_VERSAO "..."`, que o compilador embute no
  binário (nada de scrape de arquivo no build);
- `docs/PoolScript.md` cita a mesma versão no título, no exemplo do
  `pool --version` e no banner do REPL — sobe junto, pra doc não defasar.

Nunca edite na mão — use o script, que aplica o rollover e mantém tudo em
sincronia:

```bash
./pool bump_version.ps        # só valida, não altera
./pool bump_version.ps lang   # +1 na linguagem
```

Mudar a versão **força o relink**: o alvo `pool` depende de `ps_versao.h`, senão
o `pool --version` ficaria preso no valor antigo (as fontes `.c` não mudaram).

## Build

Tudo é C. Makefile, fontes e binário na raiz:

```bash
make pool       # gera ./pool
make check      # compila o binário + a suíte e roda
make verifica   # ldd, símbolos de Python (tem que ser vazio), tamanho
make bundle     # dist/pool-portable/ (VPS sem apt install)
```

Precisa de `gcc` e das libs de dev: postgresql, mariadb (`libmariadb-dev`),
mongoc, openssl.

Entram **estáticos** no ELF: sqlite, libpq, odbc, openssl, png,
expat, z. Ficam **dinâmicos**: `libmongoc`/`libbson` (não têm `.a` no sistema),
a cauda de auth do libpq (ldap/gssapi/gnutls/krb5), libX11, libgmp e o glibc.

### Bundle portátil (rodar em VPS sem apt install)

`make bundle` roda o `build_bundle.sh` e monta
`dist/pool-portable/` = `pool` (wrapper) + `pool.bin` + `lib/` com TODAS as
`.so` (via `ldd`); o wrapper aponta `LD_LIBRARY_PATH` pra esse `lib/`. NÃO
empacota o núcleo do glibc (libc/m/pthread/dl/rt/resolv + loader) — esse vem do
alvo, senão o getaddrinfo/DNS (NSS) quebra. glibc é retrocompatível, então o
único requisito do VPS é ter glibc ≥ a do build. Gera também
`dist/pool-portable.tar.gz`; no VPS:
`tar xzf … && ./pool-portable/pool app.ps`.

## Testes: em C, rodando o binário de verdade

A suíte vive em `teste/`. Cada caso é um programa `.ps` + o que se espera dele
(stdout exato, trecho do stderr, código de saída), e o runner faz **fork/exec do
`./pool`** — um subprocesso por caso. Isso é deliberado: boa parte dos casos
existe porque a VM MORREU (segfault, SIGFPE, OOM) rodando aquele fonte, e teste
que morre junto não relata nada. Com subprocesso, morte vira resultado
(`WIFSIGNALED`), e trava vira `SIGALRM`.

```bash
make check    # tudo
./testar linguagem                # só um grupo
./testar -v "closure"             # filtra pelo nome do caso
```

Grupos: `crash`, `inteiros`, `erros`, `linguagem`, `pendentes`, `cobertura`,
`diferencial`, `equivalencia`, `oraculo`, `robustez`. O grupo
**`pendentes`** é a fila de trabalho: cada caso lá dentro codifica o
comportamento CORRETO de algo que ainda não funciona — ele falha de propósito
até a correção entrar. Nada de skip, nada de teste que passa escondendo erro.

**Um número só.** Não existe varredura "por fora" que rode à parte e dê outro
placar — tudo o que se mede entra em `make check`. Três grupos são **gerados**
(não se edita na mão; se o comportamento mudou, regera e o diff mostra o que
mudou):

| Grupo | Gerador | O que trava |
|---|---|---|
| `diferencial` | — (colhido de um commit) | a saída de ontem, caso a caso |
| `equivalencia` | `teste/geradores/equivalencia.ps` | formas redundantes têm que concordar entre si |
| `oraculo` | `teste/geradores/oraculo.ps` | o produto (receptor × método × argumento) contra o **Python** como oráculo |
| `robustez` | `teste/geradores/robustez.ps` | chamadas com aridade errada, uma por tipo |

Os geradores são PoolScript rodando no `./pool` — não há Python nem JavaScript
no projeto. O `oraculo` consulta o CPython como ferramenta **externa** (escreve
um driver em `/tmp` na hora e o chama), do mesmo jeito que outro teste consulta
um banco de dados: nenhum `.py` fica versionado aqui.

O `oraculo` guarda o valor que a linguagem produz hoje e marca `DIVERGE` nos
casos em que o Python daria outra coisa, com o valor dele no comentário — a
diferença fica no arquivo, visível, em vez de sumir. As divergências que
sobram são decisão de projeto (índice fora da faixa → `Null`, `.len()` como
método, `.keys()` devolvendo lista, `type()` com nome curto, `Null` no lugar de
`None`). Quantas são, hoje: o cabeçalho de `teste/casos_oraculo.c` diz — ele é
reescrito pelo gerador.

O `robustez` é um caso por TIPO, não por chamada — um subprocesso por chamada
levaria mais de um minuto. Cada programa roda as chamadas erradas do seu tipo
em `try/catch` e relata as que passaram **caladas**, com o método. Crash e trava
viram falha pelo `WIFSIGNALED`/`SIGALRM` do runner.

## Scripts de apoio: em PoolScript

Automação, auditoria e smoke tests de apoio são escritos em `.ps` e rodados no
`./pool`. Todo tropeço escrevendo `.ps` é bug ou limitação candidata — anotar em
`notas/LIMITACOES.md` ou corrigir na hora.

## Editor: servidor LSP, em PoolScript

O suporte a editor é um **servidor LSP escrito em PoolScript** (`lsp/`), rodado
pelo `pool`. Não há JavaScript no projeto, e não há extensão VS Code
versionada aqui — qualquer editor que fale LSP conversa com esse servidor.

O modelo de tipos vem do próprio binário: `pool --metadata` lista módulos,
membros, tipos e métodos lidos das tabelas do VM. É o que alimenta o hover —
a assinatura real e o tipo de retorno, sem nada digitado à mão.

O completion é type-aware pela mesma fonte, e cada sugestão carrega nome,
assinatura com tipo de retorno e a **prosa de `docs/`** — a mesma página que o
`scripts/audita_doc.ps` confere. Palavra da linguagem NÃO entra: sugestão sem
conteúdo empurra a útil pra baixo.

## Doc: assinatura vem do CÓDIGO, nunca digitada

O título `# \`lib.fn(a, b=1)\`` de cada página `docs/<lib>/<m>/<m>.md` (e
`docs/<lib>/<Classe>/<m>/<m>.md`) tem que bater com a assinatura real que o
motor expõe — nomes E defaults. A fonte é `pool --metadata`, não a memória de
ninguém.

Todo método de `str`/`list`/`dict`/`tup` do motor precisa ter página: as tabelas
`METODOS_*` de `vm/poolscript_vm.c` são a lista de verdade.

## Mudou o comportamento? Atualiza TUDO na mesma mudança

O que descreve o comportamento faz parte do comportamento. Trocar o motor e
deixar a descrição pra trás não é "faltou doc": é criar um documento que MENTE,
e depois acreditar nele.

Já custou caro duas vezes, e as duas nesta ordem:

- **I11** tirou o `//` de comentário e ninguém tocou em `examples/`. Os 16
  exemplos — a primeira coisa que se lê pra aprender a linguagem — quebraram
  com `SyntaxError: caractere inesperado: 'ê'` num comentário, e quatro dias
  depois isso foi investigado como se fosse defeito novo do lexer.
- **`notas/ROTA.md`** listava como abertas as fases 0, 1 e 3, todas fechadas no
  código. A lista foi lida e repassada como pendência real.

Então, no MESMO commit que muda comportamento:

| mudou | atualiza também |
|---|---|
| tabela `METODOS_*`, nativa, operador | `docs/<tipo>/…`, `docs/linguagem/12`, e o gerador de doc quando houver |
| mensagem de erro | os casos que a esperam, e `docs/exceptions/` |
| sintaxe (o `//` é o caso-tipo) | `examples/`, `docs/linguagem/01…`, realce do editor |
| qualquer item de `notas/ROTA.md` ou `TAREFAS.md` | a linha correspondente, ou apaga |

E NÚMERO MEDIDO NÃO SE ESCREVE EM PROSA. Cobertura, contagem de caso, contagem
de divergência: ou sai de um comando (`make cobertura`, `./testar`), ou não
entra no texto. Número em prosa envelhece calado e depois é citado como fato —
foi o que fez a ROTA dizer `ps_mail.c 16,9%` quando eram 56,9%.

Quem cobra: `make check` roda `audita_doc`, `audita_exemplos_doc`,
`exemplos_roda`, `confere_metadata`, `confere_assercoes` e `confere_duplicados`.
Rode o PORTÃO INTEIRO antes de dizer que terminou — rodar só `./testar` é como
a quebra dos exemplos passou.

## Regra que não se negocia

Nada de falso verde. Skip, `try` que engole erro, teste ajustado pro bug —
nenhum dos três. Se não passa, ou corrige o motor, ou o caso vai pra
`teste/casos_pendentes.c` falhando de propósito, com o comportamento certo
escrito.
