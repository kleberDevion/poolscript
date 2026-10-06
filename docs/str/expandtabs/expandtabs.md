# `s.expandtabs(tabsize=8)`

Troca cada tab por espaços até a próxima parada de tabulação (não por um número fixo de espaços).

## Parâmetros

| nome | tipo | default | nota |
|---|---|---|---|
| `tabsize` | int | 8 | largura da coluna de tabulação |

## Retorno

str

## Exemplos

```ps
post("a\tbc\td".expandtabs(4))
```

```saida
a   bc  d
```

## Bordas

- o número de espaços varia com a posição: o tab alinha na próxima parada, não insere `tabsize` espaços fixos
- `tabsize` tem o mesmo teto de `ljust` (256 MB): acima dele é `MemoryError:
  memória insuficiente: expandtabs() pediu largura N`, antes de alocar
  qualquer coisa. Vale igual em `byte`.

[← índice](../str.md)
