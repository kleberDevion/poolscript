# `input(prompt=Null)`

Lê uma linha do stdin; o prompt opcional é impresso antes, sem quebra.

## Parâmetros

| nome | tipo | default | nota |
|---|---|---|---|
| `prompt` | str | Null |  |

## Retorno

str, ou `Null` no fim da entrada

## Bordas

- devolve `str`: quem quer número converte, `int x = int(input("idade: "))`. Declarar o tipo não converte, e `int x = input()` é erro antes de rodar
- no fim da entrada (stdin fechado, ou vazio) devolve `Null`
- o `\r\n` do Windows não entra no texto lido

[← índice](../builtins.md)
