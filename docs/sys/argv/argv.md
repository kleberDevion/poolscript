# `sys.argv`

Os **argumentos da linha de comando** passados quando você rodou o programa —
uma lista de strings.

```
sys.argv   # lista
```

---

## Uso

`argv[0]` é o **nome do script**; os seus argumentos vêm a partir de `argv[1]`
— a mesma convenção de C, Python e JS. Rodando
`jinga app.pr entrada.txt saida.txt`:

```
import sys

post(sys.argv)        # ['app.pr', 'entrada.txt', 'saida.txt']  — aspas simples

# pegar o primeiro argumento (checando se existe)
if (len(sys.argv) > 1) {
    arquivo = sys.argv[1]      # "entrada.txt"
    post("processando:", arquivo)
}
```

| índice | é |
|---|---|
| `argv[0]` | o nome do script (`app.pr`) — `__main__` quando o código vem de `jinga -e` |
| `argv[1]`, `argv[2]`, … | os seus argumentos, na ordem |

Tudo o que vem **depois** do arquivo é do programa, inclusive o que parece
opção do `jinga`: `jinga app.pr a --sem-jit` entrega `['app.pr', 'a',
'--sem-jit']`. Opção pro próprio `jinga` (`--sem-jit`, `--utf16`) vai antes do
arquivo. Com `-e`, os argumentos vêm depois do código: `jinga -e
'import sys; post(sys.argv)' a b` imprime `['__main__', 'a', 'b']`.

Então **conte quantos você espera contando o script**: um programa que precisa
de um argumento exige `len(sys.argv) >= 2` (o script + o argumento).

Indexar fora do intervalo **levanta** `IndexError: list index out of range` —
por isso a guarda com `len()`. Sem argumento nenhum, `sys.argv` tem só o
`[0]` (o script), e `argv[1]` estoura.

---

## Relacionados

- [`sys.exit()`](../exit/exit.md) — encerrar (ex: se faltar argumento)
