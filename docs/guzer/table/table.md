# `table(typeinp=, placeholder=, value=, name=, href=, src=, alt=, target=, forid=, action=, methd=, rows=, cols=, onclick=)`

Elemento `table` — Tabela — contêiner de `caption`, `thead`/`tbody`/`tfoot` e `tr`. **Hoje o guzer empilha**: cada `tr` é uma caixa e as células ficam dentro dela, uma abaixo da outra — não há grade com colunas alinhadas.

É **contêiner**: `x = app.table()` e depois `x.p()`, `x.entry()`, `x.button()`...
criam filhos **dentro** dele (borda de 8px, empilhados). Sem `height` explícito, a
caixa cresce pra caber os filhos.

## O que o guzer faz com ele hoje

- Renderiza uma **caixa** (default 200×28, fundo `#F0F0F0`, texto `#101418`) empilhada
  na janela — ou dentro do pai, se foi criado por um contêiner.
- O texto mostrado é o `.text(...)`; sem ele, o `placeholder=`.
- `type()` devolve `"table"` na janela nativa (X11).
- `.stylesheet({...})` e `.text(...)` encadeiam (devolvem o próprio elemento).

## Atributos

| argumento | efeito hoje |
|---|---|
| `placeholder=` | texto mostrado quando não há `.text(...)` |
| `value=` | o que `.value` devolve (sem ele: o `.text()`; sem os dois: o `placeholder=`) |
| `name=` | identifica no registro: `app.POOLHTMLElements.getitemByIdentify("nome").value` |
| `onclick=` | reaction (por referência) que roda no clique |
| `href=`, `src=`, `alt=`, `target=`, `forid=`, `action=`, `methd=`, `rows=`, `cols=`, `typeinp=` | aceitos e guardados no elemento; **sem efeito visual** em `table` |

Chaves de estilo: `background` (ou `bg`), `color`, `width`, `height` e `font-size`
(ignorada: o backend X11 usa a fonte do sistema).

## Exemplo

```
import guzer
import scripts

app = guzer.UI("Demo")
t = app.table()
t.caption().text("Estoque")
linha = t.tr()
linha.th().text("produto")
linha.th().text("qtd")
linha2 = t.tr()
linha2.td().text("café")
linha2.td().text("12")
```

[← índice](../guzer.md)
