# `sys.platform()`

Devolve qual **sistema operacional** está rodando, como string.

```
sys.platform() -> str
```

| Retorno | Sistema |
|---|---|
| `"windows"` | Windows |
| `"linux"` | Linux |
| `"darwin"` | macOS |

---

## Uso

```
import sys

so = sys.platform()

if (so == "windows") {
    caminho = "C:\dados"
} else {
    caminho = "/home/dados"
}
```

Útil pra ajustar comportamento por sistema (caminhos, comandos de terminal).

---

## Relacionados

- [`os.cmd()`](../../os/cmd/cmd.md) — rodar comandos (que variam por SO)
