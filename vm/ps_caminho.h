/* ps_caminho.h — caminho de arquivo nos três sistemas: o que separa pasta e o
 * que é absoluto. Só texto, sem chamada de sistema: quem precisa (o motor, o
 * compilador, o jpkg) inclui sem arrastar os cabeçalhos do sistema.
 *
 * SEPARADOR. `/` separa pasta em todo sistema; no Windows `\` também — é o que
 * o `os.cwd()`, o `sys.argv[0]`, o editor (o LSP manda `C:\...`) e o próprio
 * usuário escrevem lá. Tirar o nome ou a pasta de um caminho só por `/` fazia,
 * no Windows, `C:\proj\main.pr` virar um nome sem pasta: o módulo vizinho não
 * era achado, o `jpkg install C:\x\f.pr` gravava num nome com `C:\` dentro, o
 * `.env` da pasta de cima não era lido, as pastas pai de `C:\x\novo\f.txt` não
 * eram criadas. Quem tira nome ou pasta de um caminho, ou pergunta se ele
 * termina em separador, usa estas funções — nunca `strrchr(s, '/')`. */
#ifndef PS_CAMINHO_H
#define PS_CAMINHO_H

#include <string.h>

static inline int ps_eh_sep(char c)
{
#if defined(_WIN32)
    return c == '/' || c == '\\';
#else
    return c == '/';
#endif
}

/* O último separador de `s`, ou NULL: o `strrchr(s, '/')` que conhece os dois. */
static inline char *ps_ultimo_sep(const char *s)
{
    const char *u = NULL;
    if (s) for (const char *p = s; *p; p++) if (ps_eh_sep(*p)) u = p;
    return (char *)u;
}

/* `pasta/` (e no Windows `pasta\`): quem grava num caminho assim quer a pasta. */
static inline int ps_termina_em_sep(const char *s)
{
    size_t n = s ? strlen(s) : 0;
    return n > 0 && ps_eh_sep(s[n - 1]);
}

/* ── caminho absoluto ─────────────────────────────────────────────────────
 * 1 = o caminho NÃO se junta com uma raiz de busca (a pasta do script, a
 * corrente, a do módulo). No Linux e no macOS é o que começa com `/`; no
 * Windows também `C:\` e `C:/` (letra de unidade) e `\` ou `\\servidor\...`
 * (raiz da unidade e UNC). Era `p[0] == '/'` em quatro lugares, e no Windows
 * o `os.loadFile(os.cwd() + "/x")`, o `import "C:/..."`, o `static_folder`
 * absoluto do jinker e o `file=` do request caíam na busca e não achavam.
 * `C:x` (relativo à pasta corrente daquela unidade) não é absoluto. */
static inline int ps_caminho_absoluto(const char *p)
{
    if (!p || !p[0]) return 0;
    if (p[0] == '/') return 1;
#if defined(_WIN32)
    if (p[0] == '\\') return 1;
    if (((p[0] >= 'A' && p[0] <= 'Z') || (p[0] >= 'a' && p[0] <= 'z'))
            && p[1] == ':' && (p[2] == '/' || p[2] == '\\'))
        return 1;
#endif
    return 0;
}

#endif
