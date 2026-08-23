# `meter(typeinp=, placeholder=, value=, name=, href=, src=, alt=, target=, forid=, action=, methd=, rows=, cols=, onclick=)`

Elemento `meter` — campo de formulário. Hoje é uma caixa que mostra o `placeholder=` (ou o `.text()`); a edição pelo teclado dentro da janela nativa ainda não existe — o valor é o que o seu código põe com `.text(...)`.

## Como todo elemento do guzer

- Vem do `app` (o `guzer.UI`): `app.meter(...)`.
- `type()` devolve `"meter"` — igual nos dois motores.
- `.stylesheet({...})` e `.text(...)` encadeiam (retornam o próprio).
- Design default: caixa **200×28**, fundo `#F0F0F0`, texto `#101418`.
- O texto mostrado é o `.text(...)`; sem ele, o `placeholder=`.
- `onclick=` recebe uma reaction por referência — roda no clique.
- Os demais atributos do HTML (`href=`, `name=`, `value=`...) são aceitos na
  assinatura.

Chaves de estilo: `background` (ou `bg`), `color`, `width`, `height`,
`font-size` (esta só no interpretador; o backend X11 usa a fonte do sistema).

## Exemplo

```
import guzer
app = guzer.UI("Demo")
app.meter().stylesheet({ "width": "320" }).text("conteúdo")
```

[← índice](../guzer.md)
