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
import imaplib
import email
import os
from email.mime.text import MIMEText
from email.mime.multipart import MIMEMultipart
from email.mime.base import MIMEBase
from email import encoders
from email.header import decode_header


# Mapeamento auto pra provedores comuns: usuário só passa "gmail.com"
HOSTS_CONFIG = {
    "gmail.com": ("smtp.gmail.com", 587),
    "yahoo.com": ("smtp.mail.yahoo.com", 587),
    "outlook.com": ("smtp.office365.com", 587),
    "hotmail.com": ("smtp.office365.com", 587),
    "live.com": ("smtp.office365.com", 587),
    "proton.me": ("smtp.protonmail.ch", 587),
}

# Mapeamento auto pra IMAP (leitura) — mesmas chaves de provedor do HOSTS_CONFIG
IMAP_HOSTS_CONFIG = {
    "gmail.com": ("imap.gmail.com", 993),
    "yahoo.com": ("imap.mail.yahoo.com", 993),
    "outlook.com": ("outlook.office365.com", 993),
    "hotmail.com": ("outlook.office365.com", 993),
    "live.com": ("outlook.office365.com", 993),
}


def _decode_mime_header(raw_value):
    """Decodifica header MIME (assunto/remetente) que pode vir em base64/quoted-printable."""
    if not raw_value:
        return ""
    partes = decode_header(raw_value)
    texto = ""
    for fragmento, encoding in partes:
        if isinstance(fragmento, bytes):
            texto += fragmento.decode(encoding or "utf-8", errors="replace")
        else:
            texto += fragmento
    return texto


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


class MailReader:
    """Conexão IMAP com TLS automático — busca e decodifica e-mails recebidos."""

    _CRITERIOS_SEM_TERM = ("ALL", "UNSEEN")
    _CRITERIOS_COM_TERM = ("SUBJECT", "FROM", "SINCE")

    def __init__(self):
        self.server = None
        self.user = None
        self.folder = None

    def conn(self, provider_or_host, port=None):
        if provider_or_host in IMAP_HOSTS_CONFIG:
            host, port = IMAP_HOSTS_CONFIG[provider_or_host]
        else:
            host = provider_or_host
            port = port or 993
        self.server = imaplib.IMAP4_SSL(host, int(port))
        return True

    def login(self, user, password):
        if self.server is None:
            raise RuntimeError("chame .conn() antes de .login()")
        self.user = user
        self.server.login(user, password)
        return True

    def select(self, folder="INBOX", readonly=True):
        if self.server is None:
            raise RuntimeError("chame .conn() e .login() antes de .select()")
        typ, _ = self.server.select(folder, readonly=readonly)
        if typ != "OK":
            raise RuntimeError(f"não foi possível selecionar a pasta '{folder}'")
        self.folder = folder
        return self

    def search(self, criterion_type="ALL", term=None, limit=None):
        if self.server is None or self.folder is None:
            raise RuntimeError("chame .select() antes de .search()")

        criterio = (criterion_type or "ALL").upper()
        if criterio in self._CRITERIOS_SEM_TERM:
            typ, data = self.server.search(None, criterio)
        elif criterio in self._CRITERIOS_COM_TERM:
            if not term:
                raise ValueError(f'.search("{criterio}") exige o argumento term')
            typ, data = self.server.search(None, criterio, f'"{term}"')
        else:
            raise ValueError(
                f"criterion_type desconhecido: '{criterion_type}'. "
                f"Use ALL, UNSEEN, SUBJECT, FROM ou SINCE."
            )

        if typ != "OK":
            raise RuntimeError(f"falha na busca IMAP: {typ}")

        ids = data[0].split()
        if limit:
            ids = ids[-int(limit):]

        resultados = []
        for eid in ids:
            typ, msg_data = self.server.fetch(eid, "(RFC822.HEADER)")
            if typ != "OK" or not msg_data or not msg_data[0]:
                continue
            msg = email.message_from_bytes(msg_data[0][1])
            resultados.append({
                "id": eid.decode(),
                "from": _decode_mime_header(msg.get("From", "")),
                "subject": _decode_mime_header(msg.get("Subject", "")),
                "date": msg.get("Date", ""),
            })
        return resultados

    def close(self):
        if self.server:
            try:
                if self.folder:
                    self.server.close()
            except Exception:
                pass
            try:
                self.server.logout()
            except Exception:
                pass
            self.server = None
            self.folder = None
        return True


EXPORTS = {
    "MailServer": MailServer,
    "MailMessage": MailMessage,
    "MailReader": MailReader,
}
