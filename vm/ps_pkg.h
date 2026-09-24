/*
 * Gerenciador de pacotes da Jinga em C — só lib/comando `.pr`.
 *
 * Gerência de pacotes: instala `.pr` como comando global
 * (com shim em ~/.jinga/bin) ou como lib importável (~/.jinga/libs,
 * onde o `import` da VM já procura). Estado em ~/.jinga/ (ou
 * $JINGA_HOME), no layout compartilhado por `jpkg` e `jinga`
 * concordarem. Origem pode ser arquivo local (`nome.pr`) ou o registry
 * configurado (baixa por HTTPS via ps_http, com verificação sha256 opcional).
 */
#ifndef PS_PKG_H
#define PS_PKG_H

/* modo de install / categoria de uninstall */
#define PS_PKG_AUTO (-1)   /* decide pelo marcador #!lib/#!cmd (install) ou acha sozinho (uninstall) */
#define PS_PKG_CMD  0
#define PS_PKG_LIB  1

/* Todos imprimem a mensagem (stdout em sucesso, stderr em erro) e devolvem
 * 0 em sucesso, !=0 em erro. */
int ps_pkg_install(const char *target, int modo);
int ps_pkg_uninstall(const char *nome, int categoria);
int ps_pkg_list(void);
int ps_pkg_registry(int argc, char **argv);   /* set-url <url> | show */

#endif /* PS_PKG_H */
