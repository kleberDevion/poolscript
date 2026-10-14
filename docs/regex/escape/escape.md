# `regex.escape(pattern)`

Escapa os caracteres especiais do texto. Serve pra casar um texto qualquer,
que pode ter metacaracteres de expressão regular, como texto literal.

```
regex.escape(pattern: str) -> str
```

Só são escapados os caracteres que têm sentido especial num padrão:
`()[]{}?*+-|^$\.&~#`, o espaço e `\t \n \r \v \f` (estes por causa do modo
VERBOSE). Letras, dígitos, `_` e o resto da pontuação passam intactos, como
no `re` desde o Python 3.7.

```ps
import regex

post(regex.escape("https://www.python.org"))
str permitidos = "abcdefghijklmnopqrstuvwxyz0123456789!#$%&'*+-.^_`|~:"
post("[" + regex.escape(permitidos) + "]+")
list ops = ["/", "-", "+", "**", "*"]
list esc = []
for each o in ops {
    esc.append(regex.escape(o))
}
post("|".join(esc))
```

```saida
https://www\.python\.org
[abcdefghijklmnopqrstuvwxyz0123456789!\#\$%\&'\*\+\-\.\^_`\|\~:]+
/|\-|\+|\*\*|\*
```

---

## Quando usar

Sempre que o padrão vier de uma **variável** (entrada do usuário, nome de
arquivo...) e for pra casar **literalmente**:

```ps
import regex

str alvo = "3.14 (pi)"
str padrao = regex.escape(alvo)
post(padrao)
post(regex.search(padrao, "o valor de 3.14 (pi) é"), regex.search(alvo, "o valor de 3.14 (pi) é"))
```

```saida
3\.14\ \(pi\)
True False
```

Sem o `escape`, o `.` casaria qualquer caractere e os `()` seriam um grupo.

---

## Não serve pro texto de troca

Esta função não deve ser usada no texto de troca do [`sub`](../sub/sub.md):
lá, só a barra precisa ser escapada. Por exemplo:

```ps
import regex

str digitos = r"\d+"
str amostra = "/usr/sbin/sendmail - 0 errors, 12 warnings"
post(regex.sub(digitos, digitos.replace("\\", r"\\"), amostra))
```

```saida
/usr/sbin/sendmail - \d+ errors, \d+ warnings
```

---

## Relacionados

- [regex](../regex.md) — a sintaxe do padrão
- [`regex.search()`](../search/search.md) / [`regex.findall()`](../findall/findall.md) — onde usar o padrão escapado
