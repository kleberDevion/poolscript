"""
Módulo `mail` da PoolScript — envio de emails via SMTP.

Baseado em smtplib (stdlib do Python). Suporta:
- MailServer: conexão + login + envio (auto-mapeia gmail/yahoo/outlook/hotmail)
- MailMessage: construção MIME profissional, HTML, anexos (PDF/imagens)

Uso em PoolScript:
    import mail
    server = mail.MailServer()
    server.conn("gmail.com")
    server.login("user@gmail.com", "senha-de-app")

    msg = mail.MailMessage()
    msg.from_address("user@gmail.com")
    msg.to("destino@site.com")
    msg.subject("Teste")
    msg.body("Olá <b>mundo</b>", true)   # true = HTML
    msg.attach("relatorio.pdf")

    server.send(msg)
    server.quit()
"""
from __future__ import annotations
import smtplib
import os
from email.mime.text import MIMEText
from email.mime.multipart import MIMEMultipart
from email.mime.base import MIMEBase
from email import encoders


# Mapeamento auto pra provedores comuns: usuário só passa "gmail.com"
HOSTS_CONFIG = {
    "gmail.com": ("smtp.gmail.com", 587),
    "yahoo.com": ("smtp.mail.yahoo.com", 587),
    "outlook.com": ("smtp.office365.com", 587),
    "hotmail.com": ("smtp.office365.com", 587),
    "live.com": ("smtp.office365.com", 587),
    "proton.me": ("smtp.protonmail.ch", 587),
}


class MailServer:
    """Conexão SMTP com TLS automático."""

    def __init__(self):
        self.server = None
        self.user = None

    def conn(self, provider_or_host, port=None):
        if provider_or_host in HOSTS_CONFIG:
            host, port = HOSTS_CONFIG[provider_or_host]
        else:
            host = provider_or_host
            port = port or 587
        self.server = smtplib.SMTP(host, int(port))
        self.server.starttls()
        return True

    def login(self, user, password):
        if self.server is None:
            raise RuntimeError("chame .conn() antes de .login()")
        self.user = user
        self.server.login(user, password)
        return True

    def send(self, to_or_msg, subject=None, body=None, html=False):
        """
        Duas formas:
          server.send(msg)                                  # objeto MailMessage
          server.send("dest@x.com", "subj", "body", html)   # direto
        """
        if self.server is None:
            raise RuntimeError("chame .conn() antes de .send()")
        if isinstance(to_or_msg, MailMessage):
            mime = to_or_msg.msg
            if not mime.get("From") and self.user:
                mime["From"] = self.user
            self.server.send_message(mime)
            return True
        # forma direta
        mime = MIMEMultipart()
        mime["From"] = self.user or ""
        mime["To"] = to_or_msg
        mime["Subject"] = subject or ""
        tipo = "html" if html else "plain"
        mime.attach(MIMEText(body or "", tipo, "utf-8"))
        self.server.send_message(mime)
        return True

    def quit(self):
        if self.server:
            try:
                self.server.quit()
            except Exception:
                pass
            self.server = None
        return True


class MailMessage:
    """Construtor MIME para emails ricos (HTML + anexos)."""

    def __init__(self):
        self.msg = MIMEMultipart()

    def from_address(self, address):
        self.msg["From"] = address
        return self

    def to(self, address):
        self.msg["To"] = address
        return self

    def subject(self, title):
        self.msg["Subject"] = title
        return self

    def body(self, content, is_html=False):
        tipo = "html" if is_html else "plain"
        self.msg.attach(MIMEText(content, tipo, "utf-8"))
        return self

    def attach(self, file_or_path):
        """Aceita caminho string, PoolFile (binário) ou PoolFileUpload (upload)."""
        # PoolFile ou PoolFileUpload — já tem os bytes em memória
        if hasattr(file_or_path, "_data") or hasattr(file_or_path, "_bytes"):
            data     = getattr(file_or_path, "_data", None) or getattr(file_or_path, "_bytes", None)
            filename = file_or_path.name
        elif isinstance(file_or_path, str):
            # caminho de arquivo
            if not os.path.exists(file_or_path):
                raise FileNotFoundError(f"arquivo não encontrado: {file_or_path}")
            filename = os.path.basename(file_or_path)
            with open(file_or_path, "rb") as f:
                data = f.read()
        else:
            raise TypeError(f"tipo não suportado para anexo: {type(file_or_path).__name__}")

        part = MIMEBase("application", "octet-stream")
        part.set_payload(data)
        encoders.encode_base64(part)
        part.add_header(
            "Content-Disposition",
            f"attachment; filename={filename}",
        )
        self.msg.attach(part)
        return True

    def get_as_string(self):
        return self.msg.as_string()


EXPORTS = {
    "MailServer": MailServer,
    "MailMessage": MailMessage,
}
