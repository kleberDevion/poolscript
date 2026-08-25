# `.value` (campo)

O **valor atual** do elemento (sem parênteses):

A ordem é `value=` → `.text(...)` → `placeholder=` → `""`. O backend X11
ainda não edita por teclado, então `entry`/`textarea` devolvem o valor do
modelo, não o que o usuário digitou.

```
app.entry(name="nome", value="kleber")
post(app.POOLHTMLElements.getitemByIdentify("nome").value)   // kleber
```

[← índice](../guzer.md)
