# `guzer.UI(title="PoolScript", icon=None)`

A **raiz do app**. Cria os elementos (um método por elemento do HTML) e, no fim
do script, abre a janela nativa sozinha (ou na hora, com `.show()`).

- `title` — título da janela.
- `icon` — caminho de um `.png` pro ícone da janela; o arquivo vem de onde
  você quiser (absoluto, relativo ao script, ou ao diretório atual).

| método | o que cria |
|---|---|
| `window()` | a janela (tamanho + fundo) |
| `button(onclick=)` | botão |
| `dialog(...)` | modal centralizado |
| `img(src=)` / `picture(src=)` | imagem desenhada do arquivo |
| `p() h1()..h6() div() span() table() form() entry()...` | **todos os elementos do HTML** — um método por tag |
| `show()` | abre a janela agora (bloqueante) |

## Exemplo

```
import guzer
app = guzer.UI("Meu app", "icone.png")
app.window().stylesheet({ "width": "500", "height": "300" })
app.h1().text("Painel")
app.img(src="logo.png")
```

[← índice](../guzer.md)
