# `label(typeinp=, placeholder=, value=, name=, href=, src=, alt=, target=, forid=, action=, methd=, rows=, cols=, onclick=)`

Elemento `label` — Rótulo de um campo — `forid=` guarda o `name=` do campo associado (é o `for` do HTML, renomeado porque `for` é palavra da linguagem). Clicar no rótulo **não** foca o campo hoje.

## O que o guzer faz com ele hoje

- Renderiza uma **caixa** (default 200×28, fundo `#F0F0F0`, texto `#101418`) empilhada
  na janela — ou dentro do pai, se foi criado por um contêiner.
- O texto mostrado é o `.text(...)`; sem ele, o `placeholder=`.
- `type()` devolve `"label"` nos dois motores (tkinter no interpretador, X11 no `pool`).
- `.stylesheet({...})` e `.text(...)` encadeiam (devolvem o próprio elemento).

## Atributos

No HTML, os atributos próprios de `label` são `forid=`.

| argumento | efeito hoje |
|---|---|
| `placeholder=` | texto mostrado quando não há `.text(...)` |
| `value=` | o que `.value` devolve (sem ele: o `.text()`; sem os dois: o `placeholder=`) |
| `name=` | identifica no registro: `app.POOLHTMLElements.getitemByIdentify("nome").value` |
| `onclick=` | reaction (por referência) que roda no clique |
| `href=`, `src=`, `alt=`, `target=`, `forid=`, `action=`, `methd=`, `rows=`, `cols=`, `typeinp=` | aceitos e guardados no elemento; **sem efeito visual** em `label` |

Chaves de estilo: `background` (ou `bg`), `color`, `width`, `height` e `font-size`
(esta só no interpretador; o backend X11 usa a fonte do sistema).

## Exemplo

```
import guzer
import scripts

app = guzer.UI("Demo")
app.label(forid="email").text("E-mail")
app.entry(name="email", placeholder="voce@exemplo.com")
```

[← índice](../guzer.md)
