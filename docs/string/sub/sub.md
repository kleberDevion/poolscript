# `s.sub(pattern, repl)`

Substitui **todas** as ocorrências do padrão regex. Não há `count` aqui — a
lib `regex` tem.

## Parâmetros

| nome | tipo | default | nota |
|---|---|---|---|
| `pattern` | str regex | — |  |
| `repl` | str | — |  |

Os nomes são estes: `"a1".sub(padrao="[0-9]", novo="#")` é
`TypeError: 'padrao' is an invalid keyword argument for sub()`.

## Retorno

str

## Erros

- **ValueError** — o padrão não é uma expressão regular válida
  (`unterminated character set at position 0`). Não é `TypeError`: um
  `catch (TypeError e)` não pega.

## Exemplos

```ps
post("a1b2".sub("[0-9]", "#"))
```

```saida
a#b#
```

[← índice](../string.md)
