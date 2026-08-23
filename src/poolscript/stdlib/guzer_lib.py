"""guzer — UI desktop da PoolScript (janela nativa, ZERO download).

Você monta a tela com os ELEMENTOS DO HTML (`div`, `p`, `h1`, `entry`,
`select`, `img`, `button`...) e estiliza cada um com chaves de estilo via
`.stylesheet({...})`. Abre uma janela NATIVA — no INTERP via `tkinter` (já vem
no Python); no binário `pool`, backend X11 próprio.

    import guzer
    import scripts

    app = guzer.UI("Meu app")
    app.window().stylesheet({ "width": "500", "height": "300", "background": "#101418" })
    app.h1().text("Cadastro")
    app.entry(placeholder="seu nome")
    app.img(src="logo.png")
    app.button(onclick=scripts.Clicker).text("Enviar")

Regras:
  - Todo elemento tem `.stylesheet({...})` e `.text(...)`, encadeáveis.
  - Todo elemento tem um DESIGN DEFAULT; o `.stylesheet` sobrescreve chave a chave.
  - Handlers são `reaction`s por REFERÊNCIA (`onclick=scripts.Clicker`).
  - `img(src=...)`: o arquivo vem de ONDE você quiser — caminho absoluto, ou
    relativo à pasta do script em execução, ou ao diretório atual.

Chaves de estilo aplicadas: `background` (ou `bg`), `color`, `width`, `height`
e `font-size` (esta só no interpretador; o backend X11 usa a fonte do sistema).
Número pelado numa medida vira pixels.
"""
from __future__ import annotations

import atexit
import os
import re as _re
import shutil as _shutil
import subprocess as _sp
from typing import Any, Callable

_NUM = _re.compile(r"-?\d+(\.\d+)?")
_ALIAS = {"bg": "background"}   # apelidos de chave -> nome canônico

# Atributos aceitos pelos elementos — o superset do HTML, renomeado quando o
# nome bate com keyword da linguagem (type->typeinp, for->forid, method->methd).
# MESMA lista do P_ELEM da VM em C.
_ATRIBUTOS = ("typeinp", "placeholder", "value", "name", "href", "src", "alt",
              "target", "forid", "action", "methd", "rows", "cols", "onclick")

# Todos os elementos (tag -> True se abre como dialog/modal). MESMA lista da
# tabela METODOS_GUZ_UI da VM em C — paridade elemento a elemento.
_TAGS_BOX = (
    "div", "section", "article", "aside", "header", "footer", "nav", "main",
    "figure", "figcaption", "address", "span", "p", "a", "strong", "em",
    "b", "i", "u", "s", "small", "mark", "sub", "sup", "code", "pre",
    "blockquote", "cite", "q", "abbr", "time", "kbd", "samp", "var",
    "del", "ins", "hr", "br", "h1", "h2", "h3", "h4", "h5", "h6",
    "ul", "ol", "li", "dl", "dt", "dd", "table", "thead", "tbody", "tfoot",
    "tr", "td", "th", "caption", "form", "entry", "textarea", "select",
    "option", "optgroup", "label", "fieldset", "legend", "datalist",
    "output", "progress", "meter", "img", "audio", "video", "canvas",
    "iframe", "details", "summary", "menu", "picture",
)
_TAG_DIALOG = "dialog"


def _px(value: Any, default: int) -> int:
    """Primeiro número da chave -> int de pixels. '8px 14px' -> 8, '500' -> 500."""
    if value is None:
        return default
    if isinstance(value, (int, float)):
        return int(value)
    m = _NUM.search(str(value))
    return int(float(m.group())) if m else default


def _resolve_arquivo(caminho: str) -> str | None:
    """src/ícone de ONDE o usuário quiser: absoluto, relativo ao script em
    execução, ou ao diretório atual (a mesma regra da lib os)."""
    if not caminho:
        return None
    if os.path.isabs(caminho):
        return caminho if os.path.isfile(caminho) else None
    try:
        from .os_lib import _SCRIPT_DIR
        if _SCRIPT_DIR is not None:
            p = os.path.join(str(_SCRIPT_DIR), caminho)
            if os.path.isfile(p):
                return p
    except Exception:
        pass
    return caminho if os.path.isfile(caminho) else None


class _Widget:
    """Base de todo elemento. `DEFAULT` é o design default; `.stylesheet(...)`
    sobrescreve por cima dele, chave a chave."""

    DEFAULT: dict = {}

    def __init__(self, ui: "UI", wid: str):
        self._ui = ui
        self._id = wid
        self._style: dict[str, Any] = dict(self.DEFAULT)
        self._overrides: set[str] = set()    # chaves que o usuário setou
        self._text: str = ""
        self._handler: Callable | None = None
        self._attrs: dict[str, Any] = {}     # placeholder/src/href/...
        self._kids: list["_Widget"] = []     # ÁRVORE: filhos (entry numa div...)
        self._raiz = True                    # False = está dentro de um contêiner

    def stylesheet(self, styles: dict | None = None) -> "_Widget":
        if styles:
            for k, v in styles.items():
                ck = _ALIAS.get(str(k), str(k))
                self._style[ck] = v
                self._overrides.add(ck)
        return self

    def text(self, value: Any) -> "_Widget":
        self._text = value if isinstance(value, str) else str(value)
        return self

    # -- estilo resolvido (default + overrides). Interno. --
    def _bg(self):
        return self._style.get("background")

    def _fg(self):
        return self._style.get("color")

    def _rotulo(self) -> str:
        """texto mostrado: .text() ou, sem ele, o placeholder (igual à VM)."""
        return self._text or str(self._attrs.get("placeholder") or "")

    @property
    def value(self):
        """O valor ATUAL do elemento. Com a janela aberta, entry/textarea
        leem o que foi DIGITADO (tkinter); sem janela, é value= > .text() >
        placeholder= > "" (a mesma ordem da VM)."""
        tkw = getattr(self, "_tk", None)
        if tkw is not None:
            try:
                if hasattr(tkw, "get") and not hasattr(tkw, "index"):  # Entry
                    return tkw.get()
                if hasattr(tkw, "get"):                                 # Text
                    return tkw.get("1.0", "end-1c")
            except Exception:
                pass
        v = self._attrs.get("value")
        if v is not None:
            return str(v)
        return self._text or str(self._attrs.get("placeholder") or "")

    def _elemento(self, tag: str, kwargs: dict) -> "_Widget":
        """Cria um elemento DENTRO deste (a árvore do HTML: entry numa div,
        inputs num form...). O elemento também entra no registro do app."""
        w = self._ui._elemento(tag, kwargs)
        w._raiz = False
        self._kids.append(w)
        return w

    def button(self, onclick: Callable | None = None) -> "Button":
        w = self._ui.button(onclick)
        w._raiz = False
        self._kids.append(w)
        return w

    def __repr__(self):
        return f"<guzer.{type(self).__name__} {self._id}>"


class Window(_Widget):
    """Janela/contêiner do app: tamanho e cor de fundo."""
    DEFAULT = {"width": "480", "height": "320", "background": "#ffffff"}


class Button(_Widget):
    """Botão. `onclick=` recebe uma reaction (por referência), que roda no clique."""
    DEFAULT = {"width": "120", "height": "34", "background": "#2196F7",
               "color": "#ffffff", "font-size": "13"}


class _Elemento(_Widget):
    """Elemento HTML genérico — caixa empilhada com texto (os mesmos defaults
    da VM em C: 200x28, fundo #F0F0F0, texto #101418)."""
    DEFAULT = {"width": "200", "height": "28", "background": "#F0F0F0",
               "color": "#101418", "font-size": "13"}


class _Dialog(_Widget):
    """dialog — modal centralizado (260x150, fundo branco), como na VM."""
    DEFAULT = {"width": "260", "height": "150", "background": "#ffffff",
               "color": "#101418", "font-size": "13"}


# Uma subclasse POR TAG, com __name__ = a tag — assim `type(el)` devolve o nome
# do elemento nos DOIS motores (a VM guarda a tag no widget pro type()).
_CLASSES: dict[str, type] = {}
for _tag in _TAGS_BOX:
    _CLASSES[_tag] = type(_tag, (_Elemento,), {})
_CLASSES[_TAG_DIALOG] = type(_TAG_DIALOG, (_Dialog,), {})


class UI:
    """Raiz do app. Cria os elementos e, ao fim do script, abre a janela nativa."""

    def __init__(self, title: str = "PoolScript", icon: str | None = None):
        self._title = title
        self._icon = icon            # caminho de um .png pro ícone da janela
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

    # -- registro dos elementos: app.POOLHTMLElements.getitemByIdentify("x").value --
    @property
    def POOLHTMLElements(self) -> "UI":
        return self

    def getitemByIdentify(self, identify):
        """Acha o elemento pelo atributo name=. Sem achar, devolve null."""
        for c in self._children:
            if str(c._attrs.get("name") or "") == str(identify):
                return c
        return None

    # -- API base --
    def window(self) -> Window:
        return self._make(Window)

    def button(self, onclick: Callable | None = None) -> Button:
        return self._make(Button, handler=onclick)

    def _elemento(self, tag: str, kwargs: dict) -> _Widget:
        w = self._make(_CLASSES[tag], handler=kwargs.get("onclick"))
        for k in _ATRIBUTOS:
            if k in kwargs and kwargs[k] is not None and k != "onclick":
                w._attrs[k] = kwargs[k]
        return w

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
        if self._icon:
            ico = _resolve_arquivo(self._icon)
            if ico:
                try:
                    root.iconphoto(True, tk.PhotoImage(file=ico))
                except Exception:
                    pass    # ícone inválido não derruba o app (igual X11)
        w_px = _px(win._style.get("width") if win else None, 480)
        h_px = _px(win._style.get("height") if win else None, 320)
        root.geometry(f"{w_px}x{h_px}")
        if win and win._bg():
            root.configure(bg=win._bg())

        def _font(w):
            return tkfont.Font(size=_px(w._style.get("font-size"), 13))

        self._imgs = []          # PhotoImage precisa de referência viva
        self._procs = []         # ffplay/ffmpeg vivos (mortos ao fechar)

        # pré-passo: carrega as imagens JÁ, pro tamanho natural valer no
        # empilhamento (senão o elemento de baixo cobria a imagem — a VM faz
        # o mesmo pré-passo com ps_guz_png_tamanho)
        for c in self._children:
            if type(c).__name__ in ("img", "picture") and c._attrs.get("src"):
                src = _resolve_arquivo(str(c._attrs["src"]))
                if src:
                    try:
                        foto = tk.PhotoImage(file=src)
                        self._imgs.append(foto)
                        c._foto = foto
                    except Exception:
                        c._foto = None

        def _altura(c):
            """altura efetiva: contêiner sem height explícito cresce pra
            couber os filhos (8px de borda, 10px de vão) — igual à VM."""
            ch = _px(c._style.get("height"), 28)
            foto = getattr(c, "_foto", None)
            if foto is not None and "height" not in c._overrides:
                return foto.height()          # img: altura natural
            if c._kids and "height" not in c._overrides:
                soma = 8 + sum(_altura(k) + 10 for k in c._kids) - 10 + 8
                return max(soma, ch)
            return ch

        plano = []               # (widget, x, y) já com posições absolutas

        def _layout(c, x, cy):
            plano.append((c, x, cy))
            ky = cy + 8
            for k in c._kids:
                _layout(k, x + 8, ky)
                ky += _altura(k) + 10

        y = 12
        for c in self._children:
            if isinstance(c, Window) or not c._raiz:
                continue
            ch = _altura(c)
            cw = _px(c._style.get("width"), 200)
            if isinstance(c, _Dialog):
                _layout(c, (w_px - cw) // 2, (h_px - ch) // 2)
            else:
                _layout(c, 12, y)
                y += ch + 10

        for c, x, cy in plano:
            cw = _px(c._style.get("width"), 200)
            ch = _altura(c)

            if isinstance(c, Button):
                el = tk.Button(root, text=c._rotulo(), font=_font(c),
                               command=(c._handler or (lambda: None)))
            elif type(c).__name__ == "entry":
                el = tk.Entry(root, font=_font(c))
                inicial = str(c._attrs.get("value") or "") or c._text
                if inicial:
                    el.insert(0, inicial)
                c._tk = el
            elif type(c).__name__ == "textarea":
                el = tk.Text(root, font=_font(c))
                inicial = str(c._attrs.get("value") or "") or c._text
                if inicial:
                    el.insert("1.0", inicial)
                c._tk = el
            elif type(c).__name__ in ("video", "audio") and c._attrs.get("src"):
                # mídia de verdade via ffmpeg (subprocesso): o vídeo entra
                # frame a frame NA JANELA; o áudio (e a trilha) no ffplay.
                el = tk.Label(root, anchor="w")
                src = _resolve_arquivo(str(c._attrs["src"]))
                if src is None:
                    el.configure(text=str(c._attrs["src"]) + " (não achei o arquivo)")
                elif type(c).__name__ == "audio":
                    if _shutil.which("ffplay"):
                        self._procs.append(_sp.Popen(
                            ["ffplay", "-nodisp", "-autoexit", "-loglevel", "quiet", src],
                            stdout=_sp.DEVNULL, stderr=_sp.DEVNULL))
                        el.configure(text=c._rotulo() or "♪ " + os.path.basename(src))
                    else:
                        el.configure(text="instale ffmpeg pra tocar áudio")
                else:
                    if _shutil.which("ffmpeg"):
                        self._toca_video(root, el, src, cw, ch)
                        if _shutil.which("ffplay"):      # trilha sonora
                            self._procs.append(_sp.Popen(
                                ["ffplay", "-nodisp", "-autoexit", "-loglevel", "quiet", src],
                                stdout=_sp.DEVNULL, stderr=_sp.DEVNULL))
                    else:
                        el.configure(text="instale ffmpeg pra reproduzir vídeo")
            elif c._attrs.get("src"):
                # img/picture/...: desenha o ARQUIVO (de onde você quiser)
                el = tk.Label(root)
                foto = getattr(c, "_foto", None)
                if foto is not None:
                    el.configure(image=foto)
                    # sem width/height explícitos, a caixa vira o tamanho
                    # natural da imagem (mesma regra da VM)
                    if "width" not in c._overrides:
                        cw = foto.width()
                    if "height" not in c._overrides:
                        ch = foto.height()
                else:
                    el.configure(text=c._rotulo() or str(c._attrs["src"]))
            else:
                el = tk.Label(root, text=c._rotulo(), font=_font(c), anchor="w")
                if c._handler:
                    el.bind("<Button-1>", lambda _e, h=c._handler: h())

            if c._bg():
                el.configure(bg=c._bg())
            if c._fg() and not c._attrs.get("src"):
                el.configure(fg=c._fg())
            el.place(x=x, y=cy, width=cw, height=ch)

        try:
            root.mainloop()
        finally:
            for pr in self._procs:
                try:
                    pr.terminate()
                except Exception:
                    pass

    def _toca_video(self, root, el, src, cw, ch):
        """ffmpeg -> frames PPM pelo pipe; cada frame vira PhotoImage no label."""
        import tkinter as tk
        proc = _sp.Popen(
            ["ffmpeg", "-loglevel", "quiet", "-re", "-i", src,
             "-vf", f"scale={cw}x{ch}", "-f", "image2pipe", "-vcodec", "ppm", "-"],
            stdout=_sp.PIPE, stderr=_sp.DEVNULL)
        self._procs.append(proc)

        def le_ppm():
            # o ffmpeg emite exatamente: b"P6\n<W> <H>\n255\n" + W*H*3 bytes
            magica = proc.stdout.readline()
            if not magica.startswith(b"P6"):
                return None
            dim = proc.stdout.readline()
            maxv = proc.stdout.readline()
            try:
                wpx, hpx = (int(x) for x in dim.split())
            except ValueError:
                return None
            corpo = proc.stdout.read(wpx * hpx * 3)
            if corpo is None or len(corpo) < wpx * hpx * 3:
                return None
            return magica + dim + maxv + corpo

        def tick():
            frame = le_ppm()
            if frame is None:
                return                       # fim do vídeo: fica no último frame
            foto = tk.PhotoImage(data=frame)
            self._imgs[:] = [f for f in self._imgs if f is not getattr(el, "_frame", None)]
            self._imgs.append(foto)
            el._frame = foto
            el.configure(image=foto)
            root.after(33, tick)

        root.after(33, tick)

    def __repr__(self):
        return f"<guzer.UI {self._title!r} ({len(self._children)} objetos)>"


# gera os métodos app.div()/app.p()/.../app.dialog() — mesma assinatura da VM
def _faz_metodo(tag: str):
    def metodo(self, typeinp=None, placeholder=None, value=None, name=None,
               href=None, src=None, alt=None, target=None, forid=None,
               action=None, methd=None, rows=None, cols=None, onclick=None):
        return self._elemento(tag, {
            "typeinp": typeinp, "placeholder": placeholder, "value": value,
            "name": name, "href": href, "src": src, "alt": alt,
            "target": target, "forid": forid, "action": action,
            "methd": methd, "rows": rows, "cols": cols, "onclick": onclick,
        })
    metodo.__name__ = tag
    metodo.__doc__ = f"Elemento `{tag}` — caixa empilhada; atributos do HTML como argumentos nomeados."
    return metodo


for _tag in _TAGS_BOX + (_TAG_DIALOG,):
    setattr(UI, _tag, _faz_metodo(_tag))
    # a ÁRVORE do HTML: os mesmos métodos nos elementos-contêiner
    # (form.entry(...), div.p(...) — o filho fica DENTRO do pai)
    setattr(_Widget, _tag, _faz_metodo(_tag))


EXPORTS = {
    "UI": UI,
    "Window": Window,
    "Button": Button,
}
