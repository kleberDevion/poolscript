# `form(typeinp=, placeholder=, value=, name=, href=, src=, alt=, target=, forid=, action=, methd=, rows=, cols=, onclick=)`

Elemento `form` — Formulário — agrupa os campos (`entry`, `textarea`, `select`) e o `button` de envio. **Não há envio HTTP automático**: `action=`/`methd=` ficam guardados; o "envio" é o `onclick=` do botão lendo os campos pelo registro (`name=`).

É **contêiner**: `x = app.form()` e depois `x.p()`, `x.entry()`, `x.button()`...
criam filhos **dentro** dele (borda de 8px, empilhados). Sem `height` explícito, a
caixa cresce pra caber os filhos.

## O que o guzer faz com ele hoje

- Renderiza uma **caixa** (default 200×28, fundo `#F0F0F0`, texto `#101418`) empilhada
  na janela — ou dentro do pai, se foi criado por um contêiner.
- O texto mostrado é o `.text(...)`; sem ele, o `placeholder=`.
- `type()` devolve `"form"` nos dois motores (tkinter no interpretador, X11 no `pool`).
- `.stylesheet({...})` e `.text(...)` encadeiam (devolvem o próprio elemento).

## Atributos

No HTML, os atributos próprios de `form` são `action=`, `methd=`.

| argumento | efeito hoje |
|---|---|
| `placeholder=` | texto mostrado quando não há `.text(...)` |
| `value=` | o que `.value` devolve (sem ele: o `.text()`; sem os dois: o `placeholder=`) |
| `name=` | identifica no registro: `app.POOLHTMLElements.getitemByIdentify("nome").value` |
| `onclick=` | reaction (por referência) que roda no clique |
| `href=`, `src=`, `alt=`, `target=`, `forid=`, `action=`, `methd=`, `rows=`, `cols=`, `typeinp=` | aceitos e guardados no elemento; **sem efeito visual** em `form` |

Chaves de estilo: `background` (ou `bg`), `color`, `width`, `height` e `font-size`
(esta só no interpretador; o backend X11 usa a fonte do sistema).

## Exemplo

```
import guzer
import scripts

app = guzer.UI("Demo")
form = app.form(action="/salvar", methd="post")
form.label(forid="nome").text("Nome")
form.entry(name="nome", placeholder="seu nome")
form.button(onclick=scripts.Enviar).text("Enviar")
// em scripts.Enviar: app.POOLHTMLElements.getitemByIdentify("nome").value
```

[← índice](../guzer.md)
