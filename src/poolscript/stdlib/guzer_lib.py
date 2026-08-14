"""guzer — UI desktop da PoolScript (janela nativa, ZERO download).

Você monta a tela com OBJETOS (`window`/`button`/`popup`) e estiliza cada um
com chaves de estilo (nomes de CSS) via `.stylesheet({...})`. Abre uma janela
NATIVA — no INTERP via `tkinter` (já vem no Python, nada pra baixar).

    import guzer
    import scripts

    app = guzer.UI("Meu app")
    app.window().stylesheet({ "width": "500", "height": "300", "background": "#101418" })
    app.button(onclick=scripts.Clicker).stylesheet({ "width": "120", "height": "34" }).text("Clique")
    app.popup(event_child=scripts.Clicker)

Regras:
  - Todo objeto tem `.stylesheet({...})` e `.text(...)`, encadeáveis (retornam o próprio).
  - Todo objeto tem um DESIGN DEFAULT; o `.stylesheet` só sobrescreve chave a chave.
  - Handlers são `reaction`s passadas por REFERÊNCIA (`onclick=scripts.Clicker`);
    a reaction roda quando o evento dispara.

Chaves mapeadas pro widget nativo: `background`/`bg`, `color`, `width`,
`height`, `font-size`, `padding`. Número pelado numa medida vira pixels.
INTERP-only por natureza (é GUI); a VM em C tem seu próprio backend nativo.
"""
from __future__ import annotations

import atexit
import os
import re as _re
from typing import Any, Callable

_NUM = _re.compile(r"-?\d+(\.\d+)?")
_ALIAS = {"bg": "background"}   # apelidos de chave -> nome canônico


def _px(value: Any, default: int) -> int:
    """Primeiro número da chave -> int de pixels. '8px 14px' -> 8, '500' -> 500."""
    if value is None:
        return default
    if isinstance(value, (int, float)):
        return int(value)
    m = _NUM.search(str(value))
    return int(float(m.group())) if m else default


class _Widget:
    """Base de todo objeto de tela. `DEFAULT` é o design default da subclasse;
    `.stylesheet(...)` sobrescreve por cima dele, chave a chave."""

    DEFAULT: dict = {}

    def __init__(self, ui: "UI", wid: str):
        self._ui = ui
        self._id = wid
        self._style: dict[str, Any] = dict(self.DEFAULT)
        self._text: str = ""
        self._handler: Callable | None = None

    def stylesheet(self, styles: dict | None = None) -> "_Widget":
        if styles:
            for k, v in styles.items():
                self._style[_ALIAS.get(str(k), str(k))] = v
        return self

    def text(self, value: Any) -> "_Widget":
        self._text = value if isinstance(value, str) else str(value)
        return self

    # -- estilo resolvido (default + overrides). Interno. --
    def _bg(self):
        return self._style.get("background")

    def _fg(self):
        return self._style.get("color")

    def __repr__(self):
        return f"<guzer.{type(self).__name__} {self._id}>"


class Window(_Widget):
    """Janela/contêiner do app: tamanho e cor de fundo."""
    DEFAULT = {"width": "480", "height": "320", "background": "#ffffff"}


class Button(_Widget):
    """Botão. `onclick=` recebe uma reaction (por referência), que roda no clique."""
    DEFAULT = {"width": "120", "height": "34", "background": "#2196F7",
               "color": "#ffffff", "font-size": "13"}


class Popup(_Widget):
    """Popup/modal centralizado. `event_child=` recebe uma reaction (roda no clique)."""
    DEFAULT = {"width": "260", "height": "150", "background": "#ffffff",
               "color": "#101418", "font-size": "13"}


class UI:
    """Raiz do app. Cria os objetos e, ao fim do script, abre a janela nativa."""

    def __init__(self, title: str = "PoolScript"):
        self._title = title
        self._children: list[_Widget] = []
        self._seq = 0
        self._shown = False
        atexit.register(self._auto_show)

    def _make(self, cls, handler=None) -> _Widget:
        self._seq += 1
        w = cls(self, f"g{self._seq}")
        w._handler = handler
        self._children.append(w)
        return w

    # -- API que você usa --
    def window(self) -> Window:
        return self._make(Window)

    def button(self, onclick: Callable | None = None) -> Button:
        return self._make(Button, handler=onclick)

    def popup(self, event_child: Callable | None = None) -> Popup:
        return self._make(Popup, handler=event_child)

    # -- exibição --
    def _auto_show(self):
        if self._shown or not self._children or os.environ.get("GUZER_HEADLESS"):
            return
        self.show()

    def show(self):
        """Abre a janela nativa com o app montado (bloqueante)."""
        if self._shown:
            return
        self._shown = True
        import tkinter as tk
        from tkinter import font as tkfont

        win = next((c for c in self._children if isinstance(c, Window)), None)
        root = tk.Tk()
        root.title(self._title)
        w_px = _px(win._style.get("width") if win else None, 480)
        h_px = _px(win._style.get("height") if win else None, 320)
        root.geometry(f"{w_px}x{h_px}")
        if win and win._bg():
            root.configure(bg=win._bg())

        def _font(w):
            return tkfont.Font(size=_px(w._style.get("font-size"), 13))

        def _bind(widget, w):
            if w._handler is not None:
                widget.bind("<Button-1>", lambda _e, fn=w._handler: fn())

        y = 12
        for c in self._children:
            if isinstance(c, Button):
                b = tk.Button(root, text=c._text or "", font=_font(c),
                              command=(c._handler or (lambda: None)))
                if c._bg():
                    b.configure(bg=c._bg())
                if c._fg():
                    b.configure(fg=c._fg())
                b.place(x=12, y=y, width=_px(c._style.get("width"), 120),
                        height=_px(c._style.get("height"), 34))
                y += _px(c._style.get("height"), 34) + 10

        for c in self._children:
            if isinstance(c, Popup):
                pw = _px(c._style.get("width"), 260)
                ph = _px(c._style.get("height"), 150)
                top = tk.Toplevel(root)
                top.title(self._title)
                top.geometry(f"{pw}x{ph}+{w_px // 2 - pw // 2}+{h_px // 2 - ph // 2}")
                if c._bg():
                    top.configure(bg=c._bg())
                lbl = tk.Label(top, text=c._text or "", font=_font(c),
                               bg=c._bg() or "#ffffff", fg=c._fg() or "#000000")
                lbl.place(relx=0.5, rely=0.5, anchor="center")
                _bind(top, c)

        root.mainloop()

    def __repr__(self):
        return f"<guzer.UI {self._title!r} ({len(self._children)} objetos)>"


EXPORTS = {
    "UI": UI,
    "Window": Window,
    "Button": Button,
    "Popup": Popup,
}
