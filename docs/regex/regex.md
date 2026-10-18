# regex — Operações com expressões regulares

Este módulo faz casamento de expressões regulares do jeito do Perl, como o
módulo `re` do Python. O padrão e o texto são `str`, e o casamento anda por
caractere do Unicode, nunca por byte.

> Esta página e as das funções seguem a documentação do módulo `re` do Python
> ([docs.python.org/3/library/re.html](https://docs.python.org/3/library/re.html),
> © Python Software Foundation, licença PSF), traduzida e adaptada à Jinga.
> O que a Jinga faz diferente está dito no lugar e reunido em
> [Diferenças do `re`](#diferenças-do-re-do-python). Cada exemplo foi rodado.

```ps
import regex
```

| Membro | O que faz | Página |
|---|---|---|
| `regex.compile(padrão)` | compila **uma vez** e devolve um `Pattern` reusável | [compile/compile.md](compile/compile.md) |
| `regex.match(padrão, texto)` | o texto **inteiro** casa? (`bool`) | [match/match.md](match/match.md) |
| `regex.fullmatch(padrão, texto)` | o mesmo que `match`, com o nome explícito | [fullmatch/fullmatch.md](fullmatch/fullmatch.md) |
| `regex.search(padrão, texto)` | casa **em algum lugar** do texto? (`bool`) | [search/search.md](search/search.md) |
| `regex.findall(padrão, texto)` | **todos** os casamentos (lista) | [findall/findall.md](findall/findall.md) |
| `regex.sub(padrão, novo, texto, count=0)` | **substitui** os casamentos | [sub/sub.md](sub/sub.md) |
| `regex.split(padrão, texto, maxsplit=0)` | **divide** o texto onde o padrão casa | [split/split.md](split/split.md) |
| `regex.escape(texto)` | escapa os caracteres especiais do texto | [escape/escape.md](escape/escape.md) |

---

## A barra invertida e a string crua

As expressões regulares usam a barra invertida (`\`) pra indicar formas
especiais ou pra deixar um caractere especial valer sem o sentido especial
dele. Isso colide com o uso que a Jinga faz do mesmo caractere, com o mesmo
propósito, nos literais de texto ([1.6.3](../linguagem/01-estrutura-lexica.md#163-strings)).
Pra casar uma barra literal, por exemplo, o padrão tem que ser `\\`, e cada
barra tem que ser escrita `\\` num literal de texto comum: o texto do padrão
fica `"\\\\"`. Repare também que um escape que o literal de texto não conhece
(`"\d"`) mantém a barra e gera um `SyntaxWarning`, mesmo sendo um escape
válido do regex; e que um escape que o literal conhece muda de sentido antes
de o regex ver o padrão: `"\b"` é o caractere backspace, não a fronteira de
palavra.

A solução é escrever o padrão com a notação de string crua: num literal com o
prefixo `r`, a barra não tem tratamento especial. Assim `r"\n"` é um texto de
dois caracteres, `\` e `n`, e `"\n"` é um texto de um caractere, a quebra de
linha. Os padrões costumam ser escritos assim.

```ps
import regex

post(regex.findall(r"\bab\b", "ab cd"))     # string crua: a barra chega ao regex
post(regex.findall("\\bab\\b", "ab cd"))    # barra dobrada: o mesmo padrão
post(regex.findall("\bab\b", "ab cd"))      # "\b" virou o backspace
post(regex.findall(r"\\", r"C:\pasta"))     # o padrão \\ casa UMA barra
```

```saida
['ab']
['ab']
[]
['\\']
```

As operações existem como função do módulo e como método do padrão compilado
([`regex.compile`](compile/compile.md)). As funções são atalhos que dispensam
compilar antes.

---

## Sintaxe das expressões regulares

Uma expressão regular (ou RE) especifica um conjunto de textos que casam com
ela; as funções deste módulo dizem se um texto casa com uma expressão regular
(ou se uma expressão regular casa com um texto, o que dá no mesmo).

Expressões regulares podem ser concatenadas pra formar outras: se *A* e *B*
são expressões regulares, *AB* também é. Em geral, se um texto *p* casa com
*A* e outro texto *q* casa com *B*, o texto *pq* casa com AB. Isso vale a não
ser que *A* ou *B* tenham operações de baixa precedência, condições de
fronteira entre *A* e *B*, ou referências a grupo numerado. Assim, expressões
complexas se constroem a partir das primitivas descritas aqui. Pra teoria e
implementação de expressões regulares, consulte o livro do Friedl
(*Mastering Regular Expressions*) ou quase qualquer livro sobre construção de
compiladores. Pra uma apresentação mais suave, há o
[Regular Expression HOWTO](https://docs.python.org/3/howto/regex.html) do
Python: a sintaxe é a mesma.

Expressões regulares têm caracteres especiais e comuns. A maior parte dos
caracteres comuns, como `'A'`, `'a'` ou `'0'`, são as expressões regulares
mais simples: casam com eles mesmos. Caracteres comuns podem ser
concatenados, então `last` casa com o texto `'last'`. (No resto desta seção,
as REs aparecem `neste estilo`, em geral sem aspas, e os textos a casar
`'entre aspas simples'`.)

Alguns caracteres, como `'|'` ou `'('`, são especiais. Caracteres especiais
ou representam classes de caracteres comuns, ou mudam como as expressões
regulares em volta deles são lidas.

Os operadores de repetição, os quantificadores (`*`, `+`, `?`, `{m,n}` etc.),
não podem ser aninhados diretamente. Isso evita ambiguidade com o sufixo `?`
do modo preguiçoso e com os outros sufixos. Pra aplicar uma segunda repetição
a uma repetição de dentro, use parênteses: `(?:a{6})*` casa qualquer múltiplo
de seis caracteres `'a'`.

Os caracteres especiais são:

**`.`** (Ponto.) No modo normal, casa qualquer caractere, menos a quebra de
linha. Com a flag DOTALL, casa qualquer caractere, inclusive a quebra de
linha. `(?s:.)` casa qualquer caractere, com qualquer flag.

**`^`** (Circunflexo.) Casa no começo do texto e, no modo MULTILINE, também
logo depois de cada quebra de linha.

**`$`** Casa no fim do texto ou logo antes da quebra de linha que fecha o
texto, e no modo MULTILINE também antes de cada quebra de linha. `foo` casa
`'foo'` e `'foobar'`, enquanto a expressão regular `foo$` casa só `'foo'`.
Mais interessante: procurar `foo.$` em `'foo1\nfoo2\n'` acha `'foo2'`
normalmente, mas também `'foo1'` no modo MULTILINE; procurar um `$` sozinho em
`'foo\n'` acha dois casamentos (vazios): um logo antes da quebra de linha e um
no fim do texto.

```ps
import regex

post(regex.findall(r"foo.$", "foo1\nfoo2\n"))
post(regex.findall(r"(?m)foo.$", "foo1\nfoo2\n"))
post(regex.findall(r"$", "foo\n"))
```

```saida
['foo2']
['foo1', 'foo2']
['', '']
```

**`*`** Faz a RE resultante casar 0 ou mais repetições da RE anterior, tantas
quantas der. `ab*` casa `'a'`, `'ab'` ou `'a'` seguido de qualquer número de
`'b'`.

**`+`** Faz a RE resultante casar 1 ou mais repetições da RE anterior. `ab+`
casa `'a'` seguido de qualquer número não zero de `'b'`; não casa só `'a'`.

**`?`** Faz a RE resultante casar 0 ou 1 repetição da RE anterior. `ab?` casa
`'a'` ou `'ab'`.

**`*?`, `+?`, `??`** Os quantificadores `'*'`, `'+'` e `'?'` são *gulosos*:
casam o máximo de texto que dá. Às vezes isso não é o que se quer: a RE
`<.*>` contra `'<a> b <c>'` casa o texto inteiro, e não só `'<a>'`. Pôr `?`
depois do quantificador faz o casamento ser *preguiçoso*, ou *mínimo*: casa o
*mínimo* de caracteres que der. A RE `<.*?>` casa só `'<a>'`.

```ps
import regex

post(regex.findall(r"<.*>", "<a> b <c>"))
post(regex.findall(r"<.*?>", "<a> b <c>"))
```

```saida
['<a> b <c>']
['<a>', '<c>']
```

**`*+`, `++`, `?+`** Como os quantificadores `'*'`, `'+'` e `'?'`, os que têm
`'+'` no fim também casam o máximo de vezes que der. Mas, ao contrário dos
gulosos de verdade, não deixam voltar atrás quando a expressão seguinte não
casa. São os quantificadores *possessivos*. Por exemplo, `a*a` casa `'aaaa'`
porque o `a*` casa os 4 `'a'`, mas, quando chega o `'a'` final, a expressão
volta atrás, e no fim o `a*` fica com 3 `'a'` e o quarto é casado pelo `'a'`
final. Já `a*+a` contra `'aaaa'`: o `a*+` casa os 4 `'a'`, e, quando o `'a'`
final não acha mais caractere pra casar, a expressão não pode voltar atrás e
não casa. `x*+`, `x++` e `x?+` equivalem a `(?>x*)`, `(?>x+)` e `(?>x?)`.

```ps
import regex

post(regex.match(r"a*a", "aaaa"))     # o a* devolve um "a" pro a final
post(regex.match(r"a*+a", "aaaa"))    # o possessivo não devolve
```

```saida
True
False
```

**`{m}`** Especifica que exatamente *m* cópias da RE anterior têm que casar;
menos que isso faz a RE inteira não casar. `a{6}` casa exatamente seis
caracteres `'a'`, não cinco.

**`{m,n}`** Faz a RE resultante casar de *m* a *n* repetições da RE anterior,
tentando o máximo de repetições que der. `a{3,5}` casa de 3 a 5 caracteres
`'a'`. Sem o *m*, o limite de baixo é zero; sem o *n*, não há limite de cima.
`a{4,}b` casa `'aaaab'` ou mil caracteres `'a'` seguidos de um `'b'`, mas não
`'aaab'`. A vírgula não pode faltar, senão o quantificador se confundiria com
a forma anterior. Um `{` que não forma repetição (`{}`, `{x}`, `a{1`) é o
próprio caractere `{`.

**`{m,n}?`** Faz a RE resultante casar de *m* a *n* repetições da RE
anterior, tentando o *mínimo* de repetições que der. É a versão preguiçosa do
quantificador anterior. No texto de 6 caracteres `'aaaaaa'`, `a{3,5}` casa 5
caracteres `'a'`, e `a{3,5}?` casa só 3.

**`{m,n}+`** Faz a RE resultante casar de *m* a *n* repetições da RE anterior,
tentando o máximo de repetições que der *sem* guardar ponto de volta. É a
versão possessiva do quantificador acima. No texto de 6 caracteres
`'aaaaaa'`, `a{3,5}+aa` tenta casar 5 `'a'` e, precisando de mais 2, vai
precisar de mais caracteres do que há, e falha; já `a{3,5}aa` casa com o
`a{3,5}` pegando 5, depois 4 `'a'` voltando atrás, e aí os 2 `'a'` finais
casam com o `aa` do fim. `x{m,n}+` equivale a `(?>x{m,n})`.

```ps
import regex

post(regex.findall(r"a{3,5}", "aaaaaa"))
post(regex.findall(r"a{3,5}?", "aaaaaa"))
post(regex.match(r"a{4,}b", "aaaab"), regex.match(r"a{4,}b", "aaab"))
post(regex.findall(r"a{,2}", "aaa"))
```

```saida
['aaaaa']
['aaa', 'aaa']
True False
['aa', 'a', '']
```

**`\`** Ou escapa os caracteres especiais (deixando casar caracteres como
`'*'`, `'?'` e os outros), ou indica uma sequência especial; as sequências
especiais vêm abaixo.

Se o padrão não é escrito como string crua, lembre que a Jinga também usa a
barra invertida como escape nos literais de texto: se o literal não conhece o
escape, a barra e o caractere seguinte ficam no texto (com um aviso); mas, se
o literal conhece a sequência, a barra tem que ser escrita duas vezes. Isso é
complicado e difícil de acompanhar, então é muito recomendado usar string
crua em toda expressão que não seja das mais simples.

**`[]`** Indica um conjunto de caracteres. Num conjunto:

- Os caracteres podem vir um a um: `[amk]` casa `'a'`, `'m'` ou `'k'`.
- Uma faixa de caracteres se indica com dois caracteres separados por `'-'`:
  `[a-z]` casa qualquer letra ASCII minúscula, `[0-5][0-9]` casa os números de
  dois dígitos de `00` a `59`, e `[0-9A-Fa-f]` casa qualquer dígito
  hexadecimal. Se o `-` vier escapado (`[a\-z]`), ou for o primeiro ou o
  último caractere (`[-a]` ou `[a-]`), ele casa um `'-'` literal.
- Os caracteres especiais perdem o sentido especial dentro do conjunto, menos
  a barra invertida. `[(+*)]` casa qualquer um dos caracteres literais `'('`,
  `'+'`, `'*'` ou `')'`.
- A barra invertida ou escapa os caracteres com sentido especial no conjunto,
  como `'-'`, `']'`, `'^'` e `'\\'`, ou indica uma sequência especial que vale
  um caractere só, como `\xa0` ou `\n`, ou uma classe de caracteres, como `\w`
  ou `\S` (definidas abaixo). Repare que `\b` vale um caractere só, o
  backspace, e não a fronteira de palavra como fora do conjunto; e que escapes
  numéricos como `\1` são sempre escapes octais, não referência a grupo.
  Sequências especiais que não casam um caractere só, como `\A` e `\z`, não
  valem no conjunto (são o erro `bad escape`). Uma ponta de faixa também não
  pode ser classe: `[\w-a]` é o erro `bad character range \w-a`.
- Os caracteres que não estão numa faixa podem ser casados *complementando* o
  conjunto. Se o primeiro caractere do conjunto é `'^'`, casam todos os
  caracteres que *não* estão nele. `[^5]` casa qualquer caractere menos
  `'5'`, e `[^^]` casa qualquer caractere menos `'^'`. O `^` não tem sentido
  especial se não for o primeiro caractere do conjunto.
- Pra casar um `']'` literal dentro do conjunto, ponha uma barra antes dele ou
  ponha-o no começo do conjunto. `[()[\]{}]` e `[]()[{}]` casam o colchete
  de fechar, e também o de abrir, as chaves e os parênteses.

```ps
import regex

post(regex.findall(r"[0-5][0-9]", "07 59 60"))
post(regex.findall(r"[(+*)]", "(a+b)*c"))
post(regex.findall(r"[^5]", "1555"))
post(regex.findall(r"[]()[{}]", "a[b]{c}(d)"))
```

```saida
['07', '59']
['(', '+', ')', '*']
['1']
['[', ']', '{', '}', '(', ')']
```

**`|`** `A|B`, onde *A* e *B* podem ser REs quaisquer, cria uma expressão
regular que casa *A* ou *B*. Qualquer número de REs pode ser separado por
`'|'` assim, e isso vale também dentro de grupos (abaixo). Conforme o texto é
percorrido, as REs separadas por `'|'` são tentadas da esquerda pra direita.
Quando um ramo casa por completo, ele é aceito. Isso quer dizer que, uma vez
que *A* casa, *B* não é mais testado, mesmo que desse um casamento total mais
longo: o operador `'|'` nunca é guloso. Pra casar um `'|'` literal, use `\|`,
ou ponha-o numa classe de caracteres, como em `[|]`.

```ps
import regex

post(regex.findall(r"a|ab", "ab"))
```

```saida
['a']
```

**`(...)`** Casa a expressão regular que estiver dentro dos parênteses e
marca o começo e o fim de um grupo. O que o grupo casou sai no `findall`, no
`split` e no texto de troca do `sub`, e pode ser casado de novo mais adiante
no texto com a sequência especial `\número`, descrita abaixo. Pra casar os
literais `'('` ou `')'`, use `\(` ou `\)`, ou ponha-os numa classe de
caracteres: `[(]`, `[)]`.

**`(?...)`** É uma notação de extensão (um `'?'` depois de um `'('` não tem
sentido de outro jeito). O primeiro caractere depois do `'?'` define o sentido
e o resto da sintaxe da construção. As extensões em geral não criam grupo
novo; o grupo com nome é a única exceção. Seguem as extensões que existem.

**`(?aimsux)`** (Uma ou mais letras de `'a'`, `'i'`, `'m'`, `'s'`, `'u'`,
`'x'`.) O grupo casa o texto vazio; as letras ligam as flags correspondentes
pra expressão regular inteira: `a` (casamento só no ASCII), `i` (sem caixa),
`m` (multilinha), `s` (o ponto casa tudo), `u` (Unicode, o normal) e `x`
(verboso), descritas em [Flags](#flags). Na Jinga as flags só existem assim,
escritas no padrão. Elas têm que vir no começo da expressão: no meio, é o
erro `global flags not at the start of the expression`. A letra `L`, que no
`re` só vale em padrão de bytes, é o erro `bad inline flags: cannot use 'L'
flag with a str pattern`.

**`(?:...)`** A versão não capturante dos parênteses comuns. Casa a expressão
regular que estiver dentro deles, mas o que o grupo casou *não* sai no
resultado nem pode ser citado mais adiante no padrão.

**`(?aimsux-imsx:...)`** (Zero ou mais letras de `'a'`, `'i'`, `'m'`, `'s'`,
`'u'`, `'x'`, seguidas, se quiser, de `'-'` e de uma ou mais letras de `'i'`,
`'m'`, `'s'`, `'x'`.) As letras ligam ou desligam as flags correspondentes só
nesse pedaço da expressão. As letras `'a'` e `'u'` se excluem como flags em
linha: não podem vir juntas nem depois do `'-'`. Em vez disso, quando uma
delas aparece num grupo, ela troca o modo de casamento do grupo de fora:
`(?a:...)` passa pro casamento só no ASCII, e `(?u:...)` pro Unicode (o
normal). A troca vale só dentro do grupo, e o modo de antes volta fora dele.

```ps
import regex

post(regex.findall(r"(?i)abc", "ABC aBc"))
post(regex.findall(r"(?i:a)b", "Ab AB ab"))
post(regex.findall(r"(?s).", "a\nb"))
```

```saida
['ABC', 'aBc']
['Ab', 'ab']
['a', '\n', 'b']
```

**`(?>...)`** Tenta casar `...` como se fosse uma expressão regular separada
e, se casar, continua casando o resto do padrão depois dela. Se o resto do
padrão não casa, a volta atrás só pode ir até um ponto *antes* do `(?>...)`,
porque, depois de sair, a expressão, chamada *grupo atômico*, jogou fora todos
os pontos de volta de dentro dela. Assim, `(?>.*).` nunca casa nada: primeiro
o `.*` casa todos os caracteres que der, e depois, sem nada sobrando, o `.`
final falha. Como não há ponto de volta guardado no grupo atômico, nem antes
dele, a expressão inteira não casa.

```ps
import regex

post(regex.search(r"(?>.*).", "abc"))
```

```saida
False
```

**`(?P<nome>...)`** Parecido com os parênteses comuns, mas o grupo também
fica acessível pelo nome simbólico *nome*. O nome tem que ser um
identificador válido (letra ou `_` no começo, depois letras, dígitos ou `_`),
e cada nome só pode ser definido uma vez na expressão regular. Um grupo com
nome também é um grupo numerado, como se não tivesse nome. A Jinga aceita
também a forma `(?<nome>...)`, a do Java, do .NET e do JavaScript; o `re` só
tem `(?P<nome>...)`.

Um grupo com nome pode ser citado em dois lugares (o `re` tem um terceiro, o
objeto Match, que a Jinga não tem). Se o padrão é `(?P<aspa>['"]).*?(?P=aspa)`
(casa um texto entre aspas simples ou duplas):

| Onde se cita o grupo "aspa" | Jeitos de citar |
|---|---|
| no próprio padrão | `(?P=aspa)` (como acima), `\1` |
| no texto de troca do `regex.sub()` | `\g<aspa>`, `\g<1>`, `\1` |

**`(?P=nome)`** Uma referência de volta a um grupo com nome: casa o mesmo
texto que o grupo de nome *nome* casou antes.

```ps
import regex

post(regex.match(r"(.+) \1", "the the"), regex.match(r"(.+) \1", "thethe"))
post(regex.findall("(?P<aspa>['\"]).*?(?P=aspa)", "diz \"oi\" ou 'olá'"))
post(regex.sub(r"(?P<p>\w+)", r"<\g<p>>", "ab cd"))
```

```saida
True False
['"', "'"]
<ab> <cd>
```

**`(?#...)`** Um comentário: o conteúdo dos parênteses é ignorado.

**`(?=...)`** Casa se `...` casa em seguida, mas não consome nada do texto. É
a *asserção à frente* (lookahead). `Isaac (?=Asimov)` casa `'Isaac '` só se
vier seguido de `'Asimov'`.

**`(?!...)`** Casa se `...` *não* casa em seguida. É a *asserção à frente
negativa*. `Isaac (?!Asimov)` casa `'Isaac '` só se *não* vier seguido de
`'Asimov'`.

**`(?<=...)`** Casa se a posição atual do texto vem depois de um casamento de
`...` que termina na posição atual. É a *asserção atrás positiva*
(lookbehind). `(?<=abc)def` acha um casamento em `'abcdef'`, porque a asserção
volta 3 caracteres e confere se o padrão de dentro casa. O padrão de dentro
só pode casar textos de largura fixa: `abc` ou `a|b` valem, mas `a*` e
`a{3,4}` não (`look-behind requires fixed-width pattern`). Uma referência a
grupo de largura fixa vale (`(a)(?<=\1)`). Repare que um padrão que começa
com asserção atrás positiva não casa no começo do texto.

**`(?<!...)`** Casa se a posição atual do texto *não* vem depois de um
casamento de `...`. É a *asserção atrás negativa*. Como na positiva, o padrão
de dentro só pode casar textos de largura fixa. Um padrão que começa com
asserção atrás negativa pode casar no começo do texto.

```ps
import regex

post(regex.findall(r"Isaac (?=Asimov)", "Isaac Asimov, Isaac Newton"))
post(regex.findall(r"Isaac (?!Asimov)", "Isaac Asimov, Isaac Newton"))
post(regex.findall(r"(?<=-)\w+", "spam-egg"))      # a palavra depois do hífen
```

```saida
['Isaac ']
['Isaac ']
['egg']
```

**`(?(id/nome)sim|não)`** Tenta casar com `sim` se o grupo de *id* ou *nome*
dado casou, e com `não` se não casou. O `não` é opcional e pode ser omitido.
Por exemplo, `(<)?(\w+@\w+(?:\.\w+)+)(?(1)>|$)` é um padrão ruim de e-mail:
casa `'<user@host.com>'` e também `'user@host.com'`, mas não
`'<user@host.com'` nem `'user@host.com>'`.

```ps
import regex

str email = r"(<)?(\w+@\w+(?:\.\w+)+)(?(1)>|$)"
post(regex.match(email, "<user@host.com>"), regex.match(email, "user@host.com"))
post(regex.match(email, "<user@host.com"), regex.match(email, "user@host.com>"))
```

```saida
True True
False False
```

As sequências especiais são `'\'` e um caractere da lista abaixo. Se o
caractere não é um dígito ASCII nem uma letra ASCII, a RE resultante casa o
próprio caractere: `\$` casa `'$'`.

**`\número`** Casa o mesmo texto que o grupo de mesmo número casou. Os grupos
são numerados a partir de 1. `(.+) \1` casa `'the the'` ou `'55 55'`, mas não
`'thethe'` (repare no espaço depois do grupo). Esta sequência só cita os 99
primeiros grupos. Se o primeiro dígito do número é 0, ou se o número tem 3
dígitos octais, ele não é lido como grupo, mas como o caractere de valor
octal *número* (`\101` é `'A'`). Dentro do `'['` e do `']'` de uma classe de
caracteres, todo escape numérico é caractere.

**`\A`** Casa só no começo do texto.

**`\b`** Casa o texto vazio, mas só no começo ou no fim de uma palavra. Uma
palavra é uma sequência de caracteres de palavra. Formalmente, `\b` é a
fronteira entre um caractere `\w` e um `\W` (ou o contrário), ou entre `\w` e
o começo ou o fim do texto. Isso quer dizer que `r"\bat\b"` casa `'at'`,
`'at.'`, `'(at)'` e `'as at ay'`, mas não `'attempt'` nem `'atlas'`. Os
caracteres de palavra são as letras e os números do Unicode e o sublinhado; a
flag ASCII muda isso. Dentro de uma classe de caracteres, `\b` é o caractere
backspace, como nos literais de texto.

**`\B`** Casa o texto vazio, mas só quando *não* está no começo nem no fim de
uma palavra. Isso quer dizer que `r"at\B"` casa `'athens'`, `'atom'`,
`'attorney'`, mas não `'at'`, `'at.'` nem `'at!'`. `\B` é o oposto de `\b`:
os caracteres de palavra são as letras e os números do Unicode e o
sublinhado, e a flag ASCII muda isso. `\B` casa o texto vazio.

**`\d`** Casa qualquer dígito decimal do Unicode, isto é, qualquer caractere
da categoria Nd: `[0-9]` e muitos outros dígitos. Com a flag ASCII, casa
`[0-9]`.

**`\D`** Casa qualquer caractere que não é dígito decimal. É o oposto de
`\d`. Com a flag ASCII, casa `[^0-9]`.

**`\s`** Casa os caracteres de branco do Unicode (os que o `isspace()` diz que
são): `[ \t\n\r\f\v]` e muitos outros, como os espaços sem quebra que a
tipografia de muitas línguas exige. Com a flag ASCII, casa `[ \t\n\r\f\v]`.

**`\S`** Casa qualquer caractere que não é branco. É o oposto de `\s`. Com a
flag ASCII, casa `[^ \t\n\r\f\v]`.

**`\w`** Casa os caracteres de palavra do Unicode: todos os alfanuméricos do
Unicode (os que o `isalnum()` diz que são) e o sublinhado (`_`). Com a flag
ASCII, casa `[a-zA-Z0-9_]`.

**`\W`** Casa qualquer caractere que não é de palavra. É o oposto de `\w`: os
caracteres, fora o sublinhado, pra os quais o `isalnum()` dá `False`. Com a
flag ASCII, casa `[^a-zA-Z0-9_]`.

**`\z`** Casa só no fim do texto.

**`\Z`** O mesmo que `\z` (é o nome antigo, no `re`).

```ps
import regex

post(regex.findall(r"\bat\b", "at at. (at) as at ay attempt atlas"))
post(regex.findall(r"at\B", "athens atom attorney at at. at!"))
post(regex.findall(r"\d+", "12 ١٢ ४२"))
post(regex.findall(r"\w+", "olá ação ß ж € x_1"))
post(regex.findall(r"(?a)\w+", "olá ação"))
```

```saida
['at', 'at', 'at', 'at']
['at', 'at', 'at']
['12', '١٢', '४२']
['olá', 'ação', 'ß', 'ж', 'x_1']
['ol', 'a', 'o']
```

A maior parte dos escapes que os literais de texto da Jinga aceitam
([1.6.3](../linguagem/01-estrutura-lexica.md#163-strings)) também vale no
padrão:

```texto
\a      \b      \f      \n
\r      \t      \u      \U
\v      \x      \\
```

(Repare que `\b` é a fronteira de palavra, e só quer dizer "backspace" dentro
de classe de caracteres.) `\x` leva exatamente 2 dígitos hexadecimais, `\u`
leva 4 e `\U` leva 8; com menos, é o erro `incomplete escape`. Um escape
desconhecido formado por uma letra ASCII é erro (`bad escape \q`).

Os escapes octais entram de forma limitada. Se o primeiro dígito é 0, ou se
há três dígitos octais, é escape octal. Se não, é referência a grupo. Como nos
literais de texto, um escape octal tem no máximo três dígitos.

O `\N{nome}` do `re` (o caractere pelo nome do Unicode, como `\N{EM DASH}`)
não existe na Jinga, nem no padrão nem no literal de texto: é o erro `bad
escape \N`.

```ps
import regex

post(regex.findall(r"\x41", "ABA"), regex.findall(r"\U000000e9", "café"))
post(regex.findall(r"\101", "ABA"), regex.findall(r"[\b]", "a\bb"))
```

```saida
['A', 'A'] ['é']
['A', 'A'] ['\x08']
```

---

## Flags

No `re`, as flags vão no parâmetro `flags` ou no padrão. Na Jinga só existe a
forma escrita no padrão: `(?i)` no começo vale pro padrão inteiro, e
`(?i:...)` vale só dentro do grupo (ver a sintaxe acima).

| Letra | No `re` | O que faz |
|---|---|---|
| `a` | `re.ASCII` | `\w \W \b \B \d \D \s \S` e a caixa só no ASCII |
| `i` | `re.IGNORECASE` | casamento sem caixa |
| `m` | `re.MULTILINE` | `^` e `$` em cada linha |
| `s` | `re.DOTALL` | o `.` casa também a quebra de linha |
| `u` | `re.UNICODE` | o casamento Unicode, que já é o normal |
| `x` | `re.VERBOSE` | branco e `#comentário` no padrão não contam |

**`a` — ASCII.** Faz `\w`, `\W`, `\b`, `\B`, `\d`, `\D`, `\s` e `\S` casarem
só no ASCII, em vez do Unicode inteiro. Com a flag `i`, só as letras ASCII
trocam de caixa.

**`i` — IGNORECASE.** Casamento sem caixa: expressões como `[A-Z]` casam
também letras minúsculas. O casamento Unicode completo (como `'Ü'` casando
`'ü'`) também funciona, a não ser que a flag ASCII desligue o não-ASCII.
Quando os padrões `[a-z]` ou `[A-Z]` são usados com esta flag, eles casam as 52
letras ASCII e mais 4 letras fora do ASCII: `'İ'` (U+0130, i maiúsculo com
ponto), `'ı'` (U+0131, i minúsculo sem ponto), `'ſ'` (U+017F, s longo) e
`'K'` (U+212A, o sinal de Kelvin). Com a flag ASCII, só as letras de `'a'` a
`'z'` e de `'A'` a `'Z'`.

**`m` — MULTILINE.** O caractere `'^'` do padrão casa no começo do texto e no
começo de cada linha (logo depois de cada quebra de linha), e o `'$'` casa no
fim do texto e no fim de cada linha (logo antes de cada quebra de linha). Sem
ela, `'^'` casa só no começo do texto, e `'$'` só no fim do texto e logo antes
da quebra de linha (se houver) do fim do texto.

**`s` — DOTALL.** Faz o `'.'` casar qualquer caractere, inclusive a quebra de
linha; sem ela, o `'.'` casa tudo *menos* a quebra de linha.

**`u` — UNICODE.** O casamento já é pelo Unicode: a flag é redundante e não
muda nada. Pra ficar no ASCII, use a flag `a`.

**`x` — VERBOSE.** Deixa escrever expressões regulares mais legíveis,
separando visualmente as partes do padrão e pondo comentários. O branco dentro
do padrão é ignorado, menos dentro de uma classe de caracteres, ou depois de
uma barra invertida não escapada, ou dentro de símbolos como `*?`, `(?:` ou
`(?P<...>`: `(? :` e `* ?` não valem. Quando uma linha tem um `#` que não está
numa classe de caracteres nem vem depois de uma barra invertida não escapada,
tudo do `#` mais à esquerda até o fim da linha é ignorado. Assim, os dois
padrões abaixo, que casam um número decimal, são o mesmo:

```ps
import regex

str decimal = r'''(?x)\d +  # a parte inteira
                  \.    # o ponto decimal
                  \d *  # os dígitos da fração'''
post(regex.match(decimal, "3.14"), regex.match(r"\d+\.\d*", "3.14"))
```

```saida
True True
```

---

## O padrão compilado

`regex.compile(padrão)` devolve um objeto `Pattern`, com os métodos
`match`, `fullmatch`, `search`, `findall`, `sub` e `split` (os mesmos das
funções, sem o padrão) e o campo `.pattern`. Detalhes em
[compile](compile/compile.md).

---

## Erros

Quando o texto passado a uma destas funções não é uma expressão regular válida
(por exemplo, tem parênteses sem par), sai `ValueError`, com a frase do `re` e
a posição (contada em caractere do padrão) onde a leitura falhou. No `re`, a
classe é `re.PatternError` (antes `re.error`). Não achar nada nunca é erro.

```ps
import regex

try {
    regex.compile("[a-")
} catch (ValueError e) {
    post(e)
}
```

```saida
unterminated character set at position 0 (linha 4)
```

No texto de troca do `sub`, o nome de grupo que não existe é `IndexError`
(`unknown group name 'x'`), como no `re`; o resto do texto de troca inválido é
`ValueError`.

O casador da Jinga volta atrás e é recursivo, e tem dois freios, cada um com a
própria frase, sempre como erro e nunca derrubando o processo:

- **passos**: `RuntimeError: regex: backtracking demais`, quando o padrão volta
  atrás demais sobre o texto (2 milhões de passos);
- **profundidade**: `RuntimeError: regex: padrao ou texto profundo demais pra
  pilha`, quando as repetições de grupo descem a pilha além da folga medida
  dela onde a busca roda. Dentro de uma `async funct` a pilha é a da tarefa,
  menor que a do programa principal, e o limite chega antes. Um literal sem
  repetição (o `escape` de um texto longo) não desce a pilha.

O padrão também tem teto: grupos aninhados além da pilha dão `ValueError:
regex: padrao aninhado demais`, e um número de repetição acima de
4294967294 dá `the repetition number is too large`.

---

## Diferenças do `re` do Python

- `regex.match(padrão, texto)` casa o texto **inteiro**: é o `fullmatch` do
  `re`. O `match` do `re` (o `prefixmatch` do Python 3.15) casa só no começo;
  na Jinga, "começa com" é o `search` com `^` no padrão.
- `match`, `fullmatch` e `search` devolvem `bool`. Não há objeto Match
  (`group()`, `start()`, `span()`): o que casou sai pelo `findall`, e a troca,
  pelo `sub`.
- Não há `finditer`, `subn`, `purge`, nem o parâmetro `flags` e as constantes
  (`re.I`, `re.M`...): as flags vão no padrão.
- O texto de troca do `sub` é `str`; o `re` aceita também uma função.
- O `Pattern` não tem `pos` e `endpos` nos métodos, nem `.flags`, `.groups` e
  `.groupindex`.
- `\N{nome}` não existe; `(?<nome>...)` existe a mais.
- O erro de padrão é `ValueError` (no `re`, `re.PatternError`), com a mesma
  frase e a mesma posição. Os limites de passos e de pilha (`RuntimeError`)
  são da Jinga.

---

## Exemplos

### Conferindo um par

Num programa de pôquer, a mão de um jogador é um texto de 5 caracteres, um
por carta: `"a"` pro ás, `"k"` pro rei, `"q"` pra dama, `"j"` pro valete,
`"t"` pro 10, e de `"2"` a `"9"` pras cartas desses valores. Pra ver se um
texto é uma mão válida:

```ps
import regex

str mao = r"[a2-9tjqk]{5}"
post(regex.match(mao, "akt5q"), regex.match(mao, "akt5e"))   # válida, inválida
post(regex.match(mao, "akt"), regex.match(mao, "727ak"))      # inválida, válida
```

```saida
True False
False True
```

A última mão, `"727ak"`, tem um par, duas cartas do mesmo valor. Pra achar o
par, o retrovisor; com um grupo só, o `findall` devolve a carta dele:

```ps
import regex

str par = r"^.*(.).*\1"
post(regex.findall(par, "717ak"), regex.findall(par, "718ak"), regex.findall(par, "354aa"))
```

```saida
['7'] [] ['a']
```

### `search` x `match`

A Jinga tem duas operações básicas com expressão regular:

- `regex.search()` procura um casamento em qualquer lugar do texto (é o que o
  Perl faz por padrão);
- `regex.match()` (o mesmo que `regex.fullmatch()`) confere se o texto
  inteiro casa.

```ps
import regex

post(regex.match("c", "abcdef"), regex.search("c", "abcdef"))
post(regex.match("p.*n", "python"), regex.match("r.*n", "python"))
```

```saida
False True
True False
```

Um padrão que começa com `'^'` prende o `search` no começo do texto; no modo
MULTILINE, no começo de cada linha:

```ps
import regex

post(regex.search("^c", "abcdef"), regex.search("^a", "abcdef"))
post(regex.search("^X", "A\nB\nX"), regex.search("(?m)^X", "A\nB\nX"))
```

```saida
False True
False True
```

### Fazendo uma agenda

O `split()` divide um texto numa lista, separando onde o padrão casa. É o
jeito de transformar dados em texto em estruturas que o programa lê e muda,
como na agenda abaixo. Primeiro, a entrada (normalmente viria de um arquivo;
aqui é um texto de várias linhas). As entradas são separadas por uma ou mais
quebras de linha, e cada linha não vazia vira um item:

```ps
import regex

str texto = '''Ross McFluff: 834.345.1254 155 Elm Street

Ronald Heathmore: 892.345.3428 436 Finley Avenue
Frank Burger: 925.541.7625 662 South Dogwood Way


Heather Albrecht: 548.326.4584 919 Park Place'''
list entradas = regex.split("\n+", texto)
for each e in entradas {
    post(regex.split(":? ", e, 3))
}
```

```saida
['Ross', 'McFluff', '834.345.1254', '155 Elm Street']
['Ronald', 'Heathmore', '892.345.3428', '436 Finley Avenue']
['Frank', 'Burger', '925.541.7625', '662 South Dogwood Way']
['Heather', 'Albrecht', '548.326.4584', '919 Park Place']
```

Cada entrada vira nome, sobrenome, telefone e endereço. O `maxsplit` (o 3)
existe porque o endereço tem espaços, que é o padrão de separação. O `:?`
casa os dois-pontos depois do sobrenome, pra eles não aparecerem no
resultado. Com `maxsplit` 4, o número da casa se separa do nome da rua.

### Achando todos os advérbios

O `findall()` acha *todos* os casamentos do padrão, não só o primeiro, como o
`search()`. Pra achar os advérbios (em inglês, os que terminam em "ly") de um
texto:

```ps
import regex

str frase = "He was carefully disguised but captured quickly by police."
post(regex.findall(r"\w+ly\b", frase))
```

```saida
['carefully', 'quickly']
```

### Notação de string crua

A string crua (`r"texto"`) mantém as expressões regulares legíveis. Sem ela,
cada barra invertida (`'\'`) do padrão teria que ganhar outra na frente. As
duas linhas abaixo são o mesmo padrão:

```ps
import regex

post(regex.search(r"\W(.)\1\W", " ff "), regex.search("\\W(.)\\1\\W", " ff "))
```

```saida
True True
```

Pra casar uma barra literal, ela tem que ser escapada no padrão. Com string
crua, isso é `r"\\"`; sem ela, `"\\\\"`.

### Simulando o `scanf()`

A Jinga não tem um `scanf()`. As expressões regulares são mais poderosas,
embora mais longas, que os formatos do `scanf()`. A tabela dá o equivalente
mais ou menos exato de cada símbolo do `scanf()`:

| Símbolo do `scanf()` | Expressão regular |
|---|---|
| `%c` | `.` |
| `%5c` | `.{5}` |
| `%d` | `[-+]?\d+` |
| `%e`, `%E`, `%f`, `%g` | `[-+]?(\d+(\.\d*)?\|\.\d+)([eE][-+]?\d+)?` |
| `%i` | `[-+]?(0[xX][\dA-Fa-f]+\|0[0-7]*\|\d+)` |
| `%o` | `[-+]?[0-7]+` |
| `%s` | `\S+` |
| `%u` | `\d+` |
| `%x`, `%X` | `[-+]?(0[xX])?[\dA-Fa-f]+` |

Pra tirar o nome do arquivo e os números de um texto como
`/usr/sbin/sendmail - 0 errors, 4 warnings`, que o `scanf()` leria com
`%s - %d errors, %d warnings`, a expressão regular equivalente é:

```ps
import regex

post(regex.findall(r"(\S+) - (\d+) errors, (\d+) warnings", "/usr/sbin/sendmail - 0 errors, 4 warnings"))
```

```saida
[('/usr/sbin/sendmail', '0', '4')]
```

---

## Relacionados

- [Strings, escapes e string crua](../linguagem/01-estrutura-lexica.md#163-strings) — a seção 1.6.3 da linguagem
- [Regular Expression HOWTO](https://docs.python.org/3/howto/regex.html) — o tutorial do Python; a sintaxe é a mesma
