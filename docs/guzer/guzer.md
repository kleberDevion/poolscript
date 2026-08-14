# guzer — UI desktop (janela nativa, zero download)

Monta telas de **app desktop** com OBJETOS (`window`, `button`, `popup`) e
estiliza cada um com chaves de estilo (nomes de CSS) via `.stylesheet({...})`.
Abre uma **janela nativa** — sem baixar nada:

- **INTERP:** usa `tkinter` (já vem no Python).
- **VM em C:** backend nativo próprio (Xlib), sem dependência externa.

```
import guzer
```

---

## O modelo

- **Todo objeto** tem `.stylesheet({...})` e `.text(...)`, encadeáveis (retornam o próprio).
- **Todo objeto** já vem com um **design default** — o `.stylesheet` só
  sobrescreve as chaves que você passar.
- **Handlers são `reaction`s passadas por referência** (sem `()`): a reaction
  roda quando o evento dispara.

Número "pelado" numa medida vira pixel: `"width": "500"` → 500px.

Chaves mapeadas no widget nativo: `background` (ou `bg`), `color`, `width`,
`height`, `font-size`. (É janela nativa, não navegador — o conjunto é o que o
widget nativo entende, não CSS completo.)

---

## Exemplo completo

`scripts.ps` — a lógica (uma reaction):

```
int reaction Clicker()
{
    post("Ola")
}
```

`app.ps` — a tela:

```
import guzer
import scripts

app = guzer.UI("Meu app")

app.window()
   .stylesheet({ "width": "500", "height": "300", "background": "#101418" })

app.button(onclick=scripts.Clicker)
   .stylesheet({ "width": "120", "height": "34" })
   .text("Clique")

app.popup(event_child=scripts.Clicker)   // popup com design default
```

Ao terminar o script, a janela nativa abre com tudo montado.

---

## `guzer.UI(title="PoolScript")` — a raiz do app

| método | retorno | o que faz |
|---|---|---|
| `window()` | `Window` | cria a janela/contêiner (tamanho + fundo) |
| `button(onclick=None)` | `Button` | cria um botão; `onclick` é uma reaction |
| `popup(event_child=None)` | `Popup` | cria um popup/modal centralizado |
| `show()` | — | abre a janela na hora (bloqueante) — normalmente não precisa |

Todo objeto aceita `.stylesheet({...})` e `.text(...)`, encadeáveis.

---

## Objetos e seus defaults

**`window`** — contêiner. Default: 480×320, fundo branco. Dê tamanho e cor.

**`button`** — default: 120×34, fundo `#2196F7`, texto branco. `onclick=` é a
reaction do clique.

**`popup`** — default: 260×150, centralizado, fundo branco. `event_child=` é a
reaction (roda ao clicar no popup).
