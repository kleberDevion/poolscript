# `sys.stderr` — saída de erro

Igual ao [`sys.stdout`](../stdout/stdout.md), mas escreve na **saída de erro**
(stderr) — o canal separado que o sistema usa pra mensagens de erro/aviso.

```
import sys
sys.stderr.write(texto, end=false)
sys.stderr.writeln(texto)
sys.stderr.flush()
```

---

## Métodos

Os mesmos do `stdout`:

| Método | O que faz |
|---|---|
| `.write(texto, end=false)` | escreve sem quebra de linha |
| `.writeln(texto)` | escreve com quebra de linha |
| `.flush()` | força aparecer agora |

---

## Por que existe um canal separado

`stdout` é a saída "normal" (o resultado do programa); `stderr` é pra
**erros e avisos**. Manter separados permite, por exemplo, redirecionar só o
resultado pra um arquivo enquanto os erros continuam aparecendo na tela:

```
import sys

sys.stdout.writeln("resultado: 42")      # saída normal
sys.stderr.writeln("aviso: cache vazio") # vai pro canal de erro
```

Mensagens de erro/log devem ir pro `stderr`, não pro `stdout`.

---

## Relacionados

- [`sys.stdout`](../stdout/stdout.md) — saída normal (mesmos métodos)
- [`sys.exit()`](../exit/exit.md) — encerrar após escrever um erro
