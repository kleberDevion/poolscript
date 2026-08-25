# Métodos de string

Chamados direto no valor: `"texto".metodo()`. Número também recebe método de string por conversão automática (`(150).isdigit()`), exceto `len`.

**55 no total** — a contagem sai da fonte (`vm/poolscript_vm.c`), não da memória de ninguém. Cada exemplo das páginas roda de verdade na suíte em C (`make check`): doc errada quebra o teste.

| nome | assinatura | o que faz |
|---|---|---|
| [`capitalize`](capitalize/capitalize.md) | `s.capitalize()` | Só a primeira letra da string em maiúscula; o resto minúsculo. |
| [`casefold`](casefold/casefold.md) | `s.casefold()` | Minúsculas agressivas para comparação sem caixa. |
| [`center`](center/center.md) | `s.center(largura, preenchimento=" ")` | Centraliza preenchendo dos dois lados. |
| [`contains`](contains/contains.md) | `s.contains(sub)` | True se contém a substring (extensão da PoolScript). |
| [`count`](count/count.md) | `s.count(sub, inicio=0, fim=null)` | Quantas ocorrências (sem sobreposição) da substring. |
| [`encode`](encode/encode.md) | `s.encode(encoding="utf-8")` | Converte a string para bytes. |
| [`endswith`](endswith/endswith.md) | `s.endswith(sufixo)` | True se termina com o sufixo. |
| [`expandtabs`](expandtabs/expandtabs.md) | `s.expandtabs(tabsize=8)` | Troca cada tab por espaços até a próxima parada de tabulação (não por um número fixo de espaços). |
| [`find`](find/find.md) | `s.find(sub, inicio=0, fim=null)` | Índice da primeira ocorrência, ou -1. |
| [`findall`](findall/findall.md) | `s.findall(padrao)` | Lista com todas as ocorrências do padrão. |
| [`format`](format/format.md) | `s.format(a, b, ...)` | Preenche {} posicionais e {nome} nomeados. |
| [`format_map`](format_map/format_map.md) | `s.format_map(dict)` | Como format, buscando os {nome} num dict. |
| [`get`](get/get.md) | `s.get(chave)` | Interpreta como JSON de objeto e devolve o valor da chave. |
| [`get_json`](get_json/get_json.md) | `s.get_json()` | Interpreta a string como JSON e devolve o valor. |
| [`has`](has/has.md) | `s.has(sub)` | Alias de contains — True se a substring aparece. |
| [`index`](index/index.md) | `s.index(sub, inicio=0, fim=null)` | Como find, mas ERRA quando não acha. |
| [`isalnum`](isalnum/isalnum.md) | `s.isalnum()` | True se só tem letras e dígitos. |
| [`isalpha`](isalpha/isalpha.md) | `s.isalpha()` | True se só tem letras (e não é vazia). |
| [`isascii`](isascii/isascii.md) | `s.isascii()` | True se todos os caracteres são ASCII. |
| [`isdecimal`](isdecimal/isdecimal.md) | `s.isdecimal()` | True se só tem dígitos decimais. |
| [`isdigit`](isdigit/isdigit.md) | `s.isdigit()` | True se só tem dígitos. |
| [`isidentifier`](isidentifier/isidentifier.md) | `s.isidentifier()` | True se a string é um identificador válido. |
| [`islower`](islower/islower.md) | `s.islower()` | True se as letras são todas minúsculas. |
| [`isnumeric`](isnumeric/isnumeric.md) | `s.isnumeric()` | True se só tem caracteres numéricos. |
| [`isprintable`](isprintable/isprintable.md) | `s.isprintable()` | True se todos os caracteres são imprimíveis. |
| [`isspace`](isspace/isspace.md) | `s.isspace()` | True se só tem espaços em branco. |
| [`istitle`](istitle/istitle.md) | `s.istitle()` | True se cada palavra começa com maiúscula. |
| [`isupper`](isupper/isupper.md) | `s.isupper()` | True se as letras são todas maiúsculas. |
| [`join`](join/join.md) | `sep.join(lista)` | Junta os itens da lista com o separador. |
| [`len`](len/len.md) | `s.len()` | Tamanho em caracteres — forma de método do builtin len. |
| [`ljust`](ljust/ljust.md) | `s.ljust(largura, preenchimento=" ")` | Preenche à direita até a largura. |
| [`lower`](lower/lower.md) | `s.lower()` | Tudo em minúsculas. |
| [`lstrip`](lstrip/lstrip.md) | `s.lstrip(chars=Null)` | Remove da ponta ESQUERDA. |
| [`maketrans`](maketrans/maketrans.md) | `s.maketrans(de, para)` | Tabela de tradução caractere-a-caractere para translate. |
| [`match`](match/match.md) | `s.match(padrao)` | True se o padrão casa a string INTEIRA (fullmatch). |
| [`partition`](partition/partition.md) | `s.partition(sep)` | Tupla (antes, sep, depois) na PRIMEIRA ocorrência. |
| [`removeprefix`](removeprefix/removeprefix.md) | `s.removeprefix(p)` | Remove o prefixo exato, se presente. |
| [`removesuffix`](removesuffix/removesuffix.md) | `s.removesuffix(p)` | Remove o sufixo exato, se presente. |
| [`replace`](replace/replace.md) | `s.replace(velho, novo, count=-1)` | Troca ocorrências; velho pode ser LISTA de alvos. |
| [`rfind`](rfind/rfind.md) | `s.rfind(sub, inicio=0, fim=null)` | Índice da ÚLTIMA ocorrência, ou -1. |
| [`rindex`](rindex/rindex.md) | `s.rindex(sub, inicio=0, fim=null)` | Como rfind, mas ERRA quando não acha. |
| [`rjust`](rjust/rjust.md) | `s.rjust(largura, preenchimento=" ")` | Preenche à esquerda até a largura. |
| [`rpartition`](rpartition/rpartition.md) | `s.rpartition(sep)` | Tupla (antes, sep, depois) na ÚLTIMA ocorrência. |
| [`rsplit`](rsplit/rsplit.md) | `s.rsplit(sep=Null, max=-1)` | Como split, mas conta as divisões da DIREITA. |
| [`rstrip`](rstrip/rstrip.md) | `s.rstrip(chars=Null)` | Remove da ponta DIREITA. |
| [`split`](split/split.md) | `s.split(sep=Null, max=-1)` | Divide em lista; sem separador, divide por espaços. |
| [`splitlines`](splitlines/splitlines.md) | `s.splitlines()` | Divide por quebras de linha, sem incluí-las. |
| [`startswith`](startswith/startswith.md) | `s.startswith(prefixo)` | True se começa com o prefixo. |
| [`strip`](strip/strip.md) | `s.strip(chars=Null)` | Remove espaços (ou os chars dados) das duas pontas. |
| [`sub`](sub/sub.md) | `s.sub(padrao, novo)` | Substitui as ocorrências do padrão regex. |
| [`swapcase`](swapcase/swapcase.md) | `s.swapcase()` | Inverte a caixa de cada letra. |
| [`title`](title/title.md) | `s.title()` | Primeira letra de cada palavra em maiúscula. |
| [`translate`](translate/translate.md) | `s.translate(tabela)` | Troca caracteres segundo a tabela do maketrans (de→para, caractere a caractere). |
| [`upper`](upper/upper.md) | `s.upper()` | Tudo em maiúsculas (unicode: ç→Ç, á→Á). |
| [`zfill`](zfill/zfill.md) | `s.zfill(largura)` | Preenche com zeros à esquerda, respeitando sinal. |
