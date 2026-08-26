/*
 * Injeção de falha de alocação — a técnica do SQLite.
 *
 * O PROBLEMA que isto resolve: toda correção da classe C da auditoria é um
 * caminho de falta de memória, e nenhum deles jamais executou. O gcov mostrava
 * `if (!mn)` avaliado 42x com o ramo verdadeiro NUNCA tomado. Corrigir sem
 * poder testar só aumenta o denominador da cobertura.
 *
 * O Linux dá `/dev/full` de graça, e foi por isso que B1-B4 (I/O) puderam ser
 * testados de verdade. Memória não tem equivalente — então a gente fabrica.
 *
 * COMO FUNCIONA: o binário `pool-oom` é ligado com
 * `-Wl,--wrap=malloc,--wrap=calloc,--wrap=realloc,--wrap=strdup`. O linker
 * manda toda chamada dessas quatro para `__wrap_*` aqui, e a de verdade fica
 * acessível como `__real_*`. Nenhuma linha do motor muda — é exatamente o que
 * se quer de um harness: o código testado é o código que roda em produção.
 *
 * PS_OOM_FALHA=N   faz a N-ésima alocação (1-based) devolver NULL.
 * PS_OOM_CONTA=1   não falha nada; escreve no descritor 3 quantas alocações
 *                  houve, pra o driver saber até onde varrer.
 *
 * O que se exige do motor: com QUALQUER N, ou o programa roda até o fim, ou
 * morre com uma mensagem de erro. O que não pode é segfault, liberação dupla
 * ou trava — e é justamente isso que os caminhos nunca exercitados escondem.
 */
#define _GNU_SOURCE 1
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

void *__real_malloc(size_t);
void *__real_calloc(size_t, size_t);
void *__real_realloc(void *, size_t);

static long g_conta = 0;      /* quantas alocações já pediram */
static long g_falha_em = -1;  /* qual delas devolve NULL (-1 = nenhuma) */
static int  g_pronto = 0;
static int  g_contando = 0;

static void oom_init(void)
{
    if (g_pronto) return;
    g_pronto = 1;                       /* antes do getenv: ele pode alocar */
    const char *f = getenv("PS_OOM_FALHA");
    const char *c = getenv("PS_OOM_CONTA");
    if (f) g_falha_em = atol(f);
    if (c && c[0] == '1') g_contando = 1;
}

/* 1 = esta alocação deve falhar.
 *
 * Só as alocações do NOSSO código passam por aqui, e isso é do `--wrap`, não
 * de filtro nenhum: o linker reescreve as chamadas dos objetos que ELE liga.
 * A libgnutls e a glibc já estão ligadas e chamam o `malloc` de verdade
 * direto. Ou seja, um estouro que apareça dentro de biblioteca de terceiro é
 * corrupção de heap causada por NÓS, não fragilidade dela — foi assim que o
 * `free` da libtasn1 apontou pro `ps_grade_set`. */
static int falha_agora(void)
{
    oom_init();
    g_conta++;
    return (g_falha_em > 0 && g_conta == g_falha_em);
}

void *__wrap_malloc(size_t n)
{
    if (falha_agora()) return NULL;
    return __real_malloc(n);
}

void *__wrap_calloc(size_t n, size_t t)
{
    if (falha_agora()) return NULL;
    return __real_calloc(n, t);
}

void *__wrap_realloc(void *p, size_t n)
{
    /* O realloc que falha NÃO libera o bloco antigo — é a regra que faz o
     * padrão `x = realloc(x, n)` ser errado, e é o defeito que este harness
     * existe pra pegar. Devolver NULL sem tocar em `p` é o comportamento
     * correto a imitar. */
    if (falha_agora()) return NULL;
    return __real_realloc(p, n);
}

char *__wrap_strdup(const char *s)
{
    if (falha_agora()) return NULL;
    size_t n = strlen(s) + 1;
    char *d = __real_malloc(n);
    if (d) memcpy(d, s, n);
    return d;
}

/* Sai depois do main. Só escreve na contagem, e no descritor 3 pra não sujar
 * stdout/stderr do programa — o driver lê de lá. */
__attribute__((destructor))
static void oom_relata(void)
{
    if (!g_contando) return;
    char b[64];
    int n = snprintf(b, sizeof b, "%ld\n", g_conta);
    if (write(3, b, (size_t)n) < 0) { /* sem cano 3: silêncio */ }
}
