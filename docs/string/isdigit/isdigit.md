# `s.isdigit()`

True se só tem dígitos.

## Retorno

bool

## Exemplos

![exemplo 1](../../assets/string__isdigit__isdigit_ex1.png)

<details><summary>código</summary>

```ps
post("123".isdigit(), "1.5".isdigit())
```

</details>

```saida
True False
```

## Bordas

- número também responde: `(150).isdigit()` é True — conversão automática pra string

[← índice](../string.md)
