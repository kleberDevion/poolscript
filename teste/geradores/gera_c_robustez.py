"""Gera `teste/casos_robustez.c`: cada metodo do motor chamado com ARIDADE
errada e TIPO errado em cada posicao.

O contrato nao precisa de oraculo — "chamada errada tem que reclamar" e
invariante:

  - erro capturavel por try/catch (nao crash, nao silencio)
  - a VM NAO morre (o runner faz fork/exec, entao sinal vira falha)
  - a VM NAO trava (o runner tem alarm)

Um caso por TIPO, nao por chamada: sao ~11 mil chamadas erradas, e uma por
subprocesso levaria mais de um minuto. Cada programa roda as chamadas do seu
tipo dentro de try/catch e imprime a lista das que passaram CALADAS. A saida
esperada e essa lista como ela e hoje — aceitacao silenciosa nova, crash ou
trava mudam o resultado e o caso falha.

Rodar:  python3 teste/geradores/gera_c_robustez.py
"""
import json, subprocess, sys, pathlib, tempfile, os

RAIZ = pathlib.Path(__file__).resolve().parents[2]
POOL = str(RAIZ / "pool")

meta = json.loads(subprocess.run([POOL, "--metadata"], capture_output=True,
                                 text=True).stdout)

# receptores construiveis sem I/O
ALVO = {
    "str": '"abc"', "list": '[1, 2]', "dict": '{"a": 1}', "tup": '(1, 2)',
    "bytes": '"oi".encode()', "Pattern": 'regex.compile("a")',
    "MailMessage": 'mail.MailMessage()', "MailServer": 'mail.MailServer()',
    "MailReader": 'mail.MailReader()', "Jinker": 'jinker.Jinker(name="r")',
    "JinkerResponse": 'jinker.JinkerResponse()', "QRImage": 'qrcode.make("x")',
    "UI": 'guzer.UI()', "socket": 'sockets.socket()',
}
IMPORTS = "import os\nimport regex\nimport mail\nimport jinker\nimport qrcode\nimport guzer\nimport sockets\n"

def prologo(tipo):
    """Chamada errada com efeito colateral EXISTE: um QRImage que recebeu a
    string "texto" como caminho salvou um PNG na raiz do repo. Cada tipo roda
    numa pasta propria e VAZIA — se sobrar arquivo de uma rodada anterior, um
    `attach("texto")` que antes errava passa a achar o arquivo e o caso vira
    intermitente."""
    d = f"/tmp/ps_robustez_{tipo}"
    return (f'os.cmd("rm -rf {d}")\n'
            f'os.mkdir("{d}", exist_ok=true)\n'
            f'os.chdir("{d}")\n')
LIXO = ['Null', '42', '-1', '3.5', 'true', '"texto"', '[1]', '{"k": 1}', '(1,)']


def chamadas(tipo):
    """(rotulo, expressao) de toda chamada ERRADA daquele tipo."""
    recv = ALVO[tipo]
    fora = []
    for m in meta["tipos"].get(tipo, []):
        if m.get("kind") == "property":
            continue
        nome = m["nome"]
        ps = m.get("params") or []
        obrig = [p for p in ps if not p.get("default")]
        if obrig:
            fora.append((nome, f'{recv}.{nome}()'))
        demais = ", ".join(["1"] * (len(ps) + 2))
        fora.append((nome, f'{recv}.{nome}({demais})'))
        # lixo em CADA posicao declarada — opcional inclusive: parametro que o
        # motor nem checa aceita qualquer coisa calado, e e isso que se quer ver
        for i in range(len(ps)):
            for lixo in LIXO:
                args = [lixo if k == i else "1" for k in range(len(ps))]
                fora.append((nome, f'{recv}.{nome}({", ".join(args)})'))
    return fora


def programa(tipo, fora):
    """Granularidade de METODO, nao de chamada: uma lista com 9 mil combinacoes
    de argumento nao da sinal nenhum num diff. O que importa e "este metodo
    deixou de reclamar" — e o total, pra o placar nao mudar sem aparecer."""
    L = [IMPORTS + prologo(tipo), "CAL = []", "N = []",
         "action calou_(nome):",
         "    if not CAL.contains(nome):",
         "        CAL.append(nome)",
         "    N.append(1)", ""]
    for met, expr in fora:
        L += ["try:",
              f"    _r = {expr}",
              f'    calou_("{met}")',
              "catch (e):",
              "    pass"]
    L += ["", "CAL.sort()", 'post(len(N), "caladas em", len(CAL), "metodos:", CAL)']
    return "\n".join(L) + "\n"


def cstr(s):
    return ('"' + s.replace("\\", "\\\\").replace('"', '\\"')
                   .replace("\n", "\\n").replace("\t", "\\t") + '"')


out = ['/*',
       ' * GERADO por teste/geradores/gera_c_robustez.py — nao edite na mao.',
       ' *',
       ' * Chamada errada (aridade e tipo) em cada metodo de cada tipo. O caso',
       ' * imprime as chamadas que passaram CALADAS; a saida esperada e a lista',
       ' * de hoje. Aceitacao silenciosa nova, crash ou trava fazem o caso falhar.',
       ' */',
       '#include "ps_teste.h"', '', 'const Caso CASOS_ROBUSTEZ[] = {']

total = 0
mudos_total = 0
for tipo in ALVO:
    fora = chamadas(tipo)
    if not fora:
        print(f"  {tipo:16} sem metodo — pulado")
        continue
    src = programa(tipo, fora)
    with tempfile.NamedTemporaryFile("w", suffix=".ps", delete=False,
                                     dir="/tmp", encoding="utf-8") as f:
        f.write(src)
        cam = f.name
    env = dict(os.environ, GUZER_HEADLESS="1")
    try:
        r = subprocess.run([POOL, cam], capture_output=True, timeout=180,
                           stdin=subprocess.DEVNULL, env=env, cwd=RAIZ)
    finally:
        os.unlink(cam)
    saida = r.stdout.decode("utf-8", "replace").strip()
    if r.returncode != 0 or " caladas em " not in saida:
        sys.exit(f"ERRO gerando {tipo}: rc={r.returncode}\n"
                 f"{r.stderr.decode('utf-8','replace')[:600]}")
    mudos = int(saida.split(" ")[0])
    total += len(fora)
    mudos_total += mudos
    print(f"  {tipo:16} {len(fora):5} chamadas erradas, {mudos} passaram caladas")
    out.append(f'/* {tipo}: {len(fora)} chamadas erradas */')
    out.append(f'{{ "robustez: {tipo}", {cstr(src)}, {cstr(saida)}, NULL, 0 }},')

out.append('};')
out.append('const int NC_ROBUSTEZ = N_CASOS(CASOS_ROBUSTEZ);')
(RAIZ / "teste/casos_robustez.c").write_text("\n".join(out) + "\n")
print(f"\nteste/casos_robustez.c: {total} chamadas erradas, "
      f"{mudos_total} passam caladas")
