# `ul(typeinp=, placeholder=, value=, name=, href=, src=, alt=, target=, forid=, action=, methd=, rows=, cols=, onclick=)`

Elemento `ul` — Lista **não ordenada** — contêiner de `li`. Os itens empilham dentro dela; não há marcador (•) automático, escreva-o no `.text()` se quiser.

É **contêiner**: `x = app.ul()` e depois `x.p()`, `x.entry()`, `x.button()`...
criam filhos **dentro** dele (borda de 8px, empilhados). Sem `height` explícito, a
caixa cresce pra caber os filhos.

## O que o guzer faz com ele hoje

- Renderiza uma **caixa** (default 200×28, fundo `#F0F0F0`, texto `#101418`) empilhada
  na janela — ou dentro do pai, se foi criado por um contêiner.
- O texto mostrado é o `.text(...)`; sem ele, o `placeholder=`.
- `type()` devolve `"ul"` nos dois motores (tkinter no interpretador, X11 no `pool`).
- `.stylesheet({...})` e `.text(...)` encadeiam (devolvem o próprio elemento).

## Atributos

| argumento | efeito hoje |
|---|---|
| `placeholder=` | texto mostrado quando não há `.text(...)` |
| `value=` | o que `.value` devolve (sem ele: o `.text()`; sem os dois: o `placeholder=`) |
| `name=` | identifica no registro: `app.POOLHTMLElements.getitemByIdentify("nome").value` |
| `onclick=` | reaction (por referência) que roda no clique |
| `href=`, `src=`, `alt=`, `target=`, `forid=`, `action=`, `methd=`, `rows=`, `cols=`, `typeinp=` | aceitos e guardados no elemento; **sem efeito visual** em `ul` |

Chaves de estilo: `background` (ou `bg`), `color`, `width`, `height` e `font-size`
(esta só no interpretador; o backend X11 usa a fonte do sistema).

## Exemplo

```
import guzer
import scripts

app = guzer.UI("Demo")
u = app.ul()
u.li().text("• leite")
u.li().text("• pão")
```

[← índice](../guzer.md)
