# Correção da base (17.0.19): a lista

Única lista de defeitos da base. A sonda (`notas/SONDA-METODO.md`) escreve; o
conserto risca o item (`~~`), cita o commit e o caso de teste. Nada novo entra
na Jinga até esta lista zerar e duas sondas seguidas não acharem nada
(`CLAUDE.md`).

Cada item: programa mínimo, o que a doc promete, o que saiu (com JIT e sem),
classificação (`motor`, `doc`, `os dois`).

## jinker

Medidos em 2026-10-16 no código do 17.0.19, app de prova com
`Jinker(__name__, static_folder="front")`, pasta `front/` com `index.html`,
`docs/index.html`, `static/sub/index.html`, `a.wasm` e um vídeo.

- **jinker-1** `GET /docs/` devolve o `index.html` da RAIZ do `static_folder`,
  não o de `docs/`; `GET /static/sub/` dá 404. Promessa: `static_folder.md` (a
  pasta é servida como um servidor de arquivos). `motor`.
- **jinker-2** `a.wasm` sai com `Content-Type: application/octet-stream`; o
  navegador recusa `WebAssembly.instantiateStreaming`. Promessa: tipo pela
  extensão (`static_folder.md`). `motor`.
- **jinker-3** Pedido com `Range:` recebe 200 com o corpo inteiro (vídeo não
  toca no Safari); nenhuma resposta traz `Last-Modified`/`ETag`, e
  `If-Modified-Since` nunca dá 304. `motor`.
- **jinker-4** `jinker.md` diz que `static_url` é ignorado; o motor e
  `static_url.md` usam como prefixo da URL. `doc` (o motor está certo).
