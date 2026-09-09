# `open(path, mode="r", encoding="utf-8")`

Abre um arquivo e devolve o handle; combine com `using` para fechar sozinho.

## Parâmetros

| nome | tipo | default | nota |
|---|---|---|---|
| `path` | str | — |  |
| `mode` | str | "r" | "r", "w", "a", "rb", "wb"... |
| `encoding` | str | "utf-8" |  |

Os nomes são estes: `open(caminho=...)` e `open(p, modo="r")` são
`TypeError: 'caminho' is an invalid keyword argument for open()`.

## Retorno

**`PoolFile`** — o arquivo aberto. `type(f)` devolve `"PoolFile"`.

Todo arquivo na linguagem é `PoolFile`: o que sai do `open()` e o que sai do
`os.loadFile()` são o **mesmo tipo**, com os mesmos membros.

| membro | devolve | o que é |
|---|---|---|
| `.read(n=-1)` | `str` (ou `bytes` em modo `"b"`) | o conteúdo; `n` limita |
| `.readline()` | `str` \| `Null` | a próxima linha; `Null` no fim |
| `.readlines()` | `list` de `str` | todas as linhas |
| `.write(texto)` | `int` | **quantos bytes escreveu** — não é o arquivo |
| `.writelines(lista)` | `Null` | escreve cada item |
| `.close()` | `Null` | fecha; o `using` faz sozinho |
| `.save(caminho=Null)` | `Null` | grava uma cópia no caminho/pasta |
| `.path()` | `str` | o caminho absoluto |
| `.bytes()` | `bytes` | o conteúdo em bytes |
| `.copy(destino)` | `PoolFile` | copia; o original fica |
| `.move(destino)` | `PoolFile` | move e passa a apontar pro novo lugar |
| `.delete()` | `bool` | apaga do disco |
| `.name` | `str` | o nome do arquivo (sem pasta) — **campo, sem `()`** |
| `.ext` | `str` | a extensão com ponto (`".png"`) — campo |
| `.size` | `int` | o tamanho em bytes — campo |

> **`.write()` devolve `int`, não o arquivo.** Encadear (`f.write(x).save(p)`)
> dá `'int' object has no attribute 'save'`. Chame no arquivo: `f.write(x)` e
> depois `f.save(p)`.

`move` e `delete` num arquivo ainda **aberto** são recusados com
`ValueError: … chame .close() antes` — o descritor aberto continuaria
escrevendo no lugar antigo.

## Erros

- **FileNotFoundError** — arquivo inexistente no modo de leitura, e diretório
  inexistente no modo de escrita: `[Errno 2] No such file or directory: '…'`.
  Não é `IOError` — um `catch (IOError e)` não pega.

## Bordas

- `using open(p, "w") as f { f.write("x") }` fecha ao sair do bloco
- sem `using`, feche com `f.close()` — o GC fecha o que sobrar, mas tarde

[← índice](../builtins.md)
