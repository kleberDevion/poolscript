# `flo(x)`

Converte para número de ponto flutuante.

## Parâmetros

| nome | tipo | default | nota |
|---|---|---|---|
| `x` | str | byte | int | bool | flo | — |  |

## Retorno

flo

## Erros

- **ValueError** — o texto não é um número: `flo("x")` →
  `could not convert string to flo: 'x'`, com o texto como foi escrito
  (`flo("1.5\x00abc")` → `could not convert string to flo: '1.5\x00abc'`).
- **OverflowError** — inteiro grande além do maior `flo`: `flo(10 ** 400)` →
  `int too large to convert to flo`.
- **TypeError** — outro tipo: `flo([1])` → `flo() argument must be a string
  or a real number, not 'list'`.

## Exemplos

```ps
post(flo("2.5"), flo(3), flo(" -1e3 "), flo(".5"), flo("inf"), flo(b"0.25"))
```

```saida
2.5 3.0 -1000.0 0.5 inf 0.25
```

## Bordas

- o nome é `flo`, não `float` — `float` não existe na linguagem
- O texto: branco só nas pontas, sinal opcional, dígitos (de qualquer
  escrita) com um ponto opcional e expoente `e`/`E` opcional, ou `inf`,
  `infinity` e `nan` sem caixa. Hexadecimal não é número aqui:
  `flo("0x1A")` é erro.
- Fora da faixa do `flo` o texto vira `inf` (ou `0.0`), como o literal:
  `flo("1e999")` é `inf`.
- O inteiro grande vira o `flo` mais próximo (`flo(2 ** 60 + 3)` é o mesmo
  de `flo("1152921504606846979")`).

[← índice](../builtins.md)
