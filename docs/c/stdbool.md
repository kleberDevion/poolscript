# stdbool.h — o tipo booleano

O `stdbool.h` dá ao tipo booleano do C, o `_Bool`, o nome `bool`, e define o
`true`, o `false` e o `__bool_true_false_are_defined`. Na Jinga, o `bool` vale
sem import: é o tipo booleano da própria Jinga, com os valores `True` e
`False`. O `__bool_true_false_are_defined` vale com `import <stdbool.h>` no
topo do arquivo, igual ao `#include` do C; sem o import, o nome é `NameError`,
com a dica do import que falta. O `true` e o `false` ficam de fora
([o C na Jinga](c.md)).

> Esta página segue a cppreference
> ([en.cppreference.com/w/c/types/boolean](https://en.cppreference.com/w/c/types/boolean),
> licença [CC BY-SA 3.0](https://creativecommons.org/licenses/by-sa/3.0/)),
> traduzida e adaptada à Jinga. As declarações são as do C11, conferidas
> contra a tabela do motor. Cada exemplo foi rodado.

| Macro | No C, expande pra | Na Jinga |
|---|---|---|
| [`bool`](#bool) | `_Bool` | o `bool` da Jinga; na memória do C, o `_Bool` |
| [`true`, `false`](#true-e-false) | as constantes inteiras `1` e `0` | ficam de fora: a Jinga escreve `True` e `False` |
| [`__bool_true_false_are_defined`](#__bool_true_false_are_defined) | a constante inteira `1` | `1`, com o `import <stdbool.h>` |

## Na Jinga

- Numa variável da Jinga (`bool pronto = True`), o `bool` é o da Jinga: só
  aceita `True` e `False`, sem conversão (`bool b = 5` é
  `AttributedValueError`).
- Na memória do C (o que o ponteiro aponta, o elemento de array, o campo de
  `struct`, o cast, o `sizeof`), o `bool` é o `_Bool` do C, de 1 byte: guardar
  ali qualquer número diferente de zero dá `True`, e o zero dá `False`, como na
  conversão do C. A leitura volta como `bool` da Jinga. O `_Bool`, que é
  palavra do C, também vale.
- Os operadores lógicos do C (`&&`, `||` e `!`) valem na Jinga, ao lado do
  `and`, do `or` e do `not`.

```ps
import <stdlib.h>

bool *p = malloc(3 * sizeof(bool))
p[0] = True
p[1] = 0
p[2] = 7          # todo número diferente de zero vira 1, como no C
post(p[0], p[1], p[2], sizeof(bool))
free(p)
```

```saida
True False True 1
```

Na variável da Jinga, o número não vira `bool`:

```ps
bool b = 5    # AttributedValueError: a variável da Jinga não converte
```

```saida
```

```erro
AttributedValueError: variável b esperava bool, recebeu int
```

## As macros

### `bool`

```c
#define bool _Bool
```

Desde o C99, a linguagem C tem a aritmética booleana com o tipo embutido
`_Bool`; com o `stdbool.h`, o tipo também se escreve `bool`. Os operadores
lógicos `&&`, `||` e `!` valem com o tipo booleano em qualquer combinação. No
C23, o `bool`, o `true` e o `false` viraram palavras da linguagem, e as macros
saíram do cabeçalho.

O exemplo da cppreference, na Jinga: o `%d` do `printf` escreve o `bool` como
`0` ou `1`.

```ps
import <stdio.h>

bool a = True
bool b = False
printf("%d\n", a && b)
printf("%d\n", a || b)
printf("%d\n", !b)
```

```saida
0
1
1
```

### `__bool_true_false_are_defined`

```c
#define __bool_true_false_are_defined 1
```

Expande pra constante inteira `1`: avisa que o `bool`, o `true` e o `false`
estão definidos. No C23, ficou obsoleta.

```ps
import <stdbool.h>

post(__bool_true_false_are_defined)
```

```saida
1
```

## Fica de fora

### `true` e `false`

```c
#define true 1
#define false 0
```

No C, o `true` expande pra constante inteira `1`, e o `false` pra `0`. Na
Jinga, os valores booleanos são `True` e `False`, com maiúscula, em todo lugar:
o `true` e o `false` dão `NameError` antes de rodar, com a dica do nome certo,
com ou sem o `import <stdbool.h>`. Nada sai no stdout:

```ps
import <stdbool.h>

bool pronto = true    # NameError: a Jinga escreve True
```

```saida
```

```erro
NameError: name 'true' is not defined. Did you mean: 'True'?
```
