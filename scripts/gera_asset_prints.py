#!/usr/bin/env python3
"""Gera os prints (PNG) dos exemplos ```ps das docs, no visual do CodeSnap
(janela estilo mac + syntax highlight da PoolScript + sombra).

- Extrai cada bloco ```ps de docs/**/*.md pra um .ps em docs/assets/src/
  (essa pasta é de trabalho e fica no .gitignore — abra um .ps lá e use a
  extensão CodeSnap se quiser refazer um print à mão).
- Renderiza um HTML com highlight (keywords vêm do LEXER REAL, não de lista
  chutada) e fotografa com o Chrome headless -> docs/assets/<md>_exN.png.

Uso:  PYTHONPATH=src python3 scripts/gera_asset_prints.py [--so docs/sockets]
"""
import html
import re
import subprocess
import sys
from pathlib import Path

RAIZ = Path(__file__).resolve().parent.parent
DOCS = RAIZ / "docs"
ASSETS = DOCS / "assets"
SRC_PS = ASSETS / "src"
CHROME = "/usr/bin/google-chrome"

sys.path.insert(0, str(RAIZ / "src"))
from poolscript.lexer import KEYWORDS  # noqa: E402  (a fonte da verdade)

# tema One Dark (o mesmo espírito do CodeSnap default)
CORES = {
    "fundo":    "#282c34",
    "texto":    "#abb2bf",
    "kw":       "#c678dd",   # keywords
    "tipo":     "#e5c07b",   # Entity/Nomes Capitalizados
    "str":      "#98c379",
    "num":      "#d19a66",
    "coment":   "#5c6370",
    "fn":       "#61afef",   # nome de função na chamada/def
    "dec":      "#e06c75",   # @decorators
}

TOKEN_RE = re.compile(
    r'("""[\s\S]*?""")'                    # docstring
    r"|('''[\s\S]*?''')"
    r"|(//[^\n]*|#[^\n]*)"                 # comentário
    r'|([fF]?"(?:\\.|[^"\\\n])*"'          # string "..."
    r"|[fF]?'(?:\\.|[^'\\\n])*')"          # string '...'
    r"|(@[A-Za-z_][\w.]*)"                 # decorator
    r"|(\b\d+(?:\.\d+)?\b)"                # número
    r"|(\b[A-Za-z_]\w*\b)"                 # identificador
)


def pinta(cod: str) -> str:
    saida = []
    pos = 0
    for m in TOKEN_RE.finditer(cod):
        saida.append(html.escape(cod[pos:m.start()]))
        t = m.group(0)
        esc = html.escape(t)
        if m.group(1) or m.group(2) or m.group(3):
            cor = "coment"
        elif m.group(4):
            cor = "str"
        elif m.group(5):
            cor = "dec"
        elif m.group(6):
            cor = "num"
        else:  # identificador
            fim = cod[m.end():m.end() + 1]
            if t in KEYWORDS or t in ("true", "false", "null", "True", "False", "None", "global", "raise", "yield", "match", "case", "count", "lambda", "none"):
                cor = "kw"
            elif t[0].isupper():
                cor = "tipo"
            elif fim == "(":
                cor = "fn"
            else:
                saida.append(esc)
                pos = m.end()
                continue
        saida.append(f'<span class="{cor}">{esc}</span>')
        pos = m.end()
    saida.append(html.escape(cod[pos:]))
    return "".join(saida)


PAGINA = """<meta charset="utf-8">
<style>
  html, body {{ margin: 0; background: transparent; }}
  .quadro {{ padding: 28px; display: inline-block; }}
  .janela {{
    background: {fundo}; border-radius: 10px; overflow: hidden;
    box-shadow: 0 14px 40px rgba(0,0,0,.55);
    font: 15px/1.5 "JetBrains Mono", "Fira Code", "DejaVu Sans Mono", monospace;
  }}
  .barra {{ display: flex; gap: 8px; padding: 12px 14px 4px; }}
  .b {{ width: 12px; height: 12px; border-radius: 50%; }}
  pre {{ margin: 0; padding: 10px 22px 20px; color: {texto}; }}
  .kw {{ color: {kw}; }} .tipo {{ color: {tipo}; }} .str {{ color: {str}; }}
  .num {{ color: {num}; }} .coment {{ color: {coment}; font-style: italic; }}
  .fn {{ color: {fn}; }} .dec {{ color: {dec}; }}
</style>
<div class="quadro"><div class="janela">
  <div class="barra">
    <div class="b" style="background:#ff5f56"></div>
    <div class="b" style="background:#ffbd2e"></div>
    <div class="b" style="background:#27c93f"></div>
  </div>
  <pre>{codigo}</pre>
</div></div>
"""


def gera_png(cod: str, png: Path, tmpdir: Path) -> bool:
    linhas = cod.split("\n")
    larg = min(max(max((len(l) for l in linhas), default=10) * 9.2 + 44 + 56, 380), 1400)
    alt = len(linhas) * 22.5 + 46 + 30 + 56
    h = tmpdir / (png.stem + ".html")
    h.write_text(PAGINA.format(codigo=pinta(cod), **CORES), encoding="utf-8")
    r = subprocess.run(
        [CHROME, "--headless=new", "--disable-gpu", "--hide-scrollbars",
         "--default-background-color=00000000",
         f"--window-size={int(larg)},{int(alt)}",
         f"--screenshot={png}", f"file://{h}"],
        capture_output=True, text=True, timeout=60)
    return png.exists() and r.returncode == 0


def main():
    so = None
    if len(sys.argv) > 2 and sys.argv[1] == "--so":
        so = (RAIZ / sys.argv[2]).resolve()
    ASSETS.mkdir(exist_ok=True)
    SRC_PS.mkdir(exist_ok=True)
    tmpdir = SRC_PS / "_html"
    tmpdir.mkdir(exist_ok=True)
    total = ok = 0
    for md in sorted(DOCS.rglob("*.md")):
        if ASSETS in md.parents:
            continue
        if so and so not in (md.parents if md.is_dir() else list(md.parents)):
            continue
        rel = md.relative_to(DOCS)
        slug = str(rel.with_suffix("")).replace("/", "__")
        texto = md.read_text(encoding="utf-8")
        blocos = re.findall(r"```ps\n(.*?)```", texto, re.DOTALL)
        for i, cod in enumerate(blocos, 1):
            cod = cod.rstrip("\n")
            if not cod.strip():
                continue
            total += 1
            nome = f"{slug}_ex{i}"
            (SRC_PS / f"{nome}.ps").write_text(cod + "\n", encoding="utf-8")
            if gera_png(cod, ASSETS / f"{nome}.png", tmpdir):
                ok += 1
            else:
                print(f"FALHOU: {nome}", file=sys.stderr)
    print(f"{ok}/{total} prints gerados em {ASSETS}")
    print(f"fontes .ps em {SRC_PS} (gitignored — abra e use o CodeSnap pra refazer um na mão)")


if __name__ == "__main__":
    main()
