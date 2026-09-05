# `regex.split(pattern, string, maxsplit=0, flags=0)`

**Divide** o texto onde o padrão bate, devolvendo uma lista de pedaços. Como o
`split` de string, mas o separador é um padrão (não um texto fixo).

```
regex.split(pattern: str, string: str, maxsplit=0, flags=0) -> list
```

| Parâmetro | Padrão | O que é |
|---|---|---|
| `pattern` | — | o separador (como regex) |
| `string` | — | o texto a dividir |
| `maxsplit` | `0` | máximo de divisões (`0` = ilimitado) |

---

## Uso

```
import regex

# dividir por qualquer pontuação/espaço
partes = regex.split(r"[,;\s]+", "ana, leo; bia  joao")
post(partes)         # ["ana", "leo", "bia", "joao"]

# dividir por um ou mais dígitos
regex.split(r"\d+", "a1b22c333d")   # ["a", "b", "c", "d"]
```

Diferente do `split` normal de string (que separa por um texto fixo), aqui o
separador é um **padrão** — bom quando o separador varia (espaço, vírgula,
ponto-e-vírgula, tudo junto).

---

## Relacionados

- [`regex.sub()`](../sub/sub.md) — substituir em vez de dividir
- [`regex.findall()`](../findall/findall.md) — extrair os pedaços que casam (o oposto de dividir)
