"""Robustez: cada metodo chamado com ARIDADE errada e TIPO errado em cada
posicao. O contrato e simples e vale pra todos:

  - erro capturavel por try/catch (nao crash, nao silencio)
  - a VM NAO morre (sinal)
  - a mensagem diz o nome do metodo

Nao precisa de oraculo: "chamada errada tem que reclamar" e invariante.
"""
import json, subprocess, pathlib
S = "/tmp/claude-1000/-home-kleberdevion-poolscript-lang/6d969e59-69de-417b-8aa4-653e4a35fc16/scratchpad/robusto"
d = json.loads(subprocess.run(["./pool","--metadata"],capture_output=True,text=True).stdout)

# receptores construiveis sem I/O
ALVO = {
 "str": '"abc"', "list": '[1, 2]', "dict": '{"a": 1}', "tup": '(1, 2)',
 "bytes": '"oi".encode()', "Pattern": 'regex.compile("a")',
 "MailMessage": 'mail.MailMessage()', "MailServer": 'mail.MailServer()',
 "MailReader": 'mail.MailReader()', "Jinker": 'jinker.Jinker(name="r")',
 "JinkerResponse": 'jinker.JinkerResponse()', "QRImage": 'qrcode.make("x")',
 "UI": 'guzer.UI()', "socket": 'sockets.socket()',
}
IMPORTS = "import regex\nimport mail\nimport jinker\nimport qrcode\nimport guzer\nimport sockets\n"
# valores de tipo ERRADO pra empurrar em cada posicao
LIXO = ['Null', '42', '-1', '3.5', 'true', '"texto"', '[1]', '{"k": 1}', '(1,)']

casos = []
for tipo, expr in ALVO.items():
    for m in d["tipos"].get(tipo, []):
        if m.get("kind") == "property": continue
        nome = m["nome"]
        ps = m.get("params") or []
        obrig = [p for p in ps if not p.get("default")]
        # aridade: 0 quando exige, e um a mais que o maximo
        if obrig:
            casos.append((tipo, nome, "sem argumento", f'{expr}.{nome}()'))
        demais = ", ".join(["1"] * (len(ps) + 2))
        casos.append((tipo, nome, "argumentos demais", f'{expr}.{nome}({demais})'))
        # tipo errado em cada posicao obrigatoria
        for i in range(len(obrig)):
            for lixo in LIXO:
                args = []
                for k in range(len(obrig)):
                    args.append(lixo if k == i else "1")
                casos.append((tipo, nome, f"lixo na pos {i}: {lixo}",
                              f'{expr}.{nome}({", ".join(args)})'))
pathlib.Path(f"{S}/casos.json").write_text(json.dumps(casos, ensure_ascii=False))
print("chamadas erradas geradas:", len(casos))
