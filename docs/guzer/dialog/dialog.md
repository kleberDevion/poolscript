# `dialog(typeinp=, placeholder=, value=, name=, href=, src=, alt=, target=, forid=, action=, methd=, rows=, cols=, onclick=)`

Elemento `dialog` — modal **centralizado** na janela (default 260×150, fundo branco). Os demais elementos empilham; o dialog fica no centro, por cima.

## Como todo elemento do guzer

- Vem do `app` (o `guzer.UI`): `app.dialog(...)`.
- `type()` devolve `"dialog"`.
- `.stylesheet({...})` e `.text(...)` encadeiam (retornam o próprio).
- Design default: caixa **200×28**, fundo `#F0F0F0`, texto `#101418`.
- O texto mostrado é o `.text(...)`; sem ele, o `placeholder=`.
- `onclick=` recebe uma reaction por referência — roda no clique.
- Os demais atributos do HTML (`href=`, `name=`, `value=`...) são aceitos na
  assinatura.

Chaves de estilo: `background` (ou `bg`), `color`, `width`, `height`,
`font-size` (ignorada: o backend X11 usa a fonte do sistema).

## Exemplo

```
import guzer
app = guzer.UI("Demo")
app.dialog().stylesheet({ "width": "320" }).text("conteúdo")
```

[← índice](../guzer.md)
