# `s.rsplit(sep=Null, maxsplit=-1)`

Como split, mas conta as divisões da DIREITA.

## Parâmetros

| nome | tipo | default | nota |
|---|---|---|---|
| `sep` | str | Null | Null = qualquer branco, colapsando (os mesmos brancos do [`isspace`](../isspace/isspace.md)) |
| `maxsplit` | int | -1 | quantas divisões, no máximo, contando da direita; negativo = todas |

## Retorno

list

## Exemplos

```ps
post("a,b,c".rsplit(",", 1))
```

```saida
['a,b', 'c']
```

## Bordas

- O separador é procurado da direita pra esquerda também sem `maxsplit`: com
  ocorrências que se sobrepõem, o resultado difere do `split`.

  ```ps
  post("aaa".rsplit("aa"), "aaa".split("aa"))
  ```

  ```saida
  ['a', ''] ['', 'a']
  ```

- Com `sep` Null e `maxsplit`, o resto da esquerda fica com o branco da
  esquerda: `"  a b c  ".rsplit(Null, 1)` é `['  a b', 'c']`.

## Erros

- **ValueError** — separador vazio: `empty separator`
- **TypeError** — `maxsplit` que não é inteiro: `"a b".rsplit(" ", "1")` →
  `'str' object cannot be interpreted as an integer`

[← índice](../str.md)
