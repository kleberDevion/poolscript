# `s.translate(tabela)`

Troca caracteres segundo a tabela do maketrans (de→para, caractere a caractere).

## Parâmetros

| nome | tipo | default | nota |
|---|---|---|---|
| `tabela` | dict | — | resultado de `maketrans(de, para)` |

## Retorno

str

## Exemplos

```ps
post("hello".translate("l".maketrans("l", "L")))
```

```saida
heLLo
```

## Erros

- **ValueError** — o destino na tabela é um codepoint fora da faixa Unicode:
  `"abc".translate({97: -1})` → `character mapping must be in range(0x110000)`;
  um substituto (`{97: 55296}`) → `character mapping is a surrogate
  (0xd800..0xdfff), not a character`. A mesma faixa do [`chr`](../../builtins/chr/chr.md).

## Bordas

- a tabela **normalmente** vem do `maketrans`, mas um dict montado à mão é
  aceito — e aí a chave precisa ser o **codepoint**, não a letra:

  ```ps
  post("hello".translate({108: 76}))    # heLLo   (108 é "l", 76 é "L")
  post("hello".translate({"l": "L"}))   # hello   — chave str é IGNORADA, em silêncio
  ```

[← índice](../str.md)
