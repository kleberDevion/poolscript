# Memória do projeto — contexto que não está no código

Notas para retomar o trabalho sem redescobrir o que já foi decidido ou
tropeçado. Fatos verificados; o que é decisão aberta está marcado.

---

## Decisão central

**A PoolScript vai ser 100% C, sem CPython em runtime.** Portável como Lua.

Isso não é otimização — é mudança de fundação. Enquanto o interpretador for
escrito em Python, o teto é ~60–160x mais lento que C **mesmo fazendo tudo
certo**. O plano e o estado detalhado estão em [`MIGRACAO_C.md`](MIGRACAO_C.md).

Ordem inegociável (cada camada depende da anterior):

```
lexer → parser → compilador → runtime → binário standalone
 ✅       ✅        🔨 ~30%      ⏸ ~35%        ⏸ 0%
```

---

## O que já foi medido (não repetir a investigação)

### mypyc não resolve

Testado de verdade: mypyc sobre a VM deu **~1,2x** no fib e deixou o loop
**mais lento**. Com tipagem perfeita, teto de **1,5x**. Motivo: o `int` do
mypyc continua sendo objeto do CPython.

O ganho real (**405x** no fib) veio do **modelo de valores próprio** —
`int64_t` numa union etiquetada, sem alocação. Não de "compilar para C".

### PyInstaller não compila para C

O binário em `dist/pool-linux` embute `libpython3.12.so` e um zip de `.pyc`.
É o mesmo bytecode, com **+0,5 s de startup** (descompacta ~50 MB em `/tmp` a
cada execução). "Virar binário" ali é distribuição, não performance.

Ganho fácil disponível: `--onedir` em vez de `--onefile`.

### Onde o tempo ia no tree-walker

Lexer + parser = **0,2 ms** (0,02% do total). 100% do custo estava na
avaliação. Tokenização nunca foi o gargalo.

---

## Limitações da linguagem

Buraco na linguagem é coisa diferente de nó não migrado: é `.ps` que deveria
funcionar e não funciona **em lugar nenhum**, nem no interpretador. Aparecem
por acaso, escrevendo teste para outra coisa.

Ficam catalogadas em [`LIMITACOES.md`](LIMITACOES.md), com a ordem dos 10
arquivos que uma correção precisa atravessar e um teste de regressão em
`tests/test_limitacoes.py`. Já corrigidas: atribuição por índice
(`l[0] = 9` não existia) e `lista + lista` / `lista * int` na VM.

---

## Armadilhas que já custaram tempo

### `setup_vm.py` direto deixa `.so` velho

O setuptools compara timestamps com granularidade de **segundo**. Editar um
`.c` e recompilar no mesmo segundo mantém o `.so` antigo — e o teste passa a
medir código que não existe mais. Aconteceu **duas vezes**, com
"divergências" que sumiam sozinhas.

**Sempre use `./rebuild_vm.sh`.**

### `ru_maxrss` é marca d'água

Não serve para detectar vazamento (nunca desce). Use `/proc/self/statm`.
Detectar vazamento de verdade: rodar N vezes e ver se o RSS **cresce** —
se estabiliza, não é vazamento.

### Ponteiro para dentro de array que faz realloc

Dois bugs assim já apareceram:
- AST guardando `token->texto` (tokens morrem antes da AST) → lixo binário
- `Unidade` guardando `PSProto*` (novo proto → realloc) → corrigido guardando
  o **índice**

Regra: em C, guarde índice, não ponteiro, quando o array pode crescer.

---

## Disciplina que está funcionando

1. **Teste diferencial em tudo.** Nenhuma expectativa escrita à mão — as duas
   implementações rodam o mesmo fonte e o resultado tem que bater. Pegou 5
   bugs que inspeção não pegaria.
2. **ASan em todo lote de C**, incluindo caminhos de erro.
3. **Nó não suportado para com erro explícito.** Nunca gerar bytecode errado
   em silêncio.
4. **Auditar antes de afirmar.** Já reportei "83% migrado" olhando só o eixo
   do parser, enquanto builtins estavam em 5% e string methods em 0%. São
   quatro eixos: parser, statements, builtins, string methods.

---

## Convenções do projeto

- **Versionamento não é semver.** Dígitos 2 e 3 vão de 0 a 99 (base 100).
  Nunca editar à mão — usar `python bump_version.py lang|ext`.
- **Extensão VSCode tem cópia do parser** em
  `psl-poolscript-vsix/bridge/poolscript_pkg/`. Ao mexer em
  `lexer.py`/`parser.py`/`ps_errors.py`, rodar `bridge/sync_parser.py` —
  senão o editor acusa erro em código válido.
- **`.so` da VM não vai pro git** (está no `.gitignore`). Gerar com
  `./rebuild_vm.sh`.
- **Comentários em português**, explicando o *porquê* (não o *quê*).

---

## Bugs conhecidos no lado Python — decisões em aberto

Expostos pela migração, **não corrigidos** de propósito (são decisões suas):

1. **`post([Null])` imprime `[None]`** — o `None` do Python vaza pelo `repr`
   da list. No topo, `post(Null)` imprime `null`. Inconsistência do
   interpretador; a VM em C é consistente. Registrado em teste.
2. **`lista[0] = x` não parseia** — limitação do parser, vale para os dois.
   O `INDEX_SET` da VM já existe, esperando o parser suportar.
3. **`docs/PoolScript.md` documenta `nomes[0] = "joao"`** com saída
   esperada — algo que a linguagem nunca fez.
4. **`CHANGELOG.md` parou na v0.6.1** enquanto `pyproject.toml` está em
   8.2.18. Não confiar nele; usar `git log`.
5. **`test_pkgmgr.py::test_install_command_creates_shim_and_tracks` falha**
   em Linux — espera um shim `.cmd` do Windows. Pré-existente, sempre
   desmarcado nas rodadas.

---

## Onde a linguagem está em velocidade (medido 2026-07-29)

Comparação honesta, melhor de 3/5 execuções, tempo de execução puro (sem o
startup do CPython, que some com o binário standalone):

| | fib(30) | loop 3M | 200k upper() |
|---|---:|---:|---:|
| C -O2 | 0,003 s | 0,006 s | — |
| Node (JIT) | ~0,03 s | ~0,02 s | — |
| Lua 5.4 | 0,083 s | 0,062 s | 0,020 s |
| **PoolScript VM** | **0,189 s** | **0,202 s** | **0,093 s** |
| CPython | 0,220 s | 0,560 s | 0,070 s |
| PoolScript (interpretador antigo) | — | ~47 s | — |

Leitura: a VM está **na classe do CPython** — ganha em laço aritmético (2,7x),
empata em chamada de função, perde em string (2x). Contra Lua fica 2–3,5x
atrás; contra C, ~50–65x. Contra o interpretador antigo, **~250x**.

Os quatro motivos de não estar mais perto do Lua, em ordem de impacto:

1. `acha_metodo_str` faz **strcmp linear** em 44 entradas a cada `.upper()`;
2. VM de PILHA com instrução de 2 slots — o Lua é de registrador e resolve
   `a+b` num despacho só, aqui são LOAD/LOAD/ADD/STORE;
3. `LOAD_NAME`/`STORE_NAME` decidem local-vs-global **em runtime**;
4. string aloca a cada operação, sem interning nem otimização de string curta.

O startup ainda é 0,13 s porque a VM é extensão do CPython (Lua: 0,00 s). Isso
sai com a camada 5.

---

## Ambiente

- Linux Mint 22.3, Python 3.12, gcc 13.3
- `pip` instalado via apt; PyInstaller e mypy via `pip --user`
- ASan disponível (`gcc -fsanitize=address,undefined`); **valgrind não**
- `unixodbc` instalado (necessário pro `pyodbc` no build do PyInstaller)
- Build do binário exige `pip install ".[all]"` — as libs opcionais são
  importadas dentro de funções e o PyInstaller só as empacota se conseguir
  importá-las
