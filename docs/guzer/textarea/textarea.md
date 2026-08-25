# `textarea(typeinp=, placeholder=, value=, name=, href=, src=, alt=, target=, forid=, action=, methd=, rows=, cols=, onclick=)`

Campo de formulário — área de VÁRIAS linhas. A caixa mostra o texto/placeholder; a
edição por teclado ainda **não existe** no backend X11, então o `.value`
devolve o estado do modelo (o que foi passado em `value=`/`.text(...)`).

- `name=` — a chave pro [getitemByIdentify](../getitemByIdentify/getitemByIdentify.md).
- `value=` — valor inicial.
- `placeholder=` — dica exibida sem texto.

```
import guzer
import scripts

app = guzer.UI("Cadastro")
form = app.form()
form.entry(name="nome", placeholder="seu nome")
form.button(onclick=scripts.Enviar).text("Enviar")
```

No handler: `app.POOLHTMLElements.getitemByIdentify("nome").value`.

[← índice](../guzer.md)
