# O C na Jinga — a biblioteca padrão do C

A Jinga chama a biblioteca padrão do C (C11) direto: as funções, os tipos e
as constantes de cada cabeçalho, com ponteiro de verdade e a sintaxe do C. O
cabeçalho se importa como o `#include` do C, e os nomes dele ficam soltos:

```ps
import <stdio.h>
import <string.h>

char buf[32]
strcpy(buf, "Jinga")
strcat(buf, " + C")
printf("%s tem %zu bytes\n", buf, strlen(buf))
```

```saida
Jinga + C tem 9 bytes
```

A parte da linguagem (o import, os tipos, o ponteiro, o array e o struct, o
cast, a chamada, o callback, o que derruba e o que fica de fora) está no
capítulo [15. O C na Jinga](../linguagem/15-c.md). Aqui fica a referência de
cada cabeçalho.

## Os cabeçalhos

| Cabeçalho | O que traz |
|---|---|
| [`<ctype.h>`](ctype.md) | classificação e caixa de caracteres (`isdigit`, `toupper`...) |
| [`<errno.h>`](errno.md) | o `errno` e os códigos `EDOM`, `ERANGE`, `EILSEQ` |
| [`<float.h>`](float.md) | os limites do ponto flutuante (`DBL_MAX`, `FLT_EPSILON`...) |
| [`<inttypes.h>`](inttypes.md) | os formatos `PRId64`, `SCNd32`..., `imaxabs`, `strtoimax`; traz o `<stdint.h>` |
| [`<limits.h>`](limits.md) | os limites dos inteiros (`INT_MAX`, `CHAR_BIT`...) |
| [`<math.h>`](math.md) | a matemática: raiz, potência, trigonometria, arredondamento, classificação |
| [`<stdbool.h>`](stdbool.md) | o `bool` |
| [`<stddef.h>`](stddef.md) | `size_t`, `ptrdiff_t`, `max_align_t`, `NULL`, `offsetof` |
| [`<stdint.h>`](stdint.md) | os inteiros de largura fixa (`int32_t`, `uint8_t`...) e os limites deles |
| [`<stdio.h>`](stdio.md) | entrada e saída: `printf`, `scanf`, `fopen`, `fgets`... |
| [`<stdlib.h>`](stdlib.md) | memória (`malloc`, `free`), conversão (`strtol`), `qsort`, `exit`, `rand`... |
| [`<string.h>`](string.md) | texto e memória do C: `strlen`, `strcpy`, `memcpy`, `strerror`... |
| [`<time.h>`](time.md) | data e hora: `time`, `gmtime`, `mktime`, `strftime`... |

Os outros cabeçalhos do C11 ficam de fora, cada um com o motivo, na tabela
da [seção 15.1](../linguagem/15-c.md#151-import-stdioh--o-cabeçalho-igual-ao-include).

## Como as páginas são feitas

- Cada página segue a cppreference
  ([en.cppreference.com/w/c](https://en.cppreference.com/w/c), licença
  [CC BY-SA 3.0](https://creativecommons.org/licenses/by-sa/3.0/)), traduzida
  e adaptada à Jinga, com a atribuição no topo.
- Cada função tem a seção dela (ou da família: `sqrt`, `sqrtf`, `sqrtl`), com
  o protótipo do C11 (Anexo B do N1570), o que faz, os parâmetros, o retorno,
  o `errno` e um exemplo em Jinga com a saída.
- O protótipo de cada seção é conferido contra a tabela do motor
  (`scripts/audita_doc.pr`), e a saída de cada exemplo é conferida rodando
  (`scripts/confere_saida_doc.pr`): a página que diz uma coisa e o motor faz
  outra não passa no `make check`.
- O que a Jinga faz diferente do C está dito no lugar. O principal: o nome
  que existe nos dois é o da Jinga (`round`, `abs` e `pow`; o do C pelo
  `lround`, `labs`, `powf`), e o `str` da Jinga só entra onde o C não escreve
  (`const char *`).

## No editor

O completion, o hover e a ajuda de assinatura mostram os nomes dos
cabeçalhos que o arquivo importou, depois dos nomes da Jinga: a declaração
como o C escreve, o cabeçalho e a seção destas páginas. Sem o import, o hover
de um nome do C diz o import que falta ([lsp.md](../lsp.md)).
