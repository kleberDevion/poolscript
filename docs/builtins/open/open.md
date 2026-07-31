# `open(caminho, modo="r")`

Abre um arquivo e devolve o handle; combine com `using` para fechar sozinho.

## Parâmetros

| nome | tipo | default | nota |
|---|---|---|---|
| `caminho` | str | — |  |
| `modo` | str | "r" | "r", "w", "a", "rb", "wb"... |

## Retorno

FileHandle (read/write/close)

## Erros

- **IOError** — arquivo inexistente no modo de leitura

## Bordas

- `using open(p, "w") as f { f.write("x") }` fecha ao sair do bloco
- sem `using`, feche com `f.close()` — o GC fecha o que sobrar, mas tarde

[← índice](../builtins.md)
