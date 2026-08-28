/*
 * Testes de UNIDADE, em C, chamando as funções direto.
 *
 * POR QUE ISTO EXISTE: a suíte inteira (`teste/ps_teste.c`, 7898 casos) é
 * fork/exec do `./pool` com um fonte `.ps`. Isso é deliberado e está certo pro
 * que ela testa — caso que MATA a VM só relata alguma coisa se rodar em outro
 * processo. Mas cria um teto: metade dos ramos de um módulo em C é tratamento
 * de erro (`malloc` que devolveu NULL, buffer curto, entrada truncada, faixa
 * invertida), e **não existe programa PoolScript que faça uma alocação
 * falhar**. Por construção, aquela suíte para por volta de 60% de ramo.
 *
 * Este binário fura o teto: linka os mesmos `.c` e chama as funções pela API
 * pública, passando o que a linguagem nunca conseguiria passar. É o mesmo
 * papel do `_testcapi` do CPython e do `test1.c` do SQLite.
 *
 * Só entram módulos com header público e SEM dependência da VM:
 *
 *   ps_hash.c    SHA-256/384/512, HMAC, PBKDF2, base64, comparação constante
 *   ps_regex.c   compilação e casamento
 *   ps_ast.c     arena e nós
 *
 * O resto do motor é `static` dentro de `poolscript_vm.c` e não tem como ser
 * chamado daqui — pra esses, o caminho continua sendo `.ps` e injeção de falha.
 *
 *     make unidade && ./unidade
 *     ./unidade hash          # só o grupo que casa com o filtro
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#include "ps_hash.h"
#include "ps_regex.h"
#include "ps_ast.h"

static int falhas = 0;
static int total = 0;
static const char *filtro = NULL;
static const char *grupo_atual = "";

static void grupo(const char *nome) { grupo_atual = nome; }

static int pula(void)
{
    return filtro && !strstr(grupo_atual, filtro);
}

#define CONF(cond, ...)                                                     \
    do {                                                                    \
        if (pula()) break;                                                  \
        total++;                                                            \
        if (!(cond)) {                                                      \
            falhas++;                                                       \
            printf("  FALHOU [%s] ", grupo_atual);                          \
            printf(__VA_ARGS__);                                            \
            printf("\n    em %s:%d\n", __FILE__, __LINE__);                 \
        }                                                                   \
    } while (0)

/* hex de um digest, pra comparar com vetor conhecido */
static void hex(const unsigned char *d, size_t n, char *out)
{
    static const char *H = "0123456789abcdef";
    for (size_t i = 0; i < n; i++) { out[i*2] = H[d[i] >> 4]; out[i*2+1] = H[d[i] & 15]; }
    out[n*2] = '\0';
}

/* ── ps_hash.c ───────────────────────────────────────────────────────────── */

static void teste_sha(void)
{
    grupo("hash/sha");
    unsigned char d[PS_SHA512_TAM];
    char h[PS_SHA512_TAM * 2 + 1];

    /* vetores do FIPS 180-4 — string vazia e "abc" */
    ps_sha256((const unsigned char *)"", 0, d);
    hex(d, PS_SHA256_TAM, h);
    CONF(!strcmp(h, "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855"),
         "sha256(\"\") = %s", h);

    ps_sha256((const unsigned char *)"abc", 3, d);
    hex(d, PS_SHA256_TAM, h);
    CONF(!strcmp(h, "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad"),
         "sha256(\"abc\") = %s", h);

    ps_sha384((const unsigned char *)"abc", 3, d);
    hex(d, PS_SHA384_TAM, h);
    CONF(!strcmp(h, "cb00753f45a35e8bb5a03d699ac65007272c32ab0eded1631a8b605a43ff5bed"
                    "8086072ba1e7cc2358baeca134c825a7"),
         "sha384(\"abc\") = %s", h);

    ps_sha512((const unsigned char *)"abc", 3, d);
    hex(d, PS_SHA512_TAM, h);
    CONF(!strcmp(h, "ddaf35a193617abacc417349ae20413112e6fa4e89a97ea20a9eeee64b55d39a"
                    "2192992a274fc1a836ba3c23a3feebbd454d4423643ce80e2a9ac94fa54ca49f"),
         "sha512(\"abc\") = %s", h);

    /* O bloco do SHA-2 é de 64 bytes (128 no 512) e o padding tem TRÊS
     * caminhos: sobra pouco, sobra exatamente, não sobra. Entrada de 55, 56,
     * 63, 64 e 119..128 bytes passa por todos — é onde mora o bug clássico de
     * implementação de hash, e nenhum programa `.ps` escolhe esses tamanhos
     * de propósito. */
    static const size_t bordas[] = { 54, 55, 56, 57, 63, 64, 65, 111, 112, 119, 127, 128, 129 };
    unsigned char buf[200];
    memset(buf, 'x', sizeof(buf));
    for (size_t i = 0; i < sizeof(bordas)/sizeof(bordas[0]); i++) {
        ps_sha256(buf, bordas[i], d);
        int zerado = 1;
        for (int k = 0; k < PS_SHA256_TAM; k++) if (d[k]) zerado = 0;
        CONF(!zerado, "sha256 de %zu bytes deu digest zerado", bordas[i]);
        ps_sha512(buf, bordas[i], d);
        zerado = 1;
        for (int k = 0; k < PS_SHA512_TAM; k++) if (d[k]) zerado = 0;
        CONF(!zerado, "sha512 de %zu bytes deu digest zerado", bordas[i]);
    }
}

static void teste_hmac(void)
{
    grupo("hash/hmac");
    unsigned char d[PS_SHA512_TAM];
    char h[PS_SHA512_TAM * 2 + 1];

    /* RFC 4231, caso 1 */
    unsigned char chave[20];
    memset(chave, 0x0b, sizeof(chave));
    ps_hmac_sha256(chave, sizeof(chave), (const unsigned char *)"Hi There", 8, d);
    hex(d, PS_SHA256_TAM, h);
    CONF(!strcmp(h, "b0344c61d8db38535ca8afceaf0bf12b881dc200c9833da726e9376c2e32cff7"),
         "hmac-sha256 caso 1 = %s", h);

    /* RFC 4231, caso 2: chave e dados em texto */
    ps_hmac_sha256((const unsigned char *)"Jefe", 4,
                   (const unsigned char *)"what do ya want for nothing?", 28, d);
    hex(d, PS_SHA256_TAM, h);
    CONF(!strcmp(h, "5bdcc146bf60754e6a042426089575c75a003f089d2739839dec58b964ec3843"),
         "hmac-sha256 caso 2 = %s", h);

    /* Chave MAIOR que o bloco (64 bytes) é o ramo que hasheia a chave antes de
     * usar — caso 4 do RFC. Chave curta é o outro ramo (padding com zero).
     * Os dois só se separam aqui; um `.ps` que use HMAC pega um dos dois. */
    unsigned char longa[131];
    memset(longa, 0xaa, sizeof(longa));
    ps_hmac_sha256(longa, sizeof(longa),
                   (const unsigned char *)"Test Using Larger Than Block-Size Key - Hash Key First", 54, d);
    hex(d, PS_SHA256_TAM, h);
    CONF(!strcmp(h, "60e431591ee0b67f0d8a26aacbf5b77f8e0bc6213728c5140546040f0ee37f54"),
         "hmac-sha256 chave longa = %s", h);

    /* ps_hmac genérico, com os três tamanhos de digest */
    size_t n = ps_hmac(PS_SHA256_TAM, chave, sizeof(chave), (const unsigned char *)"x", 1, d);
    CONF(n == PS_SHA256_TAM, "ps_hmac(256) devolveu %zu", n);
    n = ps_hmac(PS_SHA384_TAM, chave, sizeof(chave), (const unsigned char *)"x", 1, d);
    CONF(n == PS_SHA384_TAM, "ps_hmac(384) devolveu %zu", n);
    n = ps_hmac(PS_SHA512_TAM, chave, sizeof(chave), (const unsigned char *)"x", 1, d);
    CONF(n == PS_SHA512_TAM, "ps_hmac(512) devolveu %zu", n);
    /* tamanho que não existe: tem que recusar, não inventar */
    n = ps_hmac(7, chave, sizeof(chave), (const unsigned char *)"x", 1, d);
    CONF(n == 0, "ps_hmac com digest invalido devolveu %zu (devia ser 0)", n);
}

static void teste_pbkdf2(void)
{
    grupo("hash/pbkdf2");
    unsigned char d[32];
    char h[65];

    /* Saída é FIXA em 32 bytes (dkLen = hLen), então não há laço de contador
     * pra exercitar — o vetor do RFC de saída longa não se aplica aqui. */
    ps_pbkdf2_sha256((const unsigned char *)"password", 8,
                     (const unsigned char *)"salt", 4, 1, d);
    hex(d, 32, h);
    CONF(!strcmp(h, "120fb6cffcf8b32c43e7225256c4f837a86548c92ccc35480805987cb70be17b"),
         "pbkdf2 c=1 = %s", h);

    ps_pbkdf2_sha256((const unsigned char *)"password", 8,
                     (const unsigned char *)"salt", 4, 2, d);
    hex(d, 32, h);
    CONF(!strcmp(h, "ae4d0c95af6b46d32d0adff928f06dd02a303f8ef3c251dfd6e2d85a95474c43"),
         "pbkdf2 c=2 = %s", h);

    /* muitas iteracoes: o laco do XOR acumulado, e sal vazio */
    ps_pbkdf2_sha256((const unsigned char *)"pw", 2, (const unsigned char *)"", 0, 100, d);
    int zerado = 1;
    for (int i = 0; i < 32; i++) if (d[i]) zerado = 0;
    CONF(!zerado, "pbkdf2 com sal vazio saiu zerado");
    /* zero iteracao e entrada degenerada: nao pode travar nem ler fora */
    ps_pbkdf2_sha256((const unsigned char *)"", 0, (const unsigned char *)"", 0, 0, d);
    CONF(1, "pbkdf2 com 0 iteracoes sobreviveu");
}

static void teste_base64(void)
{
    grupo("hash/base64");
    char cod[256];
    unsigned char dec[256];

    /* RFC 4648: os três restos (0, 1 e 2 bytes) são três caminhos de padding */
    struct { const char *entra; const char *sai; } vet[] = {
        { "",       ""         },
        { "f",      "Zg=="     },
        { "fo",     "Zm8="     },
        { "foo",    "Zm9v"     },
        { "foob",   "Zm9vYg==" },
        { "fooba",  "Zm9vYmE=" },
        { "foobar", "Zm9vYmFy" },
    };
    for (size_t i = 0; i < sizeof(vet)/sizeof(vet[0]); i++) {
        size_t n = ps_base64_encode((const unsigned char *)vet[i].entra,
                                    strlen(vet[i].entra), cod);
        cod[n] = '\0';
        CONF(!strcmp(cod, vet[i].sai), "base64(\"%s\") = \"%s\", esperado \"%s\"",
             vet[i].entra, cod, vet[i].sai);
        long m = ps_base64_decode(vet[i].sai, strlen(vet[i].sai), dec, sizeof(dec));
        CONF(m == (long)strlen(vet[i].entra), "decode(\"%s\") devolveu %ld", vet[i].sai, m);
        if (m > 0) CONF(!memcmp(dec, vet[i].entra, (size_t)m), "decode(\"%s\") deu conteudo errado", vet[i].sai);
    }

    /* ENTRADA TORTA — o que a linguagem não manda e um atacante manda.
     * Padding inválido virando "dado parcial em vez de erro" já foi achado
     * uma vez (AUDITORIA-PROFUNDA §8); estes casos são o que impede a volta. */
    CONF(ps_base64_decode("Zg=", 3, dec, sizeof(dec)) < 0, "padding curto foi aceito");
    CONF(ps_base64_decode("Z", 1, dec, sizeof(dec)) < 0, "1 caractere solto foi aceito");
    CONF(ps_base64_decode("Zm9v!!!!", 8, dec, sizeof(dec)) < 0, "caractere invalido foi aceito");
    CONF(ps_base64_decode("Zm9=v", 5, dec, sizeof(dec)) < 0, "padding no meio foi aceito");
    /* buffer de saída pequeno demais: tem que recusar, não escrever fora */
    unsigned char apertado[2];
    CONF(ps_base64_decode("Zm9vYmFy", 8, apertado, sizeof(apertado)) < 0,
         "decode escreveu num buffer menor que o resultado");

    /* As quatro combinações de urlsafe x padding. O JWT usa urlsafe SEM
     * padding, e errar qualquer um dos dois gera token que não valida em lugar
     * nenhum — são quatro ramos e a linguagem só passa por um. */
    size_t n = ps_base64_encode_ex((const unsigned char *)"fo", 2, cod, 0, 0);
    cod[n] = '\0';
    CONF(!strcmp(cod, "Zm8"), "base64 sem padding = \"%s\"", cod);
    n = ps_base64_encode_ex((const unsigned char *)"fo", 2, cod, 0, 1);
    cod[n] = '\0';
    CONF(!strcmp(cod, "Zm8="), "base64 com padding = \"%s\"", cod);
    /* 0xfb 0xff força os caracteres 62 e 63, que são os que mudam no urlsafe */
    const unsigned char bin[] = { 0xfb, 0xff, 0xbf };
    n = ps_base64_encode_ex(bin, 3, cod, 0, 1);
    cod[n] = '\0';
    CONF(strchr(cod, '+') || strchr(cod, '/'), "padrao nao usou +/ : \"%s\"", cod);
    n = ps_base64_encode_ex(bin, 3, cod, 1, 1);
    cod[n] = '\0';
    CONF(!strchr(cod, '+') && !strchr(cod, '/'), "urlsafe deixou +/ : \"%s\"", cod);
    CONF(strchr(cod, '-') || strchr(cod, '_'), "urlsafe nao usou -_ : \"%s\"", cod);
    /* e o decode aceita as DUAS variantes, que é o contrato do header */
    long m = ps_base64_decode(cod, n, dec, sizeof(dec));
    CONF(m == 3 && !memcmp(dec, bin, 3), "decode de urlsafe deu %ld", m);
}

static void teste_util(void)
{
    grupo("hash/util");
    unsigned char a[32], b[32];
    memset(a, 0x5a, sizeof(a));
    memcpy(b, a, sizeof(b));
    CONF(ps_iguais_constante(a, b, sizeof(a)) == 1, "iguais nao bateram");
    b[31] ^= 1;
    CONF(ps_iguais_constante(a, b, sizeof(a)) == 0, "diferentes bateram");
    b[31] ^= 1; b[0] ^= 0x80;
    CONF(ps_iguais_constante(a, b, sizeof(a)) == 0, "diferenca no 1o byte passou");
    CONF(ps_iguais_constante(a, b, 0) == 1, "comparacao de 0 bytes nao e igual");

    /* CSPRNG: o contrato é encher tudo e não devolver o mesmo duas vezes.
     * Não dá pra testar aleatoriedade num teste unitário, mas dá pra pegar o
     * modo de falhar que importa — devolver buffer intocado. */
    unsigned char r1[64], r2[64];
    memset(r1, 0, sizeof(r1));
    memset(r2, 0, sizeof(r2));
    CONF(ps_random_bytes(r1, sizeof(r1)) == 0, "ps_random_bytes falhou");
    CONF(ps_random_bytes(r2, sizeof(r2)) == 0, "ps_random_bytes falhou (2)");
    CONF(memcmp(r1, r2, sizeof(r1)) != 0, "duas chamadas deram os MESMOS bytes");
    int tudo_zero = 1;
    for (size_t i = 0; i < sizeof(r1); i++) if (r1[i]) tudo_zero = 0;
    CONF(!tudo_zero, "ps_random_bytes deixou o buffer todo zero");
    CONF(ps_random_bytes(r1, 0) == 0, "ps_random_bytes(0) devia ser sucesso trivial");
}

/* ── ps_regex.c ──────────────────────────────────────────────────────────── */

static void teste_regex(void)
{
    grupo("regex");
    char erro[256];

    /* PADRÃO INVÁLIDO: cada mensagem é um ramo, e a linguagem só alcança
     * alguns deles porque o lexer barra antes. */
    struct { const char *p; const char *diz; } ruins[] = {
        { "[",        "classe nao fechada"  },
        { "[a-",      NULL                  },
        { "[z-a]",    "faixa invertida"     },
        { "(",        NULL                  },
        { ")",        NULL                  },
        { "a{2,1}",   NULL                  },
        { "*",        NULL                  },
        { "\\",       NULL                  },
        { "(?P<",     NULL                  },
        { "(?",       NULL                  },
    };
    for (size_t i = 0; i < sizeof(ruins)/sizeof(ruins[0]); i++) {
        erro[0] = '\0';
        PSRegex *r = ps_regex_compila(ruins[i].p, (int)strlen(ruins[i].p), erro, sizeof(erro));
        CONF(r == NULL, "padrao invalido \"%s\" COMPILOU", ruins[i].p);
        if (r) { ps_regex_free(r); continue; }
        CONF(erro[0] != '\0', "padrao \"%s\" falhou sem mensagem", ruins[i].p);
        if (ruins[i].diz)
            CONF(strstr(erro, ruins[i].diz) != NULL,
                 "padrao \"%s\": esperava \"%s\", veio \"%s\"", ruins[i].p, ruins[i].diz, erro);
    }

    /* classe negada dentro de [] — era erro de sintaxe até 28/08 */
    PSRegex *r = ps_regex_compila("[\\s\\S]", 6, erro, sizeof(erro));
    CONF(r != NULL, "[\\s\\S] nao compilou: %s", erro);
    if (r) {
        RxCaptura cap;
        CONF(ps_regex_busca(r, "\n", 1, 0, &cap) == 1, "[\\s\\S] nao casou \\n");
        ps_regex_free(r);
    }

    /* grupos nomeados: nome existente, nome inexistente, índice fora */
    r = ps_regex_compila("(?P<ano>\\d{4})-(?P<mes>\\d{2})", 30, erro, sizeof(erro));
    CONF(r != NULL, "grupo nomeado nao compilou: %s", erro);
    if (r) {
        CONF(ps_regex_ngrupos(r) == 2, "ngrupos = %d", ps_regex_ngrupos(r));
        CONF(ps_regex_grupo_por_nome(r, "ano", 3) == 1, "grupo `ano` nao e 1");
        CONF(ps_regex_grupo_por_nome(r, "mes", 3) == 2, "grupo `mes` nao e 2");
        CONF(ps_regex_grupo_por_nome(r, "dia", 3) < 0, "grupo inexistente foi achado");
        const char *nome = ps_regex_nome_do_grupo(r, 1);
        CONF(nome && !strcmp(nome, "ano"), "nome do grupo 1 = %s", nome ? nome : "(null)");
        CONF(ps_regex_nome_do_grupo(r, 99) == NULL, "grupo 99 devolveu nome");
        CONF(ps_regex_nome_do_grupo(r, -1) == NULL, "grupo -1 devolveu nome");
        ps_regex_free(r);
    }

    /* busca a partir de um deslocamento, e além do fim */
    r = ps_regex_compila("ab", 2, erro, sizeof(erro));
    CONF(r != NULL, "padrao simples nao compilou");
    if (r) {
        RxCaptura cap;
        CONF(ps_regex_busca(r, "xxabxx", 6, 0, &cap) == 1, "nao achou em 0");
        CONF(ps_regex_busca(r, "xxabxx", 6, 4, &cap) == 0, "achou depois do fim do casamento");
        CONF(ps_regex_busca(r, "", 0, 0, &cap) == 0, "casou em string vazia");
        CONF(ps_regex_casa_tudo(r, "ab", 2, &cap) == 1, "casa_tudo nao casou exato");
        CONF(ps_regex_casa_tudo(r, "abc", 3, &cap) == 0, "casa_tudo casou sobrando");
        ps_regex_free(r);
    }

    /* ps_regex_free(NULL) tem que ser inofensivo — é contrato de todo free */
    ps_regex_free(NULL);
    CONF(1, "free(NULL) sobreviveu");
}

/* ── ps_ast.c ────────────────────────────────────────────────────────────── */

static void teste_ast(void)
{
    grupo("ast");
    PSArena a;
    ps_arena_init(&a);

    /* alocação normal, alocação grande (força bloco novo) e alinhamento */
    void *p1 = ps_arena_alloc(&a, 8);
    void *p2 = ps_arena_alloc(&a, 1);
    void *p3 = ps_arena_alloc(&a, 1024 * 1024);
    CONF(p1 && p2 && p3, "arena devolveu NULL numa alocacao normal");
    CONF(((uintptr_t)p2 % sizeof(void *)) == 0, "arena devolveu ponteiro desalinhado");
    CONF(ps_arena_alloc(&a, 0) != NULL || 1, "alloc(0) nao quebrou");

    char *s = ps_arena_strdup(&a, "abc", 3);
    CONF(s && !strcmp(s, "abc"), "strdup na arena deu \"%s\"", s ? s : "(null)");
    char *vazio = ps_arena_strdup(&a, "", 0);
    CONF(vazio && vazio[0] == '\0', "strdup de vazio nao terminou em NUL");
    /* len menor que a string: tem que cortar E terminar */
    char *corta = ps_arena_strdup(&a, "abcdef", 3);
    CONF(corta && !strcmp(corta, "abc"), "strdup com len curto deu \"%s\"", corta ? corta : "(null)");

    PSNode *n = ps_node_novo(&a, N_LITERAL, 7, 3);
    CONF(n != NULL, "ps_node_novo devolveu NULL");
    CONF(n && n->kind == N_LITERAL, "kind errado");
    CONF(n && n->line == 7 && n->col == 3, "linha/coluna erradas");

    /* o vetor cresce por dobra: 40 itens passam por várias realocações */
    PSNodeVec v;
    memset(&v, 0, sizeof(v));
    for (int i = 0; i < 40; i++)
        CONF(ps_vec_push(&a, &v, ps_node_novo(&a, N_NAME, i, 0)) == 0,
             "push %d falhou", i);
    CONF(v.n == 40, "vetor com %d itens depois de 40 pushes", v.n);
    CONF(v.itens[39] != NULL && v.itens[39]->line == 39, "item 39 errado");

    ps_arena_free(&a);
    /* free de arena já liberada / nunca usada: contrato de idempotência */
    PSArena zerada;
    ps_arena_init(&zerada);
    ps_arena_free(&zerada);
    CONF(1, "arena vazia sobreviveu ao free");

    /* TODO kind tem nome. Sem isto, um nó novo entra e vira "?" no dump do
     * parser sem ninguém notar — e "?" é o tipo de defeito que só aparece
     * quando alguém está depurando outra coisa. */
    grupo("ast/nomes");
    int sem_nome = 0;
    for (int k = 0; k <= N_UNPACK_TARGET; k++) {
        const char *nm = ps_node_nome((PSNodeKind)k);
        if (!nm || !strcmp(nm, "?")) { sem_nome++; printf("  kind %d sem nome\n", k); }
    }
    CONF(sem_nome == 0, "%d kinds de no sem nome", sem_nome);
    /* e um valor fora da faixa tem que cair no "?" em vez de ler lixo */
    CONF(!strcmp(ps_node_nome((PSNodeKind)9999), "?"), "kind invalido nao virou \"?\"");
}

/* Compila, casa contra `alvo` e confere o TRECHO casado. `espera` NULL = tem
 * que não casar. Devolve 1 se o teste passou. */
static int casa(const char *padrao, const char *alvo, const char *espera)
{
    char erro[256] = {0};
    PSRegex *r = ps_regex_compila(padrao, (int)strlen(padrao), erro, sizeof(erro));
    if (!r) {
        printf("  FALHOU [regex] \"%s\" nao compilou: %s\n", padrao, erro);
        return 0;
    }
    RxCaptura cap;
    int achou = ps_regex_busca(r, alvo, (int)strlen(alvo), 0, &cap);
    int ok;
    if (!espera) {
        ok = (achou == 0);
        if (!ok) printf("  FALHOU [regex] \"%s\" casou \"%s\" e nao devia\n", padrao, alvo);
    } else if (achou != 1) {
        ok = 0;
        printf("  FALHOU [regex] \"%s\" nao casou \"%s\"\n", padrao, alvo);
    } else {
        int n = cap.fim[0] - cap.inicio[0];
        ok = (n == (int)strlen(espera) && !memcmp(alvo + cap.inicio[0], espera, (size_t)n));
        if (!ok)
            printf("  FALHOU [regex] \"%s\" em \"%s\": casou \"%.*s\", esperado \"%s\"\n",
                   padrao, alvo, n, alvo + cap.inicio[0], espera);
    }
    ps_regex_free(r);
    return ok;
}

/* Como `casa`, mas confere o conteúdo de um GRUPO. */
static int grupo_e(const char *padrao, const char *alvo, int g, const char *espera)
{
    char erro[256] = {0};
    PSRegex *r = ps_regex_compila(padrao, (int)strlen(padrao), erro, sizeof(erro));
    if (!r) { printf("  FALHOU [regex] \"%s\" nao compilou: %s\n", padrao, erro); return 0; }
    RxCaptura cap;
    int ok = 0;
    if (ps_regex_busca(r, alvo, (int)strlen(alvo), 0, &cap) == 1 && g <= cap.ngrupos
            && cap.inicio[g] >= 0) {
        int n = cap.fim[g] - cap.inicio[g];
        ok = (n == (int)strlen(espera) && !memcmp(alvo + cap.inicio[g], espera, (size_t)n));
        if (!ok) printf("  FALHOU [regex] \"%s\" grupo %d = \"%.*s\", esperado \"%s\"\n",
                        padrao, g, n, alvo + cap.inicio[g], espera);
    } else {
        printf("  FALHOU [regex] \"%s\" nao casou/nao capturou grupo %d em \"%s\"\n",
               padrao, g, alvo);
    }
    ps_regex_free(r);
    return ok;
}

/* Famílias inteiras do motor de regex que a linguagem exercita de raspão.
 * Cada bloco aqui vale por uma região de dezenas de ramos que estava em zero:
 * flags inline, âncoras de palavra, retrovisor, lookaround, quantificador
 * contado, case-insensitive e UTF-8. */
static void teste_regex_fundo(void)
{
    grupo("regex/flags");
    /* (?i) ligada no padrão inteiro, e com escopo `(?i:...)` */
    CONF(casa("(?i)abc", "xxABCxx", "ABC"), "(?i) nao ignorou caixa");
    CONF(casa("(?i)[a-z]+", "XYZ", "XYZ"), "(?i) nao valeu na classe");
    CONF(casa("(?i:ab)c", "ABc", "ABc"), "(?i:...) com escopo falhou");
    CONF(casa("(?i:ab)c", "ABC", NULL), "(?i:...) vazou pro resto do padrao");
    /* (?s): o ponto passa a casar \n */
    CONF(casa("a.b", "a\nb", NULL), "ponto casou \\n sem (?s)");
    CONF(casa("(?s)a.b", "a\nb", "a\nb"), "(?s) nao fez o ponto casar \\n");
    /* (?m): ^ e $ por linha */
    CONF(casa("^b", "a\nb", NULL), "^ casou no meio sem (?m)");
    CONF(casa("(?m)^b", "a\nb", "b"), "(?m) nao fez ^ valer por linha");
    CONF(casa("(?m)a$", "a\nb", "a"), "(?m) nao fez $ valer por linha");

    grupo("regex/ancoras");
    CONF(casa("\\bfoo\\b", "um foo aqui", "foo"), "\\b nao achou palavra isolada");
    CONF(casa("\\bfoo\\b", "umfooaqui", NULL), "\\b casou dentro de palavra");
    CONF(casa("\\Boo", "foo", "oo"), "\\B nao casou dentro de palavra");
    CONF(casa("\\Bfoo", "foo bar", NULL), "\\B casou no comeco");
    CONF(casa("\\Aabc", "abc", "abc"), "\\A nao casou no inicio");
    CONF(casa("(?m)\\Ab", "a\nb", NULL), "\\A cedeu ao MULTILINE (nao devia)");
    CONF(casa("abc\\Z", "xabc", "abc"), "\\Z nao casou no fim");
    CONF(casa("a\\Z", "a\nb", NULL), "\\Z casou antes do fim");

    grupo("regex/retrovisor");
    CONF(casa("(ab)\\1", "abab", "abab"), "retrovisor \\1 nao casou");
    CONF(casa("(ab)\\1", "abcd", NULL), "retrovisor casou coisa diferente");
    CONF(casa("(a)(b)\\2\\1", "abba", "abba"), "\\1 e \\2 juntos falharam");
    CONF(casa("(?i)(ab)\\1", "abAB", "abAB"), "retrovisor nao respeitou (?i)");

    grupo("regex/lookaround");
    CONF(casa("foo(?=bar)", "foobar", "foo"), "lookahead positivo falhou");
    CONF(casa("foo(?=bar)", "foobaz", NULL), "lookahead positivo casou errado");
    CONF(casa("foo(?!bar)", "foobaz", "foo"), "lookahead negativo falhou");
    CONF(casa("foo(?!bar)", "foobar", NULL), "lookahead negativo casou errado");
    CONF(casa("(?<=R\\$)\\d+", "R$42", "42"), "lookbehind positivo falhou");
    CONF(casa("(?<=R\\$)\\d+", "US42", NULL), "lookbehind positivo casou errado");
    CONF(casa("(?<!R\\$)\\d+", "US42", "42"), "lookbehind negativo falhou");
    /* lookbehind de largura variável não é suportado — tem que RECUSAR na
     * compilação, não casar errado em silêncio */
    {
        char erro[256] = {0};
        PSRegex *r = ps_regex_compila("(?<=a+)b", 8, erro, sizeof(erro));
        CONF(r == NULL, "lookbehind de largura variavel foi aceito");
        if (r) ps_regex_free(r);
    }

    grupo("regex/quantificador");
    CONF(casa("a{3}", "aaaa", "aaa"), "{3} exato falhou");
    CONF(casa("a{2,}", "aaaa", "aaaa"), "{2,} sem teto falhou");
    CONF(casa("a{2,3}", "aaaa", "aaa"), "{2,3} falhou");
    CONF(casa("a{4}", "aaa", NULL), "{4} casou com 3");
    CONF(casa("a{0,2}b", "b", "b"), "{0,2} com zero falhou");
    /* preguiçoso: `+?` e `{n,m}?` param no primeiro que serve */
    CONF(casa("a+?", "aaa", "a"), "+? nao foi preguicoso");
    CONF(casa("<.+?>", "<a><b>", "<a>"), ".+? pegou demais");
    CONF(casa("a{2,3}?", "aaa", "aa"), "{2,3}? nao foi preguicoso");
    CONF(casa("a??b", "ab", "ab"), "?? falhou");

    grupo("regex/utf8");
    /* codepoint de 2, 3 e 4 bytes: o `.` tem que consumir o caractere
     * INTEIRO, senão o casamento parte um UTF-8 no meio */
    CONF(casa(".", "\xc3\xa7", "\xc3\xa7"), "ponto partiu um 2-bytes");
    CONF(casa(".", "\xe2\x82\xac", "\xe2\x82\xac"), "ponto partiu um 3-bytes");
    CONF(casa(".", "\xf0\x9f\x98\x80", "\xf0\x9f\x98\x80"), "ponto partiu um 4-bytes");
    CONF(casa("[^x]", "\xc3\xa7", "\xc3\xa7"), "classe negada partiu um 2-bytes");
    CONF(casa("\\w+", "a\xc3\xa7\x61o", "a\xc3\xa7\x61o"), "\\w nao pegou acento");
    CONF(casa("[\xc3\xa1-\xc3\xba]", "\xc3\xa7", "\xc3\xa7"), "faixa fora do ASCII falhou");

    grupo("regex/grupos");
    CONF(grupo_e("(\\d+)-(\\d+)", "ab 12-34", 1, "12"), "grupo 1");
    CONF(grupo_e("(\\d+)-(\\d+)", "ab 12-34", 2, "34"), "grupo 2");
    CONF(grupo_e("(?P<ano>\\d{4})", "em 2026", 1, "2026"), "grupo nomeado");
    /* grupo não capturante NÃO conta */
    {
        char erro[256] = {0};
        PSRegex *r = ps_regex_compila("(?:a)(b)", 8, erro, sizeof(erro));
        CONF(r != NULL, "(?:...) nao compilou: %s", erro);
        if (r) { CONF(ps_regex_ngrupos(r) == 1, "(?:...) contou como capturante"); ps_regex_free(r); }
    }
    /* alternância com grupo que não participa: o grupo fica sem posição */
    {
        char erro[256] = {0};
        PSRegex *r = ps_regex_compila("(a)|(b)", 7, erro, sizeof(erro));
        CONF(r != NULL, "alternancia nao compilou");
        if (r) {
            RxCaptura cap;
            CONF(ps_regex_busca(r, "b", 1, 0, &cap) == 1, "alternancia nao casou");
            CONF(cap.inicio[1] < 0, "grupo que nao participou veio com posicao");
            CONF(cap.inicio[2] == 0, "grupo que participou veio sem posicao");
            ps_regex_free(r);
        }
    }

    grupo("regex/vazio");
    /* padrão que casa vazio é a fonte clássica de laço infinito em findall */
    CONF(casa("a*", "bbb", ""), "a* nao casou vazio");
    CONF(casa("", "abc", ""), "padrao vazio nao casou vazio");
    CONF(casa("()", "abc", ""), "grupo vazio nao casou vazio");
    CONF(casa("(a*)*b", "b", "b"), "estrela sobre estrela travou ou nao casou");
}

int main(int argc, char **argv)
{
    if (argc > 1) filtro = argv[1];

    teste_sha();
    teste_hmac();
    teste_pbkdf2();
    teste_base64();
    teste_util();
    teste_regex();
    teste_regex_fundo();
    teste_ast();

    printf("\nunidade: %d checagens, %d falharam\n", total, falhas);
    return falhas ? 1 : 0;
}
