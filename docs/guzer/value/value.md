# `.value` (campo)

O **valor atual** do elemento (sem parênteses):

- no **interpretador**, com a janela aberta, `entry`/`textarea` devolvem o que
  o usuário **digitou** (campo nativo);
- fora disso, a ordem é `value=` → `.text(...)` → `placeholder=` → `""` —
  igual nos dois motores.

```
app.entry(name="nome", value="kleber")
post(app.POOLHTMLElements.getitemByIdentify("nome").value)   // kleber
```

[← índice](../guzer.md)
