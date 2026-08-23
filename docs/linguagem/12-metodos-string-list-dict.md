# Referência da Linguagem — 12. Métodos de string, list e dict

Estes métodos são **parte da linguagem** (não vêm de `import`): qualquer `str`,
`list` ou `dict` os expõe direto, com a sintaxe `valor.metodo(...)`. Esta seção
é a referência agrupada; os métodos de **string** têm ainda uma página
detalhada cada em [`docs/string/`](../string/string.md), com exemplos que
**rodam nos dois motores** pela suíte.

Tudo aqui foi verificado rodando o mesmo fonte no interpretador e na VM em C.

---

## 12.1. Métodos de `str` (54)

### Caixa

| Método | Faz |
|---|---|
| `upper()` / `lower()` | tudo maiúsculo / minúsculo |
| `title()` | Primeira De Cada Palavra Maiúscula |
| `capitalize()` | só a primeira letra da string |
| `swapcase()` | inverte a caixa de cada letra |
| `casefold()` | minúsculo agressivo (comparação sem caixa) |

### Bordas e preenchimento

| Método | Faz |
|---|---|
| `strip(chars=null)` / `lstrip` / `rstrip` | remove espaços (ou os `chars` dados) das duas pontas / esquerda / direita |
| `ljust(n, ch=" ")` / `rjust(n, ch=" ")` / `center(n, ch=" ")` | preenche até largura `n` à esquerda / direita / centro |
| `zfill(n)` | preenche com zeros à esquerda até `n` (respeita o sinal) |
| `expandtabs(n=8)` | troca TABs por espaços |

### Busca

| Método | Faz |
|---|---|
| `find(sub)` / `rfind(sub)` | índice da 1ª / última ocorrência, ou **-1** se não achar |
| `index(sub)` / `rindex(sub)` | como find/rfind, mas **erro** se não achar |
| `count(sub)` | quantas vezes `sub` aparece |
| `contains(sub)` / `has(sub)` | `sub` está na string? (`bool`) |
| `startswith(pre)` / `endswith(suf)` | começa / termina com? (`bool`) |

> `startswith`/`endswith` recebem **um** prefixo/sufixo `str` — hoje **não**
> aceitam os argumentos `start`/`end` nem lista de opções (mesma limitação nos
> dois motores). Para testar só um trecho, **fatie antes**: `s[0:7].endswith("vo")`.
> (O fatiamento `s[a:b]` — inclusive negativo e passo — existe e é descrito na
> seção de tipos/coleções.)

### Testes de conteúdo (`is…`) — todos devolvem `bool`

`isalpha`, `isdigit`, `isnumeric`, `isdecimal`, `isalnum`, `isspace`,
`isupper`, `islower`, `isascii`, `istitle`, `isprintable`, `isidentifier`.

### Divisão e junção

| Método | Faz |
|---|---|
| `split(sep=null, max=-1)` | divide numa lista; sem `sep`, quebra por espaços (colapsando); `max` limita o nº de cortes (o resto fica junto) |
| `rsplit(sep=null, max=-1)` | igual, mas conta os cortes **da direita** |
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
| `format(...)` / `format_map(dict)` | preenche `{}`/`{0}`/`{nome}` no template |
| `match(padrao)` | casa a regex no início; devolve o match ou vazio |
| `findall(padrao)` | lista de todas as ocorrências da regex |
| `sub(padrao, novo, count=…)` | substitui as ocorrências da regex |

### Outros

| Método | Faz |
|---|---|
| `len()` | nº de caracteres (igual a `len(s)`) |
| `encode(enc="utf-8")` | string → `bytes` |
| `get_json(chave=null)` | interpreta a string como JSON e devolve os dados (ou a chave) |
| `get(...)` | acessa dado dentro de uma string JSON |

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
| `index(item)` | posição da 1ª ocorrência (erro se não achar) | não |
| `count(item)` | quantas vezes aparece | não |
| `contains(item)` / `has(item)` | está na lista? (`bool`) | não |
| `copy()` | cópia rasa (nova lista) | não |
| `len()` | tamanho (igual a `len(l)`) | não |

![exemplo 1](../assets/linguagem__12-metodos-string-list-dict_ex1.png)

<details><summary>código</summary>

```ps
l = [3, 1, 2]
l.append(4)          // l == [3, 1, 2, 4]
l.sort()             // l == [1, 2, 3, 4]
post(l.pop())        // 4   (e l == [1, 2, 3])
post(l.index(2))     // 1
```

</details>

> Cópia é **rasa**: `l.copy()` cria uma lista nova, mas os itens são
> compartilhados. Para não mutar, use `sorted(l)`/`reversed(l)` (devolvem cópia)
> em vez de `l.sort()`/`l.reverse()`.

---

## 12.3. Métodos de `dict` (11)

| Método | Faz |
|---|---|
| `keys()` | lista das chaves |
| `values()` | lista dos valores |
| `items()` | lista de tuplas `(chave, valor)` |
| `get(chave, default=null)` | valor da chave, ou `default` (ou `null`) se não existir — **não dá erro** |
| `has(chave)` / `contains(chave)` | a chave existe? (`bool`) |
| `pop(chave)` | remove e devolve o valor da chave |
| `update(outro)` | mescla os pares de outro dict |
| `clear()` | esvazia |
| `copy()` | cópia rasa |
| `len()` | nº de pares (igual a `len(d)`) |

![exemplo 2](../assets/linguagem__12-metodos-string-list-dict_ex2.png)

<details><summary>código</summary>

```ps
d = { "nome": "ana", "idade": 30 }
post(d.get("nome"))          // ana
post(d.get("cidade", "?"))   // ?    (default; não dá erro)
post(d.has("idade"))         // True
for each k in d.keys():
    post(k, d[k])
```

</details>

### Acesso por chave: `[]` e `.chave`

Além dos métodos, uma chave é lida/escrita por colchete **ou por atributo**
(equivalentes):

![exemplo 3](../assets/linguagem__12-metodos-string-list-dict_ex3.png)

<details><summary>código</summary>

```ps
d = { "nome": "ana" }
post(d["nome"], d.nome)      // ana ana
d.idade = 30                 // == d["idade"] = 30
```

</details>

Uma chave inexistente por `[]`/`.chave` dá `KeyError`; para um acesso que não
falha, use `d.get(chave)`. (Um método com o mesmo nome de uma chave — `d.keys`
etc. — tem prioridade sobre o acesso por atributo.)

---

## 12.4. Resumo

- Os métodos são chamados por `valor.metodo(...)` e são parte da linguagem
  (sem `import`).
- **`str`**: 54 métodos (caixa, bordas/preenchimento, busca, testes `is…`,
  divisão/junção, modificação com `replace` aceitando **lista** de alvos,
  formatação e regex). Detalhe por método em `docs/string/`.
- **`list`**: 14 — a maioria muta a lista (`append`/`sort`/`pop`/…); `sorted`/
  `reversed` (builtins) devolvem cópia.
- **`dict`**: 11 — `keys`/`values`/`items`, `get` (com default, sem erro),
  `has`, `pop`, `update`, `copy`… mais acesso por `[]` e por `.chave`.
