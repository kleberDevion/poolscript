# `assert(cond, esperado=Null, mensagem=Null)`

Falha com `AssertionError` quando a condição não passa. É o que permite testar
código escrito em PoolScript sem montar a checagem à mão.

## Parâmetros

| nome | tipo | default | nota |
|---|---|---|---|
| `cond` | qualquer | — | o valor testado; com 2 valores, o RECEBIDO |
| `esperado` | qualquer | Null | quando não é `str`, vira comparação |
| `mensagem` | str | Null | texto que vai na frente do erro |

## Retorno

`true` quando passa — então serve dentro de expressão.

## As três formas

```ps
assert(x > 0)                         # falhou -> AssertionError: 0 nao e verdadeiro
assert(x > 0, "saldo tem que ser positivo")
assert(soma(2, 2), 4, "soma de dois")  # falhou -> AssertionError: soma de dois — veio 5, esperava 4
```

A forma de **dois valores** existe porque `assert(a == b)` é a que mais se
escreve e a que menos ajuda quando quebra: o `==` já virou `false` e a
mensagem não tem como dizer o que veio. Recebendo os dois lados, ela diz.

## Bordas

- `esperado` do tipo `str` é lido como MENSAGEM, não como valor esperado:
  `assert(x, "explicando")` testa a verdade de `x`. Pra comparar contra texto,
  use a forma de três: `assert(x, "abc", "nota")`.
- Por nome, `assert(x, mensagem="…")` é a forma de dois (testa `x` e usa o
  texto na falha); `esperado=` sozinho compara. Os dois juntos são a forma de
  três.
- A comparação é a mesma do operador `==` — não há uma segunda noção de
  igualdade.
- `AssertionError` é capturável: um runner pode contar as falhas em vez de
  parar na primeira.

```ps
falhas = 0
for each caso in casos {
    try { assert(caso["veio"], caso["esperado"], caso["nome"]) }
    catch (e) { falhas = falhas + 1  post("  " + str(e)) }
}
post(falhas, "falharam")
```

[← índice](../builtins.md)
