# `img(typeinp=, placeholder=, value=, name=, href=, src=, alt=, target=, forid=, action=, methd=, rows=, cols=, onclick=)`

Elemento `img` — **desenha a imagem** (`.png`) do `src=` na janela nativa, (libpng + X11).

## Como todo elemento do guzer

- Vem do `app` (o `guzer.UI`): `app.img(...)`.
- `type()` devolve `"img"`.
- `.stylesheet({...})` e `.text(...)` encadeiam (retornam o próprio).
- Design default: caixa **200×28**, fundo `#F0F0F0`, texto `#101418`.
- O texto mostrado é o `.text(...)`; sem ele, o `placeholder=`.
- `onclick=` recebe uma reaction por referência — roda no clique.
- Os demais atributos do HTML (`href=`, `name=`, `value=`...) são aceitos na
  assinatura.

Chaves de estilo: `background` (ou `bg`), `color`, `width`, `height`,
`font-size` (ignorada: o backend X11 usa a fonte do sistema).

## O `src=` vem de onde você quiser

Caminho **absoluto**, **relativo à pasta do script em execução**, ou ao
**diretório atual** — a mesma regra de resolução da lib `os`.

Sem `width`/`height` no `.stylesheet`, a caixa vira o **tamanho natural** da
imagem. Com eles, a imagem é **escalada** pra caixa. Transparência (alpha) é
misturada com o fundo do elemento.

## Exemplo

```
import guzer
app = guzer.UI("Demo")
app.img().stylesheet({ "width": "320" }).text("conteúdo")
```

[← índice](../guzer.md)
