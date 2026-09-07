# `Route` — não existe

Esta página descrevia um tipo `Route`, com `.auth`, `.handler`, `.methods`,
`.middleware` e `.path`. **Nada disso existe na VM**: `type()` nunca devolve
`Route`, o nome não aparece em `pool --metadata`, e nenhum dos cinco atributos
é alcançável.

O que existe de verdade:

- **`app.route("/x", ...)`** devolve um registrador, e `type()` dele é
  `_RouteRegistrar`. Ele serve para o decorador prender a `funct` na rota —
  não é uma rota inspecionável.
- **`app.routes` não existe**: `'Jinker' object has no attribute 'routes'`. Não
  há como listar as rotas registradas a partir do objeto do app.
- O que a rota faz está documentado onde ela é declarada:
  [`route`](../jinker/route/route.md) e os atalhos por verbo
  ([`get`](../jinker/get/get.md), [`post`](../jinker/post/post.md), …).

A página fica aqui, e não some, porque outras páginas apontam para ela — e
porque saber que o tipo **não existe** vale mais do que a lista inventada que
estava escrita.

[← índice](objetos-internos.md)
