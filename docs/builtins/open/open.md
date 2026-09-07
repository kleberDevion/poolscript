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

FileHandle (read/write/close)

## Erros

- **FileNotFoundError** — arquivo inexistente no modo de leitura, e diretório
  inexistente no modo de escrita: `[Errno 2] No such file or directory: '…'`.
  Não é `IOError` — um `catch (IOError e)` não pega.

## Bordas

- `using open(p, "w") as f { f.write("x") }` fecha ao sair do bloco
- sem `using`, feche com `f.close()` — o GC fecha o que sobrar, mas tarde

[← índice](../builtins.md)
