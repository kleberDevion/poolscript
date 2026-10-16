# A sonda da base: como roda

Decisão do Kleber (2026-10-16): a base (`main`, 17.0.19) se corrige até
acabar, e nada novo entra antes. Esta é a sonda que acha os defeitos. Ela roda
sempre do mesmo jeito; quem muda este arquivo é ele.

## O critério

O que a doc de cada módulo promete, o motor cumpre, para qualquer programa: no
`--check` e rodando, com o JIT ligado e desligado. Divergência entre doc e
motor é defeito; o item diz qual dos dois está errado.

## O alvo

Um módulo por vez, nesta lista (a pasta é a doc dele em `docs/`):

`linguagem` (os 14 capítulos, um por agente), `builtins`, `exceptions`, `str`,
`list`, `dict`, `tup`, `byte`, `bytes`, `objetos-internos`, `json`, `date`,
`regex`, `Parsing`, `os`, `sys`, `dotenv`, `jwt`, `hash`, `sqlite3`, `mail`,
`request`, `qrcode`, `manpu`, `psodbc`, `jinker`, `sockets`, `datasentity`,
`swagger`.

## Os cinco olhares

Por módulo, cinco agentes, cada um com uma lente só:

1. **Doc contra motor.** Cada frase que promete comportamento e cada exemplo
   da doc vira um programa rodado. Saída diferente da prometida é achado.
2. **Argumento errado.** Tipo errado, quantidade errada, `Null`, vazio, nome
   de parâmetro errado, em cada função e método. O que se confere é a frase do
   erro (aponta a causa?), o tipo da exceção, a linha e a coluna, e o `rc`.
3. **Borda.** Vazio, enorme, negativo, zero, limite do tipo, Unicode (acento,
   CJK, emoji de 4 bytes, NUL), caminho longo, entrada que termina no meio.
4. **`--check` contra rodando.** O que o `--check` aprova e quebra rodando, e o
   que ele recusa e rodaria. O mesmo programa com `jinga` e `jinga --sem-jit`:
   saída diferente entre os dois é achado.
5. **Uso de verdade.** Um programa de 30 a 80 linhas, como um usuário escreve,
   de ponta a ponta (ler, processar, gravar, servir), usando o módulo com os
   vizinhos dele. O que der errado no caminho é achado.

## Cada achado

- Programa mínimo em Jinga **tipada** (`int x = 1`, `str s = ""`, parâmetro e
  retorno tipados; `#` é comentário; `post` imprime; `__name__ == "main"`),
  que passa no `--check` quando o achado não é do `--check`.
- O que a doc promete (citação e página), o que saiu (saída e `rc`), com JIT
  e sem.
- Classificação: `motor` (o motor erra), `doc` (a doc erra), `os dois`.

## Confirmação

Outro agente, sem ver o raciocínio do primeiro, roda o programa do zero. Só
entra na lista o que reproduz. Achado que não reproduz é descartado com o
motivo.

## Até quando

O módulo repete os cinco olhares até duas rodadas seguidas sem achado novo.

## A saída

`notas/CORRECAO-BASE.md`, uma seção por módulo, itens numerados por módulo
(`str-1`, `str-2`...), cada item com o programa, a promessa, a saída e a
classificação. O conserto risca o item e cita o commit e o caso de teste.

## Regras para os agentes

- O binário é o da base: `/usr/local/bin/jinga` (conferir `jinga --version`
  = 17.0.19) ou o `./pool` compilado do `main` (`make pool`; o `./pool` da
  árvore pode ser de outro ramo). Sem JIT: `jinga --sem-jit arq.pr`.
- Nada no repositório é alterado pela sonda. Programas de prova ficam na pasta
  de rascunho da sessão.
- Programa que abre porta usa `127.0.0.1` e porta acima de 18000; nunca 8080.
- Programa que precisa de banco, servidor de e-mail ou rede externa: o achado
  diz o que faltou e para; não se instala nada no Mac.
- Teto de tempo por programa: 20 segundos (`perl -e 'alarm shift; exec @ARGV'
  20 jinga arq.pr`). Travar é achado.
- Português, sem emoji, no programa e no texto.
