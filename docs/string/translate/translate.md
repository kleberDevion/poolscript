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

## Bordas

- a tabela **normalmente** vem do `maketrans`, mas um dict montado à mão é
  aceito — e aí a chave precisa ser o **codepoint**, não a letra:

  ```ps
  post("hello".translate({108: 76}))    # heLLo   (108 é "l", 76 é "L")
  post("hello".translate({"l": "L"}))   # hello   — chave str é IGNORADA, em silêncio
  ```

[← índice](../string.md)
