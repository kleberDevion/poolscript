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

- a tabela vem sempre do `maketrans` — não é um dict qualquer montado à mão

[← índice](../string.md)
