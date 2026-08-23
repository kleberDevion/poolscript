# guzer — UI desktop (janela nativa, zero download)

Monta telas de **app desktop** com os **elementos do HTML** — um método por
tag — e estiliza cada um com `.stylesheet({...})`. Abre uma **janela
nativa**, sem baixar nada:

- **interpretador:** `tkinter` (já vem no Python);
- **binário `pool`:** backend X11 próprio (libX11 do sistema).

```
import guzer
```

## Base

| membro | página |
|---|---|
| `guzer.UI(title, icon)` — a raiz do app | [UI/UI.md](UI/UI.md) |
| `window()` — a janela | [window/window.md](window/window.md) |
| `button(onclick=)` — botão | [button/button.md](button/button.md) |
| `show()` — abre agora | [show/show.md](show/show.md) |
| `.stylesheet({...})` — estilo | [stylesheet/stylesheet.md](stylesheet/stylesheet.md) |
| `.text(...)` — texto | [text/text.md](text/text.md) |

## A árvore (elemento DENTRO de elemento)

Contêiner cria filho — igual ao HTML: o filho fica **dentro** da caixa do pai
(borda de 8px, empilhado), e o pai sem `height` explícito **cresce** pra caber:

```
form = app.form()
form.entry(name="nome", placeholder="seu nome")
form.button(onclick=scripts.Enviar).text("Enviar")

d = app.div()
d.p().text("dentro da div")
```

## O registro — ler valores

| membro | página |
|---|---|
| `app.POOLHTMLElements` — o registro | [POOLHTMLElements/POOLHTMLElements.md](POOLHTMLElements/POOLHTMLElements.md) |
| `getitemByIdentify(name)` — acha o elemento | [getitemByIdentify/getitemByIdentify.md](getitemByIdentify/getitemByIdentify.md) |
| `.value` — o valor atual | [value/value.md](value/value.md) |

```
nome = app.POOLHTMLElements.getitemByIdentify("nome").value
```

## O modelo

- Cada elemento é uma **caixa empilhada** na janela (o `dialog` centraliza; o
  `img` desenha o arquivo; `video`/`audio` REPRODUZEM via ffmpeg do sistema).
- `type()` de um elemento devolve a **tag** (`"div"`, `"h1"`, ...) — igual nos
  dois motores.
- Handlers são **reactions por referência** (`onclick=scripts.Clicker`).
- Atributos do HTML entram como argumentos nomeados: `placeholder=`, `src=`,
  `href=`, `name=`, `value=`... (`type`→`typeinp`, `for`→`forid`,
  `method`→`methd`, por serem palavras da linguagem).

## Todos os elementos (81)

| elemento |
|---|
| [`a`](a/a.md) |
| [`abbr`](abbr/abbr.md) |
| [`address`](address/address.md) |
| [`article`](article/article.md) |
| [`aside`](aside/aside.md) |
| [`audio`](audio/audio.md) |
| [`b`](b/b.md) |
| [`blockquote`](blockquote/blockquote.md) |
| [`br`](br/br.md) |
| [`canvas`](canvas/canvas.md) |
| [`caption`](caption/caption.md) |
| [`cite`](cite/cite.md) |
| [`code`](code/code.md) |
| [`datalist`](datalist/datalist.md) |
| [`dd`](dd/dd.md) |
| [`del`](del/del.md) |
| [`details`](details/details.md) |
| [`dialog`](dialog/dialog.md) |
| [`div`](div/div.md) |
| [`dl`](dl/dl.md) |
| [`dt`](dt/dt.md) |
| [`em`](em/em.md) |
| [`entry`](entry/entry.md) |
| [`fieldset`](fieldset/fieldset.md) |
| [`figcaption`](figcaption/figcaption.md) |
| [`figure`](figure/figure.md) |
| [`footer`](footer/footer.md) |
| [`form`](form/form.md) |
| [`h1`](h1/h1.md) |
| [`h2`](h2/h2.md) |
| [`h3`](h3/h3.md) |
| [`h4`](h4/h4.md) |
| [`h5`](h5/h5.md) |
| [`h6`](h6/h6.md) |
| [`header`](header/header.md) |
| [`hr`](hr/hr.md) |
| [`i`](i/i.md) |
| [`iframe`](iframe/iframe.md) |
| [`img`](img/img.md) |
| [`ins`](ins/ins.md) |
| [`kbd`](kbd/kbd.md) |
| [`label`](label/label.md) |
| [`legend`](legend/legend.md) |
| [`li`](li/li.md) |
| [`main`](main/main.md) |
| [`mark`](mark/mark.md) |
| [`menu`](menu/menu.md) |
| [`meter`](meter/meter.md) |
| [`nav`](nav/nav.md) |
| [`ol`](ol/ol.md) |
| [`optgroup`](optgroup/optgroup.md) |
| [`option`](option/option.md) |
| [`output`](output/output.md) |
| [`p`](p/p.md) |
| [`picture`](picture/picture.md) |
| [`pre`](pre/pre.md) |
| [`progress`](progress/progress.md) |
| [`q`](q/q.md) |
| [`s`](s/s.md) |
| [`samp`](samp/samp.md) |
| [`section`](section/section.md) |
| [`select`](select/select.md) |
| [`small`](small/small.md) |
| [`span`](span/span.md) |
| [`strong`](strong/strong.md) |
| [`sub`](sub/sub.md) |
| [`summary`](summary/summary.md) |
| [`sup`](sup/sup.md) |
| [`table`](table/table.md) |
| [`tbody`](tbody/tbody.md) |
| [`td`](td/td.md) |
| [`textarea`](textarea/textarea.md) |
| [`tfoot`](tfoot/tfoot.md) |
| [`th`](th/th.md) |
| [`thead`](thead/thead.md) |
| [`time`](time/time.md) |
| [`tr`](tr/tr.md) |
| [`u`](u/u.md) |
| [`ul`](ul/ul.md) |
| [`var`](var/var.md) |
| [`video`](video/video.md) |

## Exemplo completo

```
import guzer
import scripts

app = guzer.UI("Meu app", "icone.png")
app.window().stylesheet({ "width": "520", "height": "380", "background": "#101418" })
app.h1().stylesheet({ "color": "#ffffff", "bg": "#101418" }).text("Cadastro")
app.entry(placeholder="seu nome")
app.img(src="logo.png")
app.button(onclick=scripts.Enviar).text("Enviar")
```
