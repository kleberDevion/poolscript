#!/usr/bin/env python3
"""IMAP de mentira com TLS de verdade, pra testar a metade LEITORA de `ps_mail.c`.

POR QUE ISTO EXISTE: `vm/ps_mail.c` tem duas metades — SMTP (enviar) e IMAP
(ler, o `MailReader`). O `teste/e2e/mail_local.ps` cobriu a primeira com o
`smtp_falso.py` e o arquivo saiu de 0,6% pra 16,9% de ramo. A segunda continuava
INTEIRA sem teste: `ps_imap_conecta`, `login`, `select`, `search` (com e sem
literal), `fetch` e `close` nunca tinham rodado, porque o único teste de leitura
fala com o Gmail e diz PULOU sem credencial.

Diferença importante pro SMTP: o IMAP aqui usa TLS IMPLÍCITO — `ps_imap_conecta`
chama `liga_tls` logo depois do `connect`, sem STARTTLS. Então o servidor tem
que fazer o handshake antes de dizer qualquer coisa. Isso também deixa testar,
do outro lado, que o certificado auto-assinado é RECUSADO sem
`PS_MAIL_TLS_INSEGURO=1`.

O diálogo segue o que o motor realmente manda (ver `vm/ps_mail.c`):

    S: * OK boas-vindas
    C: A1 LOGIN "user" "senha"
    C: A2 SELECT "INBOX"          (ou EXAMINE, quando readonly)
    C: A3 SEARCH ALL
       ou  A3 SEARCH CHARSET UTF-8 SUBJECT {12}   -> S: +  -> C: <termo>
    C: A4 FETCH 2 (RFC822)        (ou RFC822.HEADER, quando sem corpo)
    C: A5 CLOSE / A6 LOGOUT

    python3 teste/imap_falso.py 0 <cert.pem> <key.pem> <caixa.txt>

Porta 0 = o sistema escolhe uma livre e a primeira linha da caixa vira
`PRONTO <porta>` — porta fixa colide com a sobra da rodada anterior.
"""
import socket
import ssl
import sys
import threading

USUARIO = "teste@local"
SENHA = "segredo123"

# Duas mensagens na caixa. A segunda tem acento pra o FETCH provar que o
# literal `{n}` conta BYTES e não caracteres — cortar por caractere aqui
# entregaria a mensagem truncada no meio de um UTF-8.
# A caixa é escolhida pra exercitar o DECODIFICADOR de cabeçalho e de corpo do
# `ps_mail.c`, que é a maior região descoberta do arquivo (90 ramos). Cada
# mensagem aciona um caminho diferente do RFC 2047 / RFC 2045:
#
#   1  cabeçalho ASCII puro, corpo em texto — o caso base
#   2  corpo com acento em UTF-8 — o literal `{n}` tem que contar BYTES
#   3  assunto em `=?UTF-8?B?...?=` (base64) — o ramo b64_decode do cabeçalho
#   4  assunto em `=?ISO-8859-1?Q?...?=` (quoted-printable + latin-1) — dois
#      ramos de uma vez: o `_` que vira espaço e o `charset_eh_latin1`, que
#      precisa reexpandir cada byte pra UTF-8
#   5  corpo MULTIPART com uma parte texto e uma anexada — o parser de corpo
#   6  corpo em base64 declarado por `Content-Transfer-Encoding`
CAIXA = {
    "1": ("From: um@local\r\nTo: teste@local\r\nSubject: primeiro\r\n"
          "\r\ncorpo do primeiro\r\n"),
    "2": ("From: dois@local\r\nTo: teste@local\r\nSubject: acentuada\r\n"
          "\r\ncorpo com ção e ê\r\n"),
    # "relatório de vendas" em base64 UTF-8
    "3": ("From: tres@local\r\nTo: teste@local\r\n"
          "Subject: =?UTF-8?B?cmVsYXTDs3JpbyBkZSB2ZW5kYXM=?=\r\n"
          "\r\ncorpo do terceiro\r\n"),
    # "não é fácil" em quoted-printable latin-1; `_` é espaço no cabeçalho
    "4": ("From: quatro@local\r\nTo: teste@local\r\n"
          "Subject: =?ISO-8859-1?Q?n=E3o_=E9_f=E1cil?=\r\n"
          "\r\ncorpo do quarto\r\n"),
    "5": ("From: cinco@local\r\nTo: teste@local\r\nSubject: com anexo\r\n"
          "MIME-Version: 1.0\r\n"
          "Content-Type: multipart/mixed; boundary=\"LIMITE\"\r\n"
          "\r\n--LIMITE\r\n"
          "Content-Type: text/plain; charset=utf-8\r\n\r\n"
          "parte de texto\r\n"
          "--LIMITE\r\n"
          "Content-Type: application/octet-stream\r\n"
          "Content-Disposition: attachment; filename=\"a.bin\"\r\n\r\n"
          "dados anexos\r\n"
          "--LIMITE--\r\n"),
    # "corpo em base64" com Content-Transfer-Encoding
    "6": ("From: seis@local\r\nTo: teste@local\r\nSubject: b64\r\n"
          "Content-Type: text/plain; charset=utf-8\r\n"
          "Content-Transfer-Encoding: base64\r\n"
          "\r\nY29ycG8gZW0gYmFzZTY0\r\n"),
}


def atende(sock, ev):
    """Um diálogo IMAP completo, já dentro do TLS."""
    resto = b""

    def linha():
        nonlocal resto
        while b"\r\n" not in resto:
            p = sock.recv(4096)
            if not p:
                return None
            resto += p
        i = resto.index(b"\r\n")
        out, resto = resto[:i], resto[i + 2:]
        return out.decode("utf-8", "replace")

    def manda(txt):
        sock.sendall(txt.encode("utf-8"))

    manda("* OK imap falso pronto\r\n")
    while True:
        cru = linha()
        if cru is None:
            return
        ev.append("C " + cru)
        partes = cru.split(" ", 2)
        if len(partes) < 2:
            continue
        tag, cmd = partes[0], partes[1].upper()
        arg = partes[2] if len(partes) > 2 else ""

        if cmd == "LOGIN":
            # vem entre aspas, escapado pelo `imap_aspas`
            campos = [c.strip('"') for c in arg.split(" ")]
            if len(campos) >= 2 and campos[0] == USUARIO and campos[1] == SENHA:
                ev.append("LOGIN_OK")
                manda(tag + " OK autenticado\r\n")
            else:
                ev.append("LOGIN_NEGADO " + arg)
                manda(tag + " NO usuario ou senha errados\r\n")

        elif cmd in ("SELECT", "EXAMINE"):
            ev.append(cmd + " " + arg)
            manda("* %d EXISTS\r\n* 0 RECENT\r\n" % len(CAIXA))
            manda("* OK [UIDVALIDITY 1] ok\r\n")
            # EXAMINE é o modo somente-leitura; o motor escolhe pelo `readonly`
            modo = "READ-ONLY" if cmd == "EXAMINE" else "READ-WRITE"
            manda(tag + " OK [" + modo + "] selecionada\r\n")

        elif cmd == "SEARCH":
            # Duas formas: `SEARCH ALL` direto, e `SEARCH CHARSET UTF-8 <crit>
            # {n}` que espera um `+` e recebe o termo cru na linha seguinte —
            # é o caminho do termo com acento, que não cabe em ASCII na linha.
            if arg.endswith("}") and "{" in arg:
                manda("+ manda o literal\r\n")
                termo = linha()
                ev.append("SEARCH_LITERAL " + str(arg) + " termo=" + str(termo))
                manda("* SEARCH 2\r\n")
            else:
                ev.append("SEARCH " + arg)
                manda("* SEARCH " + " ".join(sorted(CAIXA)) + "\r\n")
            manda(tag + " OK busca feita\r\n")

        elif cmd == "FETCH":
            campos = arg.split(" ", 1)
            ident = campos[0]
            item = campos[1].strip("()") if len(campos) > 1 else "RFC822"
            msg = CAIXA.get(ident)
            if msg is None:
                ev.append("FETCH_INEXISTENTE " + ident)
                manda(tag + " NO mensagem nao existe\r\n")
                continue
            if item.upper().startswith("RFC822.HEADER"):
                msg = msg.split("\r\n\r\n")[0] + "\r\n\r\n"
            bruto = msg.encode("utf-8")
            ev.append("FETCH " + ident + " " + item + " " + str(len(bruto)) + "B")
            manda("* %s FETCH (%s {%d}\r\n" % (ident, item, len(bruto)))
            sock.sendall(bruto)
            manda(")\r\n")
            manda(tag + " OK entregue\r\n")

        elif cmd == "CLOSE":
            ev.append("CLOSE")
            manda(tag + " OK fechada\r\n")

        elif cmd == "LOGOUT":
            ev.append("LOGOUT")
            manda("* BYE tchau\r\n" + tag + " OK ate mais\r\n")
            return

        else:
            ev.append("DESCONHECIDO " + cmd)
            manda(tag + " BAD nao entendi\r\n")


def main():
    porta = int(sys.argv[1])
    cert, chave, caixa = sys.argv[2], sys.argv[3], sys.argv[4]

    ctx = ssl.SSLContext(ssl.PROTOCOL_TLS_SERVER)
    ctx.load_cert_chain(cert, chave)

    srv = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    srv.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    srv.bind(("127.0.0.1", porta))
    srv.listen(8)
    srv.settimeout(60)

    with open(caixa, "w") as f:
        f.write("PRONTO %d\n" % srv.getsockname()[1])

    def laco():
        while True:
            try:
                bruto, _ = srv.accept()
            except (socket.timeout, OSError):
                return
            ev = []
            try:
                # TLS IMPLÍCITO: o handshake vem ANTES de qualquer byte de
                # protocolo. Handshake recusado é resultado esperado em metade
                # dos testes — é o cliente rejeitando o certificado.
                sock = ctx.wrap_socket(bruto, server_side=True)
            except (ssl.SSLError, OSError) as e:
                ev.append("HANDSHAKE_RECUSADO " + type(e).__name__)
                with open(caixa, "a") as f:
                    f.write("\n".join(ev) + "\n--- fim do dialogo\n")
                continue
            ev.append("TLS_OK")
            try:
                atende(sock, ev)
            except Exception as e:                 # servidor de teste não cai
                ev.append("EXCECAO " + type(e).__name__ + " " + str(e))
            finally:
                try:
                    sock.close()
                except OSError:
                    pass
            with open(caixa, "a") as f:
                f.write("\n".join(ev) + "\n--- fim do dialogo\n")

    t = threading.Thread(target=laco, daemon=True)
    t.start()
    t.join(60)


if __name__ == "__main__":
    main()
