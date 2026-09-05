# `sys.argv`

Os **argumentos da linha de comando** passados quando você rodou o programa —
uma lista de strings.

```
sys.argv   # lista
```

---

## Uso

Se você rodar `pool app.ps entrada.txt saida.txt`:

```
import sys

post(sys.argv)        # ["entrada.txt", "saida.txt"]

# pegar um argumento (checando se existe)
if (len(sys.argv) > 0) {
    arquivo = sys.argv[0]      # "entrada.txt"
    post("processando:", arquivo)
}
```

`argv[0]` é o **primeiro argumento seu**, não o nome do script — diferente do
Python. O nome do script não entra na lista.

> Esta página dizia o contrário, e o exemplo (`len(sys.argv) > 1` / `argv[1]`)
> ensinava a pular o primeiro argumento de verdade. `docs/sys.md` sempre esteve
> certa; as duas se contradiziam.

Indexar fora do intervalo **levanta** `IndexError: list index out of range`, por
isso a guarda com `len()`.

---

## Relacionados

- [`sys.exit()`](../exit/exit.md) — encerrar (ex: se faltar argumento)
