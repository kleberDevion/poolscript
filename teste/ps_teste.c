/*
 * Runner da suíte. Roda `./pool` num subprocesso pra cada caso e compara.
 *
 *   make teste && ./teste            roda tudo
 *   ./teste crash                    só os casos cujo nome/grupo casa "crash"
 *   ./teste -v                       mostra também os que passaram
 *
 * Sai com 1 se algum caso falhar — é o que serve pra CI e pra mim não seguir
 * em frente achando que está verde.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/wait.h>
#include <poll.h>
#include <dirent.h>
#include <limits.h>

#include "ps_teste.h"

#define POOL "./pool"

/* Caminho ABSOLUTO do binário, resolvido uma vez. O filho faz `chdir` pra um
 * diretório próprio antes do exec (ver `roda`), e daí `./pool` não existe
 * mais. */
static char POOL_ABS[4096];
static void resolve_pool(void)
{
    if (POOL_ABS[0]) return;
    /* `PS_POOL` troca o binário sob teste sem tocar no runner — é o que deixa
     * `make cobertura` rodar a MESMA suíte contra a build instrumentada, e
     * `make check-asan` contra a com sanitizer. Sem isso não havia como medir
     * cobertura, que foi o buraco apontado pela auditoria de testes. */
    const char *alt = getenv("PS_POOL");
    const char *bin = (alt && alt[0]) ? alt : POOL;
    if (!realpath(bin, POOL_ABS)) snprintf(POOL_ABS, sizeof POOL_ABS, "%s", bin);
}

/* Apaga o diretório do caso. Um nível de recursão basta: caso que cria
 * subpasta (`__DIR__/sub/`) não passa de dois. */
static void apaga_arvore(const char *dir)
{
    DIR *d = opendir(dir);
    if (!d) return;
    struct dirent *e;
    while ((e = readdir(d))) {
        if (!strcmp(e->d_name, ".") || !strcmp(e->d_name, "..")) continue;
        char p[4096];
        snprintf(p, sizeof p, "%s/%s", dir, e->d_name);
        if (unlink(p) != 0) apaga_arvore(p);   /* era diretório */
    }
    closedir(d);
    rmdir(dir);
}
#define TEMPO_MAX 20        /* segundos por caso: caso que trava é FALHA */

static const Grupo GRUPOS[] = {
    { "crash",     CASOS_CRASH,     0 },
    { "inteiros",  CASOS_INTEIROS,  0 },
    { "erros",     CASOS_ERROS,     0 },
    { "linguagem", CASOS_LINGUAGEM, 0 },
    { "pendentes", CASOS_PENDENTES, 0 },
    { "cobertura", CASOS_COBERTURA, 0 },
    { "diferencial", CASOS_DIFERENCIAL, 0 },
    { "equivalencia", CASOS_EQUIVALENCIA, 0 },
    { "oraculo",   CASOS_ORACULO,   0 },
    { "robustez",  CASOS_ROBUSTEZ,  0 },
    { "libs",      CASOS_LIBS,      0 },
};

/* preenchido em tempo de execução (os `extern int` vivem nos casos_*.c) */
static int tamanho_do_grupo(const char *g)
{
    if (!strcmp(g, "crash"))     return NC_CRASH;
    if (!strcmp(g, "inteiros"))  return NC_INTEIROS;
    if (!strcmp(g, "erros"))     return NC_ERROS;
    if (!strcmp(g, "linguagem")) return NC_LINGUAGEM;
    if (!strcmp(g, "pendentes")) return NC_PENDENTES;
    if (!strcmp(g, "cobertura")) return NC_COBERTURA;
    if (!strcmp(g, "diferencial")) return NC_DIFERENCIAL;
    if (!strcmp(g, "equivalencia")) return NC_EQUIVALENCIA;
    if (!strcmp(g, "oraculo"))   return NC_ORACULO;
    if (!strcmp(g, "robustez"))  return NC_ROBUSTEZ;
    if (!strcmp(g, "libs"))      return NC_LIBS;
    return 0;
}

/* ── execução de um caso ─────────────────────────────────────────────────── */

typedef struct {
    char *saida;      /* stdout */
    char *erro;       /* stderr */
    int   rc;         /* código de saída (ou -1) */
    int   sinal;      /* != 0 = MORREU com este sinal (segfault, SIGFPE...) */
    int   estourou;   /* 1 = passou do TEMPO_MAX */
} Resultado;

/* Lê os DOIS canos ao mesmo tempo.
 *
 * Ler um até o fim e só então o outro é o impasse clássico: cada cano guarda
 * 64 KB, então um caso que despeje mais que isso no stderr enche o cano, o
 * filho bloqueia escrevendo, e o pai fica esperando um stdout que nunca chega.
 * O `alarm(20)` quebrava o abraço e o caso era relatado como "TRAVOU" — falha
 * que não existia. Com `poll` nos dois, nenhum dos lados espera o outro. */
typedef struct { char *b; size_t n, cap; int aberto; } Cano;

static int cano_le(Cano *c, int fd)
{
    if (c->n + 4096 > c->cap) {
        size_t nc = c->cap ? c->cap * 2 : 8192;
        while (nc < c->n + 4096) nc *= 2;
        char *nb = realloc(c->b, nc);
        if (!nb) return -1;
        c->b = nb; c->cap = nc;
    }
    ssize_t r = read(fd, c->b + c->n, c->cap - c->n - 1);
    if (r <= 0) { c->aberto = 0; return 0; }
    c->n += (size_t)r;
    return 0;
}

static int le_os_dois(int fo, int fe, char **saida, char **erro)
{
    Cano so = {0}, se = {0};
    so.aberto = se.aberto = 1;
    while (so.aberto || se.aberto) {
        struct pollfd pf[2];
        int np = 0;
        int io = -1, ie = -1;
        if (so.aberto) { io = np; pf[np].fd = fo; pf[np].events = POLLIN; np++; }
        if (se.aberto) { ie = np; pf[np].fd = fe; pf[np].events = POLLIN; np++; }
        if (poll(pf, (nfds_t)np, -1) < 0) { if (errno == EINTR) continue; break; }
        if (io >= 0 && (pf[io].revents & (POLLIN | POLLHUP)) && cano_le(&so, fo) != 0) break;
        if (ie >= 0 && (pf[ie].revents & (POLLIN | POLLHUP)) && cano_le(&se, fe) != 0) break;
    }
    if (!so.b) { so.b = malloc(1); so.cap = 1; }
    if (!se.b) { se.b = malloc(1); se.cap = 1; }
    if (!so.b || !se.b) { free(so.b); free(se.b); return -1; }
    so.b[so.n] = '\0'; se.b[se.n] = '\0';
    *saida = so.b; *erro = se.b;
    return 0;
}

static void solta(Resultado *r) { free(r->saida); free(r->erro); r->saida = r->erro = NULL; }

static int roda(const Caso *c, Resultado *out)
{
    memset(out, 0, sizeof(*out));

    char arquivo[256];
    char fonte_ajustada[8192];
    const char *fonte = c->fonte;
    int fd;
    if (c->arquivo) {
        /* Nome fixo colidia entre execuções concorrentes: uma apagava o
         * arquivo enquanto a outra ainda rodava ("nao consegui abrir"), e o
         * caso falhava de forma intermitente. O nome ganha o PID, e o MESMO
         * sufixo é aplicado ao fonte — que precisa se importar pelo nome. */
        char base[128];
        snprintf(base, sizeof(base), "%s", c->arquivo);
        char *ponto = strrchr(base, '.');
        if (ponto) *ponto = '\0';
        char unico[192];
        snprintf(unico, sizeof(unico), "%s_%d", base, (int)getpid());
        snprintf(arquivo, sizeof(arquivo), "/tmp/%s.ps", unico);
        /* troca cada ocorrência do nome base pelo nome único, no fonte */
        /* Se o fonte não couber, o caso roda um programa PELA METADE e a
         * comparação vale nada — falha calada, do pior tipo. Melhor recusar. */
        if (strlen(c->fonte) + 64 >= sizeof(fonte_ajustada)) {
            fprintf(stderr, "caso '%s': fonte de %zu bytes nao cabe em %zu — "
                            "aumente fonte_ajustada[] em ps_teste.c\n",
                    c->nome, strlen(c->fonte), sizeof(fonte_ajustada));
            return -1;
        }
        size_t nb = strlen(base), j = 0;
        for (const char *q = c->fonte; *q && j < sizeof(fonte_ajustada) - 256; ) {
            if (strncmp(q, base, nb) == 0) {
                j += (size_t)snprintf(fonte_ajustada + j, sizeof(fonte_ajustada) - j,
                                      "%s", unico);
                q += nb;
            } else {
                fonte_ajustada[j++] = *q++;
            }
        }
        fonte_ajustada[j] = '\0';
        fonte = fonte_ajustada;
        fd = open(arquivo, O_WRONLY | O_CREAT | O_TRUNC, 0600);
        if (fd < 0) { perror("open"); return -1; }
    } else {
        snprintf(arquivo, sizeof(arquivo), "/tmp/ps_teste_XXXXXX");
        fd = mkstemp(arquivo);
        if (fd < 0) { perror("mkstemp"); return -1; }
    }
    if (write(fd, fonte, strlen(fonte)) < 0) { close(fd); unlink(arquivo); return -1; }
    close(fd);

    int po[2], pe[2];
    if (pipe(po) != 0 || pipe(pe) != 0) { unlink(arquivo); return -1; }

    /* Cada caso roda num diretório SÓ DELE. Sem isso, um caso com caminho
     * relativo (`os.writeFile("__DIR__/sub/nota.txt")`) grava na pasta de onde
     * a suíte foi chamada — a raiz do repositório — e o arquivo acaba
     * versionado. Aconteceu: `__DIR__/` e `__DOCS__/` estavam no git.
     * Com o `chdir`, a classe inteira fica impossível, não só os dois casos. */
    resolve_pool();
    char dir_caso[] = "/tmp/ps_caso_XXXXXX";
    const char *dir_ok = mkdtemp(dir_caso);

    pid_t pid = fork();
    if (pid < 0) { unlink(arquivo); if (dir_ok) apaga_arvore(dir_caso); return -1; }
    if (pid == 0) {
        if (dir_ok) { if (chdir(dir_caso) != 0) _exit(126); }
        dup2(po[1], STDOUT_FILENO); dup2(pe[1], STDERR_FILENO);
        close(po[0]); close(po[1]); close(pe[0]); close(pe[1]);
        /* stdin fechado (= /dev/null): caso nenhum pode ler o terminal. Sem
         * isso um `input()` num caso trava esperando quem roda a suíte. */
        int nulo = open("/dev/null", O_RDONLY);
        if (nulo >= 0) { dup2(nulo, STDIN_FILENO); if (nulo > 2) close(nulo); }
        alarm(TEMPO_MAX);                       /* trava = SIGALRM, não espera eterna */
        execl(POOL_ABS, POOL_ABS, arquivo, (char *)NULL);
        _exit(127);
    }
    close(po[1]); close(pe[1]);
    if (le_os_dois(po[0], pe[0], &out->saida, &out->erro) != 0) {
        close(po[0]); close(pe[0]); waitpid(pid, NULL, 0);
        unlink(arquivo); if (dir_ok) apaga_arvore(dir_caso);
        return -1;
    }
    close(po[0]); close(pe[0]);

    int st = 0;
    waitpid(pid, &st, 0);
    unlink(arquivo);
    if (dir_ok) apaga_arvore(dir_caso);
    if (WIFSIGNALED(st)) {
        out->sinal = WTERMSIG(st);
        out->estourou = (out->sinal == SIGALRM);
        out->rc = -1;
    } else {
        out->rc = WEXITSTATUS(st);
    }
    return 0;
}

/* ── comparação ──────────────────────────────────────────────────────────── */

static void sem_fim_de_linha(char *s)
{
    if (!s) return;
    size_t n = strlen(s);
    while (n > 0 && (s[n-1] == '\n' || s[n-1] == '\r')) s[--n] = '\0';
}

/* devolve NULL se passou; senão, o motivo (buffer estático) */
static const char *confere(const Caso *c, Resultado *r)
{
    static char motivo[4096];

    if (r->estourou) {
        snprintf(motivo, sizeof motivo, "TRAVOU (passou de %ds)", TEMPO_MAX);
        return motivo;
    }
    if (r->sinal) {
        snprintf(motivo, sizeof motivo,
                 "a VM MORREU com sinal %d (%s)", r->sinal,
                 r->sinal == SIGSEGV ? "segfault" :
                 r->sinal == SIGFPE  ? "erro aritmético" :
                 r->sinal == SIGABRT ? "abort" : "?");
        return motivo;
    }
    sem_fim_de_linha(r->saida);
    sem_fim_de_linha(r->erro);

    if (c->saida && strcmp(r->saida ? r->saida : "", c->saida) != 0) {
        snprintf(motivo, sizeof motivo, "saída\n     esperada: %s\n     obtida  : %s",
                 c->saida, r->saida ? r->saida : "");
        return motivo;
    }
    if (c->erro) {
        if (!r->erro || !strstr(r->erro, c->erro)) {
            snprintf(motivo, sizeof motivo, "erro esperado com %s\n     obtido  : %s",
                     c->erro, (r->erro && r->erro[0]) ? r->erro : "(nenhum)");
            return motivo;
        }
    } else if (r->rc != 0) {
        snprintf(motivo, sizeof motivo, "esperava sucesso, saiu com rc=%d\n     erro: %s",
                 r->rc, r->erro ? r->erro : "");
        return motivo;
    }
    if (c->rc >= 0 && r->rc != c->rc) {
        snprintf(motivo, sizeof motivo, "código de saída: esperado %d, obtido %d", c->rc, r->rc);
        return motivo;
    }
    return NULL;
}

/* ── principal ───────────────────────────────────────────────────────────── */

int main(int argc, char **argv)
{
    const char *filtro = NULL;
    int verboso = 0;
    for (int i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "-v")) verboso = 1;
        else filtro = argv[i];
    }

    if (access(POOL, X_OK) != 0) {
        fprintf(stderr, "teste: `%s` não existe ou não é executável — rode `make pool`\n", POOL);
        return 2;
    }

    int total = 0, passou = 0, falhou = 0;
    for (int g = 0; g < (int)(sizeof(GRUPOS) / sizeof(GRUPOS[0])); g++) {
        const char *nome_g = GRUPOS[g].grupo;
        int n = tamanho_do_grupo(nome_g);
        int cabecalho = 0;
        for (int i = 0; i < n; i++) {
            const Caso *c = &GRUPOS[g].casos[i];
            if (filtro && !strstr(nome_g, filtro) && !strstr(c->nome, filtro)) continue;
            if (!cabecalho) { printf("\n── %s ──\n", nome_g); cabecalho = 1; }
            total++;
            Resultado r;
            if (roda(c, &r) != 0) {
                printf("  ERRO INTERNO  %s\n", c->nome);
                falhou++;
                continue;
            }
            const char *motivo = confere(c, &r);
            /* Caso pendente TEM que falhar: ele descreve o que o motor ainda
             * não faz. Passar significa que já foi corrigido, e aí o veredito
             * se inverte — senão a fila esvazia sem ninguém saber. */
            if (c->pendente) {
                if (motivo) {
                    passou++;
                    if (verboso) printf("  pendente (falha esperada)  %s\n", c->nome);
                } else {
                    printf("  JA FUNCIONA  %s\n     tire de casos_pendentes.c e "
                           "mova pro grupo de regressao a que pertence\n", c->nome);
                    falhou++;
                }
                solta(&r);
                continue;
            }
            if (motivo) {
                printf("  FALHOU  %s\n     %s\n", c->nome, motivo);
                /* Grava também em arquivo. Uma falha INTERMITENTE some da tela
                 * na próxima execução e aí não dá pra investigar; aqui ela
                 * fica, com o fonte e a saída completa. */
                FILE *lg = fopen("teste/ultima_falha.txt", "a");
                if (lg) {
                    fprintf(lg, "=== %s\n--- motivo: %s\n--- fonte:\n%s"
                                "--- rc: %d  sinal: %d\n--- saida:\n%s"
                                "--- erro:\n%s\n\n",
                            c->nome, motivo, c->fonte, r.rc, r.sinal,
                            r.saida ? r.saida : "", r.erro ? r.erro : "");
                    fclose(lg);
                }
                falhou++;
            } else {
                passou++;
                if (verboso) printf("  ok      %s\n", c->nome);
            }
            solta(&r);
        }
    }

    printf("\n%d casos: %d passaram, %d falharam\n", total, passou, falhou);
    if (total == 0) {
        printf("nenhum caso casou com o filtro — isso é falha, não sucesso\n");
        return 2;
    }
    return falhou ? 1 : 0;
}
