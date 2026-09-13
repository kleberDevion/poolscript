/*
 * `pool` — o executável da PoolScript.
 *
 * Roda o fonte pela `ps_roda_fonte`; o erro vira texto no stderr e código de
 * saída. Os comandos de RUNTIME (rodar, repl, build, help…) e os de PACOTE
 * (`psl install`/`uninstall`/`list`/`registry`, implementados em ps_pkg.c)
 * vivem no MESMO binário: `pool` roda, `psl` gerencia pacotes, e o help e a
 * doc respeitam essa divisão.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>   /* chmod — o executavel gerado por `-o` nasce +x */
#include <unistd.h>   /* getcwd — encurta o caminho do traceback pro relativo */

#include "ps_vm.h"
#include "ps_lexer.h"   /* --contexto: o editor pergunta pro lexer, nao pro regex */
#include "ps_parser.h"  /* --ast: e pra ARVORE que ele pergunta, pelo mesmo motivo */
#include "ps_pkg.h"

#include "ps_versao.h"

#define SPEC_URL "https://github.com/kleberDevion/poolscript"

static void ajuda(void)
{
    printf(
"PoolScript %s — PSVM (VM em C, runtime standalone)\n"
"\n"
"Uso:\n"
"  pool arquivo.ps           Roda um arquivo\n"
"  pool arquivo.ps -o nome   Gera um executavel que roda sozinho (sem o fonte\n"
"                            e sem o pool instalado); vale .ps, .p e .psl\n"
"  pool -e \"<codigo>\"        Roda codigo inline (uma linha)\n"
"  pool build                Roda todos os .ps da pasta atual\n"
"  pool --check [arq.ps]     So analisa (nao roda); JSON com o erro. Sem\n"
"                            arquivo, le da entrada padrao\n"
"  pool //doc                Mostra a URL da especificacao\n"
"  pool --contexto L:C       O que o cursor toca (pro editor); fonte no stdin\n"
"  pool --tokens             Tokens do lexer em JSON (pro realce); fonte no stdin\n"
"  pool --ast                A arvore do parser em JSON (pro editor); stdin\n"
"  pool --version / -V       Mostra a versao\n"
"  pool --help / -h          Mostra esta ajuda\n"
"\n"
"Pacotes (lib e comando .ps):\n"
"  psl install <arq.ps>          Instala (o arquivo decide via #!lib / #!cmd)\n"
"  psl install <arq.ps> -asLib   Forca lib importavel (import nome)\n"
"  psl install <nome>            Busca <nome> no registry configurado\n"
"  psl uninstall <nome>          Remove (acha sozinho: comando ou lib)\n"
"  psl uninstall <nome> -asLib   Forca a categoria lib\n"
"  psl list                      Lista comandos e libs instalados\n"
"  psl registry set-url <url>    Configura o indice de pacotes\n"
"  psl registry show             Mostra o registry configurado\n"
"\n"
"Libs internas: %s\n"
"\n"
"Docs: " SPEC_URL "\n", PS_VERSAO, ps_modulos_publicos());
}

/* Lê o arquivo inteiro. Devolve NULL e reclama no stderr se não der. */
static char *le_arquivo(const char *caminho, size_t *tam)
{
    FILE *f = fopen(caminho, "rb");
    if (!f) {
        fprintf(stderr, "pool: nao consegui abrir '%s'\n", caminho);
        return NULL;
    }
    if (fseek(f, 0, SEEK_END) != 0) { fclose(f); return NULL; }
    long n = ftell(f);
    if (n < 0) { fclose(f); return NULL; }
    rewind(f);
    char *buf = malloc((size_t)n + 1);
    if (!buf) { fclose(f); fprintf(stderr, "pool: sem memoria\n"); return NULL; }
    size_t lidos = fread(buf, 1, (size_t)n, f);
    fclose(f);
    buf[lidos] = '\0';
    *tam = lidos;
    return buf;
}

/* Imprime a localização + o trecho do fonte, como o interpretador:
 *     -> arquivo, linha N
 *     | <a linha de código>
 *     | ^
 * Só quando há linha e o arquivo abre (origem "<-e>"/stdin não tem trecho). */
/* Encurta o caminho pra exibição: relativo-ao-cwd se estiver sob ele, senão
 * só o basename — espelha o `_clean_filename` do interpretador (a autoridade),
 * pra que os dois motores mostrem o MESMO nome no traceback. O caminho cheio
 * ainda é usado pra abrir o arquivo e ler o trecho. */
static const char *limpa_arquivo(const char *fn, char *buf, size_t cap)
{
    if (!fn || !fn[0] || fn[0] == '<') return (fn && fn[0]) ? fn : "<script>";
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd))) {
        size_t lc = strlen(cwd);
        if (strncmp(fn, cwd, lc) == 0 && fn[lc] == '/') {   /* sob o cwd => relativo */
            snprintf(buf, cap, "%s", fn + lc + 1);
            return buf;
        }
    }
    const char *b = strrchr(fn, '/');           /* fora do cwd => basename */
    return b ? b + 1 : fn;
}

/* Um quadro: "  em <arq>, linha N" + a linha do fonte + o cursor `^^^`. Igual
 * ao `_fmt_frame` do interpretador. `col<=0` => cursor na 1ª não-branco. */
static void imprime_quadro(const char *origem, int linha, int col)
{
    char nome_buf[1024];
    fprintf(stderr, "  em %s, linha %d\n", limpa_arquivo(origem, nome_buf, sizeof(nome_buf)), linha);
    if (linha <= 0 || !origem || origem[0] == '<') return;
    FILE *f = fopen(origem, "rb");
    if (!f) return;
    char buf[4096];
    int atual = 0;
    while (fgets(buf, sizeof(buf), f)) {
        if (++atual != linha) continue;
        size_t l = strlen(buf);
        while (l > 0 && (buf[l-1] == '\n' || buf[l-1] == '\r')) buf[--l] = '\0';
        int sp;
        if (col > 0) sp = col - 1;
        else { sp = 0; while (buf[sp] == ' ' || buf[sp] == '\t') sp++; }
        fprintf(stderr, "  | %s\n  | %*s^^^\n", buf, sp, "");
        break;
    }
    fclose(f);
}

/* Erro do usuário sai EXATAMENTE como o interpretador (a autoridade): a
 * mensagem primeiro, depois o traceback do mais interno ao mais externo, e o
 * quadro do erro por último. O header só aparece quando há chamadores. */
static int reporta(const PSErroExec *e, const char *origem)
{
    switch (e->tipo) {
        case PS_ERRO_SINTAXE:
            fprintf(stderr, "SyntaxError: %s\n", e->msg);
            imprime_quadro(origem, e->linha, e->col);
            return 2;
        case PS_ERRO_NAO_SUPORTADO:
            fprintf(stderr, "NotImplementedError: %s\n", e->msg);
            imprime_quadro(origem, e->linha, e->col);
            return 3;
        case PS_ERRO_MEMORIA:
            fprintf(stderr, "MemoryError: %s\n", e->msg);
            return 4;
        default:
            fprintf(stderr, "%s: %s\n",
                    e->tipo_nome[0] ? e->tipo_nome : "RuntimeError", e->msg);
            if (e->ntb > 0) {
                if (e->ntb > 1)
                    fprintf(stderr, "\nTraceback (arquivo mais recente por último):\n");
                for (int i = 0; i < e->ntb; i++) {
                    const char *arq = e->tb[i].arquivo[0] ? e->tb[i].arquivo : origem;
                    imprime_quadro(arq, e->tb[i].linha, e->tb[i].col);
                }
            } else if (e->linha > 0) {
                imprime_quadro(origem, e->linha, e->col);
            }
            return 1;
    }
}

/* ── `-o`: gerar um executavel que roda sozinho ──────────────────────────────
 *
 * `pool programa.ps -o programa` produz um binario que NAO precisa do fonte
 * nem do `pool` instalado: e uma copia deste mesmo binario com o programa
 * grudado no fim, mais um rodape que diz onde ele comeca.
 *
 * Rodape (os ultimos RODAPE_TAM bytes do arquivo):
 *
 *     [ ...binario do pool... ][ fonte ][ tamanho do fonte, 16 digitos ][ MAGIA ]
 *
 * Na partida, o binario le o proprio arquivo, ve a magia no fim e, se estiver
 * la, roda o fonte embutido em vez de olhar os argumentos. Assim o mesmo
 * executavel serve de compilador e de programa compilado, sem toolchain
 * nenhuma no meio — nao ha compilador de C na maquina de quem so quer rodar.
 *
 * Compilar a partir de um binario JA compilado corta o payload velho antes de
 * grudar o novo: senao cada geracao carregaria o programa da anterior. */
#define PS_MAGIA_EMB   "PSPOOLEXE1"
#define PS_MAGIA_TAM   10
#define PS_DIG_TAM     16
#define PS_RODAPE_TAM  (PS_DIG_TAM + PS_MAGIA_TAM)

/* Caminho do executavel em execucao. */
static int ps_meu_caminho(char *out, size_t cap)
{
    ssize_t n = readlink("/proc/self/exe", out, cap - 1);
    if (n <= 0) return -1;
    out[n] = '\0';
    return 0;
}

/* O fonte embutido neste binario, ou NULL. `base` recebe o tamanho do binario
 * SEM o payload (e o que se copia ao compilar de novo). */
static char *ps_payload(const char *caminho, size_t *tam_out, long *base)
{
    FILE *f = fopen(caminho, "rb");
    if (!f) return NULL;
    if (fseek(f, 0, SEEK_END) != 0) { fclose(f); return NULL; }
    long fim = ftell(f);
    if (base) *base = fim;
    if (fim < PS_RODAPE_TAM) { fclose(f); return NULL; }

    char rodape[PS_RODAPE_TAM + 1];
    if (fseek(f, fim - PS_RODAPE_TAM, SEEK_SET) != 0
            || fread(rodape, 1, PS_RODAPE_TAM, f) != PS_RODAPE_TAM) { fclose(f); return NULL; }
    rodape[PS_RODAPE_TAM] = '\0';
    if (memcmp(rodape + PS_DIG_TAM, PS_MAGIA_EMB, PS_MAGIA_TAM) != 0) { fclose(f); return NULL; }

    char dig[PS_DIG_TAM + 1];
    memcpy(dig, rodape, PS_DIG_TAM); dig[PS_DIG_TAM] = '\0';
    char *fimp = NULL;
    long tam = strtol(dig, &fimp, 10);
    if (!fimp || *fimp != '\0' || tam <= 0 || tam > fim - PS_RODAPE_TAM) { fclose(f); return NULL; }

    long inicio = fim - PS_RODAPE_TAM - tam;
    char *buf = malloc((size_t)tam + 1);
    if (!buf) { fclose(f); return NULL; }
    if (fseek(f, inicio, SEEK_SET) != 0 || fread(buf, 1, (size_t)tam, f) != (size_t)tam) {
        free(buf); fclose(f); return NULL;
    }
    fclose(f);
    buf[tam] = '\0';
    if (tam_out) *tam_out = (size_t)tam;
    if (base) *base = inicio;          /* o binario limpo termina aqui */
    return buf;
}

/* `pool fonte.ps -o saida` */
static int cmd_compila(const char *fonte_arq, const char *saida)
{
    size_t tam = 0;
    char *fonte = le_arquivo(fonte_arq, &tam);
    if (!fonte) return 66;

    /* Nao gera executavel que ja nasce quebrado: o fonte tem que compilar. */
    PSTokenList *tl = ps_lexer_tokenize(fonte, tam);
    if (!tl || !tl->ok) {
        fprintf(stderr, "%s: %s (linha %d)\n", fonte_arq,
                tl ? tl->erro : "sem memoria", tl ? tl->erro_linha : 0);
        if (tl) ps_lexer_free(tl);
        free(fonte);
        return 65;
    }
    PSParseResult *pr = ps_parse(tl->tokens, tl->n);
    ps_lexer_free(tl);
    if (!pr || !pr->ok) {
        fprintf(stderr, "%s: %s (linha %d)\n", fonte_arq,
                pr ? pr->erro : "sem memoria", pr ? pr->erro_linha : 0);
        if (pr) ps_parse_free(pr);
        free(fonte);
        return 65;
    }
    ps_parse_free(pr);

    char meu[4096];
    if (ps_meu_caminho(meu, sizeof(meu)) != 0) {
        fprintf(stderr, "pool: nao consegui achar o proprio executavel\n");
        free(fonte); return 70;
    }
    long base = 0;
    char *velho = ps_payload(meu, NULL, &base);   /* corta payload anterior */
    free(velho);

    FILE *in = fopen(meu, "rb");
    if (!in) { fprintf(stderr, "pool: nao consegui ler %s\n", meu); free(fonte); return 70; }
    FILE *out = fopen(saida, "wb");
    if (!out) {
        fprintf(stderr, "pool: nao consegui escrever %s\n", saida);
        fclose(in); free(fonte); return 73;
    }
    char buf[65536];
    long resta = base;
    while (resta > 0) {
        size_t quer = (size_t)(resta < (long)sizeof(buf) ? resta : (long)sizeof(buf));
        size_t lidos = fread(buf, 1, quer, in);
        if (lidos == 0) break;
        if (fwrite(buf, 1, lidos, out) != lidos) {
            fprintf(stderr, "pool: escrita incompleta em %s\n", saida);
            fclose(in); fclose(out); free(fonte); return 73;
        }
        resta -= (long)lidos;
    }
    fclose(in);
    char rodape[PS_RODAPE_TAM + 1];
    snprintf(rodape, sizeof(rodape), "%0*zu%s", PS_DIG_TAM, tam, PS_MAGIA_EMB);
    if (fwrite(fonte, 1, tam, out) != tam
            || fwrite(rodape, 1, PS_RODAPE_TAM, out) != PS_RODAPE_TAM) {
        fprintf(stderr, "pool: escrita incompleta em %s\n", saida);
        fclose(out); free(fonte); return 73;
    }
    fclose(out);
    free(fonte);
    if (chmod(saida, 0755) != 0) {
        fprintf(stderr, "pool: gerado, mas nao consegui dar permissao de execucao a %s\n", saida);
        return 73;
    }
    printf("gerado: %s\n", saida);
    return 0;
}

/* `build`: roda todos os .ps da pasta atual, em ordem, e conta OK/erro. */
static int cmd_build(void)
{
    DIR *d = opendir(".");
    if (!d) { fprintf(stderr, "pool: nao consegui abrir a pasta atual\n"); return 1; }

    /* coleta os arquivos .ps/.psl/.p e ordena */
    char **nomes = NULL; int n = 0, cap = 0;
    struct dirent *ent;
    while ((ent = readdir(d)) != NULL) {
        size_t l = strlen(ent->d_name);
        const char *nm = ent->d_name;
        int eh = (l > 3 && strcmp(nm + l - 3, ".ps")  == 0)
              || (l > 4 && strcmp(nm + l - 4, ".psl") == 0)
              || (l > 2 && strcmp(nm + l - 2, ".p")   == 0);
        if (!eh) continue;
        if (n == cap) {
            cap = cap ? cap * 2 : 16;
            /* Por temporária: `x = realloc(x, …)` que falha devolve NULL SEM
             * liberar o bloco antigo — perde-se a única referência a ele, e a
             * linha seguinte escreve em NULL. */
            char **novo = realloc(nomes, sizeof(char *) * (size_t)cap);
            if (!novo) {
                for (int i = 0; i < n; i++) free(nomes[i]);
                free(nomes); closedir(d);
                fprintf(stderr, "sem memoria listando a pasta\n");
                return 1;
            }
            nomes = novo;
        }
        nomes[n++] = strdup(ent->d_name);
    }
    closedir(d);
    if (n == 0) { fprintf(stderr, "nenhum arquivo .ps/.psl/.p encontrado na pasta atual\n"); free(nomes); return 1; }
    for (int i = 0; i < n; i++)          /* ordenação simples (n pequeno) */
        for (int j = i + 1; j < n; j++)
            if (strcmp(nomes[i], nomes[j]) > 0) { char *t = nomes[i]; nomes[i] = nomes[j]; nomes[j] = t; }

    int rc = 0, ok = 0, falhou = 0;
    for (int i = 0; i < n; i++) {
        printf("=== %s ===\n", nomes[i]);
        fflush(stdout);
        size_t tam = 0;
        char *fonte = le_arquivo(nomes[i], &tam);
        if (!fonte) { falhou++; rc = 1; free(nomes[i]); continue; }
        PSErroExec e;
        int r = ps_roda_fonte(fonte, tam, nomes[i], &e);
        free(fonte);
        if (r != 0) { reporta(&e, nomes[i]); falhou++; rc = r; }
        else ok++;
        free(nomes[i]);
    }
    free(nomes);
    printf("\n%d arquivo(s) OK, %d com erro(s).\n", ok, falhou);
    return rc;
}

/* escreve `s` como string JSON (aspas + escapes) no stdout */
static void json_str(const char *s)
{
    putchar('"');
    for (const unsigned char *p = (const unsigned char *)s; *p; p++) {
        switch (*p) {
            case '"':  fputs("\\\"", stdout); break;
            case '\\': fputs("\\\\", stdout); break;
            case '\n': fputs("\\n", stdout);  break;
            case '\r': fputs("\\r", stdout);  break;
            case '\t': fputs("\\t", stdout);  break;
            default:
                if (*p < 0x20) printf("\\u%04x", *p);
                else putchar(*p);
        }
    }
    putchar('"');
}

/* CÓDIGO DE SAÍDA do `--check`: 0 quando compila, 1 quando não.
 *
 * Ele saía 0 SEMPRE, inclusive imprimindo `{"ok":false}`. Isso torna
 * `pool --check f.ps || exit 1` e `set -e` falso verde — e o `--check` é o que
 * o editor roda a cada tecla, o que a CI roda em arquivo que veio de fora, e o
 * que o `psl install` roda em pacote de TERCEIRO.
 *
 * É a convenção de todo conferidor de sintaxe: o código de saída diz se
 * passou. O JSON no stdout não muda — quem lê o JSON continua lendo igual;
 * quem lê o rc passa a poder confiar nele.
 *
 * Erro de LEITURA (arquivo que não abre, sem memória) também sai != 0: não
 * conseguir conferir não é conferir e aprovar. */
#define CHECK_RC_FALHA 1

/* `pool --check [arquivo.ps]` — lexer/parser/compilador da VM, SEM rodar, com
 * o resultado em JSON pro editor. Sem arquivo, lê o buffer do stdin (o editor
 * manda o conteúdo não salvo). NUNCA executa o código. */
static int cmd_check(const char *arquivo)
{
    size_t tam = 0;
    char *fonte = NULL;
    if (arquivo) {
        fonte = le_arquivo(arquivo, &tam);
        if (!fonte) { printf("{\"ok\":false,\"tipo\":\"IOError\",\"msg\":\"nao consegui abrir o arquivo\",\"linha\":1,\"coluna\":1}\n"); return CHECK_RC_FALHA; }
    } else {
        size_t cap = 65536; tam = 0;
        fonte = malloc(cap);
        if (!fonte) { printf("{\"ok\":false,\"tipo\":\"MemoryError\",\"msg\":\"sem memoria\",\"linha\":1,\"coluna\":1}\n"); return CHECK_RC_FALHA; }
        size_t r;
        while ((r = fread(fonte + tam, 1, cap - tam, stdin)) > 0) {
            tam += r;
            if (tam == cap) { cap *= 2; char *nb = realloc(fonte, cap); if (!nb) { free(fonte); printf("{\"ok\":false,\"tipo\":\"MemoryError\",\"msg\":\"sem memoria\",\"linha\":1,\"coluna\":1}\n"); return CHECK_RC_FALHA; } fonte = nb; }
        }
        fonte[tam] = '\0';
    }

    PSErroExec e;
    PSAviso *avisos = NULL;
    int32_t navisos = 0;
    int rc = ps_verifica_fonte(fonte, tam, arquivo, &e, &avisos, &navisos);
    free(fonte);

    /* Os AVISOS entram no mesmo JSON, e entram tanto no caso `ok` quanto no de
     * erro: um `\p` que nao e escape nao impede o programa de compilar, mas o
     * editor tem que poder sublinhar. Sem isto o aviso so existiria pra quem
     * roda no terminal — e o editor e onde ele seria visto. */
    if (rc == 0) {
        printf("{\"ok\":true");
        if (navisos > 0) {
            printf(",\"avisos\":[");
            for (int32_t i = 0; i < navisos; i++) {
                if (i) putchar(',');
                printf("{\"msg\":");
                json_str(avisos[i].msg);
                printf(",\"linha\":%d,\"coluna\":%d}", avisos[i].linha, avisos[i].col);
            }
            putchar(']');
        }
        printf("}\n");
        free(avisos);
        return 0;
    }
    free(avisos);

    const char *tipo = e.tipo == PS_ERRO_SINTAXE ? "SyntaxError"
                     : e.tipo == PS_ERRO_NAO_SUPORTADO ? "NotImplementedError"
                     : e.tipo == PS_ERRO_MEMORIA ? "MemoryError" : "RuntimeError";
    printf("{\"ok\":false,\"tipo\":\"%s\",\"msg\":", tipo);
    json_str(e.msg);
    printf(",\"linha\":%d,\"coluna\":%d}\n", e.linha, e.col);
    return CHECK_RC_FALHA;
}

/* ── `pool --contexto <linha>:<coluna>` ───────────────────────────────────
 *
 * O que o editor precisa saber pra completar: o que vem antes do cursor.
 * Responde com o LEXER de verdade, não com regex.
 *
 * A extensão adivinhava isso com expressão regular sobre o texto da linha, e
 * errava o previsível: regex não sabe o que é string (`"a.b".up`), não fecha
 * parêntese aninhado (`f(a, b).x`) e quebrava com um nome parcial depois do
 * ponto — `jinker.request.ge` deixava de casar, caía no completion de topo e
 * oferecia módulo e builtin DEPOIS de um ponto.
 *
 * Lê o fonte do stdin (o editor manda o buffer não salvo, igual ao --check) e
 * devolve JSON:
 *
 *   {"contexto":"membro","receptor":"jinker.request","parcial":"ge"}
 *   {"contexto":"topo","parcial":"gath"}
 *   {"contexto":"decorador","parcial":"rou","receptor":"app"}
 *   {"contexto":"import","parcial":"os"}
 *
 * NUNCA executa o código — só tokeniza. */
static char *le_stdin_todo(size_t *tam)
{
    size_t cap = 65536; *tam = 0;
    char *b = malloc(cap);
    if (!b) return NULL;
    size_t r;
    while ((r = fread(b + *tam, 1, cap - *tam, stdin)) > 0) {
        *tam += r;
        if (*tam == cap) {
            cap *= 2;
            char *nb = realloc(b, cap);
            if (!nb) { free(b); return NULL; }
            b = nb;
        }
    }
    b[*tam] = '\0';
    return b;
}

/* Índice do último token que TERMINA em ou antes de (linha, col). -1 se não há.
 * O lexer dá o início do token; o fim sai do texto (ou de 1 pra pontuação). */
static int tok_antes(const PSTokenList *tl, int linha, int col)
{
    int achado = -1;
    for (int i = 0; i < tl->n; i++) {
        const PSToken *t = &tl->tokens[i];
        if (t->type == T_NEWLINE || t->type == T_INDENT
            || t->type == T_DEDENT || t->type == T_EOF) continue;
        if (t->line > linha) break;
        if (t->line < linha) { achado = i; continue; }
        int fim = t->col + (t->texto ? t->texto_len : 1);
        if (fim <= col) achado = i;
    }
    return achado;
}

/* Anda pra trás montando a cadeia receptora a partir de `i` (que já é o token
 * ANTES do ponto). Devolve a posição do primeiro token da cadeia, ou -1 se o
 * receptor não é nomeável (literal, por exemplo). */
static int inicio_da_cadeia(const PSTokenList *tl, int i)
{
    int ini = -1;
    for (;;) {
        if (i < 0) break;
        PSTokType t = tl->tokens[i].type;
        if (t == T_RPAREN || t == T_RBRACK) {
            PSTokType abre = (t == T_RPAREN) ? T_LPAREN : T_LBRACK;
            int d = 0;
            for (i--; i >= 0; i--) {
                if (tl->tokens[i].type == t) d++;
                else if (tl->tokens[i].type == abre) { if (d == 0) { i--; break; } d--; }
            }
            continue;                     /* antes do grupo tem que vir o nome */
        }
        if (t == T_IDENT || t == T_IDENT_UPPER || t == T_KW) {
            ini = i;
            if (i >= 2 && tl->tokens[i - 1].type == T_DOT) { i -= 2; continue; }
            break;
        }
        break;                            /* literal, operador: não é receptor */
    }
    return ini;
}

static void jsonf(const char *chave, const char *valor)
{
    printf(",\"%s\":", chave);
    json_str(valor);
}

/* `pool --tokens` — o fonte vem pelo stdin e sai a lista de tokens do LEXER
 * DE VERDADE, em JSON:
 *
 *   [{"t":"KW","l":1,"c":1,"n":5,"v":"funct"}, ...]
 *
 * É o que dá realce ao editor sem existir uma segunda gramática pra divergir
 * do motor. `n` é o comprimento em CARACTERES (o editor conta caractere, não
 * byte); INDENT/DEDENT/NEWLINE saem com `n` 0, porque não ocupam texto — quem
 * pinta ignora, quem precisa da ESTRUTURA (converter de bloco, por exemplo)
 * usa. Comentário entra como "COMMENT" — o compilador descarta, o realce
 * precisa. NUNCA executa o código: só tokeniza. */
static int json_escapa(FILE *f, const char *s, int n)
{
    for (int i = 0; i < n; i++) {
        unsigned char c = (unsigned char)s[i];
        if (c == '"' || c == '\\') fprintf(f, "\\%c", c);
        else if (c == '\n') fputs("\\n", f);
        else if (c == '\r') fputs("\\r", f);
        else if (c == '\t') fputs("\\t", f);
        else if (c < 0x20) fprintf(f, "\\u%04x", c);
        else fputc(c, f);
    }
    return 0;
}

static int cmd_tokens(void)
{
    size_t tam = 0;
    char *fonte = le_stdin_todo(&tam);
    if (!fonte) { printf("[]\n"); return 1; }
    PSTokenList *tl = ps_lexer_tokenize_editor(fonte, tam);
    if (!tl) { free(fonte); printf("[]\n"); return 1; }
    /* Lexer que ERROU não entrega lista confiável: os tokens param no ponto do
     * erro e quem consome (o realce, o conversor de bloco) acabaria decidindo
     * com dado pela metade. Vazio + código 1 diz "não dá", em vez de mentir. */
    if (!tl->ok) {
        fprintf(stderr, "pool --tokens: %s (linha %d, coluna %d)\n",
                tl->erro, tl->erro_linha, tl->erro_col);
        printf("[]\n");
        ps_lexer_free(tl);
        free(fonte);
        return 1;
    }

    fputc('[', stdout);
    int primeiro = 1;
    for (int32_t i = 0; i < tl->n; i++) {
        const PSToken *t = &tl->tokens[i];
        if (t->type == T_EOF) continue;
        /* quanto o token ocupa NO FONTE (com aspas, com prefixo `f`), em
         * caracteres. Só cai no texto quando o lexer não mediu o span. */
        int nch = t->nchars;
        if (nch <= 0 && t->texto) {
            for (int k = 0; k < t->texto_len; k++)
                if (((unsigned char)t->texto[k] & 0xC0) != 0x80) nch++;
        }
        if (!primeiro) fputc(',', stdout);
        primeiro = 0;
        printf("{\"t\":\"%s\",\"l\":%d,\"c\":%d,\"n\":%d,\"v\":\"",
               ps_tok_nome(t->type), t->line, t->col, nch);
        if (t->texto) json_escapa(stdout, t->texto, t->texto_len);
        fputs("\"}", stdout);
    }
    fputs("]\n", stdout);
    ps_lexer_free(tl);
    free(fonte);
    return 0;
}

/* ── `--ast`: a ÁRVORE, em JSON, pro editor ──────────────────────────────
 *
 * POR QUE ESTE COMANDO EXISTE.
 *
 * O servidor de linguagem tinha `--tokens` (o léxico), `--metadata` (as
 * tabelas do VM) e `--check` (o erro). Faltava a única coisa que responde
 * "o que está em escopo NESTA linha": a árvore.
 *
 * Sem ela, o servidor casava PADRÃO em cima do texto — uma regex pra `self.`,
 * outra pra `alvo.`, uma varredura à mão pra achar parâmetro, outra pra achar
 * Entity. Cada forma nova que o usuário escrevia era um ramo novo escrito à
 * mão, e por isso sempre faltava um: `a.b.c`, método dentro de classe,
 * variável declarada três linhas acima. Não era análise, era remendo.
 *
 * Quem sabe o que é variável, de quem é o membro e o que está em escopo é o
 * PARSER — o mesmo que compila o programa. Publicar a árvore acaba com a
 * segunda gramática do lado do editor.
 *
 * O emissor é GENÉRICO sobre a struct do nó: percorre `a/b/c/e` e as três
 * listas, sem um caso por tipo. Nó novo na linguagem já sai aqui, sem ninguém
 * tocar neste arquivo — que é a mesma razão de `--metadata` sair das tabelas
 * do VM em vez de uma lista digitada.
 *
 * A LISTA DE ERRO NÃO IMPEDE A ÁRVORE: o editor precisa completar enquanto o
 * arquivo está pela metade (é justamente quando se digita). Com erro de
 * sintaxe sai `{"ok":false,...}` COM a árvore parcial que o parser conseguiu,
 * em vez de vazio.
 */
static void ast_json(FILE *f, const PSNode *n);

static void ast_lista(FILE *f, const char *chave, const PSNodeVec *v, int *virg)
{
    if (v->n == 0) return;
    if (*virg) fputc(',', f);
    *virg = 1;
    fprintf(f, "\"%s\":[", chave);
    for (int32_t i = 0; i < v->n; i++) {
        if (i) fputc(',', f);
        ast_json(f, v->itens[i]);
    }
    fputc(']', f);
}

static void ast_filho(FILE *f, const char *chave, const PSNode *c, int *virg)
{
    if (!c) return;
    if (*virg) fputc(',', f);
    *virg = 1;
    fprintf(f, "\"%s\":", chave);
    ast_json(f, c);
}

static void ast_txt(FILE *f, const char *chave, const char *s, int *virg)
{
    if (!s) return;
    if (*virg) fputc(',', f);
    *virg = 1;
    fprintf(f, "\"%s\":\"", chave);
    json_escapa(f, s, (int)strlen(s));
    fputc('"', f);
}

static void ast_json(FILE *f, const PSNode *n)
{
    if (!n) { fputs("null", f); return; }
    int virg = 0;
    fputc('{', f);
    fprintf(f, "\"k\":\"%s\",\"l\":%d,\"c\":%d", ps_node_nome(n->kind), n->line, n->col);
    if (n->linha_fim) fprintf(f, ",\"l2\":%d", n->linha_fim);
    virg = 1;
    ast_txt(f, "texto",  n->texto,  &virg);
    ast_txt(f, "texto2", n->texto2, &virg);
    ast_txt(f, "texto3", n->texto3, &virg);
    ast_txt(f, "estilo", n->estilo, &virg);
    if (n->is_async)   { fputs(",\"async\":true", f); }
    if (n->is_private) { fputs(",\"private\":true", f); }
    if (n->is_static)  { fputs(",\"static\":true", f); }
    if (n->is_nonnull) { fputs(",\"nonnull\":true", f); }
    if (n->i2)         { fprintf(f, ",\"i2\":%d", n->i2); }
    ast_filho(f, "a", n->a, &virg);
    ast_filho(f, "b", n->b, &virg);
    ast_filho(f, "c", n->c, &virg);
    ast_filho(f, "e", n->e, &virg);
    ast_lista(f, "lista",  &n->lista,  &virg);
    ast_lista(f, "lista2", &n->lista2, &virg);
    ast_lista(f, "alias",  &n->lista2_alias, &virg);
    fputc('}', f);
}

static int cmd_ast(void)
{
    size_t tam = 0;
    char *fonte = le_stdin_todo(&tam);
    if (!fonte) { printf("{\"ok\":false,\"msg\":\"sem entrada\"}\n"); return 1; }

    PSTokenList *tl = ps_lexer_tokenize(fonte, tam);
    free(fonte);
    if (!tl || !tl->ok) {
        printf("{\"ok\":false,\"tipo\":\"SyntaxError\",\"msg\":\"");
        if (tl) json_escapa(stdout, tl->erro, (int)strlen(tl->erro));
        printf("\",\"linha\":%d,\"coluna\":%d,\"arvore\":null}\n",
               tl ? tl->erro_linha : 0, tl ? tl->erro_col : 0);
        if (tl) ps_lexer_free(tl);
        return 1;
    }
    PSParseResult *r = ps_parse(tl->tokens, tl->n);
    ps_lexer_free(tl);
    if (!r) { printf("{\"ok\":false,\"msg\":\"sem memoria\",\"arvore\":null}\n"); return 1; }

    if (!r->ok) {
        printf("{\"ok\":false,\"tipo\":\"SyntaxError\",\"msg\":\"");
        json_escapa(stdout, r->erro, (int)strlen(r->erro));
        printf("\",\"linha\":%d,\"coluna\":%d,\"arvore\":", r->erro_linha, r->erro_col);
    } else {
        printf("{\"ok\":true,\"arvore\":");
    }
    ast_json(stdout, r->programa);
    fputs("}\n", stdout);
    int rc = r->ok ? 0 : 1;
    ps_parse_free(r);
    return rc;
}

static int cmd_contexto(const char *pos)
{
    int linha = 0, col = 0;
    if (!pos || sscanf(pos, "%d:%d", &linha, &col) != 2 || linha < 1 || col < 1) {
        printf("{\"contexto\":\"erro\",\"msg\":\"uso: pool --contexto <linha>:<coluna>\"}\n");
        return 1;
    }
    size_t tam = 0;
    char *fonte = le_stdin_todo(&tam);
    if (!fonte) { printf("{\"contexto\":\"erro\",\"msg\":\"sem memoria\"}\n"); return 1; }

    PSTokenList *tl = ps_lexer_tokenize(fonte, tam);
    if (!tl) { free(fonte); printf("{\"contexto\":\"erro\",\"msg\":\"lexer falhou\"}\n"); return 1; }

    /* nome parcial: só conta se ENCOSTA no cursor (`ge|`, não `ge |`) */
    const char *parcial = "";
    int i = tok_antes(tl, linha, col);
    if (i >= 0) {
        const PSToken *t = &tl->tokens[i];
        if ((t->type == T_IDENT || t->type == T_IDENT_UPPER || t->type == T_KW)
            && t->line == linha && t->col + t->texto_len == col) {
            parcial = t->texto ? t->texto : "";
            i--;
        }
    }

    const char *ctx = "topo";
    char *recv = NULL;

    if (i >= 0 && tl->tokens[i].type == T_DOT) {
        int ini = inicio_da_cadeia(tl, i - 1);
        if (ini >= 0) {
            /* fatia o fonte do início da cadeia até o ponto — preserva o texto
             * original (`f(a, b)`), que é o que o resolvedor de tipo espera */
            const PSToken *a = &tl->tokens[ini];
            const PSToken *p = &tl->tokens[i];
            size_t off_a = 0, off_p = 0, off = 0;
            int ln = 1, cl = 1;
            for (size_t k = 0; k <= tam; k++) {
                if (ln == a->line && cl == a->col) off_a = k;
                if (ln == p->line && cl == p->col) { off_p = k; break; }
                if (k < tam && fonte[k] == '\n') { ln++; cl = 1; } else cl++;
                off = k;
            }
            (void)off;
            if (off_p > off_a) {
                recv = malloc(off_p - off_a + 1);
                if (recv) {
                    memcpy(recv, fonte + off_a, off_p - off_a);
                    recv[off_p - off_a] = '\0';
                    /* apara espaço da direita (o `.` pode vir depois de espaço) */
                    for (char *e = recv + strlen(recv); e > recv && (e[-1]==' '||e[-1]=='\t'); e--) e[-1] = '\0';
                    ctx = "membro";
                }
            }
        }
        /* Receptor LITERAL (`"a.b".up`, `[1,2].so`): não tem nome pra resolver,
         * mas tem tipo — e tipo tem método. O regex devolvia null aqui e o
         * editor não oferecia nada. */
        if (!recv && i >= 1) {
            const char *tipo = NULL;
            switch (tl->tokens[i - 1].type) {
                case T_STR: case T_FSTRING: tipo = "str";  break;
                case T_INT:                 tipo = "int";  break;
                case T_FLO:                 tipo = "flo";  break;
                case T_BOOL:                tipo = "bool"; break;
                case T_RBRACK:              tipo = "list"; break;
                case T_RBRACE:              tipo = "dict"; break;
                default: break;
            }
            if (tipo) {
                printf("{\"contexto\":\"membro\"");
                jsonf("parcial", parcial);
                jsonf("tipo", tipo);
                printf("}\n");
                ps_lexer_free(tl); free(fonte);
                return 0;
            }
        }
        /* nem nome nem tipo: NADA a oferecer — nunca cair no topo depois de um ponto */
        if (!recv) ctx = "nenhum";
    } else if (i >= 0 && tl->tokens[i].type == T_AT) {
        ctx = "decorador";
    } else {
        /* `import x` / `from x import y` */
        for (int k = i; k >= 0 && tl->tokens[k].line == linha; k--) {
            const PSToken *t = &tl->tokens[k];
            if (t->type == T_KW && t->texto
                && (!strcmp(t->texto, "import") || !strcmp(t->texto, "from"))) {
                ctx = "import";
                break;
            }
        }
    }

    printf("{\"contexto\":\"%s\"", ctx);
    jsonf("parcial", parcial);
    if (recv) jsonf("receptor", recv);
    printf("}\n");

    free(recv);
    ps_lexer_free(tl);
    free(fonte);
    return 0;
}

/* -asLib nos argumentos? (força instalar/remover como lib) */
static int tem_flag(int argc, char **argv, int de, const char *flag)
{
    for (int i = de; i < argc; i++) if (!strcmp(argv[i], flag)) return 1;
    return 0;
}

int main(int argc, char **argv)
{
    /* Marca a base da pilha ANTES de tudo: é a referência da medição de folga
     * que impede a recursão de estourar (ver `ps_pilha_apertada`). Feito no
     * main porque aqui a pilha ainda está praticamente intocada. */
    ps_pilha_marca_processo();

    /* Executavel gerado por `-o`: o programa esta grudado neste binario. Roda
     * ele e ignora os subcomandos — quem chama um programa compilado espera o
     * PROGRAMA, não a CLI do pool. Os argumentos vão inteiros pro `sys.argv`. */
    {
        char meu[4096];
        if (ps_meu_caminho(meu, sizeof(meu)) == 0) {
            size_t tam_emb = 0;
            char *emb = ps_payload(meu, &tam_emb, NULL);
            if (emb) {
                PSErroExec ee;
                ps_set_argv(argc - 1, argv + 1);
                int rc = ps_roda_fonte(emb, tam_emb, meu, &ee);
                free(emb);
                if (rc != 0) return reporta(&ee, meu);
                return 0;
            }
        }
    }

    if (argc < 2) { ajuda(); return 0; }

    const char *cmd = argv[1];

    if (!strcmp(cmd, "--version") || !strcmp(cmd, "-V") || !strcmp(cmd, "-v")) {
        printf("PoolScript %s [PSVM]\n", PS_VERSAO);
        return 0;
    }
    if (!strcmp(cmd, "--help") || !strcmp(cmd, "-h") || !strcmp(cmd, "help")) {
        ajuda();
        return 0;
    }
    if (!strcmp(cmd, "//doc")) {
        printf("Especificacao da PoolScript:\n  %s\n", SPEC_URL);
        return 0;
    }
    if (!strcmp(cmd, "--check") || !strcmp(cmd, "check"))
        return cmd_check(argc >= 3 ? argv[2] : NULL);
    /* modelo de tipos direto do motor — o editor e a auditoria de doc leem
     * daqui em vez de introspectar a stdlib do interpretador */
    /* o editor pergunta o contexto do cursor pro LEXER, nao pra um regex */
    if (!strcmp(cmd, "--tokens") || !strcmp(cmd, "tokens"))
        return cmd_tokens();
    if (!strcmp(cmd, "--ast") || !strcmp(cmd, "ast"))
        return cmd_ast();
    if (!strcmp(cmd, "--contexto") || !strcmp(cmd, "contexto"))
        return cmd_contexto(argc >= 3 ? argv[2] : NULL);
    if (!strcmp(cmd, "--metadata") || !strcmp(cmd, "metadata")) {
        ps_metadata_json(stdout);
        return 0;
    }
    /* Depurador: `pool --debug <porta> arquivo.ps`. O motor escuta DAP na porta
     * e o editor conecta — é a extensão do VS Code quem escolhe a porta livre e
     * passa aqui. Ver docs/debugger.md. */
    if (!strcmp(cmd, "--debug") || !strcmp(cmd, "debug")) {
        if (argc < 4) {
            fprintf(stderr, "uso: pool --debug <porta> <arquivo.ps>\n");
            return 64;
        }
        char *fim = NULL;
        long porta = strtol(argv[2], &fim, 10);
        if (fim == argv[2] || *fim || porta < 1 || porta > 65535) {
            fprintf(stderr, "pool --debug: porta invalida: %s\n", argv[2]);
            return 64;
        }
        ps_debug_porta((int)porta);
        /* Consome o `--debug <porta>` e deixa a linha como se o usuário tivesse
         * escrito `pool arquivo.ps <args>`: o arquivo volta pra argv[1] e os
         * argumentos dele continuam depois, senão o `sys.argv` do programa
         * receberia o próprio nome do arquivo como primeiro argumento. */
        for (int k = 1; k + 2 < argc; k++) argv[k] = argv[k + 2];
        argc -= 2;
        cmd = argv[1];
    }
    if (!strcmp(cmd, "build")) return cmd_build();
    if (!strcmp(cmd, "compile")) {
        if (argc < 4) { fprintf(stderr, "uso: pool compile <arquivo.ps> -o <saida>\n"); return 64; }
        if (strcmp(argv[3], "-o") != 0 || argc < 5) {
            fprintf(stderr, "uso: pool compile <arquivo.ps> -o <saida>\n"); return 64;
        }
        return cmd_compila(argv[2], argv[4]);
    }
    /* ── pacotes (só .ps: lib/comando) ──────────────────────────────── */
    if (!strcmp(cmd, "install")) {
        if (argc < 3) { fprintf(stderr, "uso: psl install <arquivo.ps | nome> [-asLib]\n"); return 1; }
        int modo = tem_flag(argc, argv, 3, "-asLib") ? PS_PKG_LIB : PS_PKG_AUTO;
        return ps_pkg_install(argv[2], modo);
    }
    if (!strcmp(cmd, "uninstall")) {
        if (argc < 3) { fprintf(stderr, "uso: psl uninstall <nome> [-asLib]\n"); return 1; }
        int cat = tem_flag(argc, argv, 3, "-asLib") ? PS_PKG_LIB : PS_PKG_AUTO;
        return ps_pkg_uninstall(argv[2], cat);
    }
    if (!strcmp(cmd, "list")) return ps_pkg_list();
    if (!strcmp(cmd, "registry")) return ps_pkg_registry(argc - 2, argv + 2);
    if (!strcmp(cmd, "repl")) {
        fprintf(stderr,
            "pool: o REPL interativo ainda nao esta no binario C "
            "(precisa de estado persistente na VM).\n"
            "      Por enquanto: `pool arquivo.ps` ou `pool -e \"<codigo>\"`.\n");
        return 64;
    }

    PSErroExec e;

    if (!strcmp(cmd, "-e")) {
        if (argc < 3) { ajuda(); return 64; }
        if (ps_roda_fonte(argv[2], strlen(argv[2]), NULL, &e) != 0)
            return reporta(&e, "<-e>");
        return 0;
    }

    /* `pool programa.ps -o saida` — gera o executavel em vez de rodar. */
    if (argc >= 4 && !strcmp(argv[2], "-o"))
        return cmd_compila(cmd, argv[3]);
    if (argc == 3 && !strcmp(argv[2], "-o")) {
        fprintf(stderr, "uso: pool %s -o <saida>\n", cmd);
        return 64;
    }

    /* tudo depois do arquivo é do usuário — é o que o `sys.argv` devolve */
    ps_set_argv(argc - 2, argv + 2);

    size_t tam = 0;
    char *fonte = le_arquivo(cmd, &tam);
    if (!fonte) return 66;
    int rc = ps_roda_fonte(fonte, tam, cmd, &e);
    free(fonte);
    if (rc != 0) return reporta(&e, cmd);
    return 0;
}
