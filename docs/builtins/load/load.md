# `load(path=Null)`

Carrega variáveis de um arquivo .env para o ambiente — atalho de dotenv.load.

## Parâmetros

| nome | tipo | default | nota |
|---|---|---|---|
| `path` | str | Null | Null procura o .env por perto |

## Retorno

`dict` com as variáveis lidas do `.env` (`{ "CHAVE": "valor", ... }`); dict
vazio se não achar nenhum `.env`.

## Bordas

- é a MESMA função de `dotenv.load` exposta sem import

[← índice](../builtins.md)
