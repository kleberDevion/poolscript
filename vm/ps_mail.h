/*
 * SMTP, IMAP e MIME em C — sem `Python.h`.
 *
 * O TLS vem do OpenSSL do sistema, SEM verificação de certificado: é o que o
 * smtplib/imaplib do interpretador fazem por padrão (contexto stdlib), e
 * divergir aqui faria o mesmo script conectar num motor e falhar no outro.
 *
 * Erro: as funções devolvem -1 (ou NULL) e escrevem a mensagem em `erro` —
 * quem converte pra DatabaseError/OSError/etc. é a VM, não esta camada.
 */
#ifndef PS_MAIL_H
#define PS_MAIL_H

#include <stddef.h>

typedef struct PSMailConn PSMailConn;

/* ── SMTP ───────────────────────────────────────────────────────────────── */
/* Conecta, lê o greeting, EHLO, STARTTLS, EHLO de novo. */
PSMailConn *ps_smtp_conecta(const char *host, int porta, char *erro, size_t cap);
/* AUTH PLAIN se anunciado, senão AUTH LOGIN. */
int  ps_smtp_login(PSMailConn *c, const char *user, const char *senha,
                   char *erro, size_t cap);
/* MAIL FROM + RCPT TO (um por endereço em `para`, separados por vírgula) +
 * DATA. A mensagem chega com \n e sai no fio com \r\n e dot-stuffing. */
int  ps_smtp_envia(PSMailConn *c, const char *de, const char *para,
                   const char *msg, size_t n, char *erro, size_t cap);
/* QUIT + fecha. Nunca falha — erro de despedida não interessa a ninguém. */
void ps_smtp_quit(PSMailConn *c);

/* ── IMAP (TLS direto na porta, sem STARTTLS) ──────────────────────────── */
PSMailConn *ps_imap_conecta(const char *host, int porta, char *erro, size_t cap);
int  ps_imap_login(PSMailConn *c, const char *user, const char *senha,
                   char *erro, size_t cap);
/* `readonly` usa EXAMINE em vez de SELECT. */
int  ps_imap_select(PSMailConn *c, const char *pasta, int readonly,
                    char *erro, size_t cap);
/* `termo` NULL para critérios sem termo (ALL, UNSEEN). Devolve os ids
 * separados por espaço em `*ids` (malloc — quem chama libera). */
int  ps_imap_search(PSMailConn *c, const char *criterio, const char *termo,
                    char **ids, char *erro, size_t cap);
/* A mensagem RFC822 (ou só o header, com `corpo`=0) em `*msg` (malloc). */
int  ps_imap_fetch(PSMailConn *c, const char *id, int corpo,
                   char **msg, size_t *n, char *erro, size_t cap);
/* CLOSE (se houve select) + LOGOUT + fecha. */
void ps_imap_close(PSMailConn *c, int teve_select);

/* Libera sem protocolo nenhum — é o caminho do GC. */
void ps_mail_solta(PSMailConn *c);

/* ── MIME (parse do que o IMAP devolve) ─────────────────────────────────── */
/* Valor de um header (case-insensitive, já desdobrado). malloc; NULL se o
 * header não existe. */
char *ps_mime_header(const char *msg, size_t n, const char *nome);
/* Decodifica as palavras RFC 2047 (=?charset?B/Q?...?=) pra UTF-8. malloc. */
char *ps_mime_decodifica_header(const char *v, size_t n);
/* O corpo da mensagem: prefere text/plain, cai pra text/html, pula anexos.
 * Sai em UTF-8 (latin-1 é convertido). malloc; "" se não há corpo. */
char *ps_mime_corpo(const char *msg, size_t n);

#endif /* PS_MAIL_H */
