# `sys.argv`

Os **argumentos da linha de comando** passados quando você rodou o programa —
uma lista de strings.

```
sys.argv   // lista
```

---

## Uso

Se você rodar `pool app.ps entrada.txt saida.txt`:

```
import sys

post(sys.argv)        // ["app.ps", "entrada.txt", "saida.txt"]

// pegar um argumento (checando se existe)
if (len(sys.argv) > 1) {
    arquivo = sys.argv[1]      // "entrada.txt"
    post("processando:", arquivo)
}
```

O primeiro item (`argv[0]`) é o nome do script; os seguintes são o que você
passou depois.

---

## Relacionados

- [`sys.exit()`](../exit/exit.md) — encerrar (ex: se faltar argumento)
