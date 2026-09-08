# Referência da Linguagem — 12. Métodos de string, list, dict e bytes

Estes métodos são **parte da linguagem** (não vêm de `import`): qualquer `str`,
`list`, `dict` ou `bytes` os expõe direto, com a sintaxe `valor.metodo(...)`.
Esta seção é a referência agrupada; cada método tem ainda uma **página
detalhada** — [`docs/string/`](../string/string.md),
[`docs/list/`](../list/list.md), [`docs/dict/`](../dict/dict.md) e
[`docs/bytes/`](../bytes/bytes.md) — com parâmetros, retorno, erros, bordas e
exemplos que **rodam de verdade** pela suíte.

Tudo aqui foi verificado rodando o fonte na VM em C.

---

## 12.1. Métodos de `str` (55)

### Caixa

| Método | Faz |
|---|---|
| `upper()` / `lower()` | tudo maiúsculo / minúsculo |
| `title()` | Primeira De Cada Palavra Maiúscula |
| `capitalize()` | só a primeira letra da string |
| `swapcase()` | inverte a caixa de cada letra |
| `casefold()` | minúsculo agressivo (comparação sem caixa) |

A tabela de caixa cobre **ASCII, Latin-1/Ext-A, grego e cirílico**, com os dois
casos que não são 1-pra-1:

```ps
post("ΣΟΦΟΣ".lower())   # σοφος  — Σ no fim de palavra vira ς, não σ
post("ß".upper())       # SS     — ß não tem maiúscula de um caractere só
post("ß".title())       # Ss     — só a inicial sobe
post("Привет".upper())  # ПРИВЕТ
post("İ".lower())       # i      — o I turco com pingo
```

### Bordas e preenchimento

| Método | Faz |
|---|---|
| `strip(chars=null)` / `lstrip` / `rstrip` | remove espaços (ou os `chars` dados) das duas pontas / esquerda / direita |
| `ljust(width, fillchar)` / `rjust(width, fillchar)` / `center(width, fillchar)` | preenche até a largura à esquerda / direita / centro; sem `fillchar`, com espaço |
| `zfill(n)` | preenche com zeros à esquerda até `n` (respeita o sinal) |
| `expandtabs(tabsize=8)` | troca TABs por espaços |

### Busca

| Método | Faz |
|---|---|
| `find(sub, inicio=0, fim=null)` / `rfind(...)` | índice da 1ª / última ocorrência, ou **-1** se não achar; `inicio`/`fim` limitam a faixa de busca (em caracteres, negativo conta do fim) |
| `index(sub, inicio=0, fim=null)` / `rindex(...)` | como find/rfind, mas **erro** se não achar |
| `count(sub, inicio=0, fim=null)` | quantas vezes `sub` aparece (na faixa) |
| `contains(sub)` / `has(sub)` | `sub` está na string? (`bool`) |
| `startswith(pre)` / `endswith(suf)` | começa / termina com? (`bool`); `pre`/`suf` pode ser **uma** `str` ou uma **tupla/lista de opções** (basta uma bater) |

```ps
post("relatorio.pdf".endswith((".pdf", ".doc")))   # True
post("olá".startswith(("x", "o")))                 # True
```

> Não existem os argumentos `start`/`end`. Para testar só um trecho, **fatie
> antes**: `s[0:7].endswith("vo")`. (O fatiamento `s[a:b]` — inclusive negativo
> e passo — está na seção de tipos/coleções.)

### Testes de conteúdo (`is…`) — todos devolvem `bool`

`isalpha`, `isdigit`, `isnumeric`, `isdecimal`, `isalnum`, `isspace`,
`isupper`, `islower`, `isascii`, `istitle`, `isprintable`, `isidentifier`.

Os três de número **não** são sinônimos, e nenhum deles para no ASCII:

| Método | Aceita |
|---|---|
| `isdecimal()` | só dígito decimal, de qualquer escrita: `0-9`, `٣` (árabe), `३` (devanágari), `๓` (tailandês), `３` (largura plena) |
| `isdigit()` | os decimais **mais** os sobrescritos/subscritos: `²`, `³`, `¹`, `⁷`, `₄` |
| `isnumeric()` | os anteriores **mais** fração e numeral romano: `½`, `¾`, `Ⅷ` |

```ps
post("²".isdigit(), "²".isdecimal())     # True False
post("½".isnumeric(), "½".isdigit())     # True False
post("٣".isdecimal())                    # True
```

**String vazia:** `isascii()` e `isprintable()` dão **True** (não há caractere
que viole a regra); todos os outros dão **False**, porque pedem pelo menos um
caractere da classe.

### Divisão e junção

| Método | Faz |
|---|---|
| `split(sep=Null, maxsplit=-1)` | divide numa lista; sem `sep`, quebra por espaços (colapsando); `maxsplit` limita o nº de cortes (o resto fica junto) |
| `rsplit(sep=Null, maxsplit=-1)` | igual, mas conta os cortes **da direita** |
| `splitlines()` | divide por quebras de linha |
| `join(lista)` | une os itens da lista usando a string como cola: `", ".join(["a","b"])` → `"a, b"` |
| `partition(sep)` / `rpartition(sep)` | divide em **3**: (antes, sep, depois), na 1ª / última ocorrência |

### Modificação

| Método | Faz |
|---|---|
| `replace(alvo, novo)` | troca todas as ocorrências. **`alvo` pode ser uma lista** de textos, todos trocados pelo mesmo `novo`: `"a-b_c".replace(["-","_"], " ")` → `"a b c"` |
| `removeprefix(pre)` / `removesuffix(suf)` | remove o prefixo / sufixo, se houver |
| `maketrans(de, para)` / `translate(tab)` | tabela de tradução caractere-a-caractere e sua aplicação |

### Formatação e regex

| Método | Faz |
|---|---|
| `format(a, b)` | preenche `{}` e `{0}` no template — **posicional só**; não há como preencher `{nome}` por aqui |
| `format_map(dict)` | preenche `{nome}`, pelas chaves do dict |
| `match(padrao)` | a string **inteira** casa com a regex? (`bool`) — o mesmo que [`regex.match`](../regex/match/match.md); casamento parcial é `regex.search` |
| `findall(padrao)` | lista de todas as ocorrências da regex |
| `sub(pattern, repl)` | substitui **todas** as ocorrências da regex — não há `count` |

### Outros

| Método | Faz |
|---|---|
| `len()` | nº de caracteres (igual a `len(s)`) |
| `encode(encoding="utf-8", errors="strict")` | string → `bytes` no encoding pedido |
| `get_json(chave=null)` | interpreta a string como JSON e devolve os dados (ou a chave) |
| `get(...)` | acessa dado dentro de uma string JSON |

#### `encode` / `decode`: o encoding vale de verdade

`s.encode(encoding, errors)` produz `bytes`; `b.decode(encoding, errors)` volta
pra `str`. Encodings aceitos (o nome ignora caixa, `-`, `_` e espaço):

`utf-8` (padrão) · `latin-1`/`iso-8859-1` · `ascii` · `utf-16-le`/`utf-16-be` ·
`utf-32-le`/`utf-32-be`

`errors` diz o que fazer com o que não cabe: `strict` (padrão, **levanta erro**),
`ignore` (some) ou `replace` (`?` no encode, `\ufffd` no decode).

```ps
post("café".encode("latin-1"))            # b'caf\xe9'
post("café".encode())                     # b'caf\xc3\xa9'
post("café".encode("ascii", "replace"))   # b'caf?'
post("café".encode("latin-1").decode("latin-1"))   # café
```

Erro é erro, não silêncio: encoding desconhecido, caractere que não cabe no
encoding e byte inválido no decode **levantam**.

```ps
import bytes
try {
    post(bytes.new([255, 254]).decode())
} catch(e) {
    post(e)     # 'utf-8' codec can't decode byte 0xff in position 0: invalid start byte
}
```

---

## 12.2. Métodos de `list` (14)

A maioria **altera a própria lista** (in-place) e devolve `null` — não encadeia.

| Método | Faz | Muta? |
|---|---|---|
| `append(item)` | anexa no fim | sim |
| `extend(outra)` | anexa todos os itens de outra lista | sim |
| `insert(i, item)` | insere `item` na posição `i` | sim |
| `pop(i=último)` | remove e **devolve** o item de `i` (ou o último) | sim |
| `remove(item)` | remove a 1ª ocorrência de `item` | sim |
| `reverse()` | inverte a lista no lugar | sim |
| `sort()` | ordena no lugar (crescente) | sim |
| `clear()` | esvazia | sim |
| `index(item, inicio=0, fim=len)` | posição da 1ª ocorrência **na faixa** (erro se não achar); posição negativa conta do fim | não |
| `count(item)` | quantas vezes aparece | não |
| `contains(item)` / `has(item)` | está na lista? (`bool`) | não |
| `copy()` | cópia rasa (nova lista) | não |
| `len()` | tamanho (igual a `len(l)`) | não |

```ps
l = [3, 1, 2]
l.append(4)          # l == [3, 1, 2, 4]
l.sort()             # l == [1, 2, 3, 4]
post(l.pop())        # 4   (e l == [1, 2, 3])
post(l.index(2))     # 1

l2 = [1, 2, 3, 2]
post(l2.index(2))       # 1   — a primeira
post(l2.index(2, 2))    # 3   — a primeira a partir da posição 2
```

> Cópia é **rasa**: `l.copy()` cria uma lista nova, mas os itens são
> compartilhados. Para não mutar, use `sorted(l)`/`reversed(l)` (devolvem cópia)
> em vez de `l.sort()`/`l.reverse()`.

---

## 12.3. Métodos de `dict` (12)

| Método | Faz |
|---|---|
| `keys()` | lista das chaves |
| `values()` / `value()` | lista dos valores (os dois nomes, o mesmo resultado) |
| `items()` | lista de tuplas `(chave, valor)` |
| `get(chave, default=null)` | valor da chave, ou `default` (ou `null`) se não existir — **não dá erro** |
| `has(chave)` / `contains(chave)` | a chave existe? (`bool`) |
| `pop(chave)` | remove e devolve o valor da chave |
| `update(outro)` | mescla os pares de outro dict |
| `clear()` | esvazia |
| `copy()` | cópia rasa |
| `len()` | nº de pares (igual a `len(d)`) |

```ps
d = { "nome": "ana", "idade": 30 }
post(d.get("nome"))          # ana
post(d.get("cidade", "?"))   # ?    (default; não dá erro)
post(d.has("idade"))         # True
for each k in d.keys() {
    post(k, d[k])
}
```

### `in` olha a CHAVE; pro VALOR, use `value()`

`x in d` testa se `x` é uma **chave** — então procurar um
valor ali dá `false` sempre, seja ele str, int, flo, list ou tup:

```ps
d = { "nome": "ana", "idade": 30, "tags": [1, 2] }

post("nome" in d)            # True   (é chave)
post("ana" in d)             # False  (é VALOR — `in` não olha valor)

post("ana" in d.value())     # True
post(30 in d.value())        # True
post([1, 2] in d.value())    # True
```

`value()` e `values()` devolvem a mesma lista; o nome curto existe justamente
pra essa leitura (`x in d.value()`).

### Acesso por chave: `[]` e `.chave`

Além dos métodos, uma chave é lida/escrita por colchete **ou por atributo**
(equivalentes):

```ps
d = { "nome": "ana" }
post(d["nome"], d.nome)      # ana ana
d.idade = 30                 # == d["idade"] = 30
```

Uma chave inexistente por `[]`/`.chave` dá `KeyError`; para um acesso que não
falha, use `d.get(chave)`. (Um método com o mesmo nome de uma chave — `d.keys`
etc. — tem prioridade sobre o acesso por atributo.)

---

## 12.4. Métodos de `bytes` (44)

`bytes` é o tipo de **dado binário** — o que sai de `"texto".encode()`, de
`open(..., "rb").read()`, do corpo de uma resposta HTTP. As páginas por método
estão em [`docs/bytes/`](../bytes/bytes.md).

Os nomes são os mesmos do `str`, e é justamente por isso que vale ler as
**quatro diferenças** — são as que se erra por analogia:

| Onde | `str` | `bytes` |
|---|---|---|
| caixa (`upper`, `lower`, `title`…) | Unicode inteiro | **só ASCII**: `bytes.new([0xc0, 65]).lower()` é `b'\xc0a'` |
| `splitlines` | `\n`, `\r` e `\r\n` | os mesmos três |
| `find`/`count`/`index` | só substring | aceitam também um **inteiro** de 0 a 255 |
| `strip(x)` | conjunto de caracteres | conjunto de **bytes** |

| Grupo | Métodos |
|---|---|
| busca | `find` `rfind` `index` `rindex` `count` `contains` `has` `startswith` `endswith` |
| caixa | `upper` `lower` `title` `capitalize` `swapcase` |
| testes | `isalpha` `isdigit` `isalnum` `isspace` `isupper` `islower` `istitle` `isascii` |
| aparar | `strip` `lstrip` `rstrip` `removeprefix` `removesuffix` |
| partir/juntar | `split` `rsplit` `splitlines` `partition` `rpartition` `join` |
| trocar | `replace` `translate` `maketrans` |
| preencher | `ljust` `rjust` `center` `zfill` `expandtabs` |
| converter | `decode` `hex` `len` |

E os operadores de sequência:

```ps
b = "Hello".encode()

post(len(b))                 # 5
post(b[0])                   # 72     — indexar dá o INTEIRO do byte
post(b[0:2])                 # b'He'  — fatiar dá bytes
post(b + "!".encode())       # b'Hello!'
post(b * 2)                  # b'HelloHello'
post("ell".encode() in b)    # True   — subsequência
post(101 in b)               # True   — esse BYTE aparece?

for each x in "abc".encode() {
    post(x)                  # 97, 98, 99 — itera em INTEIROS
}
```

**Indexar dá inteiro, fatiar dá bytes, iterar dá inteiro.** É o que faz
`if x == 0` procurar byte NUL direto, sem passar por hexadecimal.

---

## 12.5. Resumo

- Os métodos são chamados por `valor.metodo(...)` e são parte da linguagem
  (sem `import`).
- **`str`**: 55 métodos (caixa, bordas/preenchimento, busca, testes `is…`,
  divisão/junção, modificação com `replace` aceitando **lista** de alvos,
  formatação e regex). Detalhe por método em `docs/string/`.
- **`list`**: 14 — a maioria muta a lista (`append`/`sort`/`pop`/…); `sorted`/
  `reversed` (builtins) devolvem cópia.
- **`dict`**: 12 — `keys`/`values` (ou `value`)/`items`, `get` (com default, sem erro),
  `has`, `pop`, `update`, `copy`… mais acesso por `[]` e por `.chave`.
- **`bytes`**: 44 — os mesmos nomes do `str`, com semântica de BYTE: caixa só
  em ASCII, `splitlines` só em `\n`/`\r`/`\r\n`, busca aceitando inteiro.
  Detalhe por método em `docs/bytes/`.
