# Verificações: perguntas que devolvem `bool`

Estes métodos **perguntam** algo sobre a string e devolvem `true`/`false`. Não
mudam nada — servem pra `if`, validação e filtros.

---

## Conteúdo: `is...`

```
"12345".isdigit()      // true    (só dígitos 0-9)
"abc".isalpha()        // true    (só letras)
"abc123".isalnum()     // true    (letras e/ou dígitos)
"   ".isspace()        // true    (só espaços em branco)
"ABC".isupper()        // true    (tudo maiúsculo)
"abc".islower()        // true    (tudo minúsculo)
"½".isnumeric()        // true    (numérico, mais amplo que isdigit)
```

Tabela rápida:

| Método | `true` quando a string tem… |
|---|---|
| `isdigit()` | só dígitos `0-9` |
| `isnumeric()` | dígitos + frações/romanos unicode |
| `isalpha()` | só letras |
| `isalnum()` | letras e/ou dígitos (sem espaço/símbolo) |
| `isspace()` | só espaços em branco |
| `isupper()` / `islower()` | letras todas maiúsc./minúsc. |

> ⚠️ Todos dão `false` na **string vazia** `""`. E `isdigit()` é `false` pra
> negativos e decimais: `"-5".isdigit()` e `"3.14".isdigit()` são `false`.

---

## Começo e fim

```
"arquivo.txt".endswith(".txt")     // true
"https://x".startswith("https")    // true
```

Aceitam também uma **tupla** de opções (nativo do Python):

```
nome.endswith((".jpg", ".png", ".gif"))   // true se terminar em qualquer um
```

---

## Contém — `contains`

Apelido legível pra "tem este trecho dentro?":

```
"poolscript".contains("script")    // true
"poolscript".contains("java")      // false
```

---

## Uso comum: validar entrada

```
idade = input("idade: ").strip()
if idade.isdigit():
    post("ok:", int(idade))
else:
    post("digite só números")
```

---

## Cuidado com o `or` pra default

Como estes devolvem `bool`, e o `or` do PoolScript **também** devolve bool, não
use `or` pra valor padrão. Use `if`:

```
// ERRADO — vira true/false:
nome = entrada or "anônimo"

// certo:
nome = "anônimo"
if entrada:
    nome = entrada
```

---

## Relacionados

- [regex — `.match`](../regex/regex.md) — validar contra um padrão completo
- [caso](../caso/caso.md) — normalizar antes de comparar
- [Parsing](../../Parsing/Parsing.md) — converter texto incerto com tolerância
