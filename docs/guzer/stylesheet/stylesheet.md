# `.stylesheet(styles=None)`

Sobrescreve o design default do elemento, **chave a chave** — as que você não
passar continuam no default. Encadeável (retorna o próprio elemento).

| chave | efeito |
|---|---|
| `background` (ou `bg`) | cor de fundo (`#RRGGBB`) |
| `color` | cor do texto |
| `width` / `height` | tamanho em pixels (número pelado = px) |
| `font-size` | tamanho da fonte — **só no interpretador** (o backend X11 usa a fonte do sistema) |

```
app.div().stylesheet({ "width": "300", "bg": "#123456", "color": "#fff" })
```

[← índice](../guzer.md)
