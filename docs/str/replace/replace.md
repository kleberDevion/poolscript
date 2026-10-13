# `s.replace(old, new, count)`

Troca ocorrências; `old` pode ser LISTA de alvos.

## Parâmetros

| nome | tipo | default | nota |
|---|---|---|---|
| `old` | str \| list | — | lista: cada alvo é trocado |
| `new` | str \| list | — | `str`: todo alvo vira ele; `list` (com `old` lista): em paralelo, o i-ésimo alvo vira o i-ésimo novo |
| `count` | int | todas | máximo de trocas (com lista, somando todos os alvos); omitir ou negativo troca todas; 0 não troca nada |

Os nomes são estes: `"banana".replace(velho="na", novo="NA")` é
`TypeError: 'velho' is an invalid keyword argument for replace()`.

## Retorno

str

## A forma com lista

Uma passada só, da esquerda pra direita. Em cada posição vale o **primeiro
alvo da lista** que casa ali, e o texto que entrou não é trocado de novo: dá
pra trocar dois alvos entre si.

```ps
post("ab".replace(["a", "b"], ["b", "a"]))
post("gato e rato".replace(["gato", "rato"], ["rato", "gato"]))
post("a-b-c_d".replace(["-", "_"], "+", 1))
post("abab".replace(["a", "ab"], ["1", "2"]), "abab".replace(["ab", "a"], ["1", "2"]))
```

```saida
ba
rato e gato
a+b-c_d
1b1b 11
```

## Erros

- **TypeError** — sem o `new`: `"banana".replace("a")` →
  `replace() takes at least 2 arguments (1 given)`.
- **TypeError** — item que não é `str`: `"x".replace([1], "y")` →
  `replace() argument 1 item 0 must be str, not int`; o mesmo com
  `argument 2` na lista de novos; `new` que não é `str` nem lista →
  `replace() argument 2 must be str or list, not int`.
- **ValueError** — as duas listas com tamanhos diferentes:
  `"x-a".replace(["a", "b"], ["z"])` → `replace(): 2 alvo(s) e 1 novo(s); em
  paralelo, as duas listas tem o mesmo tamanho`.
- **ValueError** — alvo vazio dentro da lista (ele casaria em toda posição e
  esconderia os outros): `replace(): o item 0 da lista de alvos e vazio; o
  alvo vazio vale sozinho: replace("", novo)`.

## Exemplos

```ps
post("banana".replace("na", "NA", 1))
```

```saida
baNAna
```

```ps
post("a-b_c".replace(["-", "_"], "."))
```

```saida
a.b.c
```

```ps
post("A e B".replace(["A", "B"], ["x", "y"]))
```

```saida
x e y
```

## Bordas

- a forma com lista é extensão da Jinga
- o alvo vazio sozinho casa em toda fronteira de caractere, inclusive nas
  pontas: `"a".replace("", "x")` é `"xax"`

[← índice](../str.md)
