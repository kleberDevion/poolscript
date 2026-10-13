# `int(x)`

Converte para inteiro: texto numérico, conteúdo de um `byte`, `flo` (trunca)
ou `bool`.

## Parâmetros

| nome | tipo | default | nota |
|---|---|---|---|
| `x` | str | byte | flo | bool | int | — |  |

## Retorno

int (inteiro grande quando não cabe em 64 bits)

## Erros

- **ValueError** — o texto não é um inteiro: `int("abc")` →
  `invalid literal for int() with base 10: 'abc'`. O texto sai como foi
  escrito, com o caractere invisível à mostra: `int("15\x00")` →
  `invalid literal for int() with base 10: '15\x00'`.
- **TypeError** — outro tipo: `int([1])` → `int() argument must be a string,
  a byte-like object or a real number, not 'list'`.

## Exemplos

```ps
post(int("7"), int(3.9), int(True), int(" +42 "), int(b"12"))
```

```saida
7 3 1 42 12
```

## Bordas

- float TRUNCA em direção ao zero (`int(3.9)` → 3), não arredonda
- O texto: branco só nas pontas (o mesmo do `strip`), sinal `+` ou `-`
  opcional, e dígitos decimais de qualquer escrita, os do
  [`isdecimal`](../../str/isdecimal/isdecimal.md): `int("٣٤")` é 34,
  `int("４２")` é 42. Espaço no meio é erro (`"9 1"`, `"99999999999999999999 1"`),
  e ponto também (`int("1.5")`: use `int(flo("1.5"))`).
- Sem teto de dígitos: `int("9" * 600)` tem os 600.
- O `byte` vale só com dígito ASCII: `int(b"12")` é 12, `int(b"1x")` é
  `invalid literal for int() with base 10: b'1x'`.

[← índice](../builtins.md)
