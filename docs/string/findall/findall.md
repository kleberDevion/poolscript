# `s.findall(padrao)`

Lista com todas as ocorrências do padrão.

## Parâmetros

| nome | tipo | default | nota |
|---|---|---|---|
| `padrao` | str regex | — |  |

## Retorno

list

## Erros

- **SomeValueUnexpected** — padrão inválido

## Exemplos

![exemplo 1](../../assets/string__findall__findall_ex1.png)

<details><summary>código</summary>

```ps
post("a1b2".findall("[0-9]"))
```

</details>

```saida
['1', '2']
```

[← índice](../string.md)
