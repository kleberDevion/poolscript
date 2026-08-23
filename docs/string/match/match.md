# `s.match(padrao)`

True se o padrão casa a string INTEIRA (fullmatch).

## Parâmetros

| nome | tipo | default | nota |
|---|---|---|---|
| `padrao` | str regex | — |  |

## Retorno

bool

## Erros

- **SomeValueUnexpected** — padrão inválido

## Exemplos

![exemplo 1](../../assets/string__match__match_ex1.png)

<details><summary>código</summary>

```ps
post("a1".match("[a-z][0-9]"), "a1b".match("[a-z][0-9]"))
```

</details>

```saida
True False
```

## Bordas

- é fullmatch: o padrão tem que cobrir a string toda — pra achar num pedaço use findall

[← índice](../string.md)
