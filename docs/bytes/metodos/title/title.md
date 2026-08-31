# `b.title()`

Primeira letra de cada palavra em maiúscula, resto em minúscula. Só ASCII; dígito não é letra e por isso separa palavra.

## Parâmetros

Nenhum.

## Retorno

bytes

## Exemplos

```ps
import bytes
post("hello wOrld 3ab".encode().title())
```

```saida
b'Hello World 3Ab'
```

[← índice](../../bytes.md)
