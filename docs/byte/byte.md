# Métodos de `byte`

O tipo do valor binário chama-se `byte` (`type("oi".encode())` → `byte`); as
funções de **criar e converter** — `bytes.new`, `bytes.fromhex`, `bytes.xor`… —
são do **módulo** `bytes`, em [`docs/bytes/`](../bytes/bytes.md). Esta página é
só do que se chama NO valor: `b.metodo()`, não `bytes.metodo(b)`.

Cada página tem a assinatura tirada do motor (`jinga --metadata`) e a saída de um
exemplo que foi RODADO — as 44 são geradas por `scripts/gera_bytes_docs.pr`.

Os nomes são os mesmos do `str`, e por isso vale ler as
[quatro diferenças](../linguagem/12-metodos-string-list-dict.md) — são as que
se erra por analogia (caixa só ASCII, `find`/`count`/`index` aceitam inteiro,
`strip` trata o argumento como conjunto de bytes).

**Buscar**

| Método | O que faz |
|---|---|
| [`b.find(sub, inicio=0, fim=Null)`](find/find.md) | posição da primeira ocorrência, ou -1 |
| [`b.rfind(sub, inicio=0, fim=Null)`](rfind/rfind.md) | posição da última ocorrência, ou -1 |
| [`b.index(sub, inicio=0, fim=Null)`](index/index.md) | como `find`, mas levanta se não achar |
| [`b.rindex(sub, inicio=0, fim=Null)`](rindex/rindex.md) | como `rfind`, mas levanta se não achar |
| [`b.count(sub, inicio=0, fim=Null)`](count/count.md) | quantas vezes aparece |
| [`b.contains(sub)`](contains/contains.md) | True se contém |
| [`b.has(sub)`](has/has.md) | o mesmo que `contains` |
| [`b.startswith(prefixo, inicio=0, fim=Null)`](startswith/startswith.md) | começa com? aceita tupla de opções |
| [`b.endswith(sufixo, inicio=0, fim=Null)`](endswith/endswith.md) | termina com? aceita tupla de opções |

**Caixa** — todos mexem SÓ no ASCII

| Método | O que faz |
|---|---|
| [`b.upper()`](upper/upper.md) | maiúsculas |
| [`b.lower()`](lower/lower.md) | minúsculas |
| [`b.title()`](title/title.md) | inicial de cada palavra em maiúscula |
| [`b.capitalize()`](capitalize/capitalize.md) | só a primeira em maiúscula |
| [`b.swapcase()`](swapcase/swapcase.md) | troca maiúscula por minúscula |

**Perguntar**

| Método | O que faz |
|---|---|
| [`b.isalpha()`](isalpha/isalpha.md) | só letras ASCII? |
| [`b.isdigit()`](isdigit/isdigit.md) | só dígitos? |
| [`b.isalnum()`](isalnum/isalnum.md) | só letra ou dígito? |
| [`b.isspace()`](isspace/isspace.md) | só branco? |
| [`b.isupper()`](isupper/isupper.md) | tem letra e nenhuma minúscula? |
| [`b.islower()`](islower/islower.md) | tem letra e nenhuma maiúscula? |
| [`b.istitle()`](istitle/istitle.md) | está em formato de título? |
| [`b.isascii()`](isascii/isascii.md) | todo byte < 0x80? (vazio é True) |

**Aparar e trocar**

| Método | O que faz |
|---|---|
| [`b.strip(chars=Null)`](strip/strip.md) | tira das duas pontas; `chars` é CONJUNTO |
| [`b.lstrip(chars=Null)`](lstrip/lstrip.md) | só da esquerda |
| [`b.rstrip(chars=Null)`](rstrip/rstrip.md) | só da direita |
| [`b.removeprefix(p)`](removeprefix/removeprefix.md) | tira o prefixo, se estiver lá |
| [`b.removesuffix(p)`](removesuffix/removesuffix.md) | tira o sufixo, se estiver lá |
| [`b.replace(old, new, count=-1)`](replace/replace.md) | troca ocorrências |
| [`b.translate(tabela, delete=Null)`](translate/translate.md) | traduz byte a byte |
| [`b.maketrans(de, para)`](maketrans/maketrans.md) | monta a tabela do `translate` |

**Partir e juntar**

| Método | O que faz |
|---|---|
| [`b.split(sep=Null, maxsplit=-1)`](split/split.md) | parte no separador, ou em branco |
| [`b.rsplit(sep=Null, maxsplit=-1)`](rsplit/rsplit.md) | o mesmo, contando do fim |
| [`b.splitlines(keepends=false)`](splitlines/splitlines.md) | parte em linhas (`\n`, `\r`, `\r\n`) |
| [`b.partition(sep)`](partition/partition.md) | `(antes, sep, depois)` na primeira |
| [`b.rpartition(sep)`](rpartition/rpartition.md) | o mesmo, na última |
| [`b.join(lista)`](join/join.md) | junta usando este valor como separador |

**Preencher**

| Método | O que faz |
|---|---|
| [`b.ljust(width, fillbyte)`](ljust/ljust.md) | enche à direita |
| [`b.rjust(width, fillbyte)`](rjust/rjust.md) | enche à esquerda |
| [`b.center(width, fillbyte)`](center/center.md) | centraliza |
| [`b.zfill(largura)`](zfill/zfill.md) | zeros à esquerda, respeitando o sinal |
| [`b.expandtabs(tabsize=8)`](expandtabs/expandtabs.md) | tabulação vira espaços |

**Converter**

| Método | O que faz |
|---|---|
| [`b.decode(encoding="utf-8", errors="strict")`](decode/decode.md) | volta pra texto |
| [`b.hex(sep=Null, bytes_per_sep=1)`](hex/hex.md) | texto hexadecimal, com separador opcional |
| [`b.len()`](len/len.md) | **quantos BYTES** — não caracteres |

`b.len()` conta byte, e a diferença importa:

```
b = "ção".encode()
post(b.len())        # 5  — em UTF-8, "ç" e "ã" ocupam 2 bytes cada
post("ção".len())    # 3  — caracteres
```

É o mesmo número que o builtin `len(b)` devolve; existe como método porque
`str`, `list`, `dict` e `tup` também têm `.len()`, e `byte` era o único de
fora — quem escrevia `b.len()` por analogia tomava erro em tempo de execução.

Os operadores de sequência (`+`, `*`, `in`, índice e fatia) estão na
[página do módulo](../bytes/bytes.md).
