# `s.isprintable()`

True se todos os caracteres são imprimíveis (a string vazia também é).

## Retorno

bool

## Exemplos

```ps
post("oi".isprintable(), "a\tb".isprintable(), "a​b".isprintable())
```

```saida
True False False
```

## Bordas

Não é imprimível:

- controle: U+0000 a U+001F, DEL (U+007F) e o bloco C1 (U+0080 a U+009F,
  onde ficam o NEL e o CSI);
- os brancos do [`isspace`](../isspace/isspace.md), menos o espaço comum;
- formatação invisível: hífen suave (U+00AD), espaço de largura zero e as
  marcas de direção (U+200B a U+200F, U+202A a U+202E, U+2060 a U+2064,
  U+2066 a U+206F), o BOM (U+FEFF) e os outros da categoria Cf;
- substituto (U+D800 a U+DFFF), uso privado e os não-caracteres (U+FDD0 a
  U+FDEF e os dois últimos de cada plano).

[← índice](../str.md)
