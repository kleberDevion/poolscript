# `gather(a, b, ...)`

Espera vários `async funct` de uma vez, **rodando-os concorrentes**, e devolve os resultados numa lista, na ordem dos argumentos. Argumento que não é future passa direto. Aceita também uma **lista** de futures (`gather(fs)`).

## Parâmetros

| nome | tipo | default | nota |
|---|---|---|---|
| `a, b...` | future/qualquer | — | futures de `async funct`; valor comum passa direto |

## Retorno

list — os valores resolvidos, na ordem dos argumentos.

## Exemplos

```ps
async funct dobro(n) {
    sleep(0.2)
    return n * 2
}

post(gather(dobro(1), dobro(2), dobro(3)))
```

```saida
[2, 4, 6]
```

```ps
async funct dobro(n) {
    sleep(0.2)
    return n * 2
}

fs = []
for each i in range(3) {
    addEnd(fs, dobro(i))
}
post(gather(fs))
```

```saida
[[0, 2, 4]]
```

Duas tarefas que falham: o `gather` espera as duas e levanta o erro da primeira, com a outra na frase.

```ps
async str funct t(int n) {
    sleep(0.01 * n)
    raise Quebrou("tarefa " + str(n))
}
try {
    post(gather(t(1), t(2)))
} catch (Quebrou e) {
    post(e)
}
```

```saida
tarefa 1 (linha 3); mais 1 tarefa do gather falhou: Quebrou: tarefa 2 (linha 3)
```

## Bordas

- Resolve os futures rodando as tasks concorrentes (fibras). Valor comum (não-future) volta como está.
- `gather(fs)` com uma lista devolve **lista dentro de lista** (um resultado por argumento; o argumento-lista vira a sub-lista dos seus valores).
- **Tarefa que falha não interrompe o `gather`:** ele espera todas e levanta o erro da primeira que falhou (na ordem dos argumentos), com o traceback dela. Se só uma falhou, o erro é o dela, como no `await` daquela tarefa; se mais de uma, as outras vão na frase: `mensagem (linha L); mais N tarefa(s) do gather falhou/falharam: Tipo: mensagem (linha L); ...`, com `L` a linha do `raise` de cada uma. Todas foram aguardadas: nenhuma sai no fim como tarefa sem `await`, e o `catch` em volta do `gather` decide como o programa sai. A mesma tarefa repetida (`gather(f, f)`) conta uma vez.
- `await [f1, f2]` (lista ou tupla) é o mesmo `gather`, com os valores trocados no lugar; a frase diz `do await`.

[← índice](../builtins.md)
