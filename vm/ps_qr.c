/*
 * QR Code — ver ps_qr.h. Implementa ISO/IEC 18004 em modo byte.
 *
 * Passos: escolhe a versão, monta o fluxo de bits (modo + contagem + dados +
 * terminador + padding), quebra em blocos, calcula Reed-Solomon por bloco,
 * intercala data+EC, desenha os padrões fixos, distribui os bits em zigue-zague,
 * testa as 8 máscaras pela penalidade do padrão e grava a escolhida com o
 * format/version info. A matriz final é a do padrão ISO/IEC 18004.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ps_qr.h"
#include "ps_qr_tables.h"

/* ── GF(256) pro Reed-Solomon ───────────────────────────────────────────── */
static uint8_t GEXP[512], GLOG[256];
static int gf_pronto = 0;

static void gf_init(void)
{
    if (gf_pronto) return;
    int x = 1;
    for (int i = 0; i < 255; i++) {
        GEXP[i] = (uint8_t)x;
        GLOG[x] = (uint8_t)i;
        x <<= 1;
        if (x & 0x100) x ^= 0x11d;      /* polinômio primitivo do QR */
    }
    for (int i = 255; i < 512; i++) GEXP[i] = GEXP[i - 255];
    gf_pronto = 1;
}

static uint8_t gf_mul(uint8_t a, uint8_t b)
{
    if (!a || !b) return 0;
    return GEXP[GLOG[a] + GLOG[b]];
}

/* Coeficientes do polinômio gerador de grau `n`. */
static void rs_gerador(int n, uint8_t *g)
{
    g[0] = 1;
    int len = 1;
    for (int i = 0; i < n; i++) {
        /* multiplica por (x - α^i) */
        g[len] = 0;
        for (int j = len; j > 0; j--)
            g[j] = g[j - 1] ^ gf_mul(g[j], GEXP[i]);
        g[0] = gf_mul(g[0], GEXP[i]);
        len++;
    }
}

/* EC codewords de um bloco: divisão polinomial. */
static void rs_ec(const uint8_t *dados, int nd, int nec, uint8_t *ec)
{
    uint8_t g[64];
    rs_gerador(nec, g);
    uint8_t r[256];
    memset(r, 0, sizeof(r));
    memcpy(r, dados, (size_t)nd);
    for (int i = 0; i < nd; i++) {
        uint8_t coef = r[i];
        if (coef == 0) continue;
        /* g[nec] é o líder (=1); alinhar ele com r[i], não o termo constante */
        for (int j = 0; j <= nec; j++)
            r[i + j] ^= gf_mul(g[nec - j], coef);
    }
    memcpy(ec, r + nd, (size_t)nec);
}

/* ── fluxo de bits ──────────────────────────────────────────────────────── */
typedef struct { uint8_t *b; int nbits, cap; } Bits;

static void bits_push(Bits *bt, uint32_t val, int n)
{
    for (int i = n - 1; i >= 0; i--) {
        int byte = bt->nbits >> 3, off = 7 - (bt->nbits & 7);
        if ((val >> i) & 1) bt->b[byte] |= (uint8_t)(1 << off);
        bt->nbits++;
    }
}

static int nivel_idx(char n)
{
    switch (n) { case 'L': return 0; case 'M': return 1;
                 case 'Q': return 2; case 'H': return 3; default: return -1; }
}

static int capacidade_dados(int v, int nv)
{
    int off = QR_BLOCO_OFF[v][nv], nb = QR_NBLOCOS[v][nv], t = 0;
    for (int i = 0; i < nb; i++) t += QR_BLOCOS[off + i].dados;
    return t;
}

/* bits do indicador de contagem em modo byte: 8 (v1-9) ou 16 (v10-40) */
static int count_bits(int v) { return v <= 9 ? 8 : 16; }

/* ── matriz: padrões fixos ──────────────────────────────────────────────── */
/* célula: bit0 = cor (1 escuro), bit1 = é função (fixo, não recebe dado) */
#define FUNC 2

static void poe(uint8_t *m, int dim, int r, int c, int cor, int func)
{
    if (r < 0 || c < 0 || r >= dim || c >= dim) return;
    m[r * dim + c] = (uint8_t)((cor ? 1 : 0) | (func ? FUNC : 0));
}

static int reservado(const uint8_t *m, int dim, int r, int c)
{
    return (m[r * dim + c] & FUNC) != 0;
}

static void finder(uint8_t *m, int dim, int r0, int c0)
{
    for (int r = -1; r <= 7; r++)
        for (int c = -1; c <= 7; c++) {
            int rr = r0 + r, cc = c0 + c;
            if (rr < 0 || cc < 0 || rr >= dim || cc >= dim) continue;
            int borda = (r >= 0 && r <= 6 && (c == 0 || c == 6))
                     || (c >= 0 && c <= 6 && (r == 0 || r == 6));
            int miolo = (r >= 2 && r <= 4 && c >= 2 && c <= 4);
            poe(m, dim, rr, cc, (borda || miolo) ? 1 : 0, 1);
        }
}

static void alinhamento(uint8_t *m, int dim, int cr, int cc)
{
    for (int r = -2; r <= 2; r++)
        for (int c = -2; c <= 2; c++) {
            int anel = (r == -2 || r == 2 || c == -2 || c == 2);
            int centro = (r == 0 && c == 0);
            poe(m, dim, cr + r, cc + c, (anel || centro) ? 1 : 0, 1);
        }
}

/* Desenha finders, separadores, timing, alinhamento, dark module e RESERVA
 * (marca como função, cor 0) as áreas de format/version info. */
static void monta_funcoes(uint8_t *m, int dim, int v)
{
    finder(m, dim, 0, 0);
    finder(m, dim, 0, dim - 7);
    finder(m, dim, dim - 7, 0);

    /* timing */
    for (int i = 8; i < dim - 8; i++) {
        poe(m, dim, 6, i, (i % 2 == 0) ? 1 : 0, 1);
        poe(m, dim, i, 6, (i % 2 == 0) ? 1 : 0, 1);
    }

    /* alinhamento: pula os que colidem com finder */
    int na = QR_NALIGN[v];
    for (int i = 0; i < na; i++)
        for (int j = 0; j < na; j++) {
            int cr = QR_ALIGN[v][i], cc = QR_ALIGN[v][j];
            int perto_finder =
                (cr <= 8 && cc <= 8) ||
                (cr <= 8 && cc >= dim - 9) ||
                (cr >= dim - 9 && cc <= 8);
            if (!perto_finder) alinhamento(m, dim, cr, cc);
        }

    /* dark module: reservado mas CLARO por ora — a penalidade é avaliada com
     * ele claro, e o valor real (escuro) entra no grava_format da máscara
     * vencedora. */
    poe(m, dim, dim - 8, 8, 0, 1);

    /* reserva format info (cor 0 por ora) */
    for (int i = 0; i < 9; i++) {
        if (i != 6) { poe(m, dim, 8, i, 0, 1); poe(m, dim, i, 8, 0, 1); }
    }
    for (int i = 0; i < 8; i++) {
        poe(m, dim, 8, dim - 1 - i, 0, 1);
        poe(m, dim, dim - 1 - i, 8, 0, 1);
    }

    /* reserva version info (v >= 7): dois blocos 3x6 */
    if (v >= 7) {
        for (int i = 0; i < 6; i++)
            for (int j = 0; j < 3; j++) {
                poe(m, dim, i, dim - 11 + j, 0, 1);
                poe(m, dim, dim - 11 + j, i, 0, 1);
            }
    }
}

/* ── zigue-zague dos dados ──────────────────────────────────────────────── */
static void poe_dados(uint8_t *m, int dim, const uint8_t *fluxo, int nbits)
{
    int bit = 0, dir = -1;   /* começa subindo */
    for (int col = dim - 1; col > 0; col -= 2) {
        if (col == 6) col--;             /* pula a coluna do timing */
        for (int i = 0; i < dim; i++) {
            int row = (dir < 0) ? (dim - 1 - i) : i;
            for (int k = 0; k < 2; k++) {
                int c = col - k;
                if (reservado(m, dim, row, c)) continue;
                int v = 0;
                if (bit < nbits) { v = (fluxo[bit >> 3] >> (7 - (bit & 7))) & 1; bit++; }
                m[row * dim + c] = (uint8_t)v;   /* dado, não função */
            }
        }
        dir = -dir;
    }
}

/* ── máscara ────────────────────────────────────────────────────────────── */
static int mascara_bit(int mask, int r, int c)
{
    switch (mask) {
        case 0: return (r + c) % 2 == 0;
        case 1: return r % 2 == 0;
        case 2: return c % 3 == 0;
        case 3: return (r + c) % 3 == 0;
        case 4: return (r / 2 + c / 3) % 2 == 0;
        case 5: return (r * c) % 2 + (r * c) % 3 == 0;
        case 6: return ((r * c) % 2 + (r * c) % 3) % 2 == 0;
        default: return ((r + c) % 2 + (r * c) % 3) % 2 == 0;
    }
}

/* Penalidade do padrão (as 4 regras da ISO/IEC 18004). */
static int penalidade(const uint8_t *m, int dim)
{
    int p = 0;
    /* regra 1: corridas >= 5 na linha e na coluna */
    for (int r = 0; r < dim; r++) {
        int run = 1;
        for (int c = 1; c < dim; c++) {
            if ((m[r*dim+c] & 1) == (m[r*dim+c-1] & 1)) { run++; if (run == 5) p += 3; else if (run > 5) p++; }
            else run = 1;
        }
    }
    for (int c = 0; c < dim; c++) {
        int run = 1;
        for (int r = 1; r < dim; r++) {
            if ((m[r*dim+c] & 1) == (m[(r-1)*dim+c] & 1)) { run++; if (run == 5) p += 3; else if (run > 5) p++; }
            else run = 1;
        }
    }
    /* regra 2: blocos 2x2 da mesma cor */
    for (int r = 0; r < dim - 1; r++)
        for (int c = 0; c < dim - 1; c++) {
            int a = m[r*dim+c] & 1;
            if (a == (m[r*dim+c+1] & 1) && a == (m[(r+1)*dim+c] & 1) && a == (m[(r+1)*dim+c+1] & 1))
                p += 3;
        }
    /* regra 3: padrão 1:1:3:1:1 com quiet (10111010000 / 00001011101) */
    static const int A[11] = {1,0,1,1,1,0,1,0,0,0,0};
    static const int B[11] = {0,0,0,0,1,0,1,1,1,0,1};
    for (int r = 0; r < dim; r++)
        for (int c = 0; c <= dim - 11; c++) {
            int ok1 = 1, ok2 = 1;
            for (int k = 0; k < 11; k++) {
                int b = m[r*dim+c+k] & 1;
                if (b != A[k]) ok1 = 0;
                if (b != B[k]) ok2 = 0;
            }
            if (ok1 || ok2) p += 40;
        }
    for (int c = 0; c < dim; c++)
        for (int r = 0; r <= dim - 11; r++) {
            int ok1 = 1, ok2 = 1;
            for (int k = 0; k < 11; k++) {
                int b = m[(r+k)*dim+c] & 1;
                if (b != A[k]) ok1 = 0;
                if (b != B[k]) ok2 = 0;
            }
            if (ok1 || ok2) p += 40;
        }
    /* regra 4: desvio da proporção de escuros — o padrão define
     * |escuros/total·100 − 50| / 5, truncado. Fazer `dark*100/total` em
     * inteiro arredonda cedo e infla a penalidade — a conta abaixo é essa
     * fração truncada, exata: floor(|100*dark - 50*total| / (5*total)). */
    int escuros = 0;
    for (int i = 0; i < dim * dim; i++) escuros += m[i] & 1;
    int total = dim * dim;
    int num = abs(100 * escuros - 50 * total);
    int k = num / (5 * total);
    p += k * 10;
    return p;
}

/* ── format & version info ──────────────────────────────────────────────── */
static int nivel_format_bits(char n)
{
    /* bits do format info (diferente do índice de tabela): L=01 M=00 Q=11 H=10 */
    switch (n) { case 'L': return 1; case 'M': return 0; case 'Q': return 3; default: return 2; }
}

static uint32_t bch15(uint32_t d)
{
    uint32_t v = d << 10;
    for (int i = 14; i >= 10; i--)
        if ((v >> i) & 1) v ^= 0x537 << (i - 10);   /* G15 = 0b10100110111 */
    return ((d << 10) | v) ^ 0x5412;                /* máscara do format info */
}

static uint32_t bch18(uint32_t v)
{
    uint32_t x = v << 12;
    for (int i = 17; i >= 12; i--)
        if ((x >> i) & 1) x ^= 0x1f25 << (i - 12);  /* G18 = 0b1111100100101 */
    return (v << 12) | x;
}

static void grava_format(uint8_t *m, int dim, char nivel, int mask)
{
    uint32_t f = bch15((uint32_t)((nivel_format_bits(nivel) << 3) | mask));
    /* 15 bits: cópia 1 em volta do finder superior-esquerdo */
    for (int i = 0; i < 15; i++) {
        int bit = (f >> i) & 1;
        /* faixa vertical do finder TL + horizontal */
        int r, c;
        if (i < 6) { r = i; c = 8; }
        else if (i == 6) { r = 7; c = 8; }
        else if (i == 7) { r = 8; c = 8; }
        else if (i == 8) { r = 8; c = 7; }
        else { r = 8; c = 14 - i; }
        m[r*dim+c] = (uint8_t)(bit | FUNC);
        /* cópia 2: distribuída nos outros dois finders */
        if (i < 8) { r = 8; c = dim - 1 - i; }
        else { r = dim - 15 + i; c = 8; }
        m[r*dim+c] = (uint8_t)(bit | FUNC);
    }
    /* dark module garantido */
    m[(dim-8)*dim + 8] = 1 | FUNC;
}

static void grava_version(uint8_t *m, int dim, int v)
{
    if (v < 7) return;
    uint32_t vi = bch18((uint32_t)v);
    for (int i = 0; i < 18; i++) {
        int bit = (vi >> i) & 1;
        int r = i / 3, c = i % 3;
        m[r*dim + (dim - 11 + c)] = (uint8_t)(bit | FUNC);
        m[(dim - 11 + c)*dim + r] = (uint8_t)(bit | FUNC);
    }
}

/* ── montagem final ─────────────────────────────────────────────────────── */
int ps_qr_matriz(const char *dados, int ndados, char nivel,
                 uint8_t **grid, int *dim_out, char *erro, size_t ecap)
{
    gf_init();
    int nv = nivel_idx(nivel);
    if (nv < 0) { snprintf(erro, ecap, "nivel de correcao invalido: %c", nivel); return -1; }

    /* escolhe a menor versão que cabe */
    int v = 0;
    for (int cand = 1; cand <= 40; cand++) {
        int cap_bits = capacidade_dados(cand, nv) * 8;
        int need = 4 + count_bits(cand) + 8 * ndados;
        if (need <= cap_bits) { v = cand; break; }
    }
    if (v == 0) {
        /* o teto do NÍVEL pedido: a frase citava sempre 2953 (o teto de L),
         * e em H o corte real é menos da metade disso */
        int max = (capacidade_dados(40, nv) * 8 - 4 - count_bits(40)) / 8;
        snprintf(erro, ecap, "dados grandes demais pro QR (max %d bytes no nivel %c)", max, nivel);
        return -1;
    }

    int cap_cw = capacidade_dados(v, nv);
    Bits bt;
    bt.cap = cap_cw + 8;
    bt.b = calloc((size_t)bt.cap, 1);
    if (!bt.b) { snprintf(erro, ecap, "sem memoria"); return -1; }
    bt.nbits = 0;
    bits_push(&bt, 0x4, 4);                     /* modo byte */
    bits_push(&bt, (uint32_t)ndados, count_bits(v));
    for (int i = 0; i < ndados; i++) bits_push(&bt, (uint8_t)dados[i], 8);
    /* terminador: até 4 zeros */
    int cap_bits = cap_cw * 8;
    int term = cap_bits - bt.nbits; if (term > 4) term = 4;
    if (term > 0) bits_push(&bt, 0, term);
    /* alinha no byte */
    while (bt.nbits % 8) bits_push(&bt, 0, 1);
    /* padding EC/11 */
    int pad = 0;
    while ((bt.nbits >> 3) < cap_cw) { bits_push(&bt, pad ? 0x11 : 0xEC, 8); pad = !pad; }

    /* blocos: separa dados, calcula EC, intercala */
    int off = QR_BLOCO_OFF[v][nv], nb = QR_NBLOCOS[v][nv];
    const uint8_t *dcw = bt.b;
    uint8_t ecw[81][40];
    const uint8_t *dptr[81];
    int dlen[81], eclen[81], maxd = 0, maxe = 0, pos = 0;
    for (int i = 0; i < nb; i++) {
        int dt = QR_BLOCOS[off + i].dados;
        int et = QR_BLOCOS[off + i].total - dt;
        dptr[i] = dcw + pos;
        dlen[i] = dt; eclen[i] = et;
        rs_ec(dptr[i], dt, et, ecw[i]);
        if (dt > maxd) maxd = dt;
        if (et > maxe) maxe = et;
        pos += dt;
    }
    int total_cw = 0;
    for (int i = 0; i < nb; i++) total_cw += QR_BLOCOS[off + i].total;
    uint8_t *inter = malloc((size_t)total_cw + 2);
    if (!inter) { free(bt.b); snprintf(erro, ecap, "sem memoria"); return -1; }
    int ip = 0;
    for (int c = 0; c < maxd; c++)
        for (int i = 0; i < nb; i++)
            if (c < dlen[i]) inter[ip++] = dptr[i][c];
    for (int c = 0; c < maxe; c++)
        for (int i = 0; i < nb; i++)
            if (c < eclen[i]) inter[ip++] = ecw[i][c];
    free(bt.b);

    /* fluxo de bits final (com remainder em zero) */
    int nbits_final = ip * 8 + QR_REMAINDER[v];
    uint8_t *fluxo = calloc((size_t)(nbits_final + 7) / 8 + 1, 1);
    if (!fluxo) { free(inter); snprintf(erro, ecap, "sem memoria"); return -1; }
    if (getenv("QR_DUMP")) { fprintf(stderr,"CW:"); for(int i=0;i<ip;i++) fprintf(stderr," %d",inter[i]); fprintf(stderr,"\n"); }
    memcpy(fluxo, inter, (size_t)ip);
    free(inter);

    int dim = 17 + 4 * v;
    uint8_t *m = calloc((size_t)dim * dim, 1);
    uint8_t *melhor = calloc((size_t)dim * dim, 1);
    if (!m || !melhor) { free(fluxo); free(m); free(melhor); snprintf(erro, ecap, "sem memoria"); return -1; }

    monta_funcoes(m, dim, v);
    poe_dados(m, dim, fluxo, ip * 8);
    free(fluxo);

    /* testa as 8 máscaras. A penalidade é medida com as áreas de format/version
     * e o dark module CLAROS — gravar os bits reais aqui mudaria a penalidade
     * e a máscara escolhida. */
    int melhor_pen = -1, melhor_mask = 0;
    for (int mask = 0; mask < 8; mask++) {
        uint8_t *t = malloc((size_t)dim * dim);
        memcpy(t, m, (size_t)dim * dim);
        for (int r = 0; r < dim; r++)
            for (int c = 0; c < dim; c++)
                if (!reservado(t, dim, r, c) && mascara_bit(mask, r, c))
                    t[r*dim+c] ^= 1;
        int pen = penalidade(t, dim);
        if (melhor_pen < 0 || pen < melhor_pen) { melhor_pen = pen; melhor_mask = mask; }
        free(t);
    }

    /* aplica a máscara vencedora e grava os bits reais de format/version */
    for (int r = 0; r < dim; r++)
        for (int c = 0; c < dim; c++)
            if (!reservado(m, dim, r, c) && mascara_bit(melhor_mask, r, c))
                m[r*dim+c] ^= 1;
    grava_format(m, dim, nivel, melhor_mask);
    grava_version(m, dim, v);
    free(melhor);

    for (int i = 0; i < dim * dim; i++) m[i] &= 1;
    *grid = m;
    *dim_out = dim;
    return 0;
}

/* ── render PNG (libpng) ────────────────────────────────────────────────── */
#include <png.h>

typedef struct { unsigned char *b; size_t n, cap; } PngBuf;

/* minúsculo ASCII local — ps_qr.c é uma unidade de tradução própria e não
 * enxerga o `minusculo` do poolscript_vm.c */
static void qr_minusculo(const char *s, char *out, size_t cap)
{
    /* `cap == 0` ANTES de qualquer coisa: `cap` é `size_t`, então `cap - 1`
     * vira SIZE_MAX e o laço só pararia no NUL de `s` — com o `out[i] = 0` do
     * fim escrevendo num buffer de tamanho zero. */
    if (cap == 0) return;
    size_t i = 0;
    /* limite ANTES da leitura: `s[i] && i < cap - 1` lê `s[i]` pra só depois
     * conferir se `i` cabe. Funciona enquanto `s` estiver terminado em NUL, e
     * é justamente o tipo de coisa que deixa de funcionar quando alguém passa
     * um buffer que não está. */
    for (; i < cap - 1 && s[i]; i++)
        out[i] = (s[i] >= 'A' && s[i] <= 'Z') ? (char)(s[i] + 32) : s[i];
    out[i] = '\0';
}

static void png_escreve_cb(png_structp p, png_bytep dados, png_size_t n)
{
    PngBuf *b = (PngBuf *)png_get_io_ptr(p);
    if (b->n + n > b->cap) {
        size_t nc = b->cap < 4096 ? 4096 : b->cap;
        while (nc < b->n + n) nc *= 2;
        unsigned char *nn = realloc(b->b, nc);
        if (!nn) return;
        b->b = nn; b->cap = nc;
    }
    memcpy(b->b + b->n, dados, n);
    b->n += n;
}
static void png_flush_cb(png_structp p) { (void)p; }

/* Nome de cor (black, white, red, green, blue, yellow, cyan, magenta, gray),
 * "#rrggbb" ou "#rgb" → RGB. Nome desconhecido cai no default do CHAMADOR
 * (`claro_padrao`): preto pro traço, branco pro fundo — sem erro. */
static void cor_rgb(const char *nome, unsigned char rgb[3], int claro_padrao)
{
    static const struct { const char *n; unsigned char r,g,b; } TAB[] = {
        {"black",0,0,0},{"white",255,255,255},{"red",255,0,0},{"green",0,128,0},
        {"blue",0,0,255},{"yellow",255,255,0},{"cyan",0,255,255},{"magenta",255,0,255},
        {"gray",128,128,128},{"grey",128,128,128},
    };
    if (nome) {
        char low[32];
        qr_minusculo(nome, low, sizeof(low));
        for (size_t i = 0; i < sizeof(TAB)/sizeof(TAB[0]); i++)
            if (!strcmp(low, TAB[i].n)) { rgb[0]=TAB[i].r; rgb[1]=TAB[i].g; rgb[2]=TAB[i].b; return; }
        if (low[0] == '#') {
            const char *h = low + 1;
            int len = (int)strlen(h);
            unsigned v = 0;
            if (len == 6 || len == 3) {
                for (const char *p = h; *p; p++) {
                    int d = (*p>='0'&&*p<='9')?*p-'0':(*p>='a'&&*p<='f')?*p-'a'+10:-1;
                    if (d < 0) { v = 0xFFFFFFFF; break; }
                    v = v*16 + (unsigned)d;
                }
                if (v != 0xFFFFFFFF) {
                    if (len == 6) { rgb[0]=(v>>16)&255; rgb[1]=(v>>8)&255; rgb[2]=v&255; }
                    else { rgb[0]=((v>>8)&15)*17; rgb[1]=((v>>4)&15)*17; rgb[2]=(v&15)*17; }
                    return;
                }
            }
        }
    }
    unsigned char d = claro_padrao ? 255 : 0;
    rgb[0]=rgb[1]=rgb[2]=d;
}

int ps_qr_png(const uint8_t *grid, int dim, int box, int border,
              const char *cor, const char *fundo, int tw, int th,
              unsigned char **png, size_t *npng, char *erro, size_t ecap)
{
    if (box < 1) box = 1;
    if (border < 0) border = 0;
    int mod = dim + 2 * border;
    int base = mod * box;
    volatile int outw = (tw > 0) ? tw : base;   /* volatile: sobrevive ao longjmp do libpng */
    volatile int outh = (th > 0) ? th : base;
    unsigned char fg[3], bg[3];
    cor_rgb(cor, fg, 0);
    cor_rgb(fundo, bg, 1);

    png_structp p = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!p) { snprintf(erro, ecap, "sem memoria (png)"); return -1; }
    png_infop info = png_create_info_struct(p);
    if (!info) { png_destroy_write_struct(&p, NULL); snprintf(erro, ecap, "sem memoria (png)"); return -1; }
    PngBuf buf = {0};
    if (setjmp(png_jmpbuf(p))) {
        png_destroy_write_struct(&p, &info);
        free(buf.b);
        snprintf(erro, ecap, "falha ao gerar PNG");
        return -1;
    }
    png_set_write_fn(p, &buf, png_escreve_cb, png_flush_cb);
    png_set_IHDR(p, info, (png_uint_32)outw, (png_uint_32)outh, 8, PNG_COLOR_TYPE_RGB,
                 PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
    png_write_info(p, info);

    unsigned char *linha = malloc((size_t)outw * 3);
    if (!linha) { png_destroy_write_struct(&p, &info); free(buf.b); snprintf(erro, ecap, "sem memoria"); return -1; }
    for (int y = 0; y < outh; y++) {
        int by = (outh == base) ? y : y * base / outh;   /* nearest pro resize */
        int my = by / box - border;
        for (int x = 0; x < outw; x++) {
            int bx = (outw == base) ? x : x * base / outw;
            int mx = bx / box - border;
            int escuro = (my >= 0 && my < dim && mx >= 0 && mx < dim) && grid[my * dim + mx];
            unsigned char *c = escuro ? fg : bg;
            linha[x*3] = c[0]; linha[x*3+1] = c[1]; linha[x*3+2] = c[2];
        }
        png_write_row(p, linha);
    }
    free(linha);
    png_write_end(p, info);
    png_destroy_write_struct(&p, &info);
    *png = buf.b;
    *npng = buf.n;
    return 0;
}
