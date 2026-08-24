# `div(typeinp=, placeholder=, value=, name=, href=, src=, alt=, target=, forid=, action=, methd=, rows=, cols=, onclick=)`

Elemento `div` — Contêiner genérico — a caixa que agrupa outros elementos sem significado próprio. É o bloco de construção do layout: você cria filhos **dentro** dela (`d.p()`, `d.entry()`, `d.button()`) e, sem `height`, ela cresce pra caber todos.

É **contêiner**: `x = app.div()` e depois `x.p()`, `x.entry()`, `x.button()`...
criam filhos **dentro** dele (borda de 8px, empilhados). Sem `height` explícito, a
caixa cresce pra caber os filhos.

## O que o guzer faz com ele hoje

- Renderiza uma **caixa** (default 200×28, fundo `#F0F0F0`, texto `#101418`) empilhada
  na janela — ou dentro do pai, se foi criado por um contêiner.
- O texto mostrado é o `.text(...)`; sem ele, o `placeholder=`.
- `type()` devolve `"div"` nos dois motores (tkinter no interpretador, X11 no `pool`).
- `.stylesheet({...})` e `.text(...)` encadeiam (devolvem o próprio elemento).

## Atributos

| argumento | efeito hoje |
|---|---|
| `placeholder=` | texto mostrado quando não há `.text(...)` |
| `value=` | o que `.value` devolve (sem ele: o `.text()`; sem os dois: o `placeholder=`) |
| `name=` | identifica no registro: `app.POOLHTMLElements.getitemByIdentify("nome").value` |
| `onclick=` | reaction (por referência) que roda no clique |
| `href=`, `src=`, `alt=`, `target=`, `forid=`, `action=`, `methd=`, `rows=`, `cols=`, `typeinp=` | aceitos e guardados no elemento; **sem efeito visual** em `div` |

Chaves de estilo: `background` (ou `bg`), `color`, `width`, `height` e `font-size`
(esta só no interpretador; o backend X11 usa a fonte do sistema).

## Exemplo

```
import guzer
import scripts

app = guzer.UI("Demo")
d = app.div().stylesheet({ "width": "320", "background": "#e8eef5" })
d.h2().text("Bloco")
d.p().text("dentro da div")
d.button(onclick=scripts.Ok).text("OK")
```

[← índice](../guzer.md)
