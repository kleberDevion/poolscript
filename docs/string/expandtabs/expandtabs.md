# `s.expandtabs(tabsize=8)`

Troca cada tab por espaços até a próxima parada de tabulação (não por um número fixo de espaços).

## Parâmetros

| nome | tipo | default | nota |
|---|---|---|---|
| `tabsize` | int | 8 | largura da coluna de tabulação |

## Retorno

str

## Exemplos

![exemplo 1](../../assets/string__expandtabs__expandtabs_ex1.png)

<details><summary>código</summary>

```ps
post("a\tbc\td".expandtabs(4))
```

</details>

```saida
a   bc  d
```

## Bordas

- o número de espaços varia com a posição: o tab alinha na próxima parada, não insere `tabsize` espaços fixos

[← índice](../string.md)
