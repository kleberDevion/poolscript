# Builtins nativos — funções sempre disponíveis

Estas funções fazem parte da linguagem — **não precisam de `import`**. Estão
sempre disponíveis em qualquer arquivo `.ps`.

```
post("olá")          // sem import nenhum
len([1, 2, 3])       // já funciona
```

---

## Saída e entrada

| Função | O que faz | Página |
|---|---|---|
| `post(...)` | imprime no terminal | [post/post.md](post/post.md) |
| `input(prompt)` | lê uma linha do usuário | [input/input.md](input/input.md) |

## Coleções — tamanho, iteração, transformação

| Função | O que faz | Página |
|---|---|---|
| `len(x)` | tamanho de lista/string/dict | [len/len.md](len/len.md) |
| `range(n)` | lista de números `0..n-1` | [range/range.md](range/range.md) |
| `map(lista, fn)` | aplica uma função em cada item | [map/map.md](map/map.md) |
| `filter(lista, fn)` | filtra os itens que passam | [filter/filter.md](filter/filter.md) |
| `enumerate(lista)` | pares `(índice, item)` | [enumerate/enumerate.md](enumerate/enumerate.md) |
| `zip(a, b, ...)` | junta listas item a item | [zip/zip.md](zip/zip.md) |
| `sorted(lista)` | lista ordenada | [sorted/sorted.md](sorted/sorted.md) |
| `reversed(lista)` | lista invertida | [reversed/reversed.md](reversed/reversed.md) |

## Editar listas (in-place)

| Função | O que faz | Página |
|---|---|---|
| `addEnd(lista, v)` | adiciona no fim | [addEnd/addEnd.md](addEnd/addEnd.md) |
| `removeEnd(lista)` | remove e devolve o último | [removeEnd/removeEnd.md](removeEnd/removeEnd.md) |
| `addStart(lista, v)` | adiciona no início | [addStart/addStart.md](addStart/addStart.md) |
| `removeStart(lista)` | remove e devolve o primeiro | [removeStart/removeStart.md](removeStart/removeStart.md) |

## Arquivos e ambiente

| Função | O que faz | Página |
|---|---|---|
| `open(caminho, modo)` | abre um arquivo (use com `using`) | [open/open.md](open/open.md) |
| `load()` | carrega o `.env` (atalho do `dotenv.load`) | ver [dotenv](../dotenv/dotenv.md) |

## Tipos e conversão

| Função | O que faz | Página |
|---|---|---|
| `type(x)` | o tipo do valor, como texto | [type/type.md](type/type.md) |
| `str/int/flo/bool/list(x)` | conversores de tipo | [conversores/conversores.md](conversores/conversores.md) |

## Async

| Função | O que faz | Página |
|---|---|---|
| `sleep(seg)` | pausa a execução | [sleep/sleep.md](sleep/sleep.md) |
| `gather(f1, f2, ...)` | aguarda vários `async` juntos | [gather/gather.md](gather/gather.md) |

## Números e utilitários

`abs`, `round`, `sum`, `min`, `max`, `id`, `hex`, `bin`, `oct`, `ord`, `chr` —
todos na página [numeros_e_utilitarios/numeros_e_utilitarios.md](numeros_e_utilitarios/numeros_e_utilitarios.md).

---

## Também sempre disponíveis (têm doc própria)

- **String methods** — `upper`, `strip`, `replace`, `split`/`join`, `match`… →
  [string](../string/string.md)
- **`Parsing`** — conversões tolerantes → [Parsing](../Parsing/Parsing.md)
- **`PoolFile`** — arquivo binário → [os/PoolFile](../os/PoolFile/PoolFile.md)
