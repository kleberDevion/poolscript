# `time(typeinp=, placeholder=, value=, name=, href=, src=, alt=, target=, forid=, action=, methd=, rows=, cols=, onclick=)`

Elemento `time` — Data/hora — `value=` guarda o formato de máquina (ISO) e `.text()` mostra o texto humano; `.value` devolve o ISO.

> No HTML `time` é **inline** (fica na mesma linha do texto ao redor). No guzer
> cada elemento é uma caixa própria na pilha — pra compor uma frase com um trecho
> marcado, hoje o jeito é um `p` com o texto inteiro.

## O que o guzer faz com ele hoje

- Renderiza uma **caixa** (default 200×28, fundo `#F0F0F0`, texto `#101418`) empilhada
  na janela — ou dentro do pai, se foi criado por um contêiner.
- O texto mostrado é o `.text(...)`; sem ele, o `placeholder=`.
- `type()` devolve `"time"` na janela nativa (X11).
- `.stylesheet({...})` e `.text(...)` encadeiam (devolvem o próprio elemento).

## Atributos

No HTML, os atributos próprios de `time` são `value=`.

| argumento | efeito hoje |
|---|---|
| `placeholder=` | texto mostrado quando não há `.text(...)` |
| `value=` | o que `.value` devolve (sem ele: o `.text()`; sem os dois: o `placeholder=`) |
| `name=` | identifica no registro: `app.POOLHTMLElements.getitemByIdentify("nome").value` |
| `onclick=` | reaction (por referência) que roda no clique |
| `href=`, `src=`, `alt=`, `target=`, `forid=`, `action=`, `methd=`, `rows=`, `cols=`, `typeinp=` | aceitos e guardados no elemento; **sem efeito visual** em `time` |

Chaves de estilo: `background` (ou `bg`), `color`, `width`, `height` e `font-size`
(ignorada: o backend X11 usa a fonte do sistema).

## Exemplo

```
import guzer
import scripts

app = guzer.UI("Demo")
app.time(value="2026-08-24", name="quando").text("24 de agosto de 2026")
// depois: app.POOLHTMLElements.getitemByIdentify("quando").value -> "2026-08-24"
```

[← índice](../guzer.md)
