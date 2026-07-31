# `sys.exit(code=0)`

Encerra o programa imediatamente, com um código de saída.

```
sys.exit(code: int = 0) -> None
```

| Parâmetro | Padrão | O que é |
|---|---|---|
| `code` | `0` | código de saída: `0` = sucesso, diferente de `0` = erro |

---

## Uso

```
import sys

if (not os.exists("config.json")) {
    post("config não encontrado")
    sys.exit(1)                // encerra com erro
}

// ... segue o programa ...
sys.exit()                     // encerra com sucesso (código 0)
```

---

## Convenção dos códigos

- **`0`** — tudo certo (sucesso). É o padrão.
- **diferente de `0`** — algo deu errado. Scripts e o sistema usam isso pra
  saber se seu programa falhou (ex: `1` = erro genérico).

---

## Relacionados

- [`sys.stderr`](../stderr/stderr.md) — escrever a mensagem de erro antes de sair
