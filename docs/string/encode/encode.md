# `s.encode(encoding="utf-8")`

Converte a string para bytes.

## Parâmetros

| nome | tipo | default | nota |
|---|---|---|---|
| `encoding` | str | "utf-8" | aceito e ignorado — tudo é utf-8 |

## Retorno

bytes

## Exemplos

![exemplo 1](../../assets/string__encode__encode_ex1.png)

<details><summary>código</summary>

```ps
post("ab".encode())
```

</details>

```saida
b'ab'
```

## Bordas

- bytes só é igual a bytes: `"ab" == "ab".encode()` é False

[← índice](../string.md)
