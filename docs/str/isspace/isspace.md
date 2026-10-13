# `s.isspace()`

True se só tem espaços em branco (e pelo menos um caractere).

## Retorno

bool

## Exemplos

```ps
post("  ".isspace(), "\U00003000".isspace(), "".isspace())
```

```saida
True True False
```

## Bordas

- Branco é a propriedade `White_Space` do Unicode, inteira: espaço, `\t`
  `\n` `\v` `\f` `\r`, NEL (U+0085), espaço sem quebra (U+00A0), ogham
  (U+1680), os espaços tipográficos U+2000 a U+200A, separador de linha e
  de parágrafo (U+2028, U+2029), espaço estreito sem quebra (U+202F),
  espaço matemático (U+205F) e o espaço ideográfico (U+3000).
- É a mesma régua do `split()` e do `rsplit()` sem separador e do `strip()`
  sem argumento.
- Os separadores de informação U+001C a U+001F são controle, não branco
  (o Python os conta como branco; a propriedade White_Space, não).
- O espaço de largura zero (U+200B) não é branco: é formatação invisível
  (e não é imprimível, ver [`isprintable`](../isprintable/isprintable.md)).

[← índice](../str.md)
