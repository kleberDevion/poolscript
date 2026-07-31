# Parsing — Conversões de tipo seguras

`Parsing` converte valores entre tipos (texto → número, etc.) de forma
**tolerante**: limpa caracteres estranhos, entende separadores, e não quebra
com entrada suja como um `int()` cru quebraria.

`Parsing` é um **builtin global** — não precisa de `import`, está sempre
disponível.

```
Parsing.integer("R$ 1.234")     // extrai o número
```

---

## Métodos

| Método | Converte pra | Página |
|---|---|---|
| `Parsing.string(v)` | texto (limpo) | [string/string.md](string/string.md) |
| `Parsing.integer(v)` | inteiro | [integer/integer.md](integer/integer.md) |
| `Parsing.floating(v)` | número decimal | [floating/floating.md](floating/floating.md) |
| `Parsing.boolean(v)` | booleano | [boolean/boolean.md](boolean/boolean.md) |
| `Parsing.JSONformatt(v)` | dict/lista (JSON) | [JSONformatt/JSONformatt.md](JSONformatt/JSONformatt.md) |
| `Parsing.Arrayformatt(v)` | lista | [Arrayformatt/Arrayformatt.md](Arrayformatt/Arrayformatt.md) |
| `Parsing.Tuplasformatt(v)` | tupla | [Tuplasformatt/Tuplasformatt.md](Tuplasformatt/Tuplasformatt.md) |
| `Parsing.TransientValue(v)` | conversão preservando o original | [TransientValue/TransientValue.md](TransientValue/TransientValue.md) |

---

## `Parsing` vs `int()`/`flo()` builtins

- **`int("abc")`** — quebra com erro se o texto não for um número limpo.
- **`Parsing.integer("R$ 1.234")`** — extrai o que dá (`1234`), tolerante.

Use os conversores builtin (`int`, `flo`) quando você **espera** um valor limpo
e quer que erro apareça se não for. Use `Parsing` quando a entrada é **suja/
incerta** (texto de usuário, planilha) e você quer o melhor esforço.

---

## Exemplo

```
// entrada suja de um formulário
preco = Parsing.floating("R$ 19,90")     // 19.90 (entende vírgula BR)
qtd   = Parsing.integer("12 unidades")   // 12
ativo = Parsing.boolean("sim")           // true
```
