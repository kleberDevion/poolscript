# `sleep(segundos)`

Pausa a execução pelo tempo dado (aceita fração).

## Parâmetros

| nome | tipo | default | nota |
|---|---|---|---|
| `segundos` | int | flo | — |  |

## Retorno

Null

## Erros

- **TypeError** — o argumento não é `int` nem `flo`

## Bordas

- dentro de uma `async funct` (ou de um handler do `jinker`), **cede**: as
  outras tarefas e requisições andam enquanto esta dorme
- no programa principal, com tarefa `async` pendente, o sono **roda as
  tarefas** até o prazo — a de 0,1 s termina dentro de um `sleep(0.5)`; sem
  tarefa pendente, é o sono do processo de sempre
- `sleep(0)` no principal dá a vez às tarefas que já podem andar
- ver [6.8 Assíncrono](../../linguagem/06-funcoes.md)

[← índice](../builtins.md)
