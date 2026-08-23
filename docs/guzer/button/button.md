# `button(onclick=None)`

Botão nativo. `onclick=` é uma **reaction por referência** (sem parênteses):
roda quando o botão é clicado, com a janela aberta.

Design default: **120×34**, fundo `#2196F7`, texto branco.

```
int reaction Salvar() {
    post("salvo")
}

app.button(onclick=Salvar).stylesheet({ "width": "160" }).text("Salvar")
```

`type()` devolve `"Button"`.

[← índice](../guzer.md)
