" Realce da PoolScript para Vim/Neovim.
"
" Espelho de editor/vscode/syntaxes/poolscript.tmLanguage.json — mesmas listas,
" e, o que mais importa, a MESMA SEPARAÇÃO: a palavra que declara função, a que
" declara tipo, o tipo primitivo, o modificador e o fluxo caem em grupos
" DIFERENTES.
"
" Era exatamente isso que faltava. A versão anterior ligava `psKeyword`,
" `psStorage`, `psStorageFunc`, `psStorageClass` e `psVerb` todos em `Keyword`:
" `if`, `int`, `Entity` e `funct` saíam da mesma cor, e a tela inteira ficava
" de um tom só.
"
" Grupos usados (todo tema define os dois lados; o Neovim já liga os `@…` do
" Tree-sitter nos clássicos, então tema que estiliza um ou outro pinta igual):
"
"   Comment  String  Number  Boolean  Constant  Keyword  Statement
"   Structure  Type  StorageClass  Function  Identifier  PreProc  Special

if exists("b:current_syntax")
  finish
endif

syn case match

" ── comentários ─────────────────────────────────────────────────────────────
" Antes das strings: o `"""` precisa ganhar do `"` simples, como na gramática
" do VS Code, onde #comments vem antes de #strings.
"
" `//` NÃO ENTRA AQUI. Nesta linguagem `//` é DIVISÃO INTEIRA, não comentário
" (I11) — pintar `x = a // b` de cinza escondia uma conta que roda.
syn region psCommentBlock start=/"""/ end=/"""/ keepend
syn match  psCommentHash  "#.*$"

" ── interpolação {..} dentro das strings ────────────────────────────────────
syn match  psEscape      "\\." contained
syn region psInterp      matchgroup=psInterpDelim start="{" end="}" contained
      \ contains=psString,psStringS,psStringT,psControl,psLogical,psType,
      \psModifier,psVerb,psBuiltin,psBoolean,psNull,psNumber,psCall,psSelf

" ── strings ─────────────────────────────────────────────────────────────────
syn region psStringT start=/[frbFRB]*'''/ end=/'''/ keepend contains=psInterp,psEscape
syn region psString  start=/[frbFRB]*"\%(""\)\@!/ end=/"/           contains=psInterp,psEscape
syn region psStringS start=/[frbFRB]*'/   end=/'/           contains=psInterp,psEscape

" ── decoradores ─────────────────────────────────────────────────────────────
syn match  psDecorator   "@[A-Za-z_]\w*\%(\.[A-Za-z_]\w*\)*" contains=psDecoratorAt
syn match  psDecoratorAt "@" contained

" ── fluxo ───────────────────────────────────────────────────────────────────
syn keyword psControl  if elif else while for each in is match case try catch
      \ finally raise return yield break continue with using await async
      \ import from as global count to of pass base
syn keyword psLogical  and or not Not
syn keyword psVerb     PUSH GET POST PUT DELETE JSON
syn keyword psSelf     self

" ── tipos ───────────────────────────────────────────────────────────────────
syn keyword psType     str int long flo bool list dict json tup bytes type char
      \ Object object
" Apelidos de tipo (`string s = "a"`): valem por POSIÇÃO, só onde um tipo vale.
" Fora dali são nome comum — `string = "a"` é variável, e não pode pintar.
syn match   psType     "\<\%(string\|String\|integer\|Integer\|tuple\|Tuple\|dictionary\|Dictionary\)\>\ze\s\+[A-Za-z_]"

" ── modificadores ───────────────────────────────────────────────────────────
syn keyword psModifier private public
" `static` e `nonnull` também valem por posição: só colados na cabeça de uma
" funct. `static = 1` continua sendo uma variável.
syn match   psModifier "\<\%(static\|nonnull\|NonNull\)\>\ze\%(\s\+\%(public\|private\|async\|static\|nonnull\|NonNull\|str\|int\|flo\|bool\)\)\{0,4}\s\+\%(funct\|action\|reaction\)\>"

" ── declarações: a palavra num grupo, o nome logo depois em outro ───────────
syn keyword psFunctKw  funct skipwhite nextgroup=psFuncName
syn keyword psClassKw  Entity class Class enum skipwhite nextgroup=psTypeName
" `model` só é declaração quando NÃO vem `=` depois. Em `@app.post("/x",
" model=Rota)` ele é NOME DE ARGUMENTO da rota, e pintar de palavra-chave
" mentia sobre o que a linha faz.
syn match   psClassKw  "\<model\>\%(\s*=\)\@!" skipwhite nextgroup=psTypeName
syn match   psFuncName "[A-Za-z_]\w*" contained
syn match   psTypeName "[A-Za-z_]\w*" contained

" ── builtins ────────────────────────────────────────────────────────────────
" Os que o `pool --metadata` publica e que NÃO são também nome de tipo (esses
" já estão em psType, e é como tipo que aparecem na maioria das linhas).
syn keyword psBuiltin  assert post len abs pow round hex bin oct ord chr range
      \ sum min max sorted reversed enumerate zip addEnd addStart removeEnd
      \ removeStart map filter open sleep gather input id load

" ── constantes ──────────────────────────────────────────────────────────────
syn keyword psBoolean true True false False
syn keyword psNull    Null null None none
syn match   psDunder  "\<__\w\+__\>"
syn match   psNumber  "\<\%(0[xX]\x\+\|0[oO]\o\+\|0[bB][01]\+\|\d\+\.\=\d*\%([eE][+-]\=\d\+\)\=\)\>"

" ── chamadas ────────────────────────────────────────────────────────────────
syn match psCall "\<[A-Za-z_]\w*\ze\s*("

" ── ligação com o tema ──────────────────────────────────────────────────────
" Uma por grupo: `hi def link` é o PRIMEIRO que vale, então repetir o mesmo
" grupo com outro alvo seria linha morta.
hi def link psCommentBlock Comment
hi def link psCommentHash  Comment
hi def link psString       String
hi def link psStringS      String
hi def link psStringT      String
hi def link psEscape       @string.escape
hi def link psInterpDelim  @punctuation.special
hi def link psInterp       Normal
hi def link psDecorator    PreProc
hi def link psDecoratorAt  PreProc
hi def link psControl      Statement
hi def link psLogical      @keyword.operator
hi def link psVerb         Keyword
hi def link psSelf         Identifier
hi def link psType         Type
hi def link psModifier     StorageClass
hi def link psFunctKw      Keyword
hi def link psClassKw      Structure
hi def link psFuncName     Function
hi def link psTypeName     Type
hi def link psBuiltin      @function.builtin
hi def link psBoolean      Boolean
hi def link psNull         Constant
hi def link psDunder       Constant
hi def link psNumber       Number
hi def link psCall         @function.call

" o bloco """ é multi-linha: sem isto o Vim sincroniza algumas linhas atrás
" e perde o meio do comentário
syn sync fromstart

let b:current_syntax = "poolscript"
