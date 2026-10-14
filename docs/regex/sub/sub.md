# `regex.sub(pattern, repl, string, count=0)`

Devolve o texto que se obtém trocando as ocorrências mais à esquerda, sem
sobreposição, de *pattern* em *string* pelo texto de troca *repl*. Se o padrão
não é achado, *string* volta sem mudança.

```
regex.sub(pattern: str, repl: str, string: str, count=0) -> str
```

| Parâmetro | Padrão | O que é |
|---|---|---|
| `pattern` | — | o que procurar |
| `repl` | — | o texto de troca |
| `string` | — | onde trocar |
| `count` | `0` | quantas trocas, no máximo (`0` = todas) |

Os escapes com barra invertida em *repl* são processados: `\n` vira uma
quebra de linha, `\r` um retorno de carro, e assim por diante. Escapes
desconhecidos de letra ASCII são erro (`bad escape \q`). Outros escapes
desconhecidos, como `\&`, ficam como estão. Referências como `\6` são
trocadas pelo texto que o grupo 6 do padrão casou. Por exemplo:

```ps
import regex

post(regex.sub(r"def\s+([a-zA-Z_][a-zA-Z_0-9]*)\s*\(\s*\):",
               r"static PyObject*\npy_\1(void)\n{",
               "def myfunc():"))
post(regex.sub(r"(?i)\sAND\s", " & ", "Baked Beans And Spam"))
```

```saida
static PyObject*
py_myfunc(void)
{
Baked Beans & Spam
```

Na Jinga, *repl* é texto. No `re` do Python ele pode ser também uma função,
chamada a cada ocorrência com o objeto Match, que devolve o texto de troca; a
Jinga não tem objeto Match nem essa forma.

O argumento opcional *count* é o número máximo de ocorrências a trocar. Se
omitido ou zero, troca todas; negativo, não troca nenhuma (como no `re`).

Casamentos vazios vizinhos não existem, mas um casamento vazio pode vir logo
depois de um não vazio. Por isso, `sub("x*", "-", "abxd")` devolve
`"-a-b--d-"`, e não `"-a-b-d-"`.

```ps
import regex

post(regex.sub("x*", "-", "abxd"))
post(regex.sub("a", "X", "banana", count=1))
post(regex.sub(" +", " ", "texto    com   espaços"))
```

```saida
-a-b--d-
bXnana
texto com espaços
```

---

## O texto de troca

Além dos escapes de caractere e das referências acima, `\g<nome>` usa o texto
que o grupo de nome *nome* casou, definido pela sintaxe `(?P<nome>...)`.
`\g<número>` usa o grupo de número correspondente: `\g<2>` equivale a `\2`,
mas não é ambíguo numa troca como `\g<2>0`, porque `\20` seria lido como o
grupo 20, não como o grupo 2 seguido do caractere `'0'`. A referência `\g<0>`
põe o texto inteiro que a RE casou. Um grupo que não participou do casamento
vale `""`.

| No texto de troca | Vira |
|---|---|
| `\1` .. `\99` | o grupo daquele número |
| `\g<1>`, `\g<nome>` | o grupo daquele número ou nome |
| `\g<0>` | o texto inteiro que casou |
| `\n`, `\t`, `\r`, `\f`, `\v`, `\a`, `\b`, `\\` | o caractere |
| `\0`, `\012`, `\351` | o caractere de valor octal (`\0` é o caractere 0, não o casamento inteiro) |
| `\&` (barra e algo que não é letra ASCII) | fica como está |
| `\q` (barra e letra ASCII sem sentido) | erro `bad escape` |

```ps
import regex

post(regex.sub(r"(?P<p>\w+)", r"<\g<p>>", "ab cd"))
post(regex.sub(r"(\w)", r"\g<1>0", "ab"))
post(regex.sub(r"(\w)", r"\g<0>!", "ab"))
post(regex.sub(r"(a)(b)?", r"[\2]", "a"))
```

```saida
<ab> <cd>
a0b0
a!b!
[]
```

O texto de troca é conferido **antes** da busca, como no `re`: o erro nele
sai mesmo quando o padrão não casa nada. O nome de grupo que não existe é
`IndexError` (`unknown group name 'x'`); o resto do texto de troca inválido é
`ValueError`, com a posição contada em caractere do texto de troca:

```ps
import regex

try {
    regex.sub("x", r"\q", "abc")
} catch (ValueError e) {
    post(e)
}
```

```saida
bad escape \q at position 0 (linha 4)
```

Pra escrever no texto de troca um texto que pode ter barras, só a barra
precisa ser escapada: o [`escape`](../escape/escape.md) escapa demais pra isso
(ver lá).

---

## Relacionados

- [regex](../regex.md) — a sintaxe do padrão
- [`regex.findall()`](../findall/findall.md) — achar sem trocar
- [`regex.split()`](../split/split.md) — dividir em vez de trocar
