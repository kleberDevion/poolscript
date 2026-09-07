# `s.match(padrao)`

True se o padrão casa a string INTEIRA (fullmatch).

## Parâmetros

| nome | tipo | default | nota |
|---|---|---|---|
| `padrao` | str regex | — |  |

## Retorno

bool

## Erros

- **ValueError** — o padrão não é uma expressão regular válida
  (`unterminated character set at position 0`). Não é `TypeError`.

## Exemplos

```ps
post("a1".match("[a-z][0-9]"), "a1b".match("[a-z][0-9]"))
```

```saida
True False
```

## Bordas

- é fullmatch: o padrão tem que cobrir a string toda — pra achar num pedaço use findall

[← índice](../string.md)
