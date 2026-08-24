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

| elemento | o que é |
|---|---|
| [`a`](a/a.md) | Link |
| [`abbr`](abbr/abbr.md) | Abreviação ou sigla (`PS`, `HTML`) |
| [`address`](address/address.md) | Informação de contato (e-mail, telefone, endereço) do autor ou da organização |
| [`article`](article/article.md) | Conteúdo autônomo, que faz sentido sozinho |
| [`aside`](aside/aside.md) | Conteúdo lateral/relacionado |
| [`audio`](audio/audio.md) | TOCA o áudio do `src=` ao abrir a janela (ffplay do sistema) |
| [`b`](b/b.md) | Negrito de estilo, sem importância semântica (nomes de produto, palavras-chave) |
| [`blockquote`](blockquote/blockquote.md) | Citação em bloco — trecho vindo de outra fonte |
| [`br`](br/br.md) | Quebra de linha |
| [`canvas`](canvas/canvas.md) | Área de desenho por script (em HTML, via JavaScript) |
| [`caption`](caption/caption.md) | Título/legenda da tabela |
| [`cite`](cite/cite.md) | Título de uma obra citada |
| [`code`](code/code.md) | Trecho de código — em HTML, fonte monoespaçada |
| [`datalist`](datalist/datalist.md) | Lista de sugestões pra um campo (`option`s filhos) |
| [`dd`](dd/dd.md) | Descrição/definição do `dt` anterior |
| [`del`](del/del.md) | Texto removido numa revisão (em HTML, riscado) |
| [`details`](details/details.md) | Bloco expansível (abre/fecha) com `summary` como título |
| [`dialog`](dialog/dialog.md) | modal centralizado na janela (260×150, fundo branco) |
| [`div`](div/div.md) | Contêiner genérico — a caixa que agrupa outros elementos sem significado próprio |
| [`dl`](dl/dl.md) | Lista de definições |
| [`dt`](dt/dt.md) | Termo de uma lista de definições (`dl`) |
| [`em`](em/em.md) | Ênfase — em HTML vira itálico |
| [`entry`](entry/entry.md) | campo de UMA linha — editável no interpretador (tkinter); `.value` lê o digitado |
| [`fieldset`](fieldset/fieldset.md) | Grupo de campos de um formulário, com um `legend` como título |
| [`figcaption`](figcaption/figcaption.md) | Legenda de um `figure` |
| [`figure`](figure/figure.md) | Conteúdo ilustrativo autocontido (imagem, gráfico, trecho de código) |
| [`footer`](footer/footer.md) | Rodapé — créditos, links finais, copyright |
| [`form`](form/form.md) | Formulário — agrupa os campos (`entry`, `textarea`, `select`) e o `button` de envio |
| [`h1`](h1/h1.md) | Título de nível 1 — o maior e mais importante (um por tela) |
| [`h2`](h2/h2.md) | Título de nível 2 |
| [`h3`](h3/h3.md) | Título de nível 3 |
| [`h4`](h4/h4.md) | Título de nível 4 |
| [`h5`](h5/h5.md) | Título de nível 5 |
| [`h6`](h6/h6.md) | Título de nível 6 |
| [`header`](header/header.md) | Cabeçalho de página ou de seção — título, logo, navegação inicial |
| [`hr`](hr/hr.md) | Quebra temática — a linha horizontal que separa blocos |
| [`i`](i/i.md) | Itálico de estilo — termo técnico, pensamento, nome de navio, palavra em outra língua |
| [`iframe`](iframe/iframe.md) | Documento embutido (outra página) |
| [`img`](img/img.md) | desenha o `.png` do `src=` (tamanho natural sem width/height) |
| [`ins`](ins/ins.md) | Texto inserido numa revisão (em HTML, sublinhado) |
| [`kbd`](kbd/kbd.md) | Entrada de teclado |
| [`label`](label/label.md) | Rótulo de um campo — `forid=` guarda o `name=` do campo associado (é o `for` do HTML, renomeado porque `for` é palavra da linguagem) |
| [`legend`](legend/legend.md) | Título de um `fieldset` |
| [`li`](li/li.md) | Item de lista |
| [`main`](main/main.md) | O conteúdo principal da janela |
| [`mark`](mark/mark.md) | Texto destacado (como marca-texto) |
| [`menu`](menu/menu.md) | Lista de comandos/ações |
| [`meter`](meter/meter.md) | Medidor escalar — nível de bateria, nota, uso de disco |
| [`nav`](nav/nav.md) | Bloco de navegação |
| [`ol`](ol/ol.md) | Lista ordenada — contêiner de `li` |
| [`optgroup`](optgroup/optgroup.md) | Grupo de `option`s com um rótulo (`.text()`), dentro de um `select` |
| [`option`](option/option.md) | Uma opção de `select` ou `datalist` |
| [`output`](output/output.md) | Resultado de um cálculo ou ação |
| [`p`](p/p.md) | Parágrafo de texto |
| [`picture`](picture/picture.md) | como `img` — desenha o `.png` do `src=` |
| [`pre`](pre/pre.md) | Texto pré-formatado — em HTML, espaços e quebras de linha são preservados |
| [`progress`](progress/progress.md) | Barra de progresso — em HTML, `value`/`max` |
| [`q`](q/q.md) | Citação curta, inline |
| [`s`](s/s.md) | Texto riscado |
| [`samp`](samp/samp.md) | Saída de exemplo de um programa |
| [`section`](section/section.md) | Seção temática — um agrupamento com título próprio (normalmente começa com um `h1`–`h6`) |
| [`select`](select/select.md) | Lista de opções (combo) — em HTML contém `option`s |
| [`small`](small/small.md) | Texto secundário/miúdo |
| [`span`](span/span.md) | Trecho inline genérico |
| [`strong`](strong/strong.md) | Importância forte — em HTML vira negrito |
| [`sub`](sub/sub.md) | Subscrito |
| [`summary`](summary/summary.md) | Título de um `details` |
| [`sup`](sup/sup.md) | Sobrescrito |
| [`table`](table/table.md) | Tabela — contêiner de `caption`, `thead`/`tbody`/`tfoot` e `tr` |
| [`tbody`](tbody/tbody.md) | Grupo das linhas do corpo da tabela |
| [`td`](td/td.md) | Célula de dados de uma linha (`tr`) |
| [`textarea`](textarea/textarea.md) | campo de VÁRIAS linhas — editável no interpretador; `.value` lê o digitado |
| [`tfoot`](tfoot/tfoot.md) | Grupo das linhas de rodapé da tabela (totais) |
| [`th`](th/th.md) | Célula de cabeçalho de uma linha (`tr`) |
| [`thead`](thead/thead.md) | Grupo das linhas de cabeçalho da tabela (as `tr` com `th`) |
| [`time`](time/time.md) | Data/hora |
| [`tr`](tr/tr.md) | Linha da tabela |
| [`u`](u/u.md) | Sublinhado — anotação não-textual (ex.: marcar um erro ortográfico) |
| [`ul`](ul/ul.md) | Lista não ordenada — contêiner de `li` |
| [`var`](var/var.md) | Variável |
| [`video`](video/video.md) | REPRODUZ o vídeo do `src=` na janela (ffmpeg do sistema) |

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
