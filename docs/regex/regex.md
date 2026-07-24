# regex — Expressões regulares

Lib pra buscar, validar e substituir **padrões** em texto usando expressões
regulares (as mesmas do Python). Útil pra validar e-mail/CPF, extrair pedaços
de um texto, limpar strings.

```
import regex
```

| Membro | O que faz | Página |
|---|---|---|
| `regex.match(padrão, texto)` | testa se o texto **começa** com o padrão | [match/match.md](match/match.md) |
| `regex.search(padrão, texto)` | acha o padrão **em qualquer lugar** | [search/search.md](search/search.md) |
| `regex.findall(padrão, texto)` | acha **todas** as ocorrências (lista) | [findall/findall.md](findall/findall.md) |
| `regex.sub(padrão, novo, texto)` | **substitui** o padrão por outro texto | [sub/sub.md](sub/sub.md) |
| `regex.split(padrão, texto)` | **divide** o texto onde o padrão bate | [split/split.md](split/split.md) |
| `regex.escape(texto)` | escapa caracteres especiais do texto | [escape/escape.md](escape/escape.md) |

---

## Exemplo rápido

```
import regex

// validar e-mail
if (regex.match("[^@]+@[^@]+\.[^@]+", "ana@email.com")) {
    post("e-mail válido")
}

// extrair todos os números de um texto
nums = regex.findall("[0-9]+", "tenho 3 gatos e 2 cães")
post(nums)          // ["3", "2"]

// limpar espaços múltiplos
limpo = regex.sub(" +", " ", "texto    com   espaços")
post(limpo)         // "texto com espaços"
```

---

## Dica: use string raw pros padrões

Padrões usam muita barra invertida (`\d`, `\w`, `\.`). Numa string normal, a
barra some (`"\d"` vira `"d"`). Use **string raw** (`r"..."`) pra preservar:

```
regex.findall(r"\d+", texto)        // certo — \d preservado
regex.findall("\d+", texto)         // errado — vira "d+"
```

---

## Relacionados

- Strings raw — ver a seção de strings em `LANGUAGE.md`
