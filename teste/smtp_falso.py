#!/usr/bin/env python3
"""SMTP de mentira com STARTTLS de verdade, pra testar `vm/ps_mail.c` offline.

POR QUE ISTO EXISTE: `ps_mail.c` são 943 linhas em **0,6% de cobertura**. O
único teste de mail (`teste/e2e/mail.ps`) fala com o Gmail e diz PULOU sem
credencial — ou seja, na prática nunca roda. Todo o SMTP (greeting, EHLO,
STARTTLS, AUTH LOGIN, MAIL FROM/RCPT TO/DATA, QUIT) e todo o tratamento de
erro nunca foram exercitados.

Não dá pra testar isso sem TLS: `ps_smtp_conecta` exige STARTTLS e aborta se o
servidor não oferecer — de propósito, e está certo. Então o servidor falso faz
o handshake pra valer, com certificado auto-assinado.

Isso permite o teste que mais importa e que nenhum outro alcança: **conectar
sem `PS_MAIL_TLS_INSEGURO=1` tem que FALHAR**, porque o certificado não é
confiável. É a prova de que `SSL_VERIFY_PEER` + `SSL_set1_host` estão fazendo
efeito — antes disso, a única evidência era o diff.

Está em Python porque o assunto é ser um servidor errado de propósito
(certificado inválido, resposta 535, corte no meio do DATA), e a stdlib do
Python já traz `ssl` com tudo isso na mão. A PoolScript fala com ele como
falaria com o Gmail.

    python3 teste/smtp_falso.py 0 <cert.pem> <key.pem> <caixa.txt>

Porta 0 = o sistema escolhe uma livre, e a PRIMEIRA LINHA de `caixa.txt` vira
`PRONTO <porta>`. Porta fixa parece mais simples e não é: duas rodadas em menos
de um minuto colidem com a sobra da anterior, o servidor não sobe e o teste
reprova por motivo que não tem nada a ver com o que ele testa.

Grava em `caixa.txt` o que recebeu, uma linha por evento, pro teste conferir.
Encerra sozinho depois de atender e ver um QUIT (ou em 60 s).
"""
import base64
import os
import socket
import ssl
import sys
import threading

USUARIO = "teste@local"
SENHA = "segredo123"


def atende(bruto, ctx, caixa):
    """Um diálogo SMTP completo. Escreve cada evento em `caixa`."""
    ev = []

    def linha(sock):
        dados = b""
        while not dados.endswith(b"\r\n"):
            p = sock.recv(1)
            if not p:
                return None
            dados += p
        return dados[:-2].decode("utf-8", "replace")

    bruto.sendall(b"220 local.invalido SMTP falso\r\n")
    while True:
        cmd = linha(bruto)
        if cmd is None:
            return ev
        ev.append("CLARO " + cmd)
        alto = cmd.upper()
        if alto.startswith("EHLO"):
            bruto.sendall(b"250-local.invalido\r\n250 STARTTLS\r\n")
        elif alto == "STARTTLS":
            bruto.sendall(b"220 manda o handshake\r\n")
            break
        elif alto == "QUIT":
            bruto.sendall(b"221 tchau\r\n")
            return ev
        else:
            bruto.sendall(b"502 antes do TLS so EHLO/STARTTLS\r\n")

    # daqui pra frente é TLS. Handshake que falha é resultado ESPERADO em
    # metade dos testes (o cliente recusando o certificado auto-assinado).
    try:
        sock = ctx.wrap_socket(bruto, server_side=True)
    except (ssl.SSLError, OSError) as e:
        ev.append("HANDSHAKE_RECUSADO " + type(e).__name__)
        return ev
    ev.append("TLS_OK")

    dentro_do_data = False
    corpo = []
    while True:
        cmd = linha(sock)
        if cmd is None:
            return ev
        if dentro_do_data:
            if cmd == ".":
                dentro_do_data = False
                ev.append("CORPO " + " | ".join(corpo))
                sock.sendall(b"250 aceito\r\n")
            else:
                corpo.append(cmd)
            continue
        ev.append("SEGURO " + cmd)
        alto = cmd.upper()
        if alto.startswith("EHLO"):
            # `SMTP_FALSO_AUTH=login` anuncia SÓ o LOGIN. Serve pra alcançar o
            # outro ramo do `ps_smtp_login`: o motor usa PLAIN quando o EHLO o
            # anuncia e LOGIN quando não — e o caminho do LOGIN (três trocas,
            # usuário e senha em base64 separados) nunca rodava.
            if os.environ.get("SMTP_FALSO_AUTH") == "login":
                sock.sendall(b"250-local.invalido\r\n250 AUTH LOGIN\r\n")
            else:
                sock.sendall(b"250-local.invalido\r\n250 AUTH LOGIN PLAIN\r\n")
        elif alto.startswith("AUTH PLAIN"):
            # RFC 4616: um blob base64 com \0usuario\0senha. O cliente manda
            # junto do comando quando o EHLO anuncia PLAIN.
            blob = cmd[len("AUTH PLAIN"):].strip()
            if not blob:
                sock.sendall(b"334 \r\n")
                blob = linha(sock) or ""
            try:
                partes = base64.b64decode(blob).split(b"\x00")
            except Exception:
                partes = []
            u = partes[1].decode("utf-8", "replace") if len(partes) > 2 else ""
            s_ = partes[2].decode("utf-8", "replace") if len(partes) > 2 else ""
            if u == USUARIO and s_ == SENHA:
                ev.append("LOGIN_OK " + u)
                sock.sendall(b"235 autenticado\r\n")
            else:
                ev.append("LOGIN_NEGADO " + u)
                sock.sendall(b"535 usuario ou senha errados\r\n")
        elif alto == "AUTH LOGIN":
            sock.sendall(b"334 " + base64.b64encode(b"Username:") + b"\r\n")
            u = base64.b64decode(linha(sock) or "").decode("utf-8", "replace")
            sock.sendall(b"334 " + base64.b64encode(b"Password:") + b"\r\n")
            s = base64.b64decode(linha(sock) or "").decode("utf-8", "replace")
            if u == USUARIO and s == SENHA:
                ev.append("LOGIN_OK " + u)
                sock.sendall(b"235 autenticado\r\n")
            else:
                ev.append("LOGIN_NEGADO " + u)
                sock.sendall(b"535 usuario ou senha errados\r\n")
        elif alto.startswith("MAIL FROM"):
            sock.sendall(b"250 remetente ok\r\n")
        elif alto.startswith("RCPT TO"):
            sock.sendall(b"250 destinatario ok\r\n")
        elif alto == "DATA":
            dentro_do_data = True
            sock.sendall(b"354 manda, termina com ponto\r\n")
        elif alto == "QUIT":
            sock.sendall(b"221 tchau\r\n")
            return ev
        else:
            sock.sendall(b"500 nao entendi\r\n")


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

    # O arquivo existir é o sinal de "estou de pé" pro teste esperar sem sleep,
    # e a porta escolhida vai junto porque com `bind(0)` só o servidor sabe.
    with open(caixa, "w") as f:
        f.write("PRONTO %d\n" % srv.getsockname()[1])

    def laco():
        while True:
            try:
                cli, _ = srv.accept()
            except (socket.timeout, OSError):
                return
            try:
                ev = atende(cli, ctx, caixa)
            except Exception as e:              # servidor de teste não cai
                ev = ["EXCECAO " + type(e).__name__ + " " + str(e)]
            finally:
                try:
                    cli.close()
                except OSError:
                    pass
            with open(caixa, "a") as f:
                for e in ev:
                    f.write(e + "\n")
                f.write("--- fim do dialogo\n")

    t = threading.Thread(target=laco, daemon=True)
    t.start()
    t.join(60)


if __name__ == "__main__":
    main()
