# `input(prompt=Null)`

Lê uma linha do stdin; o prompt opcional é impresso antes, sem quebra.

## Parâmetros

| nome | tipo | default | nota |
|---|---|---|---|
| `prompt` | str | Null |  |

## Retorno

str

## Bordas

- SEMPRE devolve str — quem quer número converte com `int(x)`
- o `\r\n` do Windows não entra no texto lido

[← índice](../builtins.md)
