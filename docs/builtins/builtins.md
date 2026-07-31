# Builtins da PoolScript

Funções disponíveis em qualquer `.ps`, sem import.

**35 no total** — a contagem sai da fonte (`vm/poolscript_vm.c`), não da memória de ninguém. Cada exemplo das páginas roda nos DOIS motores pela suíte (`tests/test_docs_exemplos.py`): doc errada quebra o teste.

| nome | assinatura | o que faz |
|---|---|---|
| [`abs`](abs/abs.md) | `abs(n)` | Valor absoluto de um número. |
| [`addEnd`](addEnd/addEnd.md) | `addEnd(lista, item)` | Anexa o item no FIM da lista, mutando a própria lista. |
| [`addStart`](addStart/addStart.md) | `addStart(lista, item)` | Insere o item no INÍCIO da lista, mutando a própria lista. |
| [`bin`](bin/bin.md) | `bin(n)` | Inteiro em binário, com prefixo 0b. |
| [`bool`](bool/bool.md) | `bool(x)` | Verdade do valor: vazio/zero/Null são falsos, o resto é verdadeiro. |
| [`chr`](chr/chr.md) | `chr(n)` | Caractere do codepoint unicode. |
| [`enumerate`](enumerate/enumerate.md) | `enumerate(lista)` | Lista de tuplas (indice, item), começando em 0. |
| [`filter`](filter/filter.md) | `filter(lista, fn)` | Nova lista só com os itens em que fn devolve verdadeiro. A LISTA vem primeiro. |
| [`flo`](flo/flo.md) | `flo(x)` | Converte para número de ponto flutuante. |
| [`gather`](gather/gather.md) | `gather(a, b, ...)` | Devolve os argumentos como lista. |
| [`hex`](hex/hex.md) | `hex(n)` | Inteiro em hexadecimal, com prefixo 0x. |
| [`id`](id/id.md) | `id(x)` | Identidade do valor: endereço para objetos, o próprio conteúdo para imediatos. |
| [`input`](input/input.md) | `input(prompt=Null)` | Lê uma linha do stdin; o prompt opcional é impresso antes, sem quebra. |
| [`int`](int/int.md) | `int(x)` | Converte para inteiro: string numérica, float (trunca) ou bool. |
| [`len`](len/len.md) | `len(x)` | Tamanho de string (em caracteres), lista, tupla, dict ou bytes. |
| [`list`](list/list.md) | `list(x)` | Converte para lista: string vira caracteres, dict vira chaves, tupla vira lista. |
| [`load`](load/load.md) | `load(path=Null)` | Carrega variáveis de um arquivo .env para o ambiente — atalho de dotenv.load. |
| [`map`](map/map.md) | `map(lista, fn)` | Nova lista com fn aplicada a cada item. A LISTA vem primeiro. |
| [`max`](max/max.md) | `max(lista) | max(a, b, ...)` | Maior valor de uma lista ou dos argumentos. |
| [`min`](min/min.md) | `min(lista) | min(a, b, ...)` | Menor valor de uma lista ou dos argumentos. |
| [`oct`](oct/oct.md) | `oct(n)` | Inteiro em octal, com prefixo 0o. |
| [`open`](open/open.md) | `open(caminho, modo="r")` | Abre um arquivo e devolve o handle; combine com `using` para fechar sozinho. |
| [`ord`](ord/ord.md) | `ord(c)` | Codepoint unicode de um caractere. |
| [`post`](post/post.md) | `post(v1, v2, ...)` | Imprime os valores no stdout, separados por espaço, com quebra de linha no final. |
| [`range`](range/range.md) | `range(fim) | range(inicio, fim, passo=1)` | Lista de inteiros de inicio (inclusive) a fim (exclusive). |
| [`removeEnd`](removeEnd/removeEnd.md) | `removeEnd(lista)` | Remove e devolve o ÚLTIMO item da lista. |
| [`removeStart`](removeStart/removeStart.md) | `removeStart(lista)` | Remove e devolve o PRIMEIRO item da lista. |
| [`reversed`](reversed/reversed.md) | `reversed(lista)` | Nova lista com os itens na ordem inversa; a original não muda. |
| [`round`](round/round.md) | `round(n, casas=0)` | Arredonda um número, opcionalmente com casas decimais. |
| [`sleep`](sleep/sleep.md) | `sleep(segundos)` | Pausa a execução pelo tempo dado (aceita fração). |
| [`sorted`](sorted/sorted.md) | `sorted(lista)` | Nova lista com os itens em ordem crescente; a original não muda. |
| [`str`](str/str.md) | `str(x)` | Converte qualquer valor para texto, na renderização da PoolScript. |
| [`sum`](sum/sum.md) | `sum(lista)` | Soma os números de uma lista. |
| [`type`](type/type.md) | `type(x)` | Nome do tipo do valor, como string. |
| [`zip`](zip/zip.md) | `zip(a, b, ...)` | Lista de tuplas pareando os iteráveis; para no menor. |
