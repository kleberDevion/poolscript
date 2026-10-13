# `s.format(a, b)`

Preenche `{}` e `{0}` no template, **por posição**.

## Parâmetros

| nome | tipo | default | nota |
|---|---|---|---|
| `a` | qualquer | Null | posicional |
| `b` | qualquer | Null | posicional |

Chamar sem argumento nenhum funciona (`"x".format()` devolve `"x"`).

## Retorno

str

## O especificador `{:...}`

Depois de `:` dentro das chaves vem o formato do valor, nesta ordem, tudo
opcional: `{:[preenche][alinha][0][largura][.precisao][tipo]}`.

| parte | valores | efeito |
|---|---|---|
| `preenche` | um caractere qualquer (`*`, `·`, `漢`) | o que completa a largura; só vale seguido do alinhamento; o padrão é o espaço |
| `alinha` | `<` `>` `^` | esquerda, direita, centro (a sobra ímpar do centro vai pra direita) |
| `0` | | num número, zeros **depois do sinal** (`{:05d}` de -42 é `-0042`, como o `zfill`); num texto, zeros à esquerda (`{:05}` de `"ab"` é `000ab`) |
| `largura` | inteiro | o mínimo de caracteres; o que passa dela não é cortado |
| `.precisao` | inteiro | no `f`, as casas decimais (padrão 6); num texto, o máximo de **caracteres** |
| `tipo` | `s` `d` `f` `x` `X` `o` `b` | a tabela abaixo |

| tipo | valor | sai |
|---|---|---|
| `s` | qualquer | o texto do valor, o mesmo do `str()` |
| `d` | `int` de qualquer tamanho, `bool` | decimal (`True` é 1) |
| `x` `X` `o` `b` | `int` de qualquer tamanho, `bool` | hexadecimal (minúsculo, maiúsculo), octal, binário, com o sinal: `{:x}` de -255 é `-ff`, como o `hex()`, sem o prefixo |
| `f` | `int`, `flo`, `bool` | ponto fixo, com `precisao` casas |
| sem tipo | qualquer | `int` como o `d`, `flo` como o `str()` dele, o resto como texto |

- Sem `alinha`, número (`int`, `flo`, e os tipos `d` `f` `x` `X` `o` `b`)
  alinha à direita; o resto, à esquerda.
- Com `alinha` escrito, o `0` só vira o preenchimento, antes do sinal
  (`{:>05d}` de -42 é `00-42`); um `preenche` escrito ganha do `0`
  (`{:*>05d}` é `**-42`).

```ps
post("[{:05d}]".format(-42), "[{:08.2f}]".format(-3.14159), "[{:x}]".format(-255))
post("[{:5}]".format(42), "[{:5}]".format("ab"), "[{:^7}]".format("meio"))
post("[{:.2}]".format("漢字かな"), "[{:·>6}]".format("ab"), "[{:d}]".format(2 ** 70))
```

```saida
[-0042] [-0003.14] [-ff]
[   42] [ab   ] [ meio  ]
[漢字] [····ab] [1180591620717411303424]
```

## Erros

- **IndexError** — o índice citado no formato não existe:
  `"{2}".format(1)` → `Replacement index 2 out of range for positional args tuple`
- **KeyError** — o formato cita um **nome**: `"{x}".format(1)` → `'x'`
- **TypeError** — argumento nomeado: `"{nome}".format(nome="ana")` →
  `'nome' is an invalid keyword argument for format()`
- **TypeError** — o tipo do formato não serve pro valor: `"{:d}".format("a")`
  → `format: 'd' espera int, recebeu str`; `"{:f}".format("a")` →
  `format: 'f' espera numero, recebeu str`.
- **ValueError** — o especificador está fora da forma: `"{:+d}".format(5)` →
  `format: especificador '+d' invalido; a forma e
  [preenche][<>^][0][largura][.precisao][tipo]`; tipo que não existe
  (`"{:e}"`) → `format: tipo 'e' nao existe; os tipos sao s d f x X o b`;
  precisão num inteiro (`"{:.2d}"`) → `format: precisao nao vale pra inteiro
  ('d'); pra casas decimais use o tipo 'f' ({:.2f})`; precisão num `flo` sem
  tipo (`"{:.2}".format(3.14)`) → `format: precisao num flo pede o tipo 'f'
  ({:.2f})`.
- **MemoryError** — largura ou precisão acima de 256 MB no spec:
  `"{:.999999999f}"` → `memória insuficiente: format() pediu precisao
  999999999`, e `pediu largura N` pra largura, com o número como foi escrito.
  O índice posicional não tem teto de dígitos: `"{2147483648}".format(1)` é o
  IndexError de sempre, com o número como foi escrito.

## Exemplos

```ps
post("{} e {}".format(1, "x"))
```

```saida
1 e x
```

## Bordas

- **`{nome}` não se preenche por aqui.** Keyword é recusada e dict posicional
  dá `KeyError`. Para nomeados existe [`format_map`](../format_map/format_map.md):
  `"{nome}".format_map({"nome": "ana"})`.
- `Null` formata como `Null`: `"{}".format(Null)` é `"Null"`.

[← índice](../str.md)
