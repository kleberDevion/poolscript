# `pow(base, expo, mod=Null)`

Potência: `base` elevado a `expo`. Com `mod`, devolve `(base ** expo) % mod`
calculado sem montar o número inteiro da potência, o que serve pra potência
modular de número grande.

## Parâmetros

| nome | tipo | default | nota |
|---|---|---|---|
| `base` | int \| flo | — |  |
| `expo` | int \| flo | — | negativo dá `flo` (`pow(2, -1)` é `0.5`) |
| `mod` | int | `Null` | só com `base` e `expo` inteiros e `expo` não negativo |

## Retorno

int | flo — `int` quando os dois são inteiros e `expo` não é negativo (sem
limite de tamanho: `pow(2, 100)` é exato); `flo` nos outros casos.

## Erros

- **TypeError** — `base` ou `expo` não é número (`pow("a", 2)`)
- **TypeError** — `mod` com algum argumento `flo`
- **ValueError** — `mod` igual a 0
- **ValueError** — `expo` negativo junto com `mod`
- **TypeError** — menos de 2 argumentos

## Exemplos

```ps
post(pow(2, 10), pow(2, -1), pow(2.5, 2))
post(pow(3, 4, 5))
post(pow(2, 100))
```

```saida
1024 0.5 6.25
1
1267650600228229401496703205376
```

[← índice](../builtins.md)
