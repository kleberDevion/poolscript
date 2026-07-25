# String methods — texto em PoolScript

Toda string em PoolScript tem métodos embutidos — **sem `import`**. Você chama
com ponto, direto no valor ou na variável:

```
"Poolscript".upper()      // "POOLSCRIPT"
nome = "  ana  "
nome.strip()              // "ana"
```

Duas coisas que fazem os métodos de string aqui serem confortáveis:

- **Encadeamento** — quase todo método que transforma texto devolve outra
  string, então você emenda um no outro:

  ```
  "  Olá Mundo  ".strip().lower().replace(" ", "-")   // "olá-mundo"
  ```

- **Todos os métodos nativos do Python também funcionam** (`.find`, `.count`,
  `.zfill`, `.center`, `.format`, `.splitlines`…). Esta doc cobre os métodos
  **estendidos/mais usados**; se você conhece um de Python, ele está aqui.

---

## Mapa dos métodos

| Grupo | Métodos | Página |
|---|---|---|
| Maiúsc./minúsc. | `upper`, `lower`, `title`, `capitalize` | [caso/caso.md](caso/caso.md) |
| Limpar bordas | `strip`, `lstrip`, `rstrip` | [strip/strip.md](strip/strip.md) |
| Trocar trechos | `replace` (aceita lista!) | [replace/replace.md](replace/replace.md) |
| Quebrar | `split` | [split/split.md](split/split.md) |
| Juntar | `join` | [join/join.md](join/join.md) |
| Perguntar (`bool`) | `isdigit`, `isalpha`, `startswith`, `contains`… | [verificacao/verificacao.md](verificacao/verificacao.md) |
| Regex integrado | `match`, `findall`, `sub` | [regex/regex.md](regex/regex.md) |

Para o **tamanho** de uma string, use o builtin [`len(s)`](../builtins/len/len.md)
(ou `s.len()`).

---

## Um detalhe importante do PoolScript

Métodos que **perguntam** (os `is...`, `startswith`, `contains`, `match`)
devolvem `bool`. Mas cuidado ao usar em defaults: o `or` do PoolScript devolve
**bool**, não o valor. Isto **não** funciona pra "valor padrão":

```
nome = entrada or "sem nome"    // ERRADO: vira true/false
```

Use `if` explícito:

```
nome = "sem nome"
if entrada:
    nome = entrada
```

---

## Relacionados

- [`len()`](../builtins/len/len.md) — tamanho da string
- [regex (lib)](../regex/regex.md) — a lib completa por trás de `.match`/`.sub`
- [Parsing](../Parsing/Parsing.md) — texto sujo → número/bool de forma tolerante
