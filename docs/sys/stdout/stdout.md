# `sys.stdout` — saída padrão

Objeto pra escrever no terminal com **controle fino** — sem a quebra de linha
automática do `post`, ou forçando a saída na hora (`flush`).

Os exemplos desta página pressupõem:

```ps contexto
import sys
str texto = "oi"
```

```ps
import sys
sys.stdout.write(texto, end=False)
sys.stdout.writeln(texto)
sys.stdout.flush()
```

---

## Métodos

| Método | O que faz |
|---|---|
| `.write(texto, end=False)` | escreve **sem** quebra de linha (a não ser que `end=True`) |
| `.writeln(texto)` | escreve **com** quebra de linha (como `post`) |
| `.flush()` | força o que foi escrito a aparecer agora |

---

## `write` vs `post`

`post` sempre pula linha no fim. `sys.stdout.write` **não** — útil pra montar
uma linha em pedaços ou fazer barra de progresso:

```ps
import sys

sys.stdout.write("carregando")
sys.stdout.write("...")
sys.stdout.write(" pronto")
sys.stdout.writeln("")        # agora quebra a linha
# saída: carregando... pronto
```

---

## `flush` — aparecer na hora

A saída às vezes fica num buffer e só aparece depois. `.flush()` força mostrar
imediatamente — importante em barras de progresso ou logs ao vivo:

```ps
sys.stdout.write("processando")
sys.stdout.flush()            # mostra "processando" agora, sem esperar
# ... trabalho demorado ...
sys.stdout.writeln(" ok")
```

---

## Relacionados

- [`sys.stderr`](../stderr/stderr.md) — o mesmo, mas na saída de **erro**
- `post` — saída simples com quebra de linha automática
