# Decisões de API esperando você — os 13 ilogismos de semântica

Os 23 ilogismos do catálogo foram reproduzidos contra o binário de hoje
(`091a408`). **Nenhum é obsoleto** — todos ainda valem. A classificação:

| classe | quantos | quem resolve |
|---|---|---|
| **motor** — defeito de implementação | 7 | eu, sem perguntar |
| **doc** — só a documentação erra | 3 | eu, sem perguntar |
| **api** — semântica da linguagem | **13** | **você, neste arquivo** |

Este arquivo é só os 13 de API. Escreva sua decisão na linha `DECISÃO:` de cada
um — pode ser "A", "B", "não mexer", ou texto livre. Eu implemento o que estiver
escrito e não toco no que estiver em branco.

O custo de cada opção foi **medido**, não estimado: onde diz "N casos da suíte
dependem disso", o número veio de `grep` nos 7903 casos.

---

## I16 — `pool --check` num arquivo quebrado sai com rc=0

**O mais grave da lista**, e o mais barato de resolver.

```
$ ./pool --check quebrado.ps
{"ok":false,"tipo":"SyntaxError",...}
$ echo $?
0
```

`pool --check f.ps || exit 1` nunca dispara. E o `--check` é o que o editor roda
a cada tecla, o que a CI roda em arquivo que veio de fora, e o que o
`psl install` roda em **pacote de terceiro**.

- **A** — não mexer. Custo zero de código; o falso verde continua.
- **B** — convenção Unix: rc=0 quando `ok:true`, rc!=0 quando `ok:false`. É o
  que fazem `node --check`, `python -m py_compile`, `ruby -c`, `tsc --noEmit` e
  `gofmt -e`. Quebra qualquer script que hoje dependa do rc=0.
- **C** — rc=0 por padrão e `pool --check --strict` pro portão. Não quebra nada,
  mas cria superfície nova e mantém a armadilha como padrão.

**DECISÃO:**

---

## I4 e I5 — três políticas para "índice/chave que não existe"

Numa lista de 3 elementos:

| operação | o que acontece | rc |
|---|---|---|
| `l[99]` (ler) | escreve `IndexOutOfBoundsWarning` no **stderr**, devolve `Null` | **0** |
| `l[99] = x` (escrever) | levanta `IndexError` | 1 |
| `d["falta"]` | levanta `KeyError` | 1 |

E o aviso da leitura **não é exceção** — é `fprintf(stderr)` cru. `try { post(l[99]) } catch (e) { ... }` não pega nada, e não há linha nem coluna.

A leitura é a única das três que **corrompe dado em silêncio** dentro de um
pipeline: o `Null` segue adiante e o erro aparece longe da causa.

Pesa contra mudar: nesta linguagem `Null` é o resultado normal de "faltou" —
`"a"[2]`, `dict.get` ausente e `lista[99]` todos devolvem `Null`, e **70 casos
da suíte** têm `null` como saída esperada.

- **A** — unificar em "avisa e devolve Null": escrita e dict passam a avisar em
  vez de levantar. É a mais frouxa.
- **B** — unificar em "levanta", como Python. Alinha os três e torna o erro
  capturável.
- **C** — manter o padrão e acrescentar `pool --strict`, que promove o aviso a
  erro.
- **D** (só o I5, independente) — manter o `Null`, mas dar **linha e coluna** ao
  aviso, que hoje sai sem endereço nenhum.

**DECISÃO:**

---

## I1, I2, I3 — `Null` compara sem ordem coerente

```
null == 0     -> True        null <= 0     -> False
null == null  -> True        null <= null  -> False
null < 1      -> False       null > 1      -> False      (as duas False)
"abc" < 5     -> LEVANTA SomeValueUnexpected
sorted([3, null, 1]) -> LEVANTA          mas  3 < null -> False
```

Três incoerências que são a mesma raiz: `a == b` não implica `a <= b`; `Null`
compara em silêncio enquanto tipo incompatível grita; e a biblioteca (`sorted`,
`min`, `max`) é mais rígida que o operador que ela usa.

Os agentes acharam que **as duas metades estão documentadas** como decisão
(`vm/poolscript_vm.c:18509` e `:2234`, `docs/linguagem/02-tipos-e-valores.md:122`).
Então isto não é deslize — é escolha sua, que talvez você queira revisar agora
que as três consequências estão juntas na mesma página.

Custo medido: **11 casos** da suíte dependem de "Null ordena e devolve False",
todos em `casos_diferencial.c` e `casos_cobertura.c`.

- **A** — `<=`/`>=` passam a seguir a igualdade (`null <= 0` vira True), `<`/`>`
  continuam False.
- **B** — `Null` vira "menor que tudo": ordem total, `sorted` para de levantar.
- **C** — comparar `Null` com não-`Null` **levanta**, como faz com `"abc" < 5`.
  Alinha as três, e é a que mais quebra.
- **D** — não mexer, e documentar as três juntas num lugar só.

**DECISÃO:**

---

## I6 — `SomeValueUnexpected` é 466 dos erros; `TypeError` são 2

`catch (TypeError e)` é inútil na prática: divisão por zero, chave mutável,
comparação incompatível e falha de I/O caem todas no mesmo balde.

Não é conserto pontual — é a taxonomia de erro da linguagem. Se você quiser
atacar, o caminho é escolher as famílias (tipo / valor / índice / chave / I/O) e
eu reclassifico as 466 ocorrências por família, em lotes por arquivo.

**DECISÃO:**

---

## I7 — `AtributtedValueError` está escrito errado

Tipo de erro **público** com o nome grafado errado ("Atributted" em vez de
"Attributed"), e ele nem aparece em `docs/exceptions/`. Aparece 4 vezes no motor
e **30 vezes** em doc e testes.

- **A** — corrigir a grafia e aceitar os dois nomes por um tempo (quem escreveu
  `catch (AtributtedValueError e)` continua funcionando).
- **B** — corrigir e quebrar: um nome só, a partir de agora.
- **C** — manter a grafia e documentá-la (o nome vira parte da API).

**DECISÃO:**

---

## I8 — `1 / 0` diz "divisão por zero: division by zero"

Mensagem duplicada em dois idiomas. **Atenção**, o agente achou uma armadilha:
apagar a metade inglesa está errado — em `%` o texto inglês é o único lugar que
distingue o caso. A parte da mensagem eu conserto (é motor); o que é seu é o
**tipo**: hoje é `SomeValueUnexpected`, que não diz nada sobre divisão.

- **A** — manter `SomeValueUnexpected`.
- **B** — criar/usar `ZeroDivisionError`, como Python.

**DECISÃO:**

---

## I10 — `is` é documentado como operador de TIPO, mas `5 is 5` dá True

Com lado direito que não é tipo, `is` cai em igualdade de valor sem avisar. Um
`x is 0` digitado por engano vira comparação em vez de erro.

- **A** — erro em tempo de execução quando o lado direito não é tipo.
- **B** — erro em tempo de **compilação** quando o lado direito é literal (o
  parser já separa literal de nome; é a mais barata e a mais cirúrgica).
- **C** — só doc: escrever a escada inteira que o motor executa.

**DECISÃO:**

---

## I11 — não existe `//`, nem `**`, nem `pow()`

```
x = 10000000000000001     # int exato, bignum ok
x / 1   -> 1e+16          # `/` é sempre real
int(x / 1) -> 10000000000000000    # perdeu 1
```

Uma linguagem com inteiro de precisão arbitrária **sem divisão inteira e sem
potência** perde precisão calada na única divisão que tem. Isto apareceu de novo
esta semana: as leis de quociente em `teste/leis.ps` tiveram que ser escritas só
com `%`, porque não há operador exato pra dividir.

- **A** — adicionar `//` (divisão inteira exata).
- **B** — adicionar `//` e `**`.
- **C** — adicionar `//`, `**` e `pow(a, b, mod)`.
- **D** — não adicionar; documentar que `/` é sempre real e que inteiro grande
  não deve ser dividido.

**DECISÃO:**

---

## I18 — módulos com nome de ecossistema alheio e semântica diferente

`flask`, `smtplib`, `mimetext`, `requests`. O caso concreto:
`@app.route("/user/<id>")` + `action perfil()` **não** injeta `id` — é
`request.path_param("id")`. Quem vem do Flask lê o nome e espera a semântica do
Flask.

- **A** — manter os nomes e documentar a diferença em cada página.
- **B** — renomear pros nomes próprios da linguagem, mantendo apelido.
- **C** — fazer a semântica bater onde é barato (injetar o path param no
  parâmetro da action, por exemplo).

**DECISÃO:**

---

## I19 — apelidos duplicados sem canônico anunciado

`request`/`requests`, `qr`/`qrcode`, `manpu`/`mp`, `psodbc`/`db`,
`sqlite`/`sqlite3`. Dois nomes pra mesma coisa multiplicam doc, teste e
completion.

- **A** — manter os dois e **anunciar o canônico** no metadata (campo novo em
  `ModuloNat`); não muda semântica nenhuma.
- **B** — declarar canônico e **depreciar** o outro: continua importando, mas
  avisa.
- **C** — remover os apelidos curtos.

**DECISÃO:**

---

## I22 — três convenções de caixa no mesmo sistema de valores

```
post(true)  -> True          type(true) -> bool
post(null)  -> null          type(null) -> Null
```

`true`/`True` e `null`/`Null` são aceitos como literal, e cada um imprime de um
jeito.

- **A** — padronizar a IMPRESSÃO (ex.: `null` minúsculo em tudo, `True`/`False`
  como Python).
- **B** — padronizar o LITERAL: aceitar um só.
- **C** — padronizar os dois.
- **D** — não mexer; documentar a tabela.

**DECISÃO:**

---

## O que eu faço sem esperar

**motor** (7): I8 (a mensagem bilíngue), I12 (o `}` na linha do último comando),
I13 (`def f(x) {` dando erro de dicionário), I14 (**feito** — a mensagem agora
diz quais tipos), I20 (os 341 parâmetros de módulo sem default), I21b (**feito**
— os 14 parâmetros com espaço no nome), I23 (**feito** — `Null` vs `null`).

**doc** (3): I9 (a doc lista 12 tipos de erro, o motor emite 18), I15
(`docs/sys/argv` ensina `sys.argv[1]` e o motor devolve o argumento em `[0]` —
todo programa que seguir a doc lê errado), I17 (`reaction f():` na doc do
swagger).
