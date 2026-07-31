# `regex.sub(padrão, novo, texto, count=0, flags=0)`

**Substitui** as ocorrências do padrão por outro texto. Devolve o texto novo
(o original não muda).

```
regex.sub(padrão: str, novo: str, texto: str, count=0, flags=0) -> str
```

| Parâmetro | Padrão | O que é |
|---|---|---|
| `padrão` | — | o que procurar |
| `novo` | — | o texto que substitui |
| `texto` | — | onde substituir |
| `count` | `0` | quantas trocas fazer (`0` = todas) |

---

## Uso

```
import regex

// trocar espaços múltiplos por um só
limpo = regex.sub(" +", " ", "texto    com   espaços")
post(limpo)          // "texto com espaços"

// remover todos os dígitos (substitui por nada)
sem_num = regex.sub(r"\d", "", "abc123def")
post(sem_num)        // "abcdef"

// censurar
censurado = regex.sub("senha=\w+", "senha=***", "user=ana senha=1234")
post(censurado)      // "user=ana senha=***"
```

---

## Limitar quantas trocas

```
regex.sub("a", "X", "banana", count=1)     // "bXnana"  (só a primeira)
```

---

## Relacionados

- [`regex.findall()`](../findall/findall.md) — achar sem substituir
- [`regex.split()`](../split/split.md) — dividir em vez de trocar
