# sys — Sistema, saída e argumentos

Lib com utilitários do runtime: encerrar o programa, saber o sistema
operacional, escrever no terminal com controle fino, e ler os argumentos da
linha de comando.

```
import sys
```

| Membro | O que faz | Página |
|---|---|---|
| `sys.exit(code)` | encerra o programa | [exit/exit.md](exit/exit.md) |
| `sys.platform()` | o sistema: `windows`/`linux`/`darwin` | [platform/platform.md](platform/platform.md) |
| `sys.RelativePath(nome)` | acha um arquivo pelo nome (busca recursiva) | [RelativePath/RelativePath.md](RelativePath/RelativePath.md) |
| `sys.stdout` | escrever na saída padrão | [stdout/stdout.md](stdout/stdout.md) |
| `sys.stderr` | escrever na saída de erro | [stderr/stderr.md](stderr/stderr.md) |
| `sys.argv` | argumentos da linha de comando | [argv/argv.md](argv/argv.md) |

---

## Exemplo rápido

```
import sys

if (sys.platform() == "windows") {
    post("rodando no Windows")
}

sys.stdout.write("sem quebra de linha")   // controle fino da saída
sys.stdout.writeln(" com quebra")

if (algo_deu_errado) {
    sys.exit(1)                            // encerra com código de erro
}
```
