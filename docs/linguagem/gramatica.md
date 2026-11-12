# Referência da Linguagem — A gramática

Esta página é a gramática inteira da Jinga: a parte léxica (os tokens) e a
sintática (como os tokens formam um programa). Ela foi tirada do parser
(`vm/ps_parser.c`) e do lexer (`vm/ps_lexer.c`), regra por regra, e cada regra
cita a função do parser que a implementa.

**Ela não é só leitura: o `make check` confere a página contra o parser a cada
commit** (o portão `gramatica`, `scripts/confere_gramatica.pr`). O portão lê
os blocos marcados ```gramatica desta página e reconhece com eles os tokens
que o `jinga --tokens` entrega:

- todo bloco ```ps da doc;
- todo programa da suíte;
- programas sorteados da própria gramática.

O que o parser aceita e a gramática recusa, ou o contrário, reprova. Toda
regra tem que ser usada por algum programa. Todo token que o motor entrega
tem que casar com a forma léxica escrita aqui pra classe dele. E os dois
modos do parser, o `--ast` e o `--check`, têm que dar o mesmo primeiro erro
de sintaxe.

A forma é a da doc do Python, que publica a gramática dela inteira
("Full Grammar specification", tirada da gramática que gera o parser do
CPython): EBNF com a escolha ordenada do PEG.

---

## A notação

| escrita | quer dizer |
|---|---|
| `regra: e` | define a regra; linhas recuadas que começam com `\|` são mais alternativas |
| `e1 e2` | `e1` e depois `e2` |
| `e1 \| e2` | **escolha ordenada**: tenta `e1`; só se `e1` falhar, tenta `e2` |
| `( e )` | agrupa |
| `[ e ]` ou `e?` | opcional |
| `e*` / `e+` | zero ou mais / um ou mais |
| `s.e+` | um ou mais `e` separados por `s` (é `e (s e)*`) |
| `&e` / `!e` | olha adiante sem consumir: tem que casar / não pode casar |
| `~` | o ponto em que o parser decidiu: se o resto falhar, nenhuma outra alternativa é tentada, nesta regra nem nas de fora, e o programa tem erro de sintaxe |
| `'texto'` | o token com esse texto: palavra reservada, nome, operador ou pontuação |
| `MAIÚSCULA` | uma classe de token (tabela abaixo) ou uma regra de contexto |
| `#` | comentário |

A repetição e o opcional são **gulosos e não voltam atrás**, como no parser: o
`e*` come o que conseguir, e o que vem depois não devolve nada pra ele. É
isso que faz a escolha ordenada ler o programa do mesmo jeito que o parser
descendente lê (`statement_no` tenta as formas de comando numa ordem fixa).

O `~` vai além do corte da gramática do Python, que só vale dentro da regra:
aqui ele vale pro programa inteiro, porque o parser da Jinga não volta atrás
depois de decidir. Visto o `import`, o comando é um import, e o `import`
sozinho é erro, e não o nome `import` usado como valor.

---

## O léxico

O lexer lê o fonte (UTF-8) e entrega a sequência de tokens que o
`jinga --tokens` mostra. As classes, com a forma de cada uma no fonte (expressão
regular na sintaxe do módulo `regex`; o token inteiro tem que casar):

```gramatica
# os nomes: o primeiro caractere decide a classe (só letra sem acento)
IDENT := [a-z_][A-Za-z0-9_]*
IDENT_UPPER := [A-Z][A-Za-z0-9_]*

# inteiro: decimal, ou 0x / 0o / 0b; o `_` separa dígitos em qualquer ponto
INT := 0[xX]_*[0-9a-fA-F][0-9a-fA-F_]*|0[oO]_*[0-7][0-7_]*|0[bB]_*[01][01_]*|[0-9][0-9_]*

# flo: a parte decimal precisa de dígito depois do ponto; o expoente, de dígito
FLO := [0-9][0-9_]*(\.[0-9][0-9_]*([eE][+-]?[0-9][0-9_]*)?|[eE][+-]?[0-9][0-9_]*)

# texto: aspas simples ou duplas, numa linha; '''tripla''' (sem prefixo, só a
# simples: """ sem prefixo é comentário); r"cru" não processa escape
STR := '''(\\[\s\S]|[^\\])*?'''|"(\\[^\n]|[^"\\\n])*"|'(\\[^\n]|[^'\\\n])*'|r('''[\s\S]*?'''|"""[\s\S]*?"""|"[^"\n]*"|'[^'\n]*')

# f-string: a f com aspas; as chaves {expressao} estão dentro do token
FSTRING := f('''(\\[\s\S]|[^\\])*?'''|"""(\\[\s\S]|[^\\])*?"""|"(\\[^\n]|[^"\\\n])*"|'(\\[^\n]|[^'\\\n])*')

# bytes: b ou B; br e rb são crus
BYTES := [bB]('''(\\[\s\S]|[^\\])*?'''|"""(\\[\s\S]|[^\\])*?"""|"(\\[^\n]|[^"\\\n])*"|'(\\[^\n]|[^'\\\n])*')|(br|rb)('''[\s\S]*?'''|"""[\s\S]*?"""|"[^"\n]*"|'[^'\n]*')

BOOL := True|False
NULL := Null

# a cor: 3 ou 6 dígitos hexadecimais, ou um dos nomes
COLOR := <([0-9A-Fa-f]{3}|[0-9A-Fa-f]{6}|red|green|blue|yellow|cyan|magenta|white|black|purple|orange|pink|gray|grey|lime|teal)>

# as 64 palavras reservadas (vm/ps_lexer.c, KEYWORDS)
KW := if|else|elif|while|for|each|in|is|and|or|not|Not|funct|return|continue|break|pass|model|enum|async|await|try|catch|as|of|using|import|from|PUSH|GET|str|int|long|flo|bool|post|input|addEnd|char|list|Class|class|type|Entity|self|private|public|match|case|yield|raise|finally|to|count|global|POST|PUT|DELETE|JSON|json|dict|tup|Object|object

# os operadores, do mais longo pro mais curto (a ordem decide `<=` contra `<`)
OP := ===|!==|==|!=|<=|>=|&&|\|\||<<|>>|\*\*|//|\+=|-=|\*=|/=|%=|\+\+|--|[-+*/%=<>!|^&~]
```

A pontuação é um token por caractere: `(` `)` `[` `]` `{` `}` `:` `;` `,` `.`
`@`. Fora disso o lexer entrega três tokens que não ocupam texto:

- **NEWLINE**, no fim de cada linha, **menos**:
  - dentro de `(` ou `[` ainda abertos;
  - dentro de um `{` de dicionário. O `{` é dicionário quando vem depois de
    operador, `(`, `[`, `{`, `,`, `:`, `return`, `yield` ou `case`, ou no
    começo do arquivo. Depois de `)`, de um nome, de um literal ou de outra
    palavra, ele abre bloco;
  - quando a linha de baixo começa com `.nome` (a cadeia que continua:
    `obj()` e embaixo `.json()`). Uma linha só de comentário ou em branco no
    meio quebra a cadeia.

  Linha vazia ou só com comentário não gera NEWLINE nenhum a mais.
- **INDENT** e **DEDENT**: o recuo só conta depois de uma linha que termina em
  `:` (o bloco da guarda `if __name__ == "main":`). A linha mais recuada abre
  (INDENT), e a primeira que volta pra antes dele fecha (DEDENT). Em todo o
  resto o recuo é estética. TAB conta como um espaço.
- **EOF**, no fim, depois dos DEDENT que faltarem.

**Comentário** é `#` até o fim da linha, ou `"""` até o próximo `"""` (pode
cruzar linhas). O comentário não vira token. `//` não é comentário: é a
divisão inteira.

O NUL, o byte que não é UTF-8, o caractere invisível fora de texto e o número
colado em letra (`0b102`, `1x`, `10.000.000`) são erro do lexer, com a frase
de cada um (capítulo 1).

---

## As regras que dependem do contexto

Algumas decisões do parser dependem de onde ele está, e não só dos tokens. A
gramática escreve cada uma como uma regra com nome em MAIÚSCULA:

- **`FIM`** — o fim de um comando. Falha quando o próximo token começa um
  comando e está na **mesma linha** do último token lido, sem `;` entre os
  dois. Começam comando: nome, palavra reservada, literal, cor, `@`, `(` e
  `[` (`exige_fim_de_comando`). Dentro de parênteses não há NEWLINE (veja
  acima), e é por esta regra que os comandos do corpo de uma lambda passada
  como argumento ficam separados.
- **`MODO_CABECA`** — a condição do `if`, do `elif` e do `while`, o iterável
  do `for each`, o contêiner do `count each`, o sujeito do `match` e a guarda
  do `case` são lidos neste modo. Nele, o `{` depois de um texto (`if x == "a" {`) abre o bloco, e não
  interpola o texto, até o primeiro parêntese, colchete de lista ou chave de
  dicionário (`chave_abre_bloco`).
- **`MODO_GRUPO`** — dentro de `( )`, de `[ ]` de lista, de `{ }` de
  dicionário e dos parênteses de uma chamada, o modo cabeça não vale
  (`grupo_depth`).
- **`INTERPOLA`** — verdadeiro fora do modo cabeça: ali o `{` depois de texto
  é a interpolação antiga (`"texto" {expr} "mais"`).
- **`MODO_ARGUMENTOS`**, **`MODO_ITENS`** e **`MODO_EXPRESSAO`** — os
  argumentos de uma chamada, os itens de lista, tupla e dicionário, e o
  resto.
- **`SUFIXO_SEPARADO`** — verdadeiro quando o próximo token é um `(` ou um `[`
  **separado por espaço** do valor de antes e se está entre argumentos ou
  itens. Ali ele não é chamada nem índice, e é erro (`post("lista:" [1])`):
  junte ao valor ou separe com vírgula (`acusa_sufixo_separado`).
- **`MODO_LIVRE`** — o corpo de bloco: nenhum dos modos de cima vale lá dentro
  (o corpo de uma lambda dentro de um argumento é comando, não item).
- **`CABECA_REPETIDA`** — verdadeiro quando a cabeça de uma funct repete um
  modificador (`async async`), tem dois tipos (`int str funct`) ou tem
  `public` e `private` juntos (`le_cabeca`).
- **`PARECE_DESEMPACOTAMENTO`** — o olhar adiante do desempacotamento
  (`parece_unpack`): alvos (nomes com `.membro` e `[índice]`, `*`, parênteses)
  separados por vírgula, até um `=` fora de parênteses, com pelo menos uma
  vírgula ou um `*`. Uma chamada (`f(`) no meio desfaz.
- **`SEM_NOME_REPETIDO`** — os parênteses que acabaram de fechar não passam o
  mesmo `nome=` duas vezes: na chamada (`f(a=1, a=2)`) e nos parâmetros do
  campo do `model` (`str(length=1, length=2)`).
- **`TEXTO_MAIN`** — o literal de texto `"main"`.
- **`TEXTO_DE_MODULO`** — o literal de texto do `import` entre aspas: não
  vazio e sem o byte nulo (`modulo_entre_aspas`).
- **`QUALQUER`** — qualquer token (a chave do padrão de dicionário do `case`).

---

## O arquivo e os blocos

```gramatica
arquivo: separador* (comando separador*)* EOF                    # ps_parse_com

separador: NEWLINE | ';' | DEDENT                                # pula_separadores
separador_largo: NEWLINE | ';' | INDENT | DEDENT                 # pula_indent_solto

# o bloco de chaves: a chave pode vir na linha de baixo (bloco_no)
bloco: MODO_LIVRE (NEWLINE | INDENT)* '{' separador_largo* (comando separador_largo*)* '}'

# o bloco da guarda aceita também `:` e o recuo (bloco_entrada)
bloco_da_guarda:
    | ':' NEWLINE+ INDENT (';' | NEWLINE)* (comando (';' | NEWLINE)*)* DEDENT
    | bloco
```

---

## Os comandos

```gramatica
comando: !palavra_de_clausula !forma_que_saiu corpo_de_comando FIM   # statement

# as palavras que só existem dentro da construção delas (recusa_clausula_solta)
palavra_de_clausula: 'else' | 'elif' | 'catch' | 'finally' | 'case' | 'each'

# as formas que saíram ou que nunca foram da linguagem, recusadas com frase
forma_que_saiu:
    | 'def' (IDENT | IDENT_UPPER) '('
    | ('action' | 'reaction') ('(' | (IDENT | IDENT_UPPER) '(')
    | KW op_de_atribuicao
    | 'run_selfwith_'

# statement_no: as formas na ordem em que o parser tenta
corpo_de_comando:
    | 'continue'
    | 'break'
    | 'pass'
    | 'raise' ~ (IDENT_UPPER ['(' [expressao] ')'] | expressao)
    | 'yield' ~ (&('}' | DEDENT | NEWLINE | EOF) | expressao)
    | 'global' ~ ','.nome+
    | classe
    | atribuicao_de_membro
    | importacao
    | decorado
    | modelo
    | enumeracao
    | contagem_com_bloco
    | escolha
    | tentativa
    | usando
    | guarda
    | se
    | enquanto
    | para_cada
    | retorno
    | funcao
    | com_visibilidade
    | campo_depois_do_tipo
    | declaracao_tipada
    | declaracao_de_objeto
    | desempacotamento
    | atribuicao
    | atribuicao_a_indice
    | expressao

op_de_atribuicao: '=' | '+=' | '-=' | '*=' | '/=' | '%='

# `obj.a.b = v` e as formas com operador
atribuicao_de_membro: (IDENT | IDENT_UPPER | 'self') ('.' nome_livre)+ op_de_atribuicao ~ expressao

# `x = v`; `base` não é nome de variável
atribuicao: !'base' (IDENT | IDENT_UPPER) op_de_atribuicao ~ expressao

# `alvo[i] = v`: o último sufixo é um índice (não fatia)
atribuicao_a_indice: primario (sufixo &sufixo)* !SUFIXO_SEPARADO '[' MODO_EXPRESSAO expressao ']' &op_de_atribuicao ~ op_de_atribuicao expressao

# o desempacotamento: `a, b = 1, 2`, `a, *resto = l`, `(a, b), c = x`
desempacotamento: PARECE_DESEMPACOTAMENTO ~ alvos '=' expressao [',' valores_seguintes]
valores_seguintes: &(NEWLINE | ';' | EOF | '}' | DEDENT) | expressao [',' valores_seguintes]
alvos:
    | alvo_simples (',' alvo_simples)* [',' '*' alvo_com_sufixos (',' alvo_simples)*] [',' &('=' | ')')]
    | '*' alvo_com_sufixos (',' alvo_simples)* [',' &('=' | ')')]
alvo_simples: '(' alvos ')' | alvo_com_sufixos
alvo_com_sufixos: nome ('.' nome_livre | '[' expressao ']')*      # alvo_com_sufixos

# as declarações com tipo
declaracao_tipada: tipo_declaravel &nome_de_declaracao ~ nome '=' expressao
declaracao_de_objeto: (IDENT | IDENT_UPPER) &(nome_de_declaracao '=') ~ nome '=' expressao
nome_de_declaracao: IDENT | IDENT_UPPER | !'funct' KW &'='       # eh_nome_de_declaracao

# `str private nome = v`: o modificador depois do tipo
campo_depois_do_tipo: (tipo_declaravel | IDENT | IDENT_UPPER) visibilidade &nome_de_declaracao ~ nome '=' expressao

# `private` / `public` na frente de uma declaração
com_visibilidade: visibilidade ~ !((IDENT | IDENT_UPPER) ':') (
    | &('model' | 'enum' | (IDENT | IDENT_UPPER) ('=' | ',') | PARECE_DESEMPACOTAMENTO) ~ declaracao_visivel FIM
    | (IDENT | IDENT_UPPER | KW) &(IDENT | IDENT_UPPER) ~ nome '=' expressao)
declaracao_visivel: modelo | enumeracao | desempacotamento | (IDENT | IDENT_UPPER) '=' expressao
visibilidade: 'private' | 'public'

# o controle de fluxo
se: 'if' ~ cabeca bloco (separador* 'elif' cabeca bloco)* [separador* 'else' bloco]
enquanto: 'while' ~ cabeca bloco
para_cada: 'for' ~ 'each' alvo_com_sufixos [',' alvos_do_laco] 'in' cabeca bloco
alvos_do_laco:
    | alvo_simples (',' alvo_simples)* [',' '*' alvo_com_sufixos (',' alvo_simples)*]
    | '*' alvo_com_sufixos (',' alvo_simples)*
retorno: 'return' ~ (&(NEWLINE | ';' | '}' | DEDENT | EOF) | expressao [',' expressao])

# a guarda do programa principal: só com o texto "main"
guarda: 'if' ('(' '__name__' '==' TEXTO_MAIN ')' | '__name__' '==' TEXTO_MAIN) (NEWLINE* &'{' | &':') ~ bloco_da_guarda

# `count each <tipo> in <container> { ... }`; sem o bloco é expressão. O
# contêiner é lido no modo cabeça: `count each str in "abc" {` abre o bloco
contagem_com_bloco: 'count' 'each' tipo_de_count ['(' [expressao] ')'] 'in' soma_de_cabeca &('{' | ':') ~ bloco

# try / catch / finally: pelo menos um catch ou o finally
tentativa: 'try' ~ bloco ((separador* clausula_catch)+ [separador* 'finally' bloco] | separador* 'finally' bloco)
clausula_catch: 'catch' '(' [IDENT_UPPER [nome] | nome] ')' bloco

usando: 'using' ~ expressao 'as' (IDENT | IDENT_UPPER | KW) bloco

# match: o corpo é de chaves. A forma com `:` saiu da linguagem (motor-20:
# o ramo dela saiu junto, e `match x:` dá a frase do `if True:`).
#
# `match` também não é comando SEMPRE: como no Python, a palavra só abre o
# comando quando a linha tem o `{` do bloco (motor-2). Sem ele, ela é a
# CHAMADA da função `match` do `regex`, e a regra que vale é a de expressão.
#
# Sem o `~` depois de `'match'`: o corte ali decidia antes de tentar a
# cabeça, e `match("a", "b")` parava no `match` como comando — a regra de
# expressão nunca era tentada. A escolha ordenada do PEG desfaz sozinha
# quando o `{` não vem.
escolha: 'match' cabeca separador* '{' separador_largo* (caso separador_largo*)* '}'
caso: 'case' padrao ['if' cabeca] separador_largo* bloco

# as expressões de cabeça
#
# O MODO_CABECA é o que faz o `{` que fecha a cabeça abrir o BLOCO em vez de
# interpolar o texto — é ele que separa `match x {` (comando) de `match(x)`
# (chamada).
cabeca: MODO_CABECA expressao
soma_de_cabeca: MODO_CABECA soma
```

---

## As declarações

```gramatica
# a funct: os modificadores e o tipo de retorno, em qualquer ordem, cada um
# uma vez (cabeca_de_funct, le_cabeca, funct_decl)
funcao: &cabeca_de_funcao ~ !CABECA_REPETIDA (modificador_de_cabeca | tipo_de_retorno)* 'funct' nome '(' parametros ')' NEWLINE* bloco
cabeca_de_funcao: (modificador_de_cabeca | tipo_de_retorno)* 'funct'
modificador_de_cabeca: 'async' | 'public' | 'private' | 'static' | 'nonnull' | 'NonNull'
tipo_de_retorno: !('static' | 'nonnull' | 'NonNull') (IDENT | IDENT_UPPER | NULL | tipo_declaravel_reservado)

# os parâmetros: comuns, depois `*args`, depois `**kwarg` (parse_parametros)
parametros: [lista_de_parametros]
lista_de_parametros:
    | parametro (',' parametro)* [',' '*' nome] [',' '**' nome] [',']
    | '*' nome [',' '**' nome] [',']
    | '**' nome [',']
parametro: [tipo_de_parametro &nome] (nome | 'self') !':' ['=' expressao]
tipo_de_parametro: !NULL tipo_de_retorno

# a classe (Entity, class e Class são a mesma)
classe: [visibilidade &palavra_de_classe] palavra_de_classe ~ nome ['(' (IDENT | IDENT_UPPER) (',' (IDENT | IDENT_UPPER))* [','] ')'] separador* '{' separador_largo* (membro separador_largo*)* '}'
palavra_de_classe: 'Entity' | 'class' | 'Class'

# o membro: decorador (que não captura a funct de baixo), método ou campo
membro:
    | &'@' ~ decorador_cabeca [&(':' | '{') bloco FIM]
    | &cabeca_de_funcao ~ funcao FIM
    | membro_campo FIM

# os modificadores do campo, antes e depois do tipo, cada um uma vez
membro_campo:
    | visibilidade 'static' !'(' corpo_de_campo_fixo
    | 'static' !'(' visibilidade corpo_de_campo_fixo
    | visibilidade corpo_de_campo_com_static
    | 'static' !'(' corpo_de_campo_com_visibilidade
    | corpo_de_campo_livre
corpo_de_campo_fixo: 'pass' | tipo_de_campo &nome_ou_visibilidade nome ['=' expressao] | campo_com_nome
corpo_de_campo_com_static: 'pass' | tipo_de_campo &nome_ou_visibilidade ['static' !'('] nome ['=' expressao] | campo_com_nome
corpo_de_campo_com_visibilidade: 'pass' | tipo_de_campo &nome_ou_visibilidade [visibilidade] nome ['=' expressao] | campo_com_nome
corpo_de_campo_livre: 'pass' | tipo_de_campo &nome_ou_visibilidade [visibilidade ['static' !'('] | 'static' !'(' [visibilidade]] nome ['=' expressao] | campo_com_nome
nome_ou_visibilidade: nome_de_declaracao | visibilidade
tipo_de_campo: !visibilidade !('static' !'(') (IDENT | IDENT_UPPER | KW)
campo_com_nome: (IDENT | IDENT_UPPER) (':' !NULL (IDENT | IDENT_UPPER | KW) !'|' ['=' expressao] | '=' expressao)

# model: um campo por linha, `nome: tipo(parametros)`
modelo: 'model' ~ nome '(' ')' separador* '{' separador_largo* (campo_de_modelo separador_largo*)* '}'
campo_de_modelo: nome ':' tipo_de_campo_de_modelo ['(' [','.parametro_de_campo+ [',']] ')' SEM_NOME_REPETIDO]
tipo_de_campo_de_modelo:
    | 'str' | 'int' | 'flo' | 'bool' | 'list' | 'dict'
    | 'string' | 'String' | 'integer' | 'Integer' | 'dictionary' | 'Dictionary' | 'json' | 'JSON'
    | !nome_da_tabela_de_tipos IDENT_UPPER
parametro_de_campo:
    | 'length' '=' INT
    | 'of' '=' (KW | IDENT | IDENT_UPPER)
    | ('regex' | 'in' | 'not_in' | 'min' | 'max' | 'optional') '=' expressao

# enum: `NOME` ou `NOME = valor`, vírgula opcional
enumeracao: 'enum' ~ nome separador* '{' separador_largo* (nome ['=' expressao] [','] separador_largo*)* '}'

# o decorador: em cima de uma funct, de uma classe, de outro decorador ou de
# um bloco
decorado: decorador_cabeca ~ (&(':' | '{') bloco | &'@' decorado | &(cabeca_de_funcao | [visibilidade] palavra_de_classe) comando)
decorador_cabeca: '@' (IDENT | IDENT_UPPER | KW) ('.' nome_livre)* ['(' separador_largo* [argumento_de_decorador (separador_largo* ',' separador_largo* argumento_de_decorador)* [separador_largo* ',']] separador_largo* ')'] separador*
argumento_de_decorador: ('**' | '*') expressao | nome_livre '=' expressao | expressao

# os imports
importacao:
    | 'import' ~ modulo ('as' nome_livre !'GET' !'*' | !'GET' ['*' !'as' !','])
    | 'from' ~ (TEXTO_DE_MODULO | '.'+ &'import' | '.'* caminho_de_modulo) !'GET' 'import' ('*' !'as' !',' | ','.nome_importado+)
    | 'PUSH' ~ modulo ('as' nome_livre !'import' ['GET' ','.nome_importado+] | !'import' ['GET' ('*' !'as' !',' | ','.nome_importado+)])
modulo: TEXTO_DE_MODULO | !'.' caminho_de_modulo
caminho_de_modulo: nome_livre ('.' nome_livre)*
nome_importado: !'*' nome_livre ['as' nome_livre]
```

---

## As expressões

A precedência, do mais fraco ao mais forte (o capítulo 3 tem a tabela com os
exemplos):

| nível | regra | operadores |
|---|---|---|
| 1 | `expressao` | `a if c else b` |
| 2 | `e_ou` | `or`, `\|\|` |
| 3 | `e_logico` | `and`, `&&` |
| 4 | `nao` | `not`, `Not`, `!` (prefixo) |
| 5 | `comparacao` | `==` `!=` `<` `>` `<=` `>=` `is` `is not` `in` `not in` `count`, sem encadear |
| 6 | `bit_ou` | `\|` |
| 7 | `bit_xou` | `^` |
| 8 | `bit_e` | `&` |
| 9 | `deslocamento` | `<<` `>>` |
| 10 | `soma` | `+` `-` |
| 11 | `produto` | `*` `/` `%` `//` |
| 12 | `unario` | `-` `+` `~` (prefixo) |
| 13 | `potencia` | `**`, à direita; o operando da direita pode ter unário |
| 14 | `posfixo` | chamada, `.membro`, índice e fatia, `++` `--` |
| 15 | `primario` | literais, nomes, grupos, lambda, `await` |

```gramatica
# o ternário: a condição é do nível `e_ou`; sem `else` não é ternário
expressao: e_ou ['if' e_ou 'else' expressao]                      # expressao_no
e_ou: e_logico (('or' | '||') e_logico)*
e_logico: nao (('and' | '&&') nao)*
nao: ('not' | 'Not' | '!') nao | comparacao

# a comparação; o `count` infixo (`int count in l`) e o sufixo
# (`int in l count`) só valem com um tipo do lado esquerdo
comparacao:
    | lado_de_count 'count' 'in' soma sufixo_de_comparacao*
    | lado_de_count 'in' bit_ou 'count' sufixo_de_comparacao*
    | bit_ou sufixo_de_comparacao*
sufixo_de_comparacao:
    | ('==' | '!=' | '<' | '>' | '<=' | '>=') bit_ou
    | 'is' ['not' | 'Not'] (lado_tipo_do_is | bit_ou)
    | 'in' bit_ou !'count'
    | ('not' | 'Not') 'is' (lado_tipo_do_is | bit_ou)
    | ('not' | 'Not') 'in' bit_ou
lado_tipo_do_is: nome_da_tabela_de_tipos !('(' | '.')
lado_de_count:
    | nome_da_tabela_de_tipos '(' MODO_GRUPO MODO_ARGUMENTOS separador_largo* [valor_de_count [separador_largo* ','] separador_largo*] ')' &('count' | 'in')
    | tipo_de_expressao !('(' | '.')
    | nome_da_tabela_de_tipos !'('
# o valor entre parênteses é um só, sem nome e sem `*` nem `**`
# (count_esquerda e count_tipo_valor, a mesma regra nas quatro formas)
valor_de_count: expressao compreensao | !(nome_livre '=') expressao

bit_ou: bit_xou ('|' bit_xou)*
bit_xou: bit_e ('^' bit_e)*
bit_e: deslocamento ('&' deslocamento)*
deslocamento: soma (('<<' | '>>') soma)*
soma: produto (('+' | '-') produto)*
produto: unario (('*' | '/' | '%' | '//') unario)*
unario: ('-' | '+' | '~') unario | potencia
potencia: posfixo ['**' unario]

# os sufixos: chamada, membro, índice e fatia, ++ e --
posfixo: primario sufixo* !SUFIXO_SEPARADO
sufixo:
    | !SUFIXO_SEPARADO '(' ~ MODO_GRUPO lista_de_argumentos ')' SEM_NOME_REPETIDO
    | '.' ~ nome_livre
    | !SUFIXO_SEPARADO '[' ~ indice ']'
    | '++'
    | '--'

# os argumentos: vírgula ou justaposição (`post("a" b)` são dois); o `{` nunca
# justapõe; a compreensão sem colchetes só como argumento único
lista_de_argumentos: MODO_ARGUMENTOS separador_largo* [argumentos] separador_largo*
argumentos: (expressao compreensao | argumento) (separador_largo* (',' separador_largo* | &inicio_justaposto) argumento)* [separador_largo* ',']
argumento: ('**' | '*') expressao | nome_livre '=' expressao | expressao
inicio_justaposto:
    | !'{' (INT | FLO | STR | FSTRING | BYTES | BOOL | NULL | COLOR | IDENT | IDENT_UPPER | '(' | '[' | palavra_que_abre_valor | '-' | '+' | '!')
    | tipo_de_expressao &'('
# A lista do `kw_abre_expr` (`vm/ps_parser.c`): a palavra reservada que ABRE
# uma expressão. É a pergunta que o parser faz em `pode_iniciar_expr` e em
# `pode_seguir` — "este token pode começar uma expressão?". A lista aí e a do
# parser são a MESMA — mudou numa, muda na outra.
palavra_que_abre_valor: 'post' | 'input' | 'create' | 'clear' | 'space' | 'addEnd' | 'char' | 'list'
    | 'json' | 'dict' | 'tup' | 'JSON' | 'self' | 'count' | 'to' | 'await'
    | 'upper' | 'lower' | 'replace' | 'split' | 'strip' | 'join' | 'startswith'
    | 'endswith' | 'find' | 'index' | 'format' | 'encode' | 'decode' | 'lstrip'
    | 'rstrip' | 'title' | 'capitalize' | 'not' | 'Not'
compreensao: 'for' 'each' nome 'in' expressao ['if' expressao]

# o índice e a fatia `[ini:fim:passo]`, qualquer parte podendo faltar
indice: MODO_EXPRESSAO (':' [expressao] [':' [expressao]] | expressao [':' [expressao] [':' [expressao]]])
```

### O primário

```gramatica
primario: !(('action' | 'reaction') '(') corpo_primario
corpo_primario:
    | &((modificador_de_cabeca | tipo_de_retorno)* 'funct' '(') ~ lambda
    | INT
    | FLO
    | BYTES
    | STR INTERPOLA '{' expressao '}' [STR] ('{' expressao '}' [STR])*
    | STR !(INTERPOLA '{')
    | FSTRING
    | BOOL
    | NULL
    | 'base' '(' ~ [(IDENT | IDENT_UPPER) &')'] ')' &'.'
    | !'base' (IDENT | IDENT_UPPER)
    | '(' ~ MODO_GRUPO (')' | MODO_EXPRESSAO expressao (',' MODO_ITENS [expressao (',' expressao)* [',']] ')' | ')'))
    | lista
    | dicionario
    | COLOR primario
    | 'to' ~ tipo_de_to
    | 'count' ~ ['each'] tipo_de_count ['(' [expressao] ')'] 'in' soma
    | 'await' ~ unario
    | tipo_de_expressao !('(' | '.')
    # KW: a palavra reservada que ABRE expressão (`post`, `count`, `to`,
    # `await`, `not`...) ou que NÃO abre. A que não abre (`while`, `if`, `for`)
    # é recusada pelo parser com a frase da palavra reservada (motor-21), mas
    # a recusa é dele, e não da gramática: aqui ela entra como KW e o
    # diagnosticador é quem fala. A regra `palavra_que_abre_valor` vale onde o
    # parser PERGUNTA se o token pode iniciar uma expressão
    # (`pode_iniciar_expr`), e é lá que ela é usada.
    | KW

# a lambda: `funct(params) { ... }`, com os mesmos modificadores da funct
lambda: !CABECA_REPETIDA (modificador_de_cabeca | tipo_de_retorno)* 'funct' '(' [lista_de_parametros_de_lambda] ')' bloco
lista_de_parametros_de_lambda:
    | parametro_de_lambda (',' parametro_de_lambda)* [',' '*' nome] [',' '**' nome] [',']
    | '*' nome [',' '**' nome] [',']
    | '**' nome [',']
parametro_de_lambda: [tipo_de_parametro &nome] nome !':' ['=' expressao]

# a lista, com a compreensão `[expr for each x in it if c]`
lista: '[' ~ MODO_GRUPO MODO_ITENS separador_largo* (
    | ']'
    | expressao compreensao separador_largo* ']'
    | expressao (separador_largo* ',' separador_largo* expressao)* [separador_largo* ','] separador_largo* ']')

# o dicionário: a chave que começa com texto é só o texto
dicionario: '{' ~ MODO_GRUPO MODO_ITENS separador_largo* [entrada (separador_largo* ',' separador_largo* entrada)* [separador_largo* ',']] separador_largo* '}'
entrada: (STR | expressao) ':' expressao

# os nomes
nome: !'base' (IDENT | IDENT_UPPER)                               # exige_nome
nome_livre: IDENT | IDENT_UPPER | KW                              # nome_livre

# os tipos, pela tabela única (vm/ps_tipos.def)
tipo_declaravel: 'str' | 'int' | 'flo' | 'bool' | 'list' | 'dict' | 'tup' | 'char' | 'Object' | 'long' | 'byte' | 'string' | 'String' | 'integer' | 'Integer' | 'tuple' | 'Tuple' | 'dictionary' | 'Dictionary' | 'json' | 'JSON' | 'object' | 'Long'
tipo_declaravel_reservado: 'str' | 'int' | 'flo' | 'bool' | 'list' | 'dict' | 'tup' | 'char' | 'Object' | 'long' | 'json' | 'JSON' | 'object'
tipo_de_expressao: 'str' | 'int' | 'flo' | 'bool' | 'list' | 'dict' | 'tup' | 'type' | 'long' | 'json' | 'JSON'
tipo_de_count: 'str' | 'int' | 'flo' | 'bool' | 'list' | 'dict' | 'tup' | 'type' | 'char' | 'Object' | 'long' | 'json' | 'JSON' | 'object'
tipo_de_to: 'int' | 'float' | 'str' | 'flo' | 'bool' | 'json' | 'list' | 'tup' | 'dict' | 'long' | 'char'
nome_da_tabela_de_tipos: 'str' | 'int' | 'flo' | 'bool' | 'list' | 'dict' | 'tup' | 'type' | 'char' | 'PoolFile' | 'Object' | 'long' | 'byte' | 'string' | 'String' | 'integer' | 'Integer' | 'tuple' | 'Tuple' | 'dictionary' | 'Dictionary' | 'json' | 'JSON' | 'object' | 'Long'
```

### Os padrões do `case`

```gramatica
padrao: padrao_simples ('|' padrao_simples)*                      # padrao, talvez_ou
padrao_simples:
    | '_'
    | '[' [padrao (',' padrao)* [',']] ']'
    | '{' [!'}' QUALQUER ':' padrao (',' !'}' QUALQUER ':' padrao)* [',']] '}'
    | STR
    | BYTES
    | INT
    | FLO
    | BOOL
    | NULL
    | IDENT
    | '-' (INT | FLO)
```

### A f-string

Cada `{...}` de uma f-string é lido como uma expressão inteira, sozinha
(`fstring_filhos`). O formato depois do primeiro `:` fora de parêntese, colchete,
chave e aspas (`{x:.2f}`, `{n:>5}`) é o do `format()` e não é expressão (capítulo
1, 1.6.3); dentro dele, `{w}` é outra expressão.

```gramatica
campo_de_fstring: MODO_LIVRE expressao EOF
```

O portão reconhece cada campo com os tokens que o `--tokens` marca
`"em":"fstring"`, até o `:` do formato, e também o campo dentro do formato.
Este último o parser não lê: quem lê é o compilador, e o `--ast` aceita
`f"{x:>{a b}}"` (motor-25 da lista de correção). A f-string que está dentro de
um campo de outra vem sem os tokens de dentro (motor-19), e os campos dela
ficam fora da conferência.

---

## O que a gramática aceita e o checador recusa

A gramática descreve o **parser**. Depois dele, o compilador confere mais coisa
(capítulo 10): o nome que não existe, o tipo, a aridade, o `break` fora de
laço, o `return` fora de funct. Três formas que o parser aceita e o compilador
recusa, com a frase que ainda não aponta a causa:

- a palavra reservada no lugar de um valor: `x = while` passa no parser como
  nome, e o checador diz `NameError: name 'while' is not defined` (motor-21
  da lista de correção);
- a palavra reservada como tipo de campo de classe: `class A { if x }` vira
  `tipo if não existe` (motor-21);
- o primeiro comando que termina num nome solto, com outro comando na mesma
  linha (`retrun x`): o parser marca, e o checador acusa o nome que não
  existe, os dois comandos na mesma linha ou a causa que ele acha (o `//`
  que não é comentário, em `int x = 1 // um comentario`). O portão conta essa
  marca como recusa do parser.
