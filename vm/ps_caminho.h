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

#include <stdio.h>
#include <string.h>

/* O maior caminho que o motor guarda: o do Linux (PATH_MAX). A pasta do
 * script e a do módulo, os caminhos da busca do import e o arquivo do quadro
 * do erro cabem nele. Eram 512 e 1024: numa pasta de 524 caracteres a pasta
 * do script saía cortada, e o `import vizinho` dava "No module named". */
#define PS_CAMINHO_MAX 4096

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

/* ── o texto do caminho numa frase de erro ────────────────────────────────
 * O começo do texto: até 40 caracteres, com a quebra de linha, o tab e o
 * byte nulo visíveis (`\n`, `\r`, `\t`, `\x00`), sem partir caractere. O teto
 * em bytes é o `cap` do destino, escolhido por quem chama pra frase inteira
 * caber no erro (256 bytes) mesmo com 40 caracteres de 4 bytes. 1 = sobrou
 * texto (a frase põe "..."). */
static inline int ps_trecho_visivel(char *out, size_t cap, const char *s, size_t len)
{
    const unsigned char *q = (const unsigned char *)s, *fim = q + len;
    size_t t = 0;
    for (int nc = 0; q < fim && nc < 40; nc++) {
        char esc[4];
        const char *de = (const char *)q;
        size_t n, anda;
        if (*q == '\n' || *q == '\r' || *q == '\t' || *q == '\0') {
            esc[0] = '\\';
            if (*q == '\0') { memcpy(esc + 1, "x00", 3); n = 4; }
            else { esc[1] = *q == '\n' ? 'n' : *q == '\r' ? 'r' : 't'; n = 2; }
            de = esc;
            anda = 1;
        } else {
            n = *q < 0x80 ? 1 : (*q >> 5) == 0x6 ? 2 : (*q >> 4) == 0xE ? 3 : (*q >> 3) == 0x1E ? 4 : 1;
            if (n > (size_t)(fim - q)) n = (size_t)(fim - q);
            anda = n;
        }
        if (t + n >= cap) break;
        memcpy(out + t, de, n);
        t += n;
        q += anda;
    }
    out[t] = '\0';
    return q < fim;
}

/* O texto da Jinga guarda o byte nulo (a string tem tamanho) e o C corta
 * nele: um caminho com `\0` chegava ao sistema só com o pedaço de antes e
 * abria, gravava ou apagava OUTRO arquivo, calado. Nenhum sistema aceita esse
 * byte em nome de arquivo nem em argumento de programa. Com o byte em `s`,
 * escreve em `buf` a frase com a posição (o índice do texto, em caractere) e
 * o texto com o `\x00` visível, e devolve 1; sem ele, 0. Byte longe do
 * começo: o trecho começa 20 caracteres antes dele, com "..." na frente, pra
 * frase MOSTRAR o byte que acusa. `oque` nomeia o argumento ("o caminho",
 * "o destino"); `nenhum`, o que o sistema não aceita ("nome de arquivo"). Um
 * lugar só pro motor e pro compilador (o import). */
static inline int ps_frase_byte_nulo(char *buf, size_t cap, const char *oque, const char *nenhum,
                                     const char *s, size_t len)
{
    const char *z = memchr(s, '\0', len);
    if (!z) return 0;
    long long pos = 0;
    for (const unsigned char *q = (const unsigned char *)s; q < (const unsigned char *)z; q++)
        if ((*q & 0xC0) != 0x80) pos++;
    const char *ini = s;
    if (pos > 30)
        for (long long pula = pos - 20; pula > 0; ini++)
            if (((unsigned char)ini[1] & 0xC0) != 0x80) pula--;
    /* 112: com o rótulo e o "nenhum" mais longos (27 e 28 bytes), a frase
     * inteira fica abaixo dos 256 do erro */
    char trecho[112];
    int sobra = ps_trecho_visivel(trecho, sizeof(trecho), ini, len - (size_t)(ini - s));
    snprintf(buf, cap, "%s tem o byte nulo \\x00 na posicao %lld: nenhum %s tem esse caractere — '%s%s%s'",
             oque, pos, nenhum, ini > s ? "..." : "", trecho, sobra ? "..." : "");
    return 1;
}

#endif
