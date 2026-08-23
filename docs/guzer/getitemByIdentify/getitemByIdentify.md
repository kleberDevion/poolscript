# `getitemByIdentify(identify)`

Acha o **elemento** cujo atributo `name=` é igual a `identify` — procura na
árvore inteira (dentro de form, div, etc.). Sem achar, devolve `null`.

```
form = app.form()
form.entry(name="email")

el = app.POOLHTMLElements.getitemByIdentify("email")
post(el.value)
post(app.POOLHTMLElements.getitemByIdentify("nao_existe") == null)   // True
```

[← índice](../guzer.md)
